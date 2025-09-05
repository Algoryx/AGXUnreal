// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Terrain/TerrainParticleTypes.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Math/Vector.h"
#include "Math/Quat.h"

// Standard library includes.
#include <memory>
#include <tuple>

struct FTerrainRef;

class FHeightFieldShapeBarrier;
class FShovelBarrier;
class FTerrainMaterialBarrier;
class FShapeMaterialBarrier;

/**
 *
 */
class AGXUNREALBARRIER_API FTerrainBarrier
{
public:
	FTerrainBarrier();
	FTerrainBarrier(std::unique_ptr<FTerrainRef> InNativeRef);
	FTerrainBarrier(FTerrainBarrier&& Other);
	~FTerrainBarrier();

	bool HasNative() const;
	void AllocateNative(FHeightFieldShapeBarrier& SourceHeightField, double MaxDepth);
	FTerrainRef* GetNative();
	const FTerrainRef* GetNative() const;
	void ReleaseNative();

	void SetCanCollide(bool bCanCollide);
	bool GetCanCollide() const;

	void SetPosition(const FVector& Position);
	FVector GetPosition() const;

	void SetRotation(const FQuat& Rotation);
	FQuat GetRotation() const;

	void SetCreateParticles(bool CreateParticles);
	bool GetCreateParticles() const;

	void SetDeleteParticlesOutsideBounds(bool DeleteParticlesOutsideBounds);
	bool GetDeleteParticlesOutsideBounds() const;

	void SetPenetrationForceVelocityScaling(double PenetrationForceVelocityScaling);
	double GetPenetrationForceVelocityScaling() const;

	void SetMaximumParticleActivationVolume(double MaximumParticleActivationVolume);
	double GetMaximumParticleActivationVolume() const;

	/// Deprecated, Shovels should be added to the Simulation instead.
	bool AddShovel(FShovelBarrier& Shovel);

	void SetShapeMaterial(const FShapeMaterialBarrier& Material);
	void SetTerrainMaterial(const FTerrainMaterialBarrier& TerrainMaterial);

	void AddCollisionGroup(const FName& GroupName);
	void AddCollisionGroups(const TArray<FName>& GroupNames);
	TArray<FName> GetCollisionGroups() const;
	void RemoveCollisionGroup(const FName& GroupName);

	/**
	 * Clears the internal terrain material.
	 */
	void ClearTerrainMaterial();

	/**
	 * Clears the internal Shape material.
	 */
	void ClearShapeMaterial();

	int32 GetGridSizeX() const;
	int32 GetGridSizeY() const;

	/**
	 * Returns the modified vertices since the last AGX Dynamics Step Forward.
	 * The x/y layout matches that of an Unreal Landscape coordinate system.
	 *
	 * Example of how to iterate over heights using GetModifiedVertices:
	 * for (const auto& VertexTuple : GetModifiedVertices())
	 * {
	 *		const int32 VertX = std::get<0>(VertexTuple);
	 *		const int32 VertY = std::get<1>(VertexTuple);
	 *		const int32 Index = VertX + VertY * TerrainVerticesX;
	 *		float Height = Heights[Index];
	 *	}
	 */
	TArray<std::tuple<int32, int32>> GetModifiedVertices() const;

	/**
	 * Fill an array with all the heights in the height field, stored in X major
	 * order, meaning that heights with increasing the X coordinates are next to
	 * each other in memory.
	 * If the ChangesOnly parameter is set to true, only height changes from the previous AGX
	 * Dynamics Step Forward is read. This can be used as a performance optimization, but use with
	 * caution; the caller may miss height changes if this is not called each AGX Dynamics Step
	 * Forward. If ChangesOnly is true, then the GetModifiedVertices can be used when iterating over
	 * the heights array at the call site, instead of iterating over all heights, as a further
	 * optimization.
	 */
	void GetHeights(TArray<float>& OutHeights, bool bChangesOnly) const;

	/**
	 * Get an array with the positions of the currently existing particles.
	 */
	TArray<FVector> GetParticlePositions() const;

	/**
	 * Get an array with the radii of the currently existing particles.
	 */
	TArray<float> GetParticleRadii() const;

	/**
	 * Get an array with the rotations of the currently existing particles.
	 */
	TArray<FQuat> GetParticleRotations() const;

	/**
	 * Get an array with the masses of the currently existing particles.
	 */
	TArray<float> GetParticleMasses() const;

	/**
	 * Get the data indicated by ToInclude of all particles.
	 */
	FParticleData GetParticleData(EParticleDataFlags ToInclude) const;

	/**
	 * Get the data indicated by the ToInclude bit set flags of all particles.
	 *
	 * The resulting buffers are populated by entity ID, not by index, which means that there may be
	 * gaps in the data. An Exists array of bools indicate whether or not there is particle data at
	 * a particular index. The Exists array is always populated, even if ToInclude is empty.
	 */
	FParticleDataById GetParticleDataById(EParticleDataFlags ToInclude) const;

	/**
	 * Returns the number of spawned Terrain particles known by the Terrain Native.
	 */
	size_t GetNumParticles() const;

private:
	FTerrainBarrier(const FTerrainBarrier&) = delete;
	void operator=(const FTerrainBarrier&) = delete;

private:
	std::unique_ptr<FTerrainRef> NativeRef;
};
