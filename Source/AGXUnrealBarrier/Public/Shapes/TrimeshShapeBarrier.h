// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Shapes/ShapeBarrier.h"

// Unreal Engine includes.
#include "Containers/Array.h"
#include "Math/Vector.h"

#include "TrimeshShapeBarrier.generated.h"

struct FTriIndices;

/**
 * A handle to an AGX Dynamics trimesh collision shape.
 *
 * An AGX Dynamics collision trimesh consists of a collection of triangles. Each triangle has three
 * vertices and a single normal. Vertex positions are frequently shared among several triangles so
 * the triangle doesn't store the vertex position but instead an index into a vertex position
 * buffer.
 *
 * VertexPositions: TArray<FVector> - One FVector per point in space where a vertex resides.
 * VertexIndices: TArray<uint32> - Three uint32s per triangle. Used to read from VertexPositions.
 * TriangleNormals: TArray<FVector> - One FVector per triangle.
 *
 * Data shared among triangles:
 *   positions: [Vec3, Vec3, Vec3, Vec3, Vec3, ... ]
 *
 * Data owned by each triangle:
 *               |  Triangle 0   |  Triangle 1   | ... |
 *   indices:    | int, int, int | int, int, int | ... |
 *   normals:    |     Vec3      |      Vec3     | ... |
 *
 * The mesh data is stored in a hidden Mesh Data class that can be shared between Trimeshes. Use
 * GetMeshDataGuid to determine if this is the case.
 *
 * As with any shape, a trimesh may contain render data. The render mesh, if any, is separate from
 * the collision mesh and uses a different storage format.
 */
USTRUCT(BlueprintType)
struct AGXUNREALBARRIER_API FTrimeshShapeBarrier : public FShapeBarrier
{
	GENERATED_BODY()

	FTrimeshShapeBarrier();
	FTrimeshShapeBarrier(std::shared_ptr<FGeometryAndShapeRef> Native);

	int32 GetNumPositions() const;

	int32 GetNumIndices() const;

	int32 GetNumTriangles() const;

	/**
	 * One FVector per vertex position. Vertex positions can be shared between
	 * triangles.
	 */
	TArray<FVector> GetVertexPositions() const;

	/**
	 * Mapping from triangles to vertex positions. Each three consecutive
	 * indices form a triangle between the three pointed-to vertex positions.
	 * Several triangles may reference the same vertex position.
	 */
	TArray<uint32> GetVertexIndices() const;

	/**
	 * Per-triangle normal.
	 */
	TArray<FVector> GetTriangleNormals() const;

	/**
	 * The source name is a user-provided string that is stored with the trimesh.
	 * it can be the name of a file on disk from which the mesh data was read,
	 * or some other form of description. May be the empty string.
	 *
	 * @return The source name that has been stored with the trimesh.
	 */
	FString GetSourceName() const;

	/**
	 * Get the GUID for the mesh data. This can be used to determine if two Trimeshes uses the same
	 * mesh data and thus must be identical.
	 *
	 * @return The GUID for the mesh data.
	 */
	FGuid GetMeshDataGuid() const;

	void AllocateNative(
		const TArray<FVector>& Vertices, const TArray<FTriIndices>& TriIndices, bool bClockwise,
		const FString& SourceName);

private:
	virtual void AllocateNativeShape() override;
	virtual void ReleaseNativeShape() override;

private:
	struct AllocationParameters
	{
		const TArray<FVector>* Vertices;
		const TArray<FTriIndices>* TriIndices;
		bool bClockwise;
		const FString& SourceName;

		AllocationParameters(const FString& InSourceName)
			: SourceName(InSourceName)
		{
		}
	};

	std::weak_ptr<AllocationParameters> TemporaryAllocationParameters;
};
