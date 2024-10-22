
#include "Terrain/AGX_TerrainMeshUtilities.h"

#include <KismetProceduralMeshLibrary.h>

TSharedPtr<HfMeshDescription> UAGX_TerrainMeshUtilities::CreateMeshDescription(
	const FVector& Center, const FVector2D& Size, const FIntVector2& Resolution, double UvScale,
	std::function<float(const FVector&)> HeightFunction, bool IsUseSkirt)
{
	// Vertex count with or without skirt
	FIntVector2 NrOfVerts;
	if (IsUseSkirt)
		NrOfVerts = FIntVector2(Resolution.X + 3, Resolution.Y + 3);
	else
		NrOfVerts = FIntVector2(Resolution.X + 1, Resolution.Y + 1);

	// Create pointer
	auto MeshDescPtr = MakeShared<HfMeshDescription>(NrOfVerts);
	auto& MeshDesc = *MeshDescPtr;

	// Populate vertices, uvs, colors
	GenerateTriangles(MeshDesc, Center, Size, Resolution, UvScale, HeightFunction, IsUseSkirt);

	// Compute tangents and normals, while skirt is still being flat
	UKismetProceduralMeshLibrary::CalculateTangentsForMesh(
		MeshDesc.Vertices, MeshDesc.Triangles, MeshDesc.UV0, MeshDesc.Normals, MeshDesc.Tangents);

	// Move skirt vertices downwards
	if (IsUseSkirt)
	{
		FVector SkirtOffset = FVector::UpVector * Size.Length() * 0.05f;
		int VertexIndex = 0;
		for (int32 y = 0; y < NrOfVerts.Y; ++y)
		{
			for (int32 x = 0; x < NrOfVerts.X; ++x)
			{
				if (x == 0 || x == NrOfVerts.X - 1 || y == 0 || y == NrOfVerts.Y - 1)
				{
					MeshDesc.Vertices[VertexIndex] -= SkirtOffset;
				}
				VertexIndex++;
			}
		}
	}

	return MeshDescPtr;
}

void UAGX_TerrainMeshUtilities::GenerateTriangles(
	HfMeshDescription& MeshDesc, const FVector& Center, const FVector2D& Size,
	const FIntVector2& Resolution, double UvScale,
	std::function<float(const FVector&)> HeightFunction, bool IsUseSkirt)
{
	// Vertex count with or without skirt
	FIntVector2 NrOfVerts;
	if (IsUseSkirt)
		NrOfVerts = FIntVector2(Resolution.X + 3, Resolution.Y + 3);
	else
		NrOfVerts = FIntVector2(Resolution.X + 1, Resolution.Y + 1);

	FVector2D TriangleSize = FVector2D(Size.X / Resolution.X, Size.Y / Resolution.Y);

	// Fill the vertices and triangles
	int StartOffset = IsUseSkirt ? -1 : 0;
	int32 VertexIndex = 0;
	int32 TriangleIndex = 0;
	for (int32 y = 0; y < NrOfVerts.Y; ++y)
	{
		for (int32 x = 0; x < NrOfVerts.X; ++x)
		{
			FVector PlanePosition = FVector(
				(x + StartOffset) * TriangleSize.X - Size.X / 2,
				(y + StartOffset) * TriangleSize.Y - Size.Y / 2, 0.0f);
			FVector LocalPosition = PlanePosition + Center;

			// Call height function
			float Height = HeightFunction(LocalPosition);

			MeshDesc.Vertices[VertexIndex] = PlanePosition + FVector::UpVector * Height;
			MeshDesc.UV0[VertexIndex] =
				FVector2D(UvScale * LocalPosition.X, UvScale * LocalPosition.Y);

			// Skip last row and column for triangles
			if (x < NrOfVerts.X - 1 && y < NrOfVerts.Y - 1)
			{
				int32 BottomLeft = VertexIndex;
				int32 BottomRight = BottomLeft + 1;
				int32 TopLeft = BottomLeft + NrOfVerts.X;
				int32 TopRight = TopLeft + 1;

				// First triangle
				MeshDesc.Triangles[TriangleIndex++] = BottomLeft;
				MeshDesc.Triangles[TriangleIndex++] = TopLeft;
				MeshDesc.Triangles[TriangleIndex++] = TopRight;

				// Second triangle
				MeshDesc.Triangles[TriangleIndex++] = BottomLeft;
				MeshDesc.Triangles[TriangleIndex++] = TopRight;
				MeshDesc.Triangles[TriangleIndex++] = BottomRight;
			}

			++VertexIndex;
		}
	}
}

float UAGX_TerrainMeshUtilities::GetBrownianNoise(
	const FVector& Pos, int Octaves, float Scale, float Persistance, float Lacunarity, float Exp)
{
	float Amplitude = 1.0;
	float Frequency = 1.0;
	float Normalization = 0;
	float Total = 0;
	for (int o = 0; o < Octaves; o++)
	{
		float noiseValue = FMath::PerlinNoise3D(Pos * Frequency / Scale) * 0.5f + 0.5f;
		Total += noiseValue * Amplitude;
		Normalization += Amplitude;
		Amplitude *= Persistance;
		Frequency *= Lacunarity;
	}
	Total /= Normalization;
	return FMath::Pow(Total, Exp);
}

float UAGX_TerrainMeshUtilities::GetRaycastedHeight(
	const FVector& Pos, const TArray<UMeshComponent*>& MeshComponents, const FVector& Up,
	const float RayLength)
{
	float BedHeight = 0.0f;
	for (auto uMesh : MeshComponents)
	{
		FHitResult Hit;
		FCollisionQueryParams Params;
		FVector start = Pos + Up * RayLength;
		FVector stop = Pos;

		if (uMesh->LineTraceComponent(Hit, start, stop, Params))
		{
			BedHeight = FMath::Max(BedHeight, RayLength - Hit.Distance);
		}
	}

	return BedHeight;
}

float UAGX_TerrainMeshUtilities::SampleHeightArray(
	FVector2D UV, const TArray<float>& HeightArray, int Width, int Height)
{
	// Normalize UV coordinates to the range [0, 1]
	UV.X = FMath::Clamp(UV.X, 0.0f, 1.0f);
	UV.Y = FMath::Clamp(UV.Y, 0.0f, 1.0f);

	// Calculate the positions of the four surrounding texels
	double X = UV.X * (Width - 1);
	double Y = UV.Y * (Height - 1);

	int32 X1 = FMath::FloorToInt(X);
	int32 Y1 = FMath::FloorToInt(Y);
	int32 X2 = FMath::Clamp(X1 + 1, 0, Width - 1);
	int32 Y2 = FMath::Clamp(Y1 + 1, 0, Height - 1);

	double FracX = X - X1;
	double FracY = Y - Y1;

	auto GetValue = [&](int32 X, int32 Y) -> float { return HeightArray[Y * Width + X]; };

	float C00 = GetValue(X1, Y1);
	float C10 = GetValue(X2, Y1);
	float C01 = GetValue(X1, Y2);
	float C11 = GetValue(X2, Y2);

	// Bilinear interpolation
	return FMath::BiLerp(C00, C10, C01, C11, FracX, FracY);
}

FBox UAGX_TerrainMeshUtilities::CreateEncapsulatingBoundingBox(
	const TArray<UMeshComponent*>& Meshes, const FTransform& worldTransform)
{
	FBox EncapsulatingBoundingBox(EForceInit::ForceInit);
	for (auto* Mesh : Meshes)
	{
		if (Mesh)
		{
			FVector Origin, BoxExtent;
			BoxExtent = worldTransform.InverseTransformVector(Mesh->Bounds.BoxExtent);
			Origin = worldTransform.InverseTransformPosition(Mesh->Bounds.Origin);

			FBox ComponentBox(Origin - BoxExtent, Origin + BoxExtent);

			EncapsulatingBoundingBox += ComponentBox;
		}
	}
	return EncapsulatingBoundingBox;
}
