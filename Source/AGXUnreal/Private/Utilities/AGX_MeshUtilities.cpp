// Copyright 2024, Algoryx Simulation AB.

#include "Utilities/AGX_MeshUtilities.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "Shapes/AGX_SimpleMeshComponent.h"
#include "Shapes/RenderDataBarrier.h"
#include "Shapes/RenderMaterial.h"
#include "Shapes/ShapeBarrier.h"
#include "Utilities/AGX_ObjectUtilities.h"

// Unreal Engine includes.
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Math/UnrealMathUtility.h"
#include "Misc/EngineVersionComparison.h"
#include "PhysicsEngine/BodySetup.h"
#include "Rendering/PositionVertexBuffer.h"
#include "RenderingThread.h"
#include "RHIGPUReadback.h"
#include "StaticMeshAttributes.h"
#include "StaticMeshResources.h"
#include "UObject/Package.h"

// Standard library includes.
#include <limits>
#include <algorithm>

#define CONE_SINGULARITY

namespace
{
	// Helper struct for scaling UV coordinates.
	struct UvCoordinateScaler
	{
		UvCoordinateScaler(float InUmin, float InUmax, float InVmin, float InVmax)
		{
			Umin = InUmin;
			Vmin = InVmin;
			Uspan = InUmax - InUmin;
			Vspan = InVmax - InVmin;
		}

		float ScaleU(float U) const
		{
			return U * Uspan + Umin;
		}

		float ScaleV(float V) const
		{
			return V * Vspan + Vmin;
		}

	private:
		float Uspan, Vspan, Umin, Vmin;
	};

	void AppendCylindricalTube(
		TArray<FVector3f>& Vertices, TArray<FVector3f>& Normals, TArray<uint32>& Indices,
		TArray<FVector2f>& TexCoords, float Radius, float Height, uint32 CircleSegments,
		uint32 HeightSegments, const UvCoordinateScaler& UvScaler = {0.f, 1.f, 0.f, 1.f})
	{
		// Arrays may already have data, we only append triangle data in this function.
		const uint32 OrigVertNum = Vertices.Num();
		const uint32 OrigNormNum = Normals.Num();
		const uint32 OrigIndNum = Indices.Num();
		const uint32 OrigTexNum = TexCoords.Num();

		const uint32 VertexRows = HeightSegments + 1;
		const uint32 VertexColumns = CircleSegments + 1;

		// 3 indices per triangle, 2 triangles per quad face.
		Indices.Reserve(OrigIndNum + 2 * CircleSegments * HeightSegments * 3);
		Vertices.Reserve(OrigVertNum + VertexRows * VertexColumns);
		Normals.Reserve(OrigNormNum + VertexRows * VertexColumns); // 1 normal per vertex.
		TexCoords.Reserve(OrigTexNum + VertexRows * VertexColumns); // 1 tex coord per vertex.

		const float SegmentSize = 2.f * PI / static_cast<float>(CircleSegments);
		const float RadiusInv = 1.f / Radius;

		// Vertex position and texture coordinates.
		float X, Y, Z, U, V;

		// Calculate and add vertices, texture coordinates and normals.
		for (uint32 RowIndex = 0; RowIndex < VertexRows; ++RowIndex)
		{
			const float RowHeight =
				Height * static_cast<float>(RowIndex) / static_cast<float>(HeightSegments) -
				Height * 0.5f;

			for (uint32 ColumnIndex = 0; ColumnIndex < VertexColumns; ++ColumnIndex)
			{
				const float ColumnAngle = ColumnIndex * SegmentSize;

				X = Radius * FMath::Cos(ColumnAngle);
				Y = RowHeight;
				Z = Radius * FMath::Sin(ColumnAngle);

				Vertices.Add(FVector3f(X, Y, Z));
				Normals.Add(FVector3f(X * RadiusInv, 0.0f, Z * RadiusInv));

				// Vertex texture coordinates (U, V) range between [0, 1].
				U = UvScaler.ScaleU(
					static_cast<float>(ColumnIndex) / static_cast<float>(CircleSegments));
				V = UvScaler.ScaleV(
					1.f - static_cast<float>(RowIndex) / static_cast<float>(HeightSegments));
				TexCoords.Add(FVector2f(U, V));
			}
		}

		// Calculate and add triangle indices.
		for (uint32 HeightSegIndex = 0; HeightSegIndex < HeightSegments; ++HeightSegIndex)
		{
			// Index of first vertex in lower row.
			uint32 K0 = OrigVertNum + HeightSegIndex * VertexColumns;

			// Index of first vertex one row above.
			uint32 K1 = K0 + VertexColumns;

			for (uint32 CircleSegIndex = 0; CircleSegIndex < CircleSegments; ++CircleSegIndex)
			{
				// First triangle.
				Indices.Add(K0);
				Indices.Add(K0 + 1);
				Indices.Add(K1);

				// Second triangle.
				Indices.Add(K0 + 1);
				Indices.Add(K1 + 1);
				Indices.Add(K1);

				K0++;
				K1++;
			}
		}

		check(Vertices.Num() == OrigVertNum + VertexRows * VertexColumns);
		check(Normals.Num() == OrigNormNum + VertexRows * VertexColumns);
		check(TexCoords.Num() == OrigTexNum + VertexRows * VertexColumns);
		check(Indices.Num() == OrigIndNum + 2 * CircleSegments * HeightSegments * 3);
	}

	void AppendDisk(
		TArray<FVector3f>& Vertices, TArray<FVector3f>& Normals, TArray<uint32>& Indices,
		TArray<FVector2f>& TexCoords, float Radius, uint32 CircleSegments, float Offset,
		bool IsBottom)
	{
		// Arrays may already have data, we only append triangle data in this function.
		const uint32 OrigVertNum = Vertices.Num();
		const uint32 OrigNormNum = Normals.Num();
		const uint32 OrigIndNum = Indices.Num();
		const uint32 OrigTexNum = TexCoords.Num();

		const uint32 VertexColumns = CircleSegments + 1;

		// 3 indices per triangle, 1 triangles per "blade".
		Indices.Reserve(OrigIndNum + (CircleSegments - 2) * 3);
		Vertices.Reserve(OrigVertNum + VertexColumns);
		Normals.Reserve(OrigNormNum + VertexColumns); // 1 normal per vertex.
		TexCoords.Reserve(OrigTexNum + VertexColumns); // 1 tex coord per vertex.

		const float SegmentSize = 2.0 * PI / CircleSegments;
		const float RadiusInv = 1.0f / Radius;
		const FVector3f Normal = IsBottom ? FVector3f(0.f, -1.f, 0.f) : FVector3f(0.f, 1.f, 0.f);

		// Vertex position and texture coordinates.
		float X, Y, Z, U, V;

		// Calculate and add vertex positions and normals.
		for (uint32 ColumnIndex = 0; ColumnIndex < VertexColumns; ++ColumnIndex)
		{
			float ColumnAngle = ColumnIndex * SegmentSize;

			X = Radius * FMath::Cos(ColumnAngle);
			Y = Offset;
			Z = Radius * FMath::Sin(ColumnAngle);

			Vertices.Add(FVector3f(X, Y, Z));
			Normals.Add(Normal);

			// Vertex texture coordinates (U, V) range between [0, 1].
			U = static_cast<float>(1 - (X / (2 * Radius) + 0.5));
			V = static_cast<float>(Z / (2 * Radius) + 0.5);
			TexCoords.Add(FVector2f(U, V));
		}

		// Calculate and add triangle indices.
		uint32 K0 = OrigVertNum;
		uint32 K1 = K0 + 1; // Second vertex.
		uint32 K2 = K1 + 1; // Third vertex.
		for (uint32 seg = 0; seg < CircleSegments - 2; ++seg)
		{
			// 1 triangles per non-centered "blade".

			// Mind the winding direction.
			if (IsBottom)
			{
				// K2 => K1 => K0.
				Indices.Add(K2);
				Indices.Add(K1);
				Indices.Add(K0);
			}
			else
			{
				// K0 => K1 => K2.
				Indices.Add(K0);
				Indices.Add(K1);
				Indices.Add(K2);
			}

			K1++;
			K2++;
		}

		check(Vertices.Num() == OrigVertNum + VertexColumns);
		check(Normals.Num() == OrigNormNum + VertexColumns);
		check(TexCoords.Num() == OrigTexNum + VertexColumns);
		check(Indices.Num() == OrigIndNum + (CircleSegments - 2) * 3);
	}

	void AppendHalfSphere(
		TArray<FVector3f>& Vertices, TArray<FVector3f>& Normals, TArray<uint32>& Indices,
		TArray<FVector2f>& TexCoords, float Radius, float Offset, uint32 NumSegments,
		uint32 NumSlices, bool IsBottom, const UvCoordinateScaler& UvScaler = {0.f, 1.f, 0.f, 1.f},
		bool AlongZ = true)
	{
		if (NumSegments < 4 || NumSlices < 1 || Radius < 1.0e-6)
		{
			return;
		}

		// Arrays may already have data, we only append triangle data in this function.
		const uint32 OrigVertNum = Vertices.Num();
		const uint32 OrigNormNum = Normals.Num();
		const uint32 OrigIndNum = Indices.Num();
		const uint32 OrigTexNum = TexCoords.Num();

		// 3 indices per triangle, 1 triangle per quad face for first slice
		// + 2 triangles per quad face for the other slices.
		const int32 NumIndices = (3 * NumSegments) + (NumSlices - 1) * (2 * 3 * NumSegments);
		const int32 NumVertices = (NumSegments + 1) * (NumSlices + 1);

		Indices.Reserve(OrigIndNum + NumIndices);
		Vertices.Reserve(OrigVertNum + NumVertices);
		Normals.Reserve(OrigNormNum + NumVertices); // 1 normal per vertex.
		TexCoords.Reserve(OrigTexNum + NumVertices); // 1 tex coord per vertex.

		const float SegmentStep = 2.0 * PI / static_cast<float>(NumSegments);
		const float SliceStep = 0.5f * PI / static_cast<float>(NumSlices);

		// Vertex position and texture coordinates.
		float X, Y, Z, U, V;
		const float RadiusInv = 1.0f / Radius;

		for (uint32 SliceIndex = 0; SliceIndex <= NumSlices;
			 ++SliceIndex) // Slice = vertical segment
		{
			// SliceAngle goes from from 0 to PI/2 for top, and PI/2 to PI for bottom.
			const float SliceAngle = IsBottom
										 ? static_cast<float>(SliceIndex) * SliceStep + 0.5f * PI
										 : static_cast<float>(SliceIndex) * SliceStep;
			const float SliceRadius = Radius * FMath::Sin(SliceAngle);
			const float SliceHeight = Radius * FMath::Cos(SliceAngle) + Offset;

			// Add (NumSegments + 1) vertices per Slice. The first and last vertex
			// have same position and normal, but different tex coords.
			for (uint32 SegmentIndex = 0; SegmentIndex <= NumSegments;
				 ++SegmentIndex) // Segment = horizontal segment
			{
				// Starting from 0 to 2pi.
				const float SegmentAngle = static_cast<float>(SegmentIndex) * SegmentStep;

				if (AlongZ)
				{
					X = SliceRadius * FMath::Sin(SegmentAngle);
					Y = SliceRadius * FMath::Cos(SegmentAngle);
					Z = SliceHeight;
				}
				else
				{
					// Along y.
					X = SliceRadius * FMath::Cos(SegmentAngle);
					Y = SliceHeight;
					Z = SliceRadius * FMath::Sin(SegmentAngle);
				}

				Vertices.Add(FVector3f(X, Y, Z));
				Normals.Add(FVector3f(X * RadiusInv, Y * RadiusInv, Z * RadiusInv));

				// Vertex tex coord (u, v) range between [0, 1].
				U = UvScaler.ScaleU(
					static_cast<float>(SegmentIndex) / static_cast<float>(NumSegments));
				V = UvScaler.ScaleV(static_cast<float>(SliceIndex) / static_cast<float>(NumSlices));
				TexCoords.Add(FVector2f(U, V));
			}
		}

		// Generate indices.
		for (uint32 SliceIndex = 0; SliceIndex < NumSlices; ++SliceIndex)
		{
			uint32 K1 = OrigVertNum + SliceIndex * (NumSegments + 1); // Beginning of current Slice.
			uint32 K2 = K1 + NumSegments + 1; // Beginning of next Slice.

			for (uint32 SegmentIndex = 0; SegmentIndex < NumSegments; ++SegmentIndex)
			{
				// 2 triangles per Segment excluding first or last Slices

				if (IsBottom || SliceIndex != 0)
				{
					Indices.Add(K2);
					Indices.Add(K1 + 1);
					Indices.Add(K1);
				}

				if (!IsBottom || SliceIndex != (NumSlices - 1))
				{
					Indices.Add(K2);
					Indices.Add(K2 + 1);
					Indices.Add(K1 + 1);
				}

				K1++;
				K2++;
			}
		}

		check(Vertices.Num() == OrigVertNum + NumVertices);
		check(Normals.Num() == OrigNormNum + NumVertices);
		check(TexCoords.Num() == OrigTexNum + NumVertices);
		check(Indices.Num() == OrigIndNum + NumIndices);
	}
}

void AGX_MeshUtilities::MakeCube(
	TArray<FVector3f>& Positions, TArray<FVector3f>& Normals, TArray<uint32>& Indices,
	TArray<FVector2f>& TexCoords, const FVector3f& HalfSize)
{
	// 8 Corners,
	// 6 Quads,
	// 12 Triangles,
	// 36 Indices,
	// 24 Vertices (6*4)
	// 24 Normals (6*4)

	// clang-format off
	static const TArray<FVector3f> StaticPositions =
	{
		FVector3f(-1.0, -1.0, 1.0),  FVector3f(1.0, -1.0, 1.0),  FVector3f(1.0, 1.0, 1.0),   FVector3f(-1.0, 1.0, 1.0),  // Up
		FVector3f(1.0, 1.0, 1.0),    FVector3f(1.0, 1.0, -1.0),  FVector3f(1.0, -1.0, -1.0), FVector3f(1.0, -1.0, 1.0),  // Forward
		FVector3f(-1.0, -1.0, -1.0), FVector3f(1.0, -1.0, -1.0), FVector3f(1.0, 1.0, -1.0),  FVector3f(-1.0, 1.0, -1.0), // Down
		FVector3f(-1.0, -1.0, -1.0), FVector3f(-1.0, -1.0, 1.0), FVector3f(-1.0, 1.0, 1.0),  FVector3f(-1.0, 1.0, -1.0), // Backward
		FVector3f(1.0, 1.0, 1.0),    FVector3f(-1.0, 1.0, 1.0),  FVector3f(-1.0, 1.0, -1.0), FVector3f(1.0, 1.0, -1.0),  // Right
		FVector3f(-1.0, -1.0, -1.0), FVector3f(1.0, -1.0, -1.0), FVector3f(1.0, -1.0, 1.0),  FVector3f(-1.0, -1.0, 1.0)  // Left
	};

	static const TArray<FVector3f> StaticNormals =
	{
		FVector3f(0.0, 0.0, 1.0),  FVector3f(0.0, 0.0, 1.0),  FVector3f(0.0, 0.0, 1.0),  FVector3f(0.0, 0.0, 1.0),  // Up
		FVector3f(1.0, 0.0, 0.0),  FVector3f(1.0, 0.0, 0.0),  FVector3f(1.0, 0.0, 0.0),  FVector3f(1.0, 0.0, 0.0),  // Forward
		FVector3f(0.0, 0.0, -1.0), FVector3f(0.0, 0.0, -1.0), FVector3f(0.0, 0.0, -1.0), FVector3f(0.0, 0.0, -1.0), // Down
		FVector3f(-1.0, 0.0, 0.0), FVector3f(-1.0, 0.0, 0.0), FVector3f(-1.0, 0.0, 0.0), FVector3f(-1.0, 0.0, 0.0), // Backward
		FVector3f(0.0, 1.0, 0.0),  FVector3f(0.0, 1.0, 0.0),  FVector3f(0.0, 1.0, 0.0),  FVector3f(0.0, 1.0, 0.0),  // Right
		FVector3f(0.0, -1.0, 0.0), FVector3f(0.0, -1.0, 0.0), FVector3f(0.0, -1.0, 0.0), FVector3f(0.0, -1.0, 0.0)  // Left
	};

	static const TArray<uint32> StaticIndices =
	{
		0, 2, 1, 0, 3, 2,		// Up
		4, 5, 6, 4, 6, 7,		// Forward
		8, 9, 10, 8, 10, 11,	// Down
		12, 14, 13, 12, 15, 14,	// Backward
		16, 17, 18, 16, 18, 19,	// Right
		20, 22, 21, 20, 23, 22	// Left
	};

	// This maps the texture the same way as an Unreal Cube.
	static const TArray<FVector2f> StaticTexCoords =
	{
		FVector2f(0,0),		FVector2f(1,0),		FVector2f(1,1),		FVector2f(0,1), //Up
		FVector2f(0,0),		FVector2f(0,1),		FVector2f(1,1),		FVector2f(1,0), //Forward
		FVector2f(1,0),		FVector2f(0,0),		FVector2f(0,1),		FVector2f(1,1), //Down
		FVector2f(0,1),		FVector2f(0,0),		FVector2f(1,0),		FVector2f(1,1), //Backward
		FVector2f(1,0),		FVector2f(0,0),		FVector2f(0,1),		FVector2f(1,1), //Right
		FVector2f(1,1),		FVector2f(0,1),		FVector2f(0,0),		FVector2f(1,0)  //Left
	};
	// clang-format on

	Positions = StaticPositions;
	Normals = StaticNormals;
	Indices = StaticIndices;
	TexCoords = StaticTexCoords;

	for (FVector3f& Position : Positions)
	{
		Position *= HalfSize;
	}

	check(Indices.Num() == 36);
	check(Positions.Num() == 24);
	check(Normals.Num() == 24);
}

AGX_MeshUtilities::SphereConstructionData::SphereConstructionData(
	float InRadius, uint32 InNumSegments)
	: Radius(InRadius)
	, Segments(InNumSegments)
	, Stacks(InNumSegments)
	, Sectors(InNumSegments)
	, Vertices((Stacks + 1) * (Sectors + 1))
	, Indices(Sectors * (6 * Stacks - 6))
{
}

void AGX_MeshUtilities::SphereConstructionData::AppendBufferSizes(
	uint32& InOutNumVertices, uint32& InOutNumIndices) const
{
	InOutNumVertices += Vertices;
	InOutNumIndices += Indices;
}

void AGX_MeshUtilities::MakeSphere(
	TArray<FVector3f>& Positions, TArray<FVector3f>& Normals, TArray<uint32>& Indices,
	TArray<FVector2f>& TexCoords, float Radius, uint32 NumSegments)
{
	if (NumSegments < 4 || Radius < 1.0e-6)
		return;

	Positions.Empty();
	Normals.Empty();
	Indices.Empty();
	TexCoords.Empty();

	// Top half.
	AppendHalfSphere(
		Positions, Normals, Indices, TexCoords, Radius, 0.f, NumSegments, NumSegments / 2, false,
		UvCoordinateScaler(0.f, 1.f, 0.f, 0.5f));

	// Bottom half.
	AppendHalfSphere(
		Positions, Normals, Indices, TexCoords, Radius, 0.f, NumSegments, NumSegments / 2, true,
		UvCoordinateScaler(0.f, 1.f, 0.5f, 1.0f));
}

void AGX_MeshUtilities::MakeSphere(
	FStaticMeshVertexBuffers& VertexBuffers, FDynamicMeshIndexBuffer32& IndexBuffer,
	uint32& NextFreeVertex, uint32& NextFreeIndex, const SphereConstructionData& Data)
{
	check(Data.Segments >= 4);
	check(Data.Segments <= uint32(TNumericLimits<uint16>::Max()));
	check(Data.Radius >= 1.0e-6);

	check(NextFreeVertex + Data.Vertices <= VertexBuffers.PositionVertexBuffer.GetNumVertices());
	check(NextFreeVertex + Data.Vertices <= VertexBuffers.StaticMeshVertexBuffer.GetNumVertices());
	check(NextFreeVertex + Data.Vertices <= VertexBuffers.ColorVertexBuffer.GetNumVertices());
	check(NextFreeIndex + Data.Indices <= static_cast<uint32>(IndexBuffer.Indices.Num()));

	const uint32 FirstVertex = NextFreeVertex;
	const uint32 FirstIndex = NextFreeIndex;

	const float SectorStep = 2.0 * PI / Data.Sectors;
	const float StackStep = PI / Data.Stacks;
	const float RadiusInv = 1.0f / Data.Radius;

	FVector3f Position, TangentX, TangentY, TangentZ;
	FColor Color = FColor::White; /// \todo Make configurable!
	FVector2f TexCoord;

	float SectorAngle;
	float StackAngle, StackRadius, StackHeight;

	// TODO: Change terminology from stack to vertical slice, and numbers accordingly!

	for (uint32 StackIndex = 0; StackIndex <= Data.Stacks; ++StackIndex) // stack = vertical segment
	{
		StackAngle = PI / 2.0 - StackIndex * StackStep; // starting from pi/2 to -pi/2
		StackRadius = Data.Radius * FMath::Cos(StackAngle);
		StackHeight = Data.Radius * FMath::Sin(StackAngle);

		// Add (NumSectors + 1) vertices per stack. The first and last vertex
		// have same position and normal, but different tex coords.
		for (uint32 SectorIndex = 0; SectorIndex <= Data.Sectors;
			 ++SectorIndex) // sector = horizontal segment
		{
			SectorAngle = SectorIndex * SectorStep; // starting from 0 to 2pi

			Position.X = StackRadius * FMath::Cos(SectorAngle);
			Position.Y = StackRadius * FMath::Sin(SectorAngle);
			Position.Z = StackHeight;

			TangentZ =
				FVector3f(Position.X * RadiusInv, Position.Y * RadiusInv, Position.Z * RadiusInv);

			// vertex tex coord (u, v) range between [0, 1]
			TexCoord.X = (float) SectorIndex / Data.Sectors;
			TexCoord.Y = (float) StackIndex / Data.Stacks;

			/// \todo Compute correctly based on texcoords!
			TangentX = FVector3f::ZeroVector;
			TangentY = FVector3f::ZeroVector;

			// Fill actual buffers
			VertexBuffers.PositionVertexBuffer.VertexPosition(NextFreeVertex) = Position;
			VertexBuffers.ColorVertexBuffer.VertexColor(NextFreeVertex) = Color;
#if UE_VERSION_OLDER_THAN(5, 0, 0)
			VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(NextFreeVertex, 0, TexCoord);
#else
			VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(
				NextFreeVertex, 0, {(float) TexCoord.X, (float) TexCoord.Y});
#endif
			VertexBuffers.StaticMeshVertexBuffer.SetVertexTangents(
				NextFreeVertex, TangentX, TangentY, TangentZ);

			NextFreeVertex++;
		}
	}

	// Generate index list of sphere triangles
	int K1, K2;
	for (uint32 StackIndex = 0; StackIndex < Data.Stacks; ++StackIndex)
	{
		K1 = FirstVertex + StackIndex * (Data.Sectors + 1); // beginning of current stack
		K2 = K1 + Data.Sectors + 1; // beginning of next stack

		for (uint32 SectorIndex = 0; SectorIndex < Data.Sectors; ++SectorIndex, ++K1, ++K2)
		{
			// 2 triangles per sector excluding first and last stacks
			// K1 => K2 => K1+1
			if (StackIndex != 0)
			{
				IndexBuffer.Indices[NextFreeIndex++] = K1;
				IndexBuffer.Indices[NextFreeIndex++] = K1 + 1;
				IndexBuffer.Indices[NextFreeIndex++] = K2;
			}

			// K1+1 => K2 => K2+1
			if (StackIndex != (Data.Stacks - 1))
			{
				IndexBuffer.Indices[NextFreeIndex++] = K1 + 1;
				IndexBuffer.Indices[NextFreeIndex++] = K2 + 1;
				IndexBuffer.Indices[NextFreeIndex++] = K2;
			}
		}
	}

	check(NextFreeVertex - FirstVertex == Data.Vertices);
	check(NextFreeIndex - FirstIndex == Data.Indices);
}

AGX_MeshUtilities::CylinderConstructionData::CylinderConstructionData(
	float InRadius, float InHeight, uint32 InNumCircleSegments, uint32 InNumHeightSegments,
	const FLinearColor& InMiddleColor, const FLinearColor& InOuterColor)
	: Radius(InRadius)
	, Height(InHeight)
	, CircleSegments(InNumCircleSegments)
	, HeightSegments(InNumHeightSegments)
	, MiddleColor(InMiddleColor)
	, OuterColor(InOuterColor)
	, VertexRows(HeightSegments + 1)
	, VertexColumns(CircleSegments + 1)
	, Caps(2)
	, VertexRowsAndCaps(VertexRows + 2)
	, Vertices(VertexRowsAndCaps * VertexColumns)
	, Indices(HeightSegments * CircleSegments * 6 + Caps * (CircleSegments - 2) * 3)
{
}

void AGX_MeshUtilities::CylinderConstructionData::AppendBufferSizes(
	uint32& InOutNumVertices, uint32& InOutNumIndices) const
{
	InOutNumVertices += Vertices;
	InOutNumIndices += Indices;
}

void AGX_MeshUtilities::MakeCylinder(
	TArray<FVector3f>& Positions, TArray<FVector3f>& Normals, TArray<uint32>& Indices,
	TArray<FVector2f>& TexCoords, const CylinderConstructionData& Data)
{
	auto LogConstructionError = [](const FString& Msg)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("AGX_MeshUtilities::MakeCylinder(): Invalid CylinderConstructionData: %s."), *Msg);
	};

	const uint32 MaxUint16 = uint32(TNumericLimits<uint16>::Max());

	if (Data.CircleSegments < 4 || Data.CircleSegments > MaxUint16)
	{
		LogConstructionError(
			"CircleSegments must be at least 4 and at most " + FString::FromInt(MaxUint16));
		return;
	}

	if (Data.HeightSegments < 1 || Data.HeightSegments > MaxUint16)
	{
		LogConstructionError(
			"HeightSegments must be at least 1 and at most " + FString::FromInt(MaxUint16));
		return;
	}

	if (Data.Radius < KINDA_SMALL_NUMBER)
	{
		LogConstructionError(
			"Radius must be at least " + FString::SanitizeFloat(KINDA_SMALL_NUMBER));
		return;
	}

	if (Data.Height < KINDA_SMALL_NUMBER)
	{
		LogConstructionError(
			"Height must be at least " + FString::SanitizeFloat(KINDA_SMALL_NUMBER));
		return;
	}

	Positions.Empty();
	Normals.Empty();
	Indices.Empty();
	TexCoords.Empty();

	// Make cylinder tube body.
	AppendCylindricalTube(
		Positions, Normals, Indices, TexCoords, Data.Radius, Data.Height, Data.CircleSegments,
		Data.HeightSegments);

	// Make bottom disk.
	AppendDisk(
		Positions, Normals, Indices, TexCoords, Data.Radius, Data.CircleSegments,
		-Data.Height / 2.f, true);

	// Make top disk.
	AppendDisk(
		Positions, Normals, Indices, TexCoords, Data.Radius, Data.CircleSegments, Data.Height / 2.f,
		false);
}

void AGX_MeshUtilities::MakeCylinder(
	FStaticMeshVertexBuffers& VertexBuffers, FDynamicMeshIndexBuffer32& IndexBuffer,
	uint32& NextFreeVertex, uint32& NextFreeIndex, const CylinderConstructionData& Data)
{
	check(Data.CircleSegments >= 4);
	check(Data.CircleSegments <= uint32(TNumericLimits<uint16>::Max()));
	check(Data.HeightSegments >= 1);
	check(Data.HeightSegments <= uint32(TNumericLimits<uint16>::Max()));
	check(Data.Radius >= 1.0e-6);
	check(Data.Height >= 1.0e-6);

	check(NextFreeVertex + Data.Vertices <= VertexBuffers.PositionVertexBuffer.GetNumVertices());
	check(NextFreeVertex + Data.Vertices <= VertexBuffers.StaticMeshVertexBuffer.GetNumVertices());
	check(NextFreeVertex + Data.Vertices <= VertexBuffers.ColorVertexBuffer.GetNumVertices());
	check(NextFreeIndex + Data.Indices <= static_cast<uint32>(IndexBuffer.Indices.Num()));

	const uint32 FirstVertex = NextFreeVertex;
	const uint32 FirstIndex = NextFreeIndex;

	const float SegmentSize = 2.0 * PI / Data.CircleSegments;
	const float RadiusInv = 1.0f / Data.Radius;

	FVector3f Position, TangentX, TangentY, TangentZ;
	FLinearColor Color; /// \todo Set vertex color to something.
	FVector2f TexCoord;

	// Generate vertex attributes.

	// Add vertices per horizontal row on the cylinder, with bottom and top vertex rows
	// duplicated for the caps, because they need different normals and tex coords.
	// The sequence of vertices are as follow:
	// Bottom Cap, Bottom Row, Bottom Row + 1, ..., Top Row - 1, Top Row, Top Cap
	for (uint32 CapsAndRowIndex = 0; CapsAndRowIndex < Data.VertexRowsAndCaps; ++CapsAndRowIndex)
	{
		const int32 CapIndex =
			CapsAndRowIndex == 0 ? 0 : (CapsAndRowIndex == Data.VertexRows + 1 ? 1 : -1);
		const bool IsCap = CapIndex != -1;
		const uint32 RowIndex = FMath::Clamp<int32>(CapsAndRowIndex - 1, 0, Data.VertexRows - 1);
		const float RowHeight = Data.Height * RowIndex / (Data.VertexRows - 1) - Data.Height * 0.5f;

		Color = FMath::Lerp(
			Data.MiddleColor, Data.OuterColor,
			FMath::Abs((static_cast<float>(RowIndex) / Data.HeightSegments) - 0.5f) * 2.0f);

		// Add Data.VertexColumns num vertices in a circle per vertex row. The first and last vertex
		// in the same row have same position and normal, but different tex coords.
		for (uint32 ColumnIndex = 0; ColumnIndex < Data.VertexColumns; ++ColumnIndex)
		{
			float ColumnAngle = ColumnIndex * SegmentSize;

			// Vertex Position
			Position.X = Data.Radius * FMath::Cos(ColumnAngle);
			Position.Y = Data.Radius * FMath::Sin(ColumnAngle);
			Position.Z = RowHeight;

			// Vertex Texture Coordinates, range between [0, 1]
			TexCoord.X = IsCap ? Position.X : ((float) ColumnIndex / Data.CircleSegments);
			TexCoord.Y = IsCap ? Position.Y : ((float) RowIndex / (Data.VertexRows - 1));

			// Vertex Normal, Tangent, and Binormal
			switch (CapIndex)
			{
				case 0: // bottom cap
					TangentZ = FVector3f(0.0f, 0.0f, -1.0f);
					break;
				case 1: // top cap
					TangentZ = FVector3f(0.0f, 0.0f, 1.0f);
					break;
				default: // normal row
					TangentZ = FVector3f(Position.X * RadiusInv, Position.Y * RadiusInv, 0.0f);
					break;
			}

			/// \todo Compute correctly based on texcoords!
			TangentX = FVector3f::ZeroVector;
			TangentY = FVector3f::ZeroVector;

			// Fill actual buffers
			VertexBuffers.PositionVertexBuffer.VertexPosition(NextFreeVertex) = Position;
			VertexBuffers.ColorVertexBuffer.VertexColor(NextFreeVertex) = Color.ToFColor(false);
#if UE_VERSION_OLDER_THAN(5, 0, 0)
			VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(NextFreeVertex, 0, TexCoord);
#else
			VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(
				NextFreeVertex, 0, {(float) TexCoord.X, (float) TexCoord.Y});
#endif
			VertexBuffers.StaticMeshVertexBuffer.SetVertexTangents(
				NextFreeVertex, TangentX, TangentY, TangentZ);

			NextFreeVertex++;
		}
	}

	// Generate triangle indexes for the side segments of the cylinder.
	uint32 K0, K1;
	for (uint32 HeightSegmentIndex = 0; HeightSegmentIndex < Data.HeightSegments;
		 ++HeightSegmentIndex)
	{
		K0 = FirstVertex + Data.VertexColumns +
			 HeightSegmentIndex * Data.VertexColumns; // first vertex in bottom vertex row of height
													  // segment (offset by cap)
		K1 = K0 + Data.VertexColumns; // first vertex in next row

		for (uint32 CircleSegmentIndex = 0; CircleSegmentIndex < Data.CircleSegments;
			 ++CircleSegmentIndex, ++K0, ++K1)
		{
			// 2 triangles per segment (i.e. quad)

			IndexBuffer.Indices[NextFreeIndex++] = K0 + 1;
			IndexBuffer.Indices[NextFreeIndex++] = K0;
			IndexBuffer.Indices[NextFreeIndex++] = K1;

			IndexBuffer.Indices[NextFreeIndex++] = K1;
			IndexBuffer.Indices[NextFreeIndex++] = K1 + 1;
			IndexBuffer.Indices[NextFreeIndex++] = K0 + 1;
		}
	}

	// Generate triangle indexes for the caps, with triangle fan pattern.
	uint32 K2;
	for (uint32 CapIndex = 0; CapIndex < Data.Caps; ++CapIndex)
	{
		K0 = FirstVertex + CapIndex * (Data.VertexRowsAndCaps - 1) *
							   Data.VertexColumns; // first vertex in cap row
		K1 = K0 + 1; // second vertex in cap row
		K2 = K1 + 1; // third vertex in cap row

		for (uint32 CircleSegmentIndex = 1; CircleSegmentIndex < Data.CircleSegments - 1;
			 ++CircleSegmentIndex, ++K1, ++K2)
		{
			// 1 triangles per segment (except for first and last segment)

			if (CapIndex == 0)
			{
				IndexBuffer.Indices[NextFreeIndex++] = K0;
				IndexBuffer.Indices[NextFreeIndex++] = K1;
				IndexBuffer.Indices[NextFreeIndex++] = K2;
			}
			else
			{
				IndexBuffer.Indices[NextFreeIndex++] = K2;
				IndexBuffer.Indices[NextFreeIndex++] = K1;
				IndexBuffer.Indices[NextFreeIndex++] = K0;
			}
		}
	}

	check(NextFreeVertex - FirstVertex == Data.Vertices);
	check(NextFreeIndex - FirstIndex == Data.Indices);
}

void AGX_MeshUtilities::MakeCylinder(
	const FVector3f& Base, const FVector3f& XAxis, const FVector3f& YAxis, const FVector3f& ZAxis,
	float Radius, float HalfHeight, uint32 Sides, TArray<FDynamicMeshVertex>& OutVerts,
	TArray<uint32>& OutIndices)
{
	const float AngleDelta = 2.0f * PI / Sides;
	FVector3f LastVertex = Base + XAxis * Radius;

	FVector2f TC = FVector2f(0.0f, 0.0f);
	float TCStep = 1.0f / Sides;

	FVector3f TopOffset = HalfHeight * ZAxis;

	int32 BaseVertIndex = OutVerts.Num();

	// Compute vertices for base circle.
	for (uint32 SideIndex = 0; SideIndex < Sides; SideIndex++)
	{
		const FVector3f Vertex = Base + (XAxis * FMath::Cos(AngleDelta * (SideIndex + 1)) +
										 YAxis * FMath::Sin(AngleDelta * (SideIndex + 1))) *
											Radius;
		FVector3f Normal = Vertex - Base;
		Normal.Normalize();

		FDynamicMeshVertex MeshVertex;

		MeshVertex.Position = Vertex - TopOffset;
#if UE_VERSION_OLDER_THAN(5, 0, 0)
		MeshVertex.TextureCoordinate[0] = TC;
#else
		MeshVertex.TextureCoordinate[0] = {(float) TC.X, (float) TC.Y};
#endif
		MeshVertex.SetTangents(-ZAxis, (-ZAxis) ^ Normal, Normal);

		OutVerts.Add(MeshVertex); // Add bottom vertex

		LastVertex = Vertex;
		TC.X += TCStep;
	}

	LastVertex = Base + XAxis * Radius;
	TC = FVector2f(0.0f, 1.0f);

	// Compute vertices for the top circle
	for (uint32 SideIndex = 0; SideIndex < Sides; SideIndex++)
	{
		const FVector3f Vertex = Base + (XAxis * FMath::Cos(AngleDelta * (SideIndex + 1)) +
										 YAxis * FMath::Sin(AngleDelta * (SideIndex + 1))) *
											Radius;
		FVector3f Normal = Vertex - Base;
		Normal.Normalize();

		FDynamicMeshVertex MeshVertex;

		MeshVertex.Position = Vertex + TopOffset;
#if UE_VERSION_OLDER_THAN(5, 0, 0)
		MeshVertex.TextureCoordinate[0] = TC;
#else
		MeshVertex.TextureCoordinate[0] = {(float) TC.X, (float) TC.Y};
#endif

		MeshVertex.SetTangents(-ZAxis, (-ZAxis) ^ Normal, Normal);

		OutVerts.Add(MeshVertex); // Add top vertex

		LastVertex = Vertex;
		TC.X += TCStep;
	}

	// Add top/bottom triangles, in the style of a fan.
	// Note if we wanted nice rendering of the caps then we need to duplicate the vertices and
	// modify texture/tangent coordinates.
	for (uint32 SideIndex = 1; SideIndex < Sides; SideIndex++)
	{
		int32 V0 = BaseVertIndex;
		int32 V1 = BaseVertIndex + SideIndex;
		int32 V2 = BaseVertIndex + ((SideIndex + 1) % Sides);

		// bottom
		OutIndices.Add(V0);
		OutIndices.Add(V1);
		OutIndices.Add(V2);

		// top
		OutIndices.Add(Sides + V2);
		OutIndices.Add(Sides + V1);
		OutIndices.Add(Sides + V0);
	}

	// Add sides.

	for (uint32 SideIndex = 0; SideIndex < Sides; SideIndex++)
	{
		int32 V0 = BaseVertIndex + SideIndex;
		int32 V1 = BaseVertIndex + ((SideIndex + 1) % Sides);
		int32 V2 = V0 + Sides;
		int32 V3 = V1 + Sides;

		OutIndices.Add(V0);
		OutIndices.Add(V2);
		OutIndices.Add(V1);

		OutIndices.Add(V2);
		OutIndices.Add(V3);
		OutIndices.Add(V1);
	}
}

AGX_MeshUtilities::CapsuleConstructionData::CapsuleConstructionData(
	float InRadius, float InHeight, uint32 InNumCircleSegments, uint32 InNumHeightSegments)
	: Radius(InRadius)
	, Height(InHeight)
	, CircleSegments(InNumCircleSegments)
	, HeightSegments(InNumHeightSegments)
{
}

void AGX_MeshUtilities::MakeCapsule(
	TArray<FVector3f>& Positions, TArray<FVector3f>& Normals, TArray<uint32>& Indices,
	TArray<FVector2f>& TexCoords, const CapsuleConstructionData& Data)
{
	auto LogConstructionError = [](const FString& Msg)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("AGX_MeshUtilities::MakeCapsule(): Invalid CapsuleConstructionData: %s."), *Msg);
	};

	const uint32 MaxUint16 = uint32(TNumericLimits<uint16>::Max());

	if (Data.CircleSegments < 4 || Data.CircleSegments > MaxUint16)
	{
		LogConstructionError(
			"CircleSegments must be at least 4 and at most " + FString::FromInt(MaxUint16));
		return;
	}

	if (Data.HeightSegments < 1 || Data.HeightSegments > MaxUint16)
	{
		LogConstructionError(
			"HeightSegments must be at least 1 and at most " + FString::FromInt(MaxUint16));
		return;
	}

	if (Data.Radius < KINDA_SMALL_NUMBER)
	{
		LogConstructionError(
			"Radius must be at least " + FString::SanitizeFloat(KINDA_SMALL_NUMBER));
		return;
	}

	if (Data.Height < KINDA_SMALL_NUMBER)
	{
		LogConstructionError(
			"Height must be at least " + FString::SanitizeFloat(KINDA_SMALL_NUMBER));
		return;
	}

	Positions.Empty();
	Normals.Empty();
	Indices.Empty();
	TexCoords.Empty();

	// Scale the V of the texture coordinates to make the capsule look like one single unit.
	const float TotalHeight = 2 * Data.Radius + Data.Height;
	const float VspanHalfSphere = Data.Radius / TotalHeight;
	const float VspanTube = Data.Height / TotalHeight;

	// Top sphere.
	AppendHalfSphere(
		Positions, Normals, Indices, TexCoords, Data.Radius, Data.Height / 2, Data.CircleSegments,
		Data.CircleSegments / 2, false, UvCoordinateScaler(0.f, 1.f, 0.f, VspanHalfSphere), false);

	AppendCylindricalTube(
		Positions, Normals, Indices, TexCoords, Data.Radius, Data.Height, Data.CircleSegments,
		Data.HeightSegments,
		UvCoordinateScaler(0.f, 1.f, VspanHalfSphere, VspanHalfSphere + VspanTube));

	// Bottom sphere.
	AppendHalfSphere(
		Positions, Normals, Indices, TexCoords, Data.Radius, -Data.Height / 2, Data.CircleSegments,
		Data.CircleSegments / 2, true,
		UvCoordinateScaler(0.f, 1.f, VspanHalfSphere + VspanTube, 1.f), false);
}

namespace AGX_Cone_Helpers
{
	/// @note This function can be completely replaced by the built in CalcConeVert located in
	/// SceneManagement.h when Engine version 4.25 is no longer supported by the plugin. The
	/// CalcConeVert can be accessed in Engine versions newer than 4.25.
	FVector3f CalcConeVert(float Angle1, float Angle2, float AzimuthAngle)
	{
		float ang1 = FMath::Clamp<float>(Angle1, 0.01f, (float) PI - 0.01f);
		float ang2 = FMath::Clamp<float>(Angle2, 0.01f, (float) PI - 0.01f);

		float sinX_2 = FMath::Sin(0.5f * ang1);
		float sinY_2 = FMath::Sin(0.5f * ang2);

		float sinSqX_2 = sinX_2 * sinX_2;
		float sinSqY_2 = sinY_2 * sinY_2;

		float tanX_2 = FMath::Tan(0.5f * ang1);
		float tanY_2 = FMath::Tan(0.5f * ang2);

		float phi =
			FMath::Atan2(FMath::Sin(AzimuthAngle) * sinY_2, FMath::Cos(AzimuthAngle) * sinX_2);
		float sinPhi = FMath::Sin(phi);
		float cosPhi = FMath::Cos(phi);
		float sinSqPhi = sinPhi * sinPhi;
		float cosSqPhi = cosPhi * cosPhi;

		float rSq, r, Sqr, alpha, beta;

		rSq = sinSqX_2 * sinSqY_2 / (sinSqX_2 * sinSqPhi + sinSqY_2 * cosSqPhi);
		r = FMath::Sqrt(rSq);
		Sqr = FMath::Sqrt(1 - rSq);
		alpha = r * cosPhi;
		beta = r * sinPhi;

		FVector3f ConeVert;

		ConeVert.X = (1 - 2 * rSq);
		ConeVert.Y = 2 * Sqr * alpha;
		ConeVert.Z = 2 * Sqr * beta;

		return ConeVert;
	}
}

void AGX_MeshUtilities::MakeCone(
	float Angle1, float Angle2, float Scale, float XOffset, uint32 NumSides,
	TArray<FDynamicMeshVertex>& OutVerts, TArray<uint32>& OutIndices)
{
	TArray<FVector3f> ConeVerts;
	ConeVerts.AddUninitialized(NumSides);

	for (uint32 i = 0; i < NumSides; i++)
	{
		float Fraction = (float) i / (float) (NumSides);
		float Azi = 2.f * PI * Fraction;
		ConeVerts[i] = (AGX_Cone_Helpers::CalcConeVert(Angle1, Angle2, Azi) * Scale) +
					   FVector3f(XOffset, 0, 0);
	}

	for (uint32 i = 0; i < NumSides; i++)
	{
		// Normal of the current face
		FVector3f TriTangentZ = ConeVerts[(i + 1) % NumSides] ^ ConeVerts[i]; // aka triangle normal
		FVector3f TriTangentY = ConeVerts[i];
		FVector3f TriTangentX = TriTangentZ ^ TriTangentY;

		FDynamicMeshVertex V0, V1, V2;

		V0.Position = FVector3f(0) + FVector3f(XOffset, 0, 0);
		V0.TextureCoordinate[0].X = 0.0f;
		V0.TextureCoordinate[0].Y = (float) i / NumSides;
		V0.SetTangents(TriTangentX, TriTangentY, FVector3f(-1, 0, 0));
		int32 I0 = OutVerts.Add(V0);

		V1.Position = ConeVerts[i];
		V1.TextureCoordinate[0].X = 1.0f;
		V1.TextureCoordinate[0].Y = (float) i / NumSides;
		FVector3f TriTangentZPrev =
			ConeVerts[i] ^ ConeVerts[i == 0 ? NumSides - 1 : i - 1]; // Normal of the previous face
																	 // connected to this face
		V1.SetTangents(TriTangentX, TriTangentY, (TriTangentZPrev + TriTangentZ).GetSafeNormal());
		int32 I1 = OutVerts.Add(V1);

		V2.Position = ConeVerts[(i + 1) % NumSides];
		V2.TextureCoordinate[0].X = 1.0f;
		V2.TextureCoordinate[0].Y = (float) ((i + 1) % NumSides) / NumSides;
		FVector3f TriTangentZNext =
			ConeVerts[(i + 2) % NumSides] ^
			ConeVerts[(i + 1) % NumSides]; // Normal of the next face connected to this face
		V2.SetTangents(TriTangentX, TriTangentY, (TriTangentZNext + TriTangentZ).GetSafeNormal());
		int32 I2 = OutVerts.Add(V2);

		// Flip winding for negative scale
		if (Scale >= 0.f)
		{
			OutIndices.Add(I0);
			OutIndices.Add(I1);
			OutIndices.Add(I2);
		}
		else
		{
			OutIndices.Add(I0);
			OutIndices.Add(I2);
			OutIndices.Add(I1);
		}
	}
}

AGX_MeshUtilities::CylindricalArrowConstructionData::CylindricalArrowConstructionData(
	float InCylinderRadius, float InCylinderHeight, float InConeRadius, float InConeHeight,
	bool bInBottomCap, uint32 InNumCircleSegments, const FLinearColor& InBaseColor,
	const FLinearColor& InTopColor)
	: CylinderRadius(InCylinderRadius)
	, CylinderHeight(InCylinderHeight)
	, ConeRadius(InConeRadius)
	, ConeHeight(InConeHeight)
	, bBottomCap(bInBottomCap)
	, CircleSegments(InNumCircleSegments)
	, BaseColor(InBaseColor)
	, TopColor(InTopColor)
	, VertexRows(bBottomCap ? 7 : 6)
	, VertexColumns(CircleSegments + 1)
	, Vertices(VertexRows * VertexColumns)
	,
#ifdef CONE_SINGULARITY
	Indices(CircleSegments * (2 * 6 + 1 * 3) + (bBottomCap ? (CircleSegments - 2) * 3 : 0))
#else
	Indices(3 * CircleSegments * 6 + (bBottomCap ? (CircleSegments - 2) * 3 : 0))
#endif
{
}

void AGX_MeshUtilities::CylindricalArrowConstructionData::AppendBufferSizes(
	uint32& InOutNumVertices, uint32& InOutNumIndices) const
{
	InOutNumVertices += Vertices;
	InOutNumIndices += Indices;
};

void AGX_MeshUtilities::MakeCylindricalArrow(
	FStaticMeshVertexBuffers& VertexBuffers, FDynamicMeshIndexBuffer32& IndexBuffer,
	uint32& NextFreeVertex, uint32& NextFreeIndex, const CylindricalArrowConstructionData& Data)
{
	check(Data.CircleSegments >= 4);
	check(Data.CircleSegments <= uint32(TNumericLimits<uint16>::Max()));
	check(Data.CylinderRadius >= 1.0e-6);
	check(Data.CylinderHeight >= 1.0e-6);
	check(Data.ConeRadius >= 1.0e-6);
	check(Data.ConeHeight >= 1.0e-6);

	check(NextFreeVertex + Data.Vertices <= VertexBuffers.PositionVertexBuffer.GetNumVertices());
	check(NextFreeVertex + Data.Vertices <= VertexBuffers.StaticMeshVertexBuffer.GetNumVertices());
	check(NextFreeVertex + Data.Vertices <= VertexBuffers.ColorVertexBuffer.GetNumVertices());
	check(NextFreeIndex + Data.Indices <= static_cast<uint32>(IndexBuffer.Indices.Num()));

	const uint32 FirstVertex = NextFreeVertex;
	const uint32 FirstIndex = NextFreeIndex;

	const float SegmentSize = 2.0f * PI / Data.CircleSegments;
	const float CylinderRadiusInv = 1.0f / Data.CylinderRadius;
	const float ConeRadiusInv = 1.0f / Data.ConeRadius;
	const float ConeFlankLength =
		FMath::Sqrt(Data.ConeRadius * Data.ConeRadius + Data.ConeHeight * Data.ConeHeight);
	const float ConeFlankLengthOverHeight = ConeFlankLength / Data.ConeHeight;
	const float ConeRadiusOverFlankLength = Data.ConeRadius / ConeFlankLength;
	// const float ConeHeightOverRadius = Data.ConeHeight / Data.ConeRadius;
	// const float ConeRadiusOverHeight = Data.ConeRadius / Data.ConeHeight;
	const float TotalHeight = Data.CylinderHeight + Data.ConeHeight;

	FVector3f Position, TangentX, TangentY;
	FVector3f TangentZ {FVector3f::ZeroVector};
	FLinearColor Color; /// \todo Set vertex color to something.
	FVector2f TexCoord;

	// Generate vertex attributes.

	// Vertex rows needs to be duplicated for hard edges. Building following layout:
	//
	// At height == 0:
	//   1 vertex row for the cap ring (optional)
	//   1 vertex row for bottom of cylinder
	//
	// At height == cylinder height:
	//   1 vertex row for top of cylinder
	//   1 vertex row for inner base ring of cone (i.e. for a hole where cone intersects top of
	//   cylinder) 1 vertex row for outer base ring of cone
	//
	// At height == cylinder height + cone height:
	//   1 vertex row for outer base ring of cone
	//   1 vertex row for top of cone
	//
	for (uint32 RowIndex = 0; RowIndex < Data.VertexRows; ++RowIndex)
	{
		const bool IsCap = Data.bBottomCap && RowIndex == 0;
		const bool IsTopRow = RowIndex + 1 == Data.VertexRows;

		const uint32 HeightIndex =
			static_cast<uint32>(
				RowIndex >=
				static_cast<uint32>(Data.bBottomCap ? 2 : 1)) + // at or above cylinder top?
			static_cast<uint32>(
				RowIndex >= static_cast<uint32>(Data.bBottomCap ? 6 : 5)); // at cone top?

		const uint32 NormalIndex =
			static_cast<uint32>(
				RowIndex >=
				static_cast<uint32>(
					Data.bBottomCap ? 1 : 0)) + // at or above cylinder shell (pointing out)?
			static_cast<uint32>(
				RowIndex >=
				static_cast<uint32>(
					Data.bBottomCap ? 3 : 2)) + // at or above cone base (pointing down)?
			static_cast<uint32>(
				RowIndex >= static_cast<uint32>(
								Data.bBottomCap ? 5 : 4)); // at cone shell (pointing upwards-out)?

		const uint32 RadiusIndex =
			static_cast<uint32>(
				RowIndex >=
				static_cast<uint32>(Data.bBottomCap ? 4 : 3)) + // at or above cone outer base ring?
			static_cast<uint32>(
				RowIndex >= static_cast<uint32>(Data.bBottomCap ? 6 : 5)); // at cone top?

		const float RowHeight =
			(HeightIndex == 1 ? Data.CylinderHeight : (HeightIndex == 2 ? TotalHeight : 0.0f)) -
			0.5f * TotalHeight;
#ifdef CONE_SINGULARITY
		const float RowRadius =
			RadiusIndex == 0 ? Data.CylinderRadius : (RadiusIndex == 1 ? Data.ConeRadius : 0.0f);
#else
		const float RowRadius =
			RadiusIndex == 0 ? Data.CylinderRadius : (RadiusIndex == 1 ? Data.ConeRadius : 0.01f);
#endif

		Color =
			FMath::Lerp(Data.BaseColor, Data.TopColor, (float) RowIndex / (Data.VertexRows - 1));

		// Add Data.VertexColumns num vertices in a circle per vertex row. The first and last vertex
		// in the same row have same position and normal, but different tex coords.
		for (uint32 ColumnIndex = 0; ColumnIndex < Data.VertexColumns; ++ColumnIndex)
		{
#ifdef CONE_SINGULARITY
			const float ColumnAngle =
				float(ColumnIndex) * SegmentSize + (IsTopRow ? SegmentSize * 0.5f : 0.0f);
#else
			const float ColumnAngle = ColumnIndex * SegmentSize;
#endif
			// Vertex Position
			Position.X = RowRadius * FMath::Cos(ColumnAngle);
			Position.Y = RowRadius * FMath::Sin(ColumnAngle);
			Position.Z = RowHeight;

			// Vertex Texture Coordinates, range between [0, 1]
			TexCoord.X = IsCap ? Position.X : ((float) ColumnIndex / Data.CircleSegments);
			TexCoord.Y = IsCap ? Position.Y : ((float) RowIndex / (Data.VertexRows - 1));

			/// \todo Normals and TexCoords needs to be fixed for top ring!! Because Quads becomes
			/// Triangles!?

			// Vertex Normal, Tangent, and Binormal
			switch (NormalIndex)
			{
				case 0: // bottom cap
					TangentZ = FVector3f(0.0f, 0.0f, -1.0f);
					break;
				case 1: // cylinder shell
					TangentZ = FVector3f(
						Position.X * CylinderRadiusInv, Position.Y * CylinderRadiusInv, 0.0f);
					break;
				case 2: // cone base
					TangentZ = FVector3f(0.0f, 0.0f, -1.0f);
					break;
				case 3: // cone shell
						/// \todo This normal is not perfect! Fix for fact that quads becomes
						/// triangles on cone!
#ifdef CONE_SINGULARITY
					if (IsTopRow)
						TangentZ = FVector3f(0.0f, 0.0f, 1.0f);
					else
#endif
						TangentZ = FVector3f(
							FMath::Cos(ColumnAngle) * ConeFlankLengthOverHeight,
							FMath::Sin(ColumnAngle) * ConeFlankLengthOverHeight,
							ConeRadiusOverFlankLength);
					TangentZ.Normalize(); /// \todo Should not be needed. But normal is a bit too
										  /// long without it. Investigate!
					// TangentZ = FVector3f(Position.X * ConeRadiusInv * ConeHeightOverRadius,
					// Position.Y * ConeRadiusInv
					// * ConeHeightOverRadius, ConeRadiusOverHeight); break;
					break;
				default:
					check(!"AGX_MeshUtilities::MakeCylindricalArrow reached invalid NormalIndex");
					break;
			}

			/// \todo Compute correctly based on texcoords!
			TangentX = FVector3f::ZeroVector;
			TangentY = FVector3f::ZeroVector;

			// Fill actual buffers
			VertexBuffers.PositionVertexBuffer.VertexPosition(NextFreeVertex) = Position;
			VertexBuffers.ColorVertexBuffer.VertexColor(NextFreeVertex) = Color.ToFColor(false);
#if UE_VERSION_OLDER_THAN(5, 0, 0)
			VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(NextFreeVertex, 0, TexCoord);
#else
			VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(
				NextFreeVertex, 0, {(float) TexCoord.X, (float) TexCoord.Y});
#endif
			VertexBuffers.StaticMeshVertexBuffer.SetVertexTangents(
				NextFreeVertex, TangentX, TangentY, TangentZ);

			NextFreeVertex++;
		}
	}

	// Generate triangle indexes for the bottom cap, with triangle fan pattern.
	uint32 K0, K1, K2;
	if (Data.bBottomCap)
	{
		K0 = FirstVertex; // first vertex in cap row
		K1 = K0 + 1; // second vertex in cap row
		K2 = K1 + 1; // third vertex in cap row

		for (uint32 CircleSegmentIndex = 1; CircleSegmentIndex < Data.CircleSegments - 1;
			 ++CircleSegmentIndex, ++K1, ++K2)
		{
			// 1 triangles per segment (except for first and last segment)

			IndexBuffer.Indices[NextFreeIndex++] = K0;
			IndexBuffer.Indices[NextFreeIndex++] = K1;
			IndexBuffer.Indices[NextFreeIndex++] = K2;
		}
	}

	// Generate triangle indexes for the side segments of the arrow (can be thought of a cylinder
	// with many segments, where each segment has its own unique vertices)
	const uint32 OffsetByCapVertices = Data.bBottomCap ? Data.VertexColumns : 0;
	const uint32 NumHeightSegments = 3;
	for (uint32 HeightSegmentIndex = 0; HeightSegmentIndex < 3; ++HeightSegmentIndex)
	{
		K0 = FirstVertex + OffsetByCapVertices + HeightSegmentIndex * 2 * Data.VertexColumns;
		K1 = K0 + Data.VertexColumns; // first vertex in next row

		for (uint32 CircleSegmentIndex = 0; CircleSegmentIndex < Data.CircleSegments;
			 ++CircleSegmentIndex, ++K0, ++K1)
		{
			// 2 triangles per segment (i.e. quad), except for at top of cone (because quads turns
			// to triangles at top)

			IndexBuffer.Indices[NextFreeIndex++] = K0 + 1;
			IndexBuffer.Indices[NextFreeIndex++] = K0;
			IndexBuffer.Indices[NextFreeIndex++] = K1;

#ifdef CONE_SINGULARITY
			if (HeightSegmentIndex + 1 < NumHeightSegments)
#endif
			{
				IndexBuffer.Indices[NextFreeIndex++] = K1;
				IndexBuffer.Indices[NextFreeIndex++] = K1 + 1;
				IndexBuffer.Indices[NextFreeIndex++] = K0 + 1;
			}
		}
	}

	check(NextFreeVertex - FirstVertex == Data.Vertices);
	check(NextFreeIndex - FirstIndex == Data.Indices);
}

#undef CONE_SINGULARITY

AGX_MeshUtilities::BendableArrowConstructionData::BendableArrowConstructionData(
	float InRectangleWidth, float InRectangleLength, float InTriangleWidth, float InTriangleLength,
	float InBendAngle, uint32 InNumSegments, const FLinearColor& InBaseColor,
	const FLinearColor& InTopColor)
	: RectangleWidth(InRectangleWidth)
	, RectangleLength(InRectangleLength)
	, TriangleWidth(InTriangleWidth)
	, TriangleLength(InTriangleLength)
	, BendAngle(InBendAngle)
	, Segments(InNumSegments)
	, BaseColor(InBaseColor)
	, TopColor(InTopColor)
	, RectangleSegments(std::max<uint32>(
		  1, std::min<uint32>(
				 InNumSegments - 1, InNumSegments * InRectangleLength /
										static_cast<float>(InRectangleLength + InTriangleLength))))
	, TriangleSegments(std::max<uint32>(1, InNumSegments - RectangleSegments))
	, RectangleVertexRows(RectangleSegments + 1)
	, TriangleVertexRows(TriangleSegments + 1)
	, Vertices(2 * (RectangleVertexRows + TriangleVertexRows))
	, Indices(6 * (RectangleSegments + TriangleSegments))
{
}

void AGX_MeshUtilities::BendableArrowConstructionData::AppendBufferSizes(
	uint32& InOutNumVertices, uint32& InOutNumIndices) const
{
	InOutNumVertices += Vertices;
	InOutNumIndices += Indices;
}

void AGX_MeshUtilities::MakeBendableArrow(
	FStaticMeshVertexBuffers& VertexBuffers, FDynamicMeshIndexBuffer32& IndexBuffer,
	uint32& NextFreeVertex, uint32& NextFreeIndex, const BendableArrowConstructionData& Data)
{
	check(Data.Segments >= 2);
	check(Data.RectangleSegments >= 1);
	check(Data.TriangleSegments >= 1);
	check(Data.Segments == Data.TriangleSegments + Data.RectangleSegments);
	check(Data.RectangleWidth >= 1.0e-6);
	check(Data.RectangleLength >= 1.0e-6);
	check(Data.TriangleWidth >= 1.0e-6);
	check(Data.TriangleLength >= 1.0e-6);
	check(Data.BendAngle >= 0.0f);

	check(NextFreeVertex + Data.Vertices <= VertexBuffers.PositionVertexBuffer.GetNumVertices());
	check(NextFreeVertex + Data.Vertices <= VertexBuffers.StaticMeshVertexBuffer.GetNumVertices());
	check(NextFreeVertex + Data.Vertices <= VertexBuffers.ColorVertexBuffer.GetNumVertices());
	check(NextFreeIndex + Data.Indices <= static_cast<uint32>(IndexBuffer.Indices.Num()));

	const uint32 FirstVertex = NextFreeVertex;
	const uint32 FirstIndex = NextFreeIndex;

	const uint32 NumTotalRows = Data.RectangleVertexRows + Data.TriangleVertexRows;
	const uint32 NumTotalSegments = Data.RectangleSegments + Data.TriangleSegments;
	const float TotalLength = Data.RectangleLength + Data.TriangleLength;
	const bool IsBending = Data.BendAngle > 1.0e-6;
	const float BendRadius = IsBending ? TotalLength / Data.BendAngle : 0.0f;

	FVector3f Position, TangentX, TangentY, TangentZ;
	FLinearColor Color; /// \todo Set vertex color to something.
	FVector2f TexCoord;

	// Generate vertex attributes.
	for (uint32 RowIndex = 0; RowIndex < NumTotalRows; ++RowIndex)
	{
		const bool IsRectangle = RowIndex < Data.RectangleVertexRows;
		const uint32 RectangleRowIndexClamped =
			FMath::Clamp<int32>(RowIndex, 0, Data.RectangleVertexRows - 1);
		const uint32 TriangleRowIndexClamped = FMath::Clamp<int32>(
			RowIndex - Data.RectangleVertexRows, 0, Data.TriangleVertexRows - 1);

		const float RowDistance =
			Data.RectangleLength * RectangleRowIndexClamped / Data.RectangleSegments +
			Data.TriangleLength * TriangleRowIndexClamped / Data.TriangleSegments;

		const float RowNormalizedDistance = RowDistance / TotalLength;
		const float RowAngularDistance = IsBending ? RowNormalizedDistance * Data.BendAngle : 0.0f;

		const float CurrentWidth =
			IsRectangle
				? Data.RectangleWidth
				: FMath::Lerp(
					  Data.TriangleWidth, 0.01f,
					  TriangleRowIndexClamped / static_cast<float>(Data.TriangleVertexRows - 1));

		Color = FMath::Lerp(Data.BaseColor, Data.TopColor, RowNormalizedDistance);

		for (uint32 VertexIndexInRow = 0; VertexIndexInRow < 2; ++VertexIndexInRow)
		{
			const float RowExtentFactor = VertexIndexInRow == 0 ? -0.5f : 0.5f;

			// Vertex Position
			Position.X = IsBending ? FMath::Sin(-RowAngularDistance) * BendRadius : 0.0f;
			Position.Y = CurrentWidth * RowExtentFactor;
			Position.Z = IsBending ? FMath::Cos(RowAngularDistance) * BendRadius : RowDistance;

			// Vertex Texture Coordinates
			TexCoord.X = Position.Y;
			TexCoord.Y = RowDistance;

			// Vertex Normal, Tangent, and Binormal
			if (IsBending)
			{
				TangentZ = FVector3f(Position.X / BendRadius, 0.0f, Position.Z / BendRadius);
			}
			else
			{
				TangentZ = FVector3f(-1.0f, 0.0f, 0.0f);
			}

			/// \todo Compute correctly based on texcoords!
			TangentX = FVector3f::ZeroVector;
			TangentY = FVector3f::ZeroVector;

			// Fill actual buffers
			VertexBuffers.PositionVertexBuffer.VertexPosition(NextFreeVertex) = Position;
			VertexBuffers.ColorVertexBuffer.VertexColor(NextFreeVertex) = Color.ToFColor(false);
#if UE_VERSION_OLDER_THAN(5, 0, 0)
			VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(NextFreeVertex, 0, TexCoord);
#else
			VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(
				NextFreeVertex, 0, {(float) TexCoord.X, (float) TexCoord.Y});
#endif
			VertexBuffers.StaticMeshVertexBuffer.SetVertexTangents(
				NextFreeVertex, TangentX, TangentY, TangentZ);

			NextFreeVertex++;
		}
	}

	// Generate triangle indexes.
	for (uint32 SegmentIndex = 0, V0 = FirstVertex; SegmentIndex < NumTotalSegments;
		 ++SegmentIndex, V0 += 2)
	{
		if (SegmentIndex == Data.RectangleSegments)
		{
			V0 += 2; // Do not use last row of arrow's rectangle part for first quad in the arrow's
					 // triangle part.
		}

		// 2 triangles per segment (i.e. quad)

		IndexBuffer.Indices[NextFreeIndex++] = V0;
		IndexBuffer.Indices[NextFreeIndex++] = V0 + 3;
		IndexBuffer.Indices[NextFreeIndex++] = V0 + 1;

		IndexBuffer.Indices[NextFreeIndex++] = V0;
		IndexBuffer.Indices[NextFreeIndex++] = V0 + 2;
		IndexBuffer.Indices[NextFreeIndex++] = V0 + 3;
	}

	check(NextFreeVertex - FirstVertex == Data.Vertices);
	check(NextFreeIndex - FirstIndex == Data.Indices);
}

void AGX_MeshUtilities::PrintMeshToLog(
	const FStaticMeshVertexBuffers& VertexBuffers, const FDynamicMeshIndexBuffer32& IndexBuffer)
{
	const uint32 NumVertices = VertexBuffers.PositionVertexBuffer.GetNumVertices();
	const uint32 NumIndices = static_cast<uint32>(IndexBuffer.Indices.Num());

	UE_LOG(LogAGX, Log, TEXT("AGX_MeshUtilities::PrintMeshToLog() : Begin printing mesh data."));

	UE_LOG(LogAGX, Log, TEXT("      ----- Vertex Buffer -----"));
	for (uint32 VertexIndex = 0; VertexIndex < NumVertices; ++VertexIndex)
	{
		UE_LOG(
			LogAGX, Log, TEXT("  Vertex[%d] Position = <%s>, Normal = <%s>"), VertexIndex,
			*VertexBuffers.PositionVertexBuffer.VertexPosition(VertexIndex).ToString(),
			*VertexBuffers.StaticMeshVertexBuffer.VertexTangentZ(VertexIndex).ToString());
	}

	UE_LOG(LogAGX, Log, TEXT("      ----- Index Buffer -----"));
	for (uint32 Index = 0; Index < NumIndices; ++Index)
	{
		UE_LOG(
			LogAGX, Log, TEXT("  Index[%d] = %d \t(Position = <%s>, Normal = <%s>)"), Index,
			IndexBuffer.Indices[Index],
			*VertexBuffers.PositionVertexBuffer.VertexPosition(IndexBuffer.Indices[Index])
				 .ToString(),
			*VertexBuffers.StaticMeshVertexBuffer.VertexTangentZ(IndexBuffer.Indices[Index])
				 .ToString());
	}

	UE_LOG(LogAGX, Log, TEXT("AGX_MeshUtilities::PrintMeshToLog() : Finished printing mesh data."));
}

AGX_MeshUtilities::DiskArrayConstructionData::DiskArrayConstructionData(
	float InRadius, uint32 InNumCircleSegments, float InSpacing, uint32 InDisks, bool bInTwoSided,
	const FLinearColor InMiddleDiskColor, const FLinearColor InOuterDiskColor,
	TArray<FTransform3f> InSpacingsOverride)
	: Radius(InRadius)
	, CircleSegments(InNumCircleSegments)
	, Spacing(InSpacing)
	, Disks(InDisks)
	, bTwoSided(bInTwoSided)
	, MiddleDiskColor(InMiddleDiskColor)
	, OuterDiskColor(InOuterDiskColor)
	, SpacingsOverride(InSpacingsOverride)
	, SidesPerDisk(bInTwoSided ? 2 : 1)
	, VerticesPerSide(CircleSegments)
	, VerticesPerDisk(VerticesPerSide * SidesPerDisk)
	, Vertices(Disks * VerticesPerDisk)
	, Indices(Disks * 3 * (CircleSegments - 2) * SidesPerDisk)
{
}

void AGX_MeshUtilities::DiskArrayConstructionData::AppendBufferSizes(
	uint32& InOutNumVertices, uint32& InOutNumIndices) const
{
	InOutNumVertices += Vertices;
	InOutNumIndices += Indices;
}

void AGX_MeshUtilities::MakeDiskArray(
	FStaticMeshVertexBuffers& VertexBuffers, FDynamicMeshIndexBuffer32& IndexBuffer,
	uint32& NextFreeVertex, uint32& NextFreeIndex, const DiskArrayConstructionData& Data)
{
	check(Data.CircleSegments >= 4);
	check(Data.CircleSegments <= uint32(TNumericLimits<uint16>::Max()));
	check(Data.Disks >= 1);
	check(Data.Disks <= uint32(TNumericLimits<uint16>::Max()));
	check(Data.Radius >= 1.0e-6);

	check(NextFreeVertex + Data.Vertices <= VertexBuffers.PositionVertexBuffer.GetNumVertices());
	check(NextFreeVertex + Data.Vertices <= VertexBuffers.StaticMeshVertexBuffer.GetNumVertices());
	check(NextFreeVertex + Data.Vertices <= VertexBuffers.ColorVertexBuffer.GetNumVertices());
	check(NextFreeIndex + Data.Indices <= static_cast<uint32>(IndexBuffer.Indices.Num()));

	const uint32 FirstVertex = NextFreeVertex;
	const uint32 FirstIndex = NextFreeIndex;

	const float SegmentSize = 2.0 * PI / Data.CircleSegments;
	const float RadiusInv = 1.0f / Data.Radius;
	const float TotalHeight = Data.Spacing * Data.Disks;

	FVector3f Position, TangentX, TangentY, TangentZ;
	FLinearColor Color; /// \todo Set vertex color to something.
	FVector2f TexCoord;

	// Generate vertex attributes.
	for (uint32 DiskIndex = 0; DiskIndex < Data.Disks; ++DiskIndex)
	{
		const float NormalizedDiskIndex =
			DiskIndex / static_cast<float>(Data.Disks > 1 ? Data.Disks - 1 : 1);
		const float DiskHeight = TotalHeight * 0.5f - NormalizedDiskIndex * TotalHeight;

		Color = FMath::Lerp(
			Data.MiddleDiskColor, Data.OuterDiskColor,
			FMath::Abs(NormalizedDiskIndex - 0.5f) * 2.0f);

		for (uint32 SideIndex = 0; SideIndex < Data.SidesPerDisk; ++SideIndex)
		{
			for (uint32 CircleVertexIndex = 0; CircleVertexIndex < Data.CircleSegments;
				 ++CircleVertexIndex)
			{
				float CircleVertexAngle = CircleVertexIndex * SegmentSize;

				// Vertex Position
				Position.X = Data.Radius * FMath::Cos(CircleVertexAngle);
				Position.Y = Data.Radius * FMath::Sin(CircleVertexAngle);
				Position.Z = DiskHeight;

				// Vertex Texture Coordinates, range between [0, 1]
				TexCoord.X = Position.X;
				TexCoord.Y = Position.Y;

				// Vertex Normal, Tangent, and Binormal
				switch (SideIndex)
				{
					case 0: // up side
						TangentZ = FVector3f(0.0f, 0.0f, 1.0f);
						break;
					case 1: // down cap
					default:
						TangentZ = FVector3f(0.0f, 0.0f, -1.0f);
						break;
				}

				if (DiskIndex < static_cast<uint32>(Data.SpacingsOverride.Num()))
				{
					// Remove spacing and use the spacing defined in the array instead. May include
					// rotation as well.
					Position.Z = 0.0f;
					Position = Data.SpacingsOverride[DiskIndex].TransformPosition(Position);
					TangentZ = Data.SpacingsOverride[DiskIndex].TransformVector(TangentZ);
				}

				/// \todo Compute correctly based on texcoords!
				TangentX = FVector3f::ZeroVector;
				TangentY = FVector3f::ZeroVector;

				// Fill actual buffers
				VertexBuffers.PositionVertexBuffer.VertexPosition(NextFreeVertex) = Position;
				VertexBuffers.ColorVertexBuffer.VertexColor(NextFreeVertex) = Color.ToFColor(false);
#if UE_VERSION_OLDER_THAN(5, 0, 0)
				VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(NextFreeVertex, 0, TexCoord);
#else
				VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(
					NextFreeVertex, 0, {(float) TexCoord.X, (float) TexCoord.Y});
#endif
				VertexBuffers.StaticMeshVertexBuffer.SetVertexTangents(
					NextFreeVertex, TangentX, TangentY, TangentZ);

				NextFreeVertex++;
			}
		}
	}

	// Generate triangle indexes for each disk (and each side), with triangle fan pattern.
	uint32 V0, V1, V2;
	for (uint32 DiskIndex = 0; DiskIndex < Data.Disks; ++DiskIndex)
	{
		for (uint32 SideIndex = 0; SideIndex < Data.SidesPerDisk; ++SideIndex)
		{
			V0 = FirstVertex + SideIndex * Data.VerticesPerSide + DiskIndex * Data.VerticesPerDisk;

			// 1 triangle per segment except for first and last.
			for (uint32 CircleSegmentIndex = 1; CircleSegmentIndex < Data.CircleSegments - 1;
				 ++CircleSegmentIndex)
			{
				V1 = V0 + CircleSegmentIndex;
				V2 = V0 + ((CircleSegmentIndex + 1) % Data.CircleSegments);

				if (SideIndex == 0)
				{
					IndexBuffer.Indices[NextFreeIndex++] = V2;
					IndexBuffer.Indices[NextFreeIndex++] = V1;
					IndexBuffer.Indices[NextFreeIndex++] = V0;
				}
				else
				{
					IndexBuffer.Indices[NextFreeIndex++] = V0;
					IndexBuffer.Indices[NextFreeIndex++] = V1;
					IndexBuffer.Indices[NextFreeIndex++] = V2;
				}
			}
		}
	}

	check(NextFreeVertex - FirstVertex == Data.Vertices);
	check(NextFreeIndex - FirstIndex == Data.Indices);
}

FAGX_MeshWithTransform AGX_MeshUtilities::FindFirstChildMesh(const USceneComponent& Component)
{
	TArray<USceneComponent*> Children;
	Component.GetChildrenComponents(/*bIncludeAllDescendants*/ false, Children);

	for (USceneComponent* Child : Children)
	{
		if (UStaticMeshComponent* MeshComponent = Cast<UStaticMeshComponent>(Child))
		{
			return FAGX_MeshWithTransform(
				MeshComponent->GetStaticMesh(), MeshComponent->GetComponentTransform());
		}
	}

	return FAGX_MeshWithTransform();
}

TArray<FAGX_MeshWithTransform> AGX_MeshUtilities::FindChildrenMeshes(
	const USceneComponent& Component, bool Recursive)
{
	TArray<UStaticMeshComponent*> MeshComponents = FindChildrenMeshComponents(Component, Recursive);
	TArray<FAGX_MeshWithTransform> Meshes;

	for (UStaticMeshComponent* Comp : MeshComponents)
	{
		if (UStaticMesh* S = Comp->GetStaticMesh())
		{
			Meshes.Add(FAGX_MeshWithTransform(S, Comp->GetComponentTransform()));
		}
	}

	return Meshes;
}

TArray<UStaticMeshComponent*> AGX_MeshUtilities::FindChildrenMeshComponents(
	const USceneComponent& Component, bool Recursive)
{
	TArray<USceneComponent*> Children;
	TArray<UStaticMeshComponent*> Meshes;
	Component.GetChildrenComponents(Recursive, Children);

	for (USceneComponent* Child : Children)
	{
		if (UStaticMeshComponent* MeshComponent = Cast<UStaticMeshComponent>(Child))
		{
			Meshes.Add(MeshComponent);
		}
	}

	return Meshes;
}

FAGX_MeshWithTransform AGX_MeshUtilities::FindFirstParentMesh(const USceneComponent& Component)
{
	TArray<USceneComponent*> Ancestors;
	Component.GetParentComponents(Ancestors);

	for (USceneComponent* Ancestor : Ancestors)
	{
		if (UStaticMeshComponent* MeshComponent = Cast<UStaticMeshComponent>(Ancestor))
		{
			return FAGX_MeshWithTransform(
				MeshComponent->GetStaticMesh(), MeshComponent->GetComponentTransform());
		}
	}

	return FAGX_MeshWithTransform();
}

namespace AGX_MeshUtilities_helpers
{
	/**
	 * Read triangle mesh data directly from the Static Mesh asset.
	 *
	 * This works for Play In Editor sessions and for meshes that has the Allow CPU Access flag set.
	 * It does not work for meshes without the Allow CPU Access flag set in cooked builds. In this
	 * case use CopyMeshBuffersFromGPUMemory instead.
	 *
	 * @param Mesh The mesh to read triangles from.
	 * @param OutPositions Array to which the vertex locations are written.
	 * @param OutIndices Array to which the vertex indices are written.
	 */
	void CopyMeshBuffersFromCPUMemory(
		const FStaticMeshLODResources& Mesh, TArray<FVector3f>& OutPositions,
		TArray<uint32>& OutIndices)
	{
		// Copy positions.
		const FPositionVertexBuffer& MeshPositions = Mesh.VertexBuffers.PositionVertexBuffer;
		const uint32 NumPositions = static_cast<uint32>(MeshPositions.GetNumVertices());
		OutPositions.Reserve(NumPositions);
		for (uint32 I = 0; I < NumPositions; ++I)
		{
			OutPositions.Add(MeshPositions.VertexPosition(I));
		}

		// Copy indices.
		const FIndexArrayView MeshIndices = Mesh.IndexBuffer.GetArrayView();
		const int32 NumIndices = MeshIndices.Num();
		check(MeshIndices.Num() == Mesh.IndexBuffer.GetNumIndices()); /// \todo Remove this line.
		OutIndices.Reserve(NumIndices);
		for (int32 I = 0; I < NumIndices; ++I)
		{
			check(MeshIndices[I] < NumPositions);
			OutIndices.Add(MeshIndices[I]);
		}
	}

	/**
	 * Enqueue, and wait for the completion of, a render command to copy the triangle mesh data from
	 * GPU memory to CPU memory.
	 *
	 * This approach is required for cooked builds unless the Static Mesh asset has the Allow CPU
	 * Access flag set.
	 *
	 * @param Mesh The mesh to read triangle mesh data from.
	 * @param OutPositions Array to which the vertex positions are written.
	 * @param OutIndices Array to which the vertex indices are written.
	 */
	void CopyMeshBuffersFromGPUMemory(
		const FStaticMeshLODResources& Mesh, TArray<FVector3f>& OutPositions,
		TArray<uint32>& OutIndices)
	{
		const uint32 NumIndices = static_cast<uint32>(Mesh.IndexBuffer.GetNumIndices());
		OutIndices.Reserve(NumIndices);

		const uint32 NumPositions = Mesh.VertexBuffers.PositionVertexBuffer.GetNumVertices();
		OutPositions.Reserve(NumPositions);

		ENQUEUE_RENDER_COMMAND(FCopyMeshBuffers)
		(
			[&](FRHICommandListImmediate& RHICmdList)
			{
				// Copy vertex buffer.
				{
					const FBufferRHIRef& Buffer =
						Mesh.VertexBuffers.PositionVertexBuffer.VertexBufferRHI;
					const uint32 NumBytes = Buffer->GetSize();
					FRHIGPUBufferReadback Readback(TEXT("BufferReadback: Positions"));
					Readback.EnqueueCopy(RHICmdList, Buffer);
					RHICmdList.BlockUntilGPUIdle();
					FVector3f* Data = static_cast<FVector3f*>(Readback.Lock(NumBytes));
					for (uint32 I = 0; I < NumPositions; I++)
					{
						OutPositions.Add(Data[I]);
					}
					Readback.Unlock();
				}

				// Copy index buffer.
				{
					const FBufferRHIRef& Buffer = Mesh.IndexBuffer.IndexBufferRHI;
					const uint32 NumBytes = Buffer->GetSize();
					FRHIGPUBufferReadback Readback(TEXT("BufferReadback: Indices"));
					Readback.EnqueueCopy(RHICmdList, Buffer);
					RHICmdList.BlockUntilGPUIdle();
					void* Data = Readback.Lock(NumBytes);
					switch (Buffer->GetStride())
					{
						case 2:
						{
							const uint16* const IndexData = static_cast<uint16*>(Data);
							for (uint32 I = 0; I < NumIndices; I++)
							{
								check(IndexData[I] < NumPositions);
								OutIndices.Add(static_cast<uint32>(IndexData[I]));
							}

							break;
						}
						case 4:
						{
							const uint32* const IndexData = static_cast<uint32*>(Data);
							for (uint32 I = 0; I < NumIndices; ++I)
							{
								check(IndexData[I] < NumPositions);
								OutIndices.Add(static_cast<uint32>(IndexData[I]));
							}
							break;
						}
						default:
							UE_LOG(
								LogAGX, Error,
								TEXT("Unexpected index size %d, cannot read Static Mesh data."),
								Buffer->GetStride());
					}
					Readback.Unlock();
				}
			});

		// Wait for rendering thread to finish.
		FlushRenderingCommands();
	}

	static int32 AddCollisionVertex(
		const FVector3f& VertexPosition, const FTransform& Transform,
		TArray<FVector>& CollisionVertices, TMap<FVector3f, int32>& MeshToCollisionVertexIndices)
	{
		if (int32* CollisionVertexIndexPtr = MeshToCollisionVertexIndices.Find(VertexPosition))
		{
			// Already been added once, so just return the index.
			return *CollisionVertexIndexPtr;
		}
		else
		{
			// Copy position from mesh to collision data.
			int CollisionVertexIndex =
				CollisionVertices.Add(Transform.TransformPosition(FromMeshVector(VertexPosition)));

			// Add collision index to map.
			MeshToCollisionVertexIndices.Add(VertexPosition, CollisionVertexIndex);

			return CollisionVertexIndex;
		}
	}
}

bool AGX_MeshUtilities::GetStaticMeshCollisionData(
	const FAGX_MeshWithTransform& InMesh, const FTransform& RelativeTo,
	TArray<FVector>& OutVertices, TArray<FTriIndices>& OutIndices, const uint32* LodIndexOverride)
{
	if (!InMesh.IsValid())
	{
		return false;
	}

	// NOTE: Code below is very similar to UStaticMesh::GetPhysicsTriMeshData,
	// only with some simplifications, so one can check that implementation for reference.
	// One important difference is that we hash on vertex position instead of index because we
	// want to re-merge vertices that has been split in the rendering data.

	// Final vertex positions will be given relative to RelativeTo,
	// and any scale needs to be baked into the positions, because AGX
	// does not support scale.
	const FTransform RelativeTransform = InMesh.Transform.GetRelativeTransform(RelativeTo);
	const UStaticMesh& StaticMesh = *InMesh.Mesh.Get();

	const uint32 LodIndex = FMath::Clamp<int32>(
		LodIndexOverride != nullptr ? *LodIndexOverride : StaticMesh.LODForCollision, 0,
		StaticMesh.GetNumLODs() - 1);

	if (!StaticMesh.HasValidRenderData(/*bCheckLODForVerts*/ true, LodIndex))
		return false;

	const FStaticMeshLODResources& Mesh = StaticMesh.GetLODForExport(LodIndex);
	TMap<FVector3f, int32> MeshToCollisionVertexIndices;

	// Copy the Index and Vertex buffers from the mesh.
	TArray<uint32> IndexBuffer;
	TArray<FVector3f> VertexBuffer;

	// Depending on if the triangle data is available in CPU memory or not, either directly copy
	// from CPU memory with the current thread, assumed to be the game thread, or use the render
	// thread to copy from GPU memory to CPU memory.
	//
	// We expect that Allow CPU Access will be false most of the time, and we don't want to require
	// the user to tick the checkbox on every mesh they want to create a Trimesh from. Should
	// the Trimesh set the flag on the Static Mesh asset? Can it? Doing it here is too late since
	// we're now in Begin Play, we need to set the flag on the Editor instance, not the Play
	// instance. The state handling of the flag will be complicated since we don't want to leave
	// them checked on Static Mesh assets that are no longer used by any Trimesh, and we don't want
	// to disable it on a Static Mesh asset on which the end-user enabled it on themselves.
	if (StaticMesh.bAllowCPUAccess)
	{
		AGX_MeshUtilities_helpers::CopyMeshBuffersFromCPUMemory(Mesh, VertexBuffer, IndexBuffer);
	}
	else
	{
#if WITH_EDITOR
		// Edito builds keep the mesh data in CPU memory regardless of whether the Allow CPU Access
		// flag has been set or not.
		AGX_MeshUtilities_helpers::CopyMeshBuffersFromCPUMemory(Mesh, VertexBuffer, IndexBuffer);
#else
		AGX_MeshUtilities_helpers::CopyMeshBuffersFromGPUMemory(Mesh, VertexBuffer, IndexBuffer);
#endif
	}

	if (IndexBuffer.Num() == 0 || VertexBuffer.Num() == 0)
	{
		return false;
	}

	check(Mesh.IndexBuffer.GetNumIndices() == IndexBuffer.Num());
	check(Mesh.VertexBuffers.PositionVertexBuffer.GetNumVertices() == VertexBuffer.Num());

	// Merge vertices at the same location.
	const uint32 NumIndices = static_cast<uint32>(IndexBuffer.Num());
	for (int32 SectionIndex = 0; SectionIndex < Mesh.Sections.Num(); ++SectionIndex)
	{
		const FStaticMeshSection& Section = Mesh.Sections[SectionIndex];
		const uint32 OnePastLastIndex = Section.FirstIndex + Section.NumTriangles * 3;

		for (uint32 Index = Section.FirstIndex; Index < OnePastLastIndex; Index += 3)
		{
			if (Index + 2 >= NumIndices)
			{
				break;
			}

			const uint32 IndexFirst = IndexBuffer[Index];
			const uint32 IndexSecond = IndexBuffer[Index + 1];
			const uint32 IndexThird = IndexBuffer[Index + 2];
			FTriIndices Triangle;

			Triangle.v0 = AGX_MeshUtilities_helpers::AddCollisionVertex(
				VertexBuffer[IndexFirst], RelativeTransform, OutVertices,
				MeshToCollisionVertexIndices);
			Triangle.v1 = AGX_MeshUtilities_helpers::AddCollisionVertex(
				VertexBuffer[IndexSecond], RelativeTransform, OutVertices,
				MeshToCollisionVertexIndices);
			Triangle.v2 = AGX_MeshUtilities_helpers::AddCollisionVertex(
				VertexBuffer[IndexThird], RelativeTransform, OutVertices,
				MeshToCollisionVertexIndices);

			OutIndices.Add(Triangle);
		}
	}

	return OutVertices.Num() > 0 && OutIndices.Num() > 0;
}

TArray<FAGX_MeshWithTransform> AGX_MeshUtilities::ToMeshWithTransformArray(
	const TArray<AStaticMeshActor*> Actors)
{
	TArray<FAGX_MeshWithTransform> Meshes;
	for (AStaticMeshActor* M : Actors)
	{
		if (M->GetStaticMeshComponent() == nullptr)
		{
			continue;
		}

		if (UStaticMesh* StaticMesh = M->GetStaticMeshComponent()->GetStaticMesh())
		{
			FAGX_MeshWithTransform MeshWithTransfom(
				StaticMesh, M->GetStaticMeshComponent()->GetComponentTransform());
			Meshes.Add(MeshWithTransfom);
		}
	}

	return Meshes;
}

UStaticMesh* AGX_MeshUtilities::CreateStaticMesh(
	const TArray<FVector3f>& Vertices, const TArray<uint32>& Indices,
	const TArray<FVector3f>& Normals, const TArray<FVector2D>& UVs,
	const TArray<FVector3f>& Tangents, const FString& Name,
	UObject& Outer, UMaterialInterface* Material)
{
	UStaticMesh* StaticMesh =
		NewObject<UStaticMesh>(&Outer, NAME_None, RF_Public | RF_Standalone);

	// Create MeshDescription.
	FMeshDescription MeshDescription;
	FStaticMeshAttributes Attributes(MeshDescription);
	Attributes.Register();

#if !WITH_EDITOR
	// Needed for render materials to show up for runtime imported meshes.
	StaticMesh->GetStaticMaterials().Add(FStaticMaterial());
#endif

	// Fill MeshDescription with vertex data.
	TMap<int32, FVertexID> VertexIDMap;

	// Create vertices.
	for (int32 I = 0; I < Vertices.Num(); I++)
	{
		FVertexID VertexID = MeshDescription.CreateVertex();
		Attributes.GetVertexPositions()[VertexID] = Vertices[I];
		VertexIDMap.Add(I, VertexID);
	}

	// Create a polygon group for the material.
	FPolygonGroupID PolygonGroupID = MeshDescription.CreatePolygonGroup();

	// Create triangles.
	for (int32 i = 0; i < Indices.Num(); i += 3)
	{
		FVertexInstanceID VertexInstanceIDs[3];

		for (int32 j = 0; j < 3; ++j)
		{
			const int32 VertexIndex = Indices[i + j];
			FVertexInstanceID VertexInstanceID =
				MeshDescription.CreateVertexInstance(VertexIDMap[VertexIndex]);
			VertexInstanceIDs[j] = VertexInstanceID;

			// Assign per-vertex-instance data (UVs, normals, tangents).
			Attributes.GetVertexInstanceUVs()[VertexInstanceID] = FVector2f(UVs[VertexIndex]);
			Attributes.GetVertexInstanceNormals()[VertexInstanceID] = Normals[VertexIndex];
			Attributes.GetVertexInstanceTangents()[VertexInstanceID] = Tangents[VertexIndex];
		}

		// Create the polygon.
		MeshDescription.CreatePolygon(
			PolygonGroupID, TArray<FVertexInstanceID> {
								VertexInstanceIDs[0], VertexInstanceIDs[1], VertexInstanceIDs[2]});
	}

	StaticMesh->Rename(*Name);

	if (Material != nullptr)
		StaticMesh->AddMaterial(Material);

	// Assign the MeshDescription to the UStaticMesh.
	UStaticMesh::FBuildMeshDescriptionsParams Params;
#if WITH_EDITOR
	Params.bFastBuild = false; // If set to true, errors occurs when trying to save this to disk.
#else
	Params.bFastBuild = true;
#endif
	Params.bBuildSimpleCollision = true;
	Params.bAllowCpuAccess = true;
	StaticMesh->BuildFromMeshDescriptions({&MeshDescription}, Params);

	return StaticMesh;
}

bool AGX_MeshUtilities::CopyStaticMesh(UStaticMesh* Source, UStaticMesh* Destination)
{
	if (Source == nullptr || Destination == nullptr || Source->GetRenderData() == nullptr)
	{
		UE_LOG(LogAGX, Warning, TEXT("CopyStaticMesh got invalid Source or Destination mesh!"));
		return false;
	}

	FMeshDescription MeshDescription;
	FStaticMeshAttributes Attributes(MeshDescription);
	Attributes.Register();

	const FStaticMeshLODResources& LOD = Source->GetRenderData()->LODResources[0];

	// Map for tracking vertices.
	TMap<int32, FVertexID> VertexIDMap;

	// Extract vertex positions.
	for (uint32 i = 0; i < LOD.VertexBuffers.PositionVertexBuffer.GetNumVertices(); i++)
	{
		FVertexID VertexID = MeshDescription.CreateVertex();
		Attributes.GetVertexPositions()[VertexID] =
			LOD.VertexBuffers.PositionVertexBuffer.VertexPosition(i);
		VertexIDMap.Add(i, VertexID);
	}

	FPolygonGroupID PolygonGroupID = MeshDescription.CreatePolygonGroup();

	// Extract index data and create faces.
	const FIndexArrayView Indices = LOD.IndexBuffer.GetArrayView();
	for (int32 i = 0; i < Indices.Num(); i += 3)
	{
		FVertexInstanceID VertexInstanceIDs[3];

		for (int32 j = 0; j < 3; ++j)
		{
			const int32 VertexIndex = Indices[i + j];
			FVertexInstanceID VertexInstanceID =
				MeshDescription.CreateVertexInstance(VertexIDMap[VertexIndex]);
			VertexInstanceIDs[j] = VertexInstanceID;

			// Copy per-vertex-instance data.
			Attributes.GetVertexInstanceUVs()[VertexInstanceID] =
				FVector2f(LOD.VertexBuffers.StaticMeshVertexBuffer.GetVertexUV(VertexIndex, 0));
			Attributes.GetVertexInstanceNormals()[VertexInstanceID] =
				LOD.VertexBuffers.StaticMeshVertexBuffer.VertexTangentZ(VertexIndex);
			Attributes.GetVertexInstanceTangents()[VertexInstanceID] =
				LOD.VertexBuffers.StaticMeshVertexBuffer.VertexTangentX(VertexIndex);
		}

		MeshDescription.CreatePolygon(
			PolygonGroupID, TArray<FVertexInstanceID> {
								VertexInstanceIDs[0], VertexInstanceIDs[1], VertexInstanceIDs[2]});
	}

	{
		Destination->GetStaticMaterials().Empty();
		int32 MaterialIndex = 0;
		while (auto Mat = Source->GetMaterial(MaterialIndex))
		{
			Destination->AddMaterial(Mat);
			MaterialIndex++;
		}
	}

	UStaticMesh::FBuildMeshDescriptionsParams Params;
#if WITH_EDITOR
	Params.bFastBuild = false;
#else
	Params.bFastBuild = true;
#endif
	Params.bBuildSimpleCollision = true;
	Params.bAllowCpuAccess = true;
	Destination->BuildFromMeshDescriptions({&MeshDescription}, Params);

	return true;
}

UStaticMesh* AGX_MeshUtilities::CreateStaticMesh(
	const FRenderDataBarrier& RenderData, UObject& Outer, UMaterialInterface* Material)
{
	if (!RenderData.HasMesh() || !RenderData.HasNative())
		return nullptr;

	TArray<FVector3f> Vertices;
	const auto VerticesAGX = RenderData.GetPositions();
	Vertices.Reserve(VerticesAGX.Num());
	for (const FVector& Position : VerticesAGX)
	{
		Vertices.Add(FVector3f(Position));
	}

	const TArray<uint32> Indices = RenderData.GetIndices();

	const auto NormalsAGX = RenderData.GetNormals();
	TArray<FVector3f> Normals;
	Normals.Reserve(NormalsAGX.Num());
	for (const FVector& Normal : NormalsAGX)
	{
		Normals.Add(FVector3f(Normal));
	}

	TArray<FVector2D> UVs;
	const TArray<FVector2D> RenderTexCoords = RenderData.GetTextureCoordinates();
	UVs.Reserve(RenderTexCoords.Num());
	for (const FVector2D& UV : RenderTexCoords)
	{
		UVs.Add(UV);
	}

	// Generate tangents (placeholder, can be computed as needed)
	TArray<FVector3f> Tangents;
	Tangents.SetNumZeroed(Vertices.Num());

	const FString Name = FAGX_ObjectUtilities::SanitizeAndMakeNameUnique(&Outer,
		FString::Printf(TEXT("SM_RenderMesh_%s"), *RenderData.GetGuid().ToString()), nullptr);
	return CreateStaticMesh(Vertices, Indices, Normals, UVs, Tangents, Name, Outer, Material);
}

bool AGX_MeshUtilities::HasRenderDataMesh(const FShapeBarrier& Shape)
{
	if (!Shape.HasValidRenderData())
		return false;

	const FRenderDataBarrier RenderData = Shape.GetRenderData();
	return RenderData.HasNative() && RenderData.HasMesh();
}

UMaterialInterface* AGX_MeshUtilities::CreateRenderMaterial(
	const FAGX_RenderMaterial& MaterialBarrier, UMaterial* Base, UObject& Owner)
{
	if (Base == nullptr)
		return nullptr;

	auto Material = UMaterialInstanceDynamic::Create(Base, &Owner);
	const FGuid Guid = MaterialBarrier.Guid;
	const FString WantedName = MaterialBarrier.Name.IsNone()
							 ? FString::Printf(TEXT("MI_RenderMaterial_%s"), *Guid.ToString())
							 : FString::Printf(TEXT("MI_%s"), *MaterialBarrier.Name.ToString());
	const FString Name = FAGX_ObjectUtilities::SanitizeAndMakeNameUnique(
		&Owner, WantedName, UMaterialInterface::StaticClass());

	Material->Rename(*Name);

	auto SetVector = [&Material](const TCHAR* Name, const FVector4& Value) {
		Material->SetVectorParameterValue(FName(Name), FAGX_RenderMaterial::ConvertToLinear(Value));
	};

	auto SetScalar = [&Material](const TCHAR* Name, float Value)
	{ Material->SetScalarParameterValue(FName(Name), Value); };

	Material->ClearParameterValues();
	if (MaterialBarrier.bHasDiffuse)
	{
		SetVector(TEXT("Diffuse"), MaterialBarrier.Diffuse);
	}
	if (MaterialBarrier.bHasAmbient)
	{
		SetVector(TEXT("Ambient"), MaterialBarrier.Ambient);
	}
	if (MaterialBarrier.bHasEmissive)
	{
		SetVector(TEXT("Emissive"), MaterialBarrier.Emissive);
	}
	if (MaterialBarrier.bHasShininess)
	{
		SetScalar(TEXT("Shininess"), MaterialBarrier.Shininess);
	}

	return Material;
}

UMaterial* AGX_MeshUtilities::GetDefaultRenderMaterial(bool bIsSensor)
{
	const TCHAR* AssetPath =
		bIsSensor ? TEXT("Material'/AGXUnreal/Runtime/Materials/M_SensorMaterial.M_SensorMaterial'")
				  : TEXT("Material'/AGXUnreal/Runtime/Materials/M_ImportedBase.M_ImportedBase'");
	UMaterial* Material = FAGX_ObjectUtilities::GetAssetFromPath<UMaterial>(AssetPath);

	if (Material == nullptr)
	{
		UE_LOG(
			LogAGX, Warning, TEXT("Could not load default%s render material from '%s'."),
			(bIsSensor ? TEXT(" sensor") : TEXT("")), AssetPath);
	}
	return Material;
}

bool AGX_MeshUtilities::AddBoxSimpleCollision(const FBox& BoundingBox, UStaticMesh& OutStaticMesh)
{
	UBodySetup* BodySetup = OutStaticMesh.GetBodySetup();
	if (!BodySetup)
		return false;

	BodySetup->InvalidatePhysicsData();

	FKBoxElem BoxElem;
	BoxElem.Center = BoundingBox.GetCenter();
	const FVector Extents = BoundingBox.GetExtent();
	BoxElem.X = Extents.X * 2.0f;
	BoxElem.Y = Extents.Y * 2.0f;
	BoxElem.Z = Extents.Z * 2.0f;
	BodySetup->AggGeom.BoxElems.Add(BoxElem);

	BodySetup->CreatePhysicsMeshes();
	return true;
}

bool AGX_MeshUtilities::AreStaticMeshesEqual(UStaticMesh* MeshA, UStaticMesh* MeshB)
{
	if (!MeshA || !MeshB)
		return false;

	if (MeshA == MeshB)
		return true;

	if (MeshA->GetMaterial(0) != nullptr || MeshB->GetMaterial(0) != nullptr)
	{
		if (!AreImportedRenderMaterialsEqual(MeshA->GetMaterial(0), MeshB->GetMaterial(0)))
			return false;
	}

	const bool AHasRenderData = MeshA->GetRenderData() != nullptr;
	const bool BHasRenderData = MeshB->GetRenderData() != nullptr;
	if (AHasRenderData != BHasRenderData)
		return false;

	if (MeshA->GetRenderData()->LODResources.Num() != MeshB->GetRenderData()->LODResources.Num())
		return false;

	for (int32 LODIndex = 0; LODIndex < MeshA->GetRenderData()->LODResources.Num(); ++LODIndex)
	{
		const FStaticMeshLODResources& LOD_A = MeshA->GetRenderData()->LODResources[LODIndex];
		const FStaticMeshLODResources& LOD_B = MeshB->GetRenderData()->LODResources[LODIndex];

		if (LOD_A.VertexBuffers.PositionVertexBuffer.GetNumVertices() !=
			LOD_B.VertexBuffers.PositionVertexBuffer.GetNumVertices())
		{
			return false;
		}

		for (uint32 i = 0; i < LOD_A.VertexBuffers.PositionVertexBuffer.GetNumVertices(); i++)
		{
			FVector3f PosA = LOD_A.VertexBuffers.PositionVertexBuffer.VertexPosition(i);
			FVector3f PosB = LOD_B.VertexBuffers.PositionVertexBuffer.VertexPosition(i);

			if (!PosA.Equals(PosB, UE_KINDA_SMALL_NUMBER))
				return false;
		}

		if (LOD_A.IndexBuffer.GetNumIndices() != LOD_B.IndexBuffer.GetNumIndices())
			return false;
	}

	return true;
}

bool AGX_MeshUtilities::AreImportedRenderMaterialsEqual(
	UMaterialInterface* MatA, UMaterialInterface* MatB)
{
	if (!MatA || !MatB)
		return false;

	if (MatA == MatB)
		return true;

	UMaterial* BaseMatA = MatA->GetBaseMaterial();
	UMaterial* BaseMatB = MatB->GetBaseMaterial();
	if (BaseMatA != BaseMatB)
		return false;

	TArray<FMaterialParameterInfo> ScalarParamsA, ScalarParamsB;
	TArray<FGuid> ScalarIdsA, ScalarIdsB;
	MatA->GetAllScalarParameterInfo(ScalarParamsA, ScalarIdsA);
	MatB->GetAllScalarParameterInfo(ScalarParamsB, ScalarIdsB);

	if (ScalarParamsA.Num() != ScalarParamsB.Num())
		return false;

	for (int32 i = 0; i < ScalarParamsA.Num(); i++)
	{
		float ValueA, ValueB;
		if (!MatA->GetScalarParameterValue(ScalarParamsA[i], ValueA) ||
			!MatB->GetScalarParameterValue(ScalarParamsB[i], ValueB))
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Unable to read scalar parameter '%s' in AreImportedRenderMaterialsEqual for "
					 "one of the Render Materials '%s' or '%s'."),
				*ScalarParamsA[i].ToString(), *MatA->GetName(), *MatB->GetName());
			return false;
		}

		if (!FMath::IsNearlyEqual(ValueA, ValueB, UE_KINDA_SMALL_NUMBER))
			return false;
	}

	TArray<FMaterialParameterInfo> VectorParamsA, VectorParamsB;
	TArray<FGuid> VectorIdsA, VectorIdsB;
	MatA->GetAllVectorParameterInfo(VectorParamsA, VectorIdsA);
	MatB->GetAllVectorParameterInfo(VectorParamsB, VectorIdsB);

	if (VectorParamsA.Num() != VectorParamsB.Num())
		return false;

	for (int32 i = 0; i < VectorParamsA.Num(); i++)
	{
		FLinearColor ColorA, ColorB;
		if (!MatA->GetVectorParameterValue(VectorParamsA[i], ColorA) ||
			!MatB->GetVectorParameterValue(VectorParamsB[i], ColorB))
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Unable to read Vector parameter '%s' in AreImportedRenderMaterialsEqual for "
					 "one of the Render Materials '%s' or '%s'."),
				*VectorParamsA[i].ToString(), *MatA->GetName(), *MatB->GetName());
			return false; // Could not retrieve vector value.
		}

		if (!ColorA.Equals(ColorB, UE_KINDA_SMALL_NUMBER))
			return false;
	}

	return true;
}
