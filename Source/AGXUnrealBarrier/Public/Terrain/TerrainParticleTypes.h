// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "Math/Vector.h"
#include "CoreMinimal.h"
#include "Containers/Array.h"

struct FParticleData
{
	TArray<FVector> Positions;
	TArray<FVector> Velocities;
	TArray<float> Radii;
	TArray<FQuat> Rotations;
	TArray<float> Masses;
};

/**
 * Particle data arrays where the per-particle data is stored by-entity-ID instead of packed
 * together. This means that data for a particular particle will always be at the same index in
 * these arrays since a particle always has the same ID. It also means that there may be gaps in
 * the arrays since IDs used by particles removed from the simulation won't be reused until enough
 * new particles has been created to fill the gaps.
 *
 * This data layout is useful when rendering with Niagara because the data reshuffling made by
 * packed data arrays is seen by Niagara as very fast moving particles, which produces rendering
 * artifacts. With this approach particles have a persistent and stable concept of "self". It is
 * possible that there are better ways to solve the problem, maybe with the PARTICLES.ID Niagara
 * attribute, but I don't yet know how.
 */
struct FParticleDataById
{
	TArray<FVector> Positions;
	TArray<FVector> Velocities;
	TArray<float> Radii;
	TArray<FQuat> Rotations;
	TArray<float> Masses;
	TArray<bool> Exists; // TArray instead of TBitArray to be compatible with Niagara Arrays.
};

namespace ParticleDataFlags
{
	enum Type
	{
		Positions = 1 << 0,
		Velocities = 1 << 1,
		Radii = 1 << 2,
		Rotations = 1 << 3,
		Masses = 1 << 4,
		All = Positions | Velocities | Radii | Rotations | Masses
	};
}

using EParticleDataFlags = ParticleDataFlags::Type;

inline EParticleDataFlags operator|(EParticleDataFlags Lhs, EParticleDataFlags Rhs)
{
	return static_cast<EParticleDataFlags>(
		static_cast<std::underlying_type<EParticleDataFlags>::type>(Lhs) |
		static_cast<std::underlying_type<EParticleDataFlags>::type>(Rhs));
}
