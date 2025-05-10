// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_MeshWithTransform.h"
#include "AGX_UE4Compatibility.h"
#include "Shapes/AGX_ShapeEnums.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Misc/EngineVersionComparison.h"
#if !UE_VERSION_OLDER_THAN(5, 2, 0)
// Possible include loop in Unreal Engine.
// - DynamicsMeshBuilder
// - RenderUtils.h
// - RHIShaderPlatform.h
//     Defines FStaticShaderPlatform, but includes RHIDefinitions.h first.
// - RHIDefinitions.h
// - DataDrivenShaderPlatformInfo.h
//   Needs FStaticShaderPlatform so includes RHIShaderPlatform.h. But that file is already being
//   included so ignored. So FStaticShaderPlatform will be defined soon, but it isn't yet. So
//   the compile fails.
//
// We work around this by including DataDrivenShaderPlatformInfo.h ourselves before all of the
// above. Now DataDrivenShaderPlatformInfo.h can include RHIShaderPlatform.h succesfully and
// FStaticShaderPlatform is defined when DataDrivenShaderPlatformInfo.h needs it. When we include
// DynamicMeshBuild.h shortly most of the include files are skipped because they have already been
// included as part of DataDrivenShaderPlatformInfo.h here.
#include "DataDrivenShaderPlatformInfo.h"
#endif
#include "DynamicMeshBuilder.h"
#include "Interface_CollisionDataProviderCore.h"

class AStaticMeshActor;
class FDynamicMeshIndexBuffer32;
class FRenderDataBarrier;
class FShapeBarrier;
class UMaterial;
class UMaterialInterface;
class UStaticMesh;
class UStaticMeshComponent;

struct FAGX_RenderMaterial;
struct FAGX_SimpleMeshTriangle;
struct FStaticMeshVertexBuffers;

/// \todo Each nested ***ConstructionData classes below could contain the respective Make-function
/// as a member function, to even furter reduce potential usage mistakes!

/**
 * Provides helper functions for creating custom Unreal Meshes.
 */
class AGXUNREAL_API AGX_MeshUtilities
{
public:
	static void MakeCube(
		TArray<FVector3f>& Positions, TArray<FVector3f>& Normals, TArray<uint32>& Indices,
		TArray<FVector2f>& TexCoords, const FVector3f& HalfSize);

	/**
	 * Used to define the geometry of a mesh sphere, and also to know the number of vertices and
	 * indices in advance.
	 */
	struct AGXUNREAL_API SphereConstructionData
	{
		// Input:
		const float Radius;
		const float Segments;

		// Derived:
		const uint32 Stacks;
		const uint32 Sectors;
		const uint32 Vertices;
		const uint32 Indices;

		SphereConstructionData(float InRadius, uint32 InNumSegments);

		void AppendBufferSizes(uint32& InOutNumVertices, uint32& InOutNumIndices) const;
	};

	/// \todo Change to use SphereConstructionData as input.
	static void MakeSphere(
		TArray<FVector3f>& Positions, TArray<FVector3f>& Normals, TArray<uint32>& Indices,
		TArray<FVector2f>& TexCoords, float Radius, uint32 NumSegments);

	/**
	 * Appends buffers with geometry data for a sphere, centered at origin.
	 *
	 * Buffers will not be resized, and must therefore already have enough space to contain the data
	 * to be written. Use CylinderConstructionData.AppendBufferSizes to calculate how much data to
	 * allocate in advance.
	 *
	 * Will start writing from NextFreeVertex and NextFreeIndex, and update them before returning
	 * such that they point to one past the last added vertex and index.
	 */
	static void MakeSphere(
		FStaticMeshVertexBuffers& VertexBuffers, FDynamicMeshIndexBuffer32& IndexBuffer,
		uint32& NextFreeVertex, uint32& NextFreeIndex,
		const SphereConstructionData& ConstructionData);

	/**
	 * Used to define the geometry of a mesh cylinder, and also to know the number of vertices and
	 * indices in advance.
	 */
	struct AGXUNREAL_API CylinderConstructionData
	{
		// Input:
		const float Radius;
		const float Height;
		const uint32 CircleSegments;
		const uint32 HeightSegments;
		const FLinearColor& MiddleColor;
		const FLinearColor& OuterColor;

		// Derived:
		const uint32 VertexRows;
		const uint32 VertexColumns;
		const uint32 Caps;
		const uint32 VertexRowsAndCaps;
		const uint32 Vertices;
		const uint32 Indices;

		CylinderConstructionData(
			float InRadius, float InHeight, uint32 InNumCircleSegments, uint32 InNumHeightSegments,
			const FLinearColor& InMiddleColor = FLinearColor(1, 1, 1, 1),
			const FLinearColor& InOuterColor = FLinearColor(1, 1, 1, 1));

		void AppendBufferSizes(uint32& InOutNumVertices, uint32& InOutNumIndices) const;
	};

	/**
	 * Initializes buffers with geometry data for a cylinder extending uniformly along the Y-Axis,
	 * centered at origin.
	 */
	static void MakeCylinder(
		TArray<FVector3f>& Positions, TArray<FVector3f>& Normals, TArray<uint32>& Indices,
		TArray<FVector2f>& TexCoords, const CylinderConstructionData& ConstructionData);

	/**
	 * Appends buffers with geometry data for a cylinder extending uniformly along the Z-Axis,
	 * centered at origin.
	 *
	 * Buffers will not be resized, and must therefore already have enough space to contain the data
	 * to be written. Use CylinderConstructionData.AppendBufferSizes to calculate how much data to
	 * allocate in advance.
	 *
	 * Will start writing from NextFreeVertex and NextFreeIndex, and update them before returning
	 * such that they point to one past the last added vertex and index.
	 */
	static void MakeCylinder(
		FStaticMeshVertexBuffers& VertexBuffers, FDynamicMeshIndexBuffer32& IndexBuffer,
		uint32& NextFreeVertex, uint32& NextFreeIndex,
		const CylinderConstructionData& ConstructionData);

	static void MakeCylinder(
		const FVector3f& Base, const FVector3f& XAxis, const FVector3f& YAxis,
		const FVector3f& ZAxis, float Radius, float HalfHeight, uint32 Sides,
		TArray<FDynamicMeshVertex>& OutVerts, TArray<uint32>& OutIndices);

	/**
	 * Used to define the geometry of a mesh capsule, and also to know the number of vertices and
	 * indices in advance.
	 */
	struct AGXUNREAL_API CapsuleConstructionData
	{
		// Input:
		const float Radius;
		const float Height;
		const uint32 CircleSegments;
		const uint32 HeightSegments;

		CapsuleConstructionData(
			float InRadius, float InHeight, uint32 InNumCircleSegments, uint32 InNumHeightSegments);
	};

	/**
	 * Initializes buffers with geometry data for a capsule extending uniformly along the Y-Axis,
	 * centered at origin.
	 */
	static void MakeCapsule(
		TArray<FVector3f>& Positions, TArray<FVector3f>& Normals, TArray<uint32>& Indices,
		TArray<FVector2f>& TexCoords, const CapsuleConstructionData& Data);

	static void MakeCone(
		float Angle1, float Angle2, float Scale, float XOffset, uint32 NumSides,
		TArray<FDynamicMeshVertex>& OutVerts, TArray<uint32>& OutIndices);

	/**
	 * Used to define the geometry of a mesh arrow, and also to know the number of vertices and
	 * indices in advance.
	 */
	struct CylindricalArrowConstructionData
	{
		// Input:
		const float CylinderRadius;
		const float CylinderHeight;
		const float ConeRadius;
		const float ConeHeight;
		const bool bBottomCap;
		const uint32 CircleSegments;
		const FLinearColor BaseColor;
		const FLinearColor TopColor;

		// Derived:
		const uint32 VertexRows;
		const uint32 VertexColumns;
		const uint32 Vertices;
		const uint32 Indices;

		CylindricalArrowConstructionData(
			float InCylinderRadius, float InCylinderHeight, float InConeRadius, float InConeHeight,
			bool bInBottomCap, uint32 InNumCircleSegments, const FLinearColor& InBaseColor,
			const FLinearColor& InTopColor);

		void AppendBufferSizes(uint32& InOutNumVertices, uint32& InOutNumIndices) const;
	};

	/**
	 * Appends buffers with geometry data for a cylinder extending uniformly along the Z-Axis,
	 * centered at origin.
	 *
	 * Buffers will not be resized, and must therefore already have enough space to contain the data
	 * to be written. Use CylindricalArrowConstructionData.AppendBufferSizes to calculate how much
	 * data to allocate in advance.
	 *
	 * Will start writing from NextFreeVertex and NextFreeIndex, and update them before returning
	 * such that they point to one past the last added vertex and index.
	 */
	static void MakeCylindricalArrow(
		FStaticMeshVertexBuffers& VertexBuffers, FDynamicMeshIndexBuffer32& IndexBuffer,
		uint32& NextFreeVertex, uint32& NextFreeIndex,
		const CylindricalArrowConstructionData& ConstructionData);

	/**
	 * Used to define the geometry of a flat bendable arrow, and also to know the number of vertices
	 * and indices in advance.
	 */
	struct BendableArrowConstructionData
	{
		// Input:
		const float RectangleWidth;
		const float RectangleLength;
		const float TriangleWidth;
		const float TriangleLength;
		const float BendAngle; // Radians of a circle the arrow will bend along. Zero means no bend.
		const uint32 Segments;
		const FLinearColor BaseColor;
		const FLinearColor TopColor;

		// Derived:
		const uint32 RectangleSegments;
		const uint32 TriangleSegments;
		const uint32 RectangleVertexRows;
		const uint32 TriangleVertexRows;
		const uint32 Vertices;
		const uint32 Indices;

		BendableArrowConstructionData(
			float InRectangleWidth, float InRectangleLength, float InTriangleWidth,
			float InTriangleLength, float InBendAngle, uint32 InNumSegments,
			const FLinearColor& InBaseColor, const FLinearColor& InTopColor);

		void AppendBufferSizes(uint32& InOutNumVertices, uint32& InOutNumIndices) const;
	};

	/**
	 * Appends buffers with geometry data for a flat bendable arrow, extending initially along the
	 * Z-Axis, centered at origin, and bending counter clockwise arond the Y-Axis.
	 *
	 * Buffers will not be resized, and must therefore already have enough space to contain the data
	 * to be written. Use CylindricalArrowConstructionData.AppendBufferSizes to calculate how much
	 * data to allocate in advance.
	 *
	 * Will start writing from NextFreeVertex and NextFreeIndex, and update them before returning
	 * such that they point to one past the last added vertex and index.
	 */
	static void MakeBendableArrow(
		FStaticMeshVertexBuffers& VertexBuffers, FDynamicMeshIndexBuffer32& IndexBuffer,
		uint32& NextFreeVertex, uint32& NextFreeIndex,
		const BendableArrowConstructionData& ConstructionData);

	static void PrintMeshToLog(
		const FStaticMeshVertexBuffers& VertexBuffers,
		const FDynamicMeshIndexBuffer32& IndexBuffer);

	/**
	 * Used to define the geometry of a mesh cylinder, and also to know the number of vertices and
	 * indices in advance.
	 */
	struct DiskArrayConstructionData
	{
		// Input:
		const float Radius;
		const uint32 CircleSegments;
		const float Spacing;
		const uint32 Disks;
		const bool bTwoSided;
		const FLinearColor MiddleDiskColor;
		const FLinearColor OuterDiskColor;
		TArray<FTransform3f> SpacingsOverride; // optional, Spacing will be ignored if defined. Zero
											   // or num Disks items.

		// Derived:
		const uint32 SidesPerDisk;
		const uint32 VerticesPerSide;
		const uint32 VerticesPerDisk;
		const uint32 Vertices;
		const uint32 Indices;

		DiskArrayConstructionData(
			float InRadius, uint32 InNumCircleSegments, float InSpacing, uint32 InDisks,
			bool bInTwoSided, const FLinearColor InMiddleDiskColor,
			const FLinearColor InOuterDiskColor, TArray<FTransform3f> InSpacingsOverride = {});

		void AppendBufferSizes(uint32& InOutNumVertices, uint32& InOutNumIndices) const;
	};

	static void MakeDiskArray(
		FStaticMeshVertexBuffers& VertexBuffers, FDynamicMeshIndexBuffer32& IndexBuffer,
		uint32& NextFreeVertex, uint32& NextFreeIndex, const DiskArrayConstructionData& Data);

	static FAGX_MeshWithTransform FindFirstChildMesh(const USceneComponent& Component);
	static TArray<FAGX_MeshWithTransform> FindChildrenMeshes(
		const USceneComponent& Component, bool SearchRecursive);
	static TArray<UStaticMeshComponent*> FindChildrenMeshComponents(
		const USceneComponent& Component, bool SearchRecursive);
	static FAGX_MeshWithTransform FindFirstParentMesh(const USceneComponent& Component);

	/**
	 * Uses data from the Static Mesh to construct a simplified
	 * vertex and index buffer. The simplification is mainly due to the fact that
	 * the source render mesh might need multiple vertices with same position but
	 * different normals, texture coordinates, etc, while the collision mesh can
	 * share vertices between triangles more aggressively.
	 *
	 * The vertex positions are given in double precision because that is what AGX Dynamics expects.
	 */
	static bool GetStaticMeshCollisionData(
		const FAGX_MeshWithTransform& InMesh, const FTransform& RelativeTo,
		TArray<FVector>& OutVertices, TArray<FTriIndices>& OutIndices,
		const uint32* LodIndexOverride = nullptr);

	static TArray<FAGX_MeshWithTransform> ToMeshWithTransformArray(
		const TArray<AStaticMeshActor*> Actors);

	/**
	 * Creates and builds a new static mesh.
	 * This function supports runtime usage.
	 */
	static UStaticMesh* CreateStaticMesh(
		const TArray<FVector3f>& Vertices, const TArray<uint32>& Triangles,
		const TArray<FVector3f>& Normals, const TArray<FVector2D>& UVs,
		const TArray<FVector3f>& Tangents, const FString& Name, UObject& Outer,
		UMaterialInterface* Material);

	/**
	 * Copies triangle information and render material from one Static Mesh to another.
	 * Does not copy other properties.
	 */
	static bool CopyStaticMesh(UStaticMesh* Source, UStaticMesh* Destination);

	/**
	 * Creates and builds a new static mesh.
	 * This function supports runtime usage.
	 */
	static UStaticMesh* CreateStaticMesh(
		const FRenderDataBarrier& RenderData, UObject& Outer, UMaterialInterface* Material);

	static bool HasRenderDataMesh(const FShapeBarrier& Shape);

	/**
	 * Creates a new render Material instance based on the given RenderMaterial Barrier and Base.
	 * If Base is nullptr, this functions returns nullptr.
	 * This function supports runtime usage.
	 */
	static UMaterialInterface* CreateRenderMaterial(
		const FAGX_RenderMaterial& MaterialBarrier, UMaterial* Base, UObject& Owner);

	/**
	 * Returns the default (AGX) render material.
	 */
	static UMaterial* GetDefaultRenderMaterial(bool bIsSensor);

	/**
	 * Add a Simple Collision Box to the given StaticMesh.
	 */
	static bool AddBoxSimpleCollision(const FBox& BoundingBox, UStaticMesh& OutStaticMesh);

	/**
	 * Simple comparison to test if two meshes are equal.
	 * Does not test all possible data, but does vertex and RenderMaterial comparisons.
	 */
	static bool AreStaticMeshesEqual(UStaticMesh* MeshA, UStaticMesh* MeshB);

	/**
	 * Checks whether two Render Materials are equal.
	 */
	static bool AreImportedRenderMaterialsEqual(UMaterialInterface* MatA, UMaterialInterface* MatB);
};
