// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <memory>

struct FRenderDataRef;
struct FAGX_RenderMaterial;

/**
 * Barrier class that gives access to the Render Data stored per shape in AGX Dynamics.
 *
 * The render data may contain a mesh and may contain a material, independently. The mesh is
 * provided as a set of Array getter functions. The material is provided as a struct containing
 * material parameters compatible with the M_ImportedBase Material asset shipped with AGX Dynamics
 * for Unreal.
 *
 * The mesh is stored as a collection of vertex attribute arrays and an index array where every
 * three consecutive indices form a triangle using the vertex data pointed to by those three
 * indices.
 *
 * Data shared among triangles:
 *    positions:  [Vec3, Vec3, Vec3, Vec3, Vec3, ... ]
 *    normals:    [Vec3, Vec3, Vec3, Vec3, Vec3, ... ]
 *    tex coords: [Vec2, Vec2, Vec2, Vec2, Vec2, ... ]
 *
 * Data owned by each triangle:
 *                |  Triangle 0   |  Triangle 1   | ... |
 *    indices:    | int, int, int | int, int, int | ... |
 *
 * Multiple shapes may share the same Render Data, and multiple Render Data may share the same
 * Render Material.
 */
class AGXUNREALBARRIER_API FRenderDataBarrier
{
public:
	FRenderDataBarrier();
	FRenderDataBarrier(FRenderDataBarrier&& Other);
	FRenderDataBarrier(std::unique_ptr<FRenderDataRef>&& InNativeRef);
	~FRenderDataBarrier();

	bool GetShouldRender() const;

	bool HasMesh() const;

	int32 GetNumTriangles() const;
	int32 GetNumIndices() const;
	int32 GetNumPositions() const;
	int32 GetNumNormals() const;
	int32 GetNumTextureCoordinates() const;

	TArray<uint32> GetIndices() const;
	TArray<FVector> GetPositions() const;
	TArray<FVector> GetNormals() const;
	TArray<FVector2D> GetTextureCoordinates() const;

	bool HasMaterial() const;
	FAGX_RenderMaterial GetMaterial() const;

	FGuid GetGuid() const;

	bool HasNative() const;
	FRenderDataRef* GetNative();
	const FRenderDataRef* GetNative() const;
	void ReleaseNative();

private:
	FRenderDataBarrier(const FRenderDataBarrier& Other) = delete;
	FRenderDataBarrier& operator=(const FRenderDataBarrier& Other) = delete;
	FRenderDataBarrier& operator=(FRenderDataRef&& Other) = delete;

private:
	std::unique_ptr<FRenderDataRef> NativeRef;
};
