// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Terrain/TerrainParticleTypes.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

class FTerrainBarrier;

struct FParticleData;
struct FParticleDataById;

class FTerrainUtilities
{
public:
	/**
	 * Writes the position of all particles known to the passed Terrain to OutPositions.
	 */
	static void AppendParticlePositions(
		const FTerrainBarrier& Terrain, TArray<FVector>& OutPositions);

	/**
	 * Writes the velocities of all particles in the given Terrain to OutVelocities.
	 */
	static void AppendParticleVelocities(
		const FTerrainBarrier& Terrain, TArray<FVector>& OutVelocities);

	/**
	 * Writes the radii of all particles known to the passed Terrain to OutRadii.
	 */
	static void AppendParticleRadii(const FTerrainBarrier& Terrain, TArray<float>& OutRadii);

	/**
	 * Writes the rotation of all particles known to the passed Terrain to OutRotations.
	 */
	static void AppendParticleRotations(
		const FTerrainBarrier& Terrain, TArray<FQuat>& OutRotations);

	/**
	 * Writes the mass of all particles known to the passed Terrain to OutMasses.
	 */
	static void AppendParticleMasses(
		const FTerrainBarrier& Terrain, TArray<float>& OutMasses);

	/**
	 * Writes the data indicated by the ToInclude bit set flags of all particles in the given
	 * Terrain to OutParticleData.
	 */
	static void AppendParticleData(
		const FTerrainBarrier& Terrain, FParticleData& OutParticleData,
		EParticleDataFlags ToInclude);

	static void GetParticleExistsById(const FTerrainBarrier& Terrain, TArray<bool>& OutExists);

	static void GetParticlePositionsById(
		const FTerrainBarrier& Terrain, TArray<FVector>& OutPositions);

	static void GetParticleVelocitiesById(
		const FTerrainBarrier& Terrain, TArray<FVector>& OutVelocities);

	static void GetParticleRadiiById(const FTerrainBarrier& Terrain, TArray<float>& OutRadii);

	static void GetParticleRotationsById(
		const FTerrainBarrier& Terrain, TArray<FQuat>& OutRotation);

	static void GetParticleMassesById(
		const FTerrainBarrier& Terrain, TArray<float>& OutMasses);

	/**
	 * Writes the data indicated by the ToInclude bit set flags of all particles known to the passed
	 * Terrain to OutParticleData. The Exists array is always populated, even when ToInclude is
	 * empty.
	 */
	static void GetParticleDataById(
		const FTerrainBarrier& Terrain, FParticleDataById& OutParticleData,
		EParticleDataFlags ToInclude);

	/**
	 * Returns the number of particles known to the passed Terrain.
	 */
	static size_t GetNumParticles(const FTerrainBarrier& Terrain);
};
