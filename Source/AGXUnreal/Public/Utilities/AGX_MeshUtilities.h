// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_MeshWithTransform.h"
#include "AGX_UE4Compatibility.h"
#include "Shapes/AGX_ShapeEnums.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Engine/HitResult.h"
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
struct FShapeBarrier;
class UMaterial;
class UMaterialInterface;
class UStaticMesh;
class UStaticMeshComponent;

struct FAGX_RenderMaterial;
struct FAGX_SimpleMeshTriangle;
struct FTrimeshShapeBarrier;

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
		
	static bool LineTraceMesh(
		FHitResult& OutHit, FVector Start, FVector Stop, FTransform Transform,
		const TArray<FVector>& Vertices, const TArray<FTriIndices>& Indices);

	template <typename VectorType, typename IndexType>
	static bool LineTraceMesh(
		FHitResult& OutHit, FVector Start, FVector Stop, FTransform Transform,
		const TArray<VectorType>& Vertices, const TArrayView<const IndexType> Indices);

	/**
	 * Low-level mesh creation function that operates on raw mesh vertex attribute arrays. Called by
	 * the higher-level mesh creation functions that take a Trimesh or a Render Data and builds the
	 * vertex attribute arrays from that.
	 *
	 * Creates a UStaticMesh from attribute data. The attribute data must conform to Unreal's
	 * attribute layout. Unreal splits the attributes into per-vertex data and per-vertex-instance
	 * data. There are three vertex instances per triangle and each vertex instance references one
	 * of the per-vertex datum.
	 *
	 * Attributes:
	 * - Positions: One per vertex.
	 * - Indices: Three per triangle, one per vertex instance.
	 * - Normals: One per vertex instance. Optional.
	 * - UVs: One per vertex instance. Optional. Often called "texture coordinates".
	 * - Tangents: One per vertex instance. Optional.
	 *
	 * Optional attributes means that the array may be empty.
	 *
	 * Normals are optional, in which case Unreal will compute smooth normals for each vertex
	 * instance. Or share the same smoothed normal among multiple vertex instances by storing it
	 * per-vertex instead, I'm not sure how that works behind the scenes.
	 *
	 * UVs, a.k.a. texture coordinates, are optional but are not computed by Unreal. Expect
	 * rendering artifacts if the mesh has no UVs.
	 *
	 * Tangents are optional and computed by Unreal if not provided.
	 *
	 * It is the responsibility of the caller to ensure that the name is valid and does not conflict
	 * with anything in the same outer.
	 *
	 * Delayed builds, i.e. passing false to bInBuild, is only allowed from WITH_EDITOR builds.
	 * Static Meshes created this way must be built elsewhere before they can be used.
	 *
	 * Box collision creation is only supported when the Static Mesh is built immediately, i.e.
	 * only when bInBuild is true.
	 *
	 * This function only creates the in-memory representation. The created Static Mesh will not be
	 * written to drive.
	 *
	 * @param InPositions The positions of the vertices.
	 * @param InIndices Indices into 'Positions', each three of which define a triangle.
	 * @param InNormals One normal per index, or empty to let Unreal compute the normals.
	 * @param InUVs One UV per index, or empty. Will not be computed by Unreal.
	 * @param InTangents One tangent per index, or empty to let Unreal compute the tangents.
	 * @param InName The name to give to the Static Mesh.
	 * @param InOuter The UObject that will own the Static Mesh.
	 * @param InMaterial Render material to assign to all triangles.
	 * @param bInBuild True to build the Static Mesh immediately. Required in non-editor builds.
	 * @param bInWithBoxCollision If bInBuild is true, whether to also create a box collision.
	 * @return The UStaticMesh created from the given mesh data.
	 */
	static UStaticMesh* CreateStaticMesh(
		const TArray<FVector3f>& InPositions, const TArray<uint32>& InIndices,
		const TArray<FVector3f>& InNormals, const TArray<FVector2f>& InUVs,
		const TArray<FVector3f>& InTangents, const FString& InName, UObject& InOuter,
		UMaterialInterface* InMaterial, bool bInBuild, bool bInWithBoxCollision);

	/**
	 * Control how normals are assigned for a mesh.
	 */
	enum class EAGX_NormalsSource
	{
		/// Let Unreal recompute smooth averaged per-vertex normals. The normals provided with the
		/// source data, if any, are ignored.
		Generated,

		/// Use normals provided with the source data. The source data must contain normals either
		/// per-vertex, per-vertex-instance, or per-triangle.
		FromImport,

		/// Use imported normals if the number of imported normals match either the number of
		/// vertices, vertex instances, or triangles. Otherwise, fall back to letting Unreal
		/// generate the normals without warning.
		Auto
	};

	/**
	 * Create a Static Mesh from the triangle held by the given Trimesh.
	 *
	 * Extracts the mesh data from the Trimesh and then calls the CreateStaticMesh overload that
	 * takes a bunch of TArrays.
	 *
	 * AGX Dynamics store one normal per triangle, not per vertex instance as Unreal does. There is
	 * an option whether the per-triangle normals should be used, which results in flat-shaded
	 * faceted triangles, or if we should disregard the per-triangle normals and let Unreal compute
	 * smoothed normals based on the triangle definition. The selection is controlled with the
	 * InNormalsSource parameter.
	 *
	 * Delayed builds, i.e. passing false to bInBuild, is only allowed from WITH_EDITOR builds.
	 * Static Meshes created this way must be built elsewhere before they can be used.
	 *
	 * Box collision creation is only supported when the Static Mesh is built immediately, i.e.
	 * only when bInBuild is true.
	 *
	 * This function only creates the in-memory representation. The created Static Mesh will not be
	 * written to drive.
	 *
	 * If the given name is empty then the Trimesh's source name will be used with the standard
	 * 'SM_' prefix. If the Trimesh doesn't have a source name then the mesh will be named
	 * 'SM_CollisionMesh_<GUID>'.
	 *
	 * @param InTrimeshBarrier The Trimesh to create a Static Mesh from.
	 * @param InOuter The UObject that will own the Static Mesh.
	 * @param InMaterial Render material to assign to all triangles.
	 * @param bInBuild True to build the Static Mesh immediately. Required in non-editor builds.
	 * @param bInWithBoxCollision If bInBuild is true, whether to also create a box collision.
	 * @param InNormalsSource Where to get normals from, either triangles or let Unreal generate.
	 * @param InName The name of the Static Mesh.
	 * @return The UStaticMesh created from the Trimesh.
	 */
	static UStaticMesh* CreateStaticMesh(
		const FTrimeshShapeBarrier& InTrimeshBarrier, UObject& InOuter,
		UMaterialInterface* InMaterial, bool bInBuild, bool bInWithBoxCollision,
		EAGX_NormalsSource InNormalsSource, const FString& InName = TEXT(""));

	/**
	 * Create a Static Mesh from the triangles held by the given Render Data.
	 *
	 * Extracts the mesh data from the Render Data and then calls the CreateStaticMesh overload that
	 * takes a bunch of TArrays.
	 *
	 * AGX Dynamics does not enforce a particular mesh attributes layout so it is not well-defined
	 * how the data we find in the Render Data should be mapped to the attributes of the Static
	 * Mesh. This function employs a count based heuristic where attribute elements are mapped
	 * depending on how may elements there are for each attribute. There is not always a valid
	 * mapping, in which case a warning is printed and no Static Mesh is created. There may be valid
	 * layouts that are not yet detected and handled by this function.
	 *
	 * Delayed builds, i.e. passing false to bInBuild, is only allowed from WITH_EDITOR builds.
	 * Static Meshes created this way must be built elsewhere before they can be used.
	 *
	 * Box collision creation is only supported when the Static Mesh is built immediately, i.e.
	 * only when bInBuild is true.
	 *
	 * This function only creates the in-memory representation. The created Static Mesh will not be
	 * written to drive.
	 *
	 * If the given name is empty then the created Static Mesh is named 'SM_RenderMesh_<GUID>'.
	 *
	 * @param InRenderDataBarrier The Render Data to create a Static Mesh from.
	 * @param InOuter The UObject that will own the Static Mesh.
	 * @param InMaterial Render material to assign to all triangles.
	 * @param bInBuild True to build the Static Mesh immediately. Required in non-editor builds.
	 * @param bInWithBoxCollision If bInBuild is true, whether to also create a box collision.
	 * @param InNormalsSource Where to get normals from, either Render Data or let Unreal compute.
	 * @param InName The name of the Static Mesh. If empty a GUID-based name is generated.
	 * @return The UStaticMesh created from the Render Data.
	 */
	static UStaticMesh* CreateStaticMesh(
		const FRenderDataBarrier& InRenderDataBarrier, UObject& InOuter,
		UMaterialInterface* InMaterial, bool bInBuild, bool bInWithBoxCollision,
		EAGX_NormalsSource InNormalsSource, const FString& InName = TEXT(""));

	/**
	 * Copies triangle information and render material from one Static Mesh to another.
	 * Does not copy other properties.
	 * Always generates a new SimpleCollision (Box primitive) to the Destination Static Mesh.
	 */
	static bool CopyStaticMesh(UStaticMesh* Source, UStaticMesh* Destination);

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
	static bool AddBoxSimpleCollision(UStaticMesh& OutStaticMesh);

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

template <typename VectorType, typename IndexType>
inline bool AGX_MeshUtilities::LineTraceMesh(
	FHitResult& OutHit, FVector Start, FVector Stop, FTransform Transform,
	const TArray<VectorType>& Vertices, const TArrayView<const IndexType> Indices)
{
	VectorType LocalStart = VectorType(Transform.InverseTransformPosition(Start));
	VectorType LocalStop = VectorType(Transform.InverseTransformPosition(Stop));

	float ClosestDistance = std::numeric_limits<float>::max();
	bool HitFound = false;

	// Direction vector from Start to Stop
	VectorType LocalDirection = LocalStop - LocalStart;
	float LineLength = LocalDirection.Length();
	LocalDirection.Normalize();

	// Iterate through each triangle in the mesh
	for (int32 i = 0; i < Indices.Num(); i += 3)
	{
		// Get the vertices of the current triangle
		VectorType V0 = Vertices[Indices[i + 0]];
		VectorType V1 = Vertices[Indices[i + 1]];
		VectorType V2 = Vertices[Indices[i + 2]];

		// Ray-Triangle intersection test (Moller-Trumbore algorithm)
		VectorType Edge1 = V1 - V0;
		VectorType Edge2 = V2 - V0;
		VectorType Pvec = VectorType::CrossProduct(LocalDirection, Edge2);
		float Det = VectorType::DotProduct(Edge1, Pvec);

		// If the determinant is near zero, the ray lies in the plane of the triangle
		if (FMath::Abs(Det) < KINDA_SMALL_NUMBER)
		{
			continue;
		}

		float InvDet = 1.0f / Det;
		VectorType Tvec = LocalStart - V0;
		float U = VectorType::DotProduct(Tvec, Pvec) * InvDet;

		// Check if the intersection lies within the triangle
		if (U < 0.0f || U > 1.0f)
		{
			continue;
		}

		VectorType Qvec = VectorType::CrossProduct(Tvec, Edge1);
		float V = VectorType::DotProduct(LocalDirection, Qvec) * InvDet;
		if (V < 0.0f || U + V > 1.0f)
		{
			continue;
		}

		// Calculate the distance along the ray to the intersection point
		float Distance = VectorType::DotProduct(Edge2, Qvec) * InvDet;
		if (Distance > 0.0f && Distance <= LineLength)
		{
			if (Distance < ClosestDistance)
			{
				ClosestDistance = Distance;

				FVector Location =
					Transform.TransformPosition(FVector(LocalStart + LocalDirection * Distance));
				OutHit.Distance = (Start - Location).Length();
				OutHit.Location = Location;
				HitFound = true;
			}
		}
	}

	return HitFound;
}

