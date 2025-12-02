// Copyright 2025, Algoryx Simulation AB.

#include "Utilities/TerrainUtilities.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "BarrierOnly/AGXRefs.h"
#include "Terrain/TerrainBarrier.h"
#include "TypeConversions.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxTerrain/Terrain.h>
#include <agx/Physics/GranularBodySystem.h>
#include <agxTerrain/Terrain.h>
#include "EndAGXIncludes.h"

namespace TerrainUtilities_helpers
{
	// Convenience getter functions.

	const agx::Physics::GranularBodySystem* GetGranularBodySystem(const FTerrainBarrier& Terrain)
	{
		return Terrain.GetNative()->Native->getSoilSimulationInterface()->getGranularBodySystem();
	}

	const agx::Physics::GranularBodyPtrArray GetGranularParticles(const FTerrainBarrier& Terrain)
	{
		return GetGranularBodySystem(Terrain)->getParticles();
	}

	const agxData::EntityStorage* GetGranularStorage(const FTerrainBarrier& Terrain)
	{
		return GetGranularBodySystem(Terrain)->getParticleStorage();
	}

	struct FParticlesWithIdToIndex
	{
		const agx::Physics::GranularBodyPtrArray Ptrs;
		const agxData::IndexArray& IdToIndex;
	};

	FParticlesWithIdToIndex GetParticlesWithIdToIndex(const FTerrainBarrier& Terrain)
	{
		return {GetGranularParticles(Terrain), GetGranularStorage(Terrain)->getIdToIndexTable()};
	}

	void GetExistsById(const FParticlesWithIdToIndex& Particles, TArray<bool>& OutExists)
	{
		verify(Particles.IdToIndex.size() < std::numeric_limits<int32>::max());
		const int32 NumIds = static_cast<int32>(Particles.IdToIndex.size());
		OutExists.SetNum(NumIds);
		const size_t NumParticles = Particles.Ptrs.size();

		for (size_t Id = 0; Id < NumIds; ++Id)
		{
			const size_t Index = Particles.IdToIndex[Id];
			OutExists[Id] = Index < NumParticles;
		}
	}

	// Type-agnostic by-Id getter.

	template <typename FUnrealT, typename FGetFunc, typename FConvertFunc>
	void GetParticleDataById(
		const FParticlesWithIdToIndex& Particles, TArray<FUnrealT>& Out, FUnrealT InvalidMarker,
		FGetFunc GetFunc, FConvertFunc ConvertFunc)
	{
		verify(Particles.IdToIndex.size() < std::numeric_limits<int32>::max());
		const int32 NumIds = static_cast<int32>(Particles.IdToIndex.size());
		Out.SetNum(NumIds);

		for (size_t Id = 0; Id < NumIds; ++Id)
		{
			const size_t Index = Particles.IdToIndex[Id];

			// This assumes that the IdToIndex array contains InvalidIndex whenever there isn't an
			// entity with that ID in the storage. Is that always guaranteed?
			if (Index != agx::InvalidIndex)
			{
				const auto& ValueAgx = GetFunc(Particles.Ptrs[Index]);
				const FUnrealT Value = ConvertFunc(ValueAgx);
				Out[Id] = Value;
			}
			else
			{
				// Some types may do validity checks, such as FVector not allowing NaN, in the
				// assignment operator. If this becomes a problem then do memcpy here instead.
				Out[Id] = InvalidMarker;
			}
		}
	}

	// Position getters.

	void AppendPositions(
		const agx::Physics::GranularBodyPtrArray& GranularParticles, TArray<FVector>& OutPositions)
	{
		if (GranularParticles.size() > OutPositions.GetSlack())
		{
			OutPositions.Reserve(OutPositions.Num() + GranularParticles.size());
		}

		for (size_t i = 0; i < GranularParticles.size(); ++i)
		{
			const agx::Vec3 PositionAGX = GranularParticles[i].position();
			const FVector Position = ConvertDisplacement(PositionAGX);
			OutPositions.Add(Position);
		}
	}

	void GetPositionsById(const FParticlesWithIdToIndex& Particles, TArray<FVector>& OutPositions)
	{
		// Create an invalid marker filled with NaN. Cannot use a constructor to initialize it
		// because the constructors check for NaN.
		FVector InvalidMarker;
		constexpr FVector::FReal NaN = std::numeric_limits<FVector::FReal>::quiet_NaN();
		InvalidMarker.X = InvalidMarker.Y = InvalidMarker.Z = NaN;
		GetParticleDataById(
			Particles, OutPositions, InvalidMarker,
			[](const agx::Physics::GranularBodyPtr& Particle) { return Particle.position(); },
			[](const agx::Vec3& ValueAgx) { return ConvertDisplacement(ValueAgx); });
	}

	// Velocity getters.

	void AppendVelocities(
		const agx::Physics::GranularBodyPtrArray& Particles, TArray<FVector>& OutVelocities)
	{
		if (Particles.size() > OutVelocities.GetSlack())
		{
			OutVelocities.Reserve(OutVelocities.Num() + Particles.size());
		}

		for (size_t I = 0; I < Particles.size(); ++I)
		{
			const agx::Vec3 VelocityAgx = Particles[I].velocity();
			const FVector Velocity = ConvertDisplacement(VelocityAgx);
			OutVelocities.Add(Velocity);
		}
	}

	void GetVelocitiesById(const FParticlesWithIdToIndex& Particles, TArray<FVector>& OutVelocities)
	{
		// Create an invalid marker filled with NaN. Cannot use a constructor to initialize it
		// because the constructors check for NaN.
		FVector InvalidMarker;
		constexpr FVector::FReal NaN = std::numeric_limits<FVector::FReal>::quiet_NaN();
		InvalidMarker.X = InvalidMarker.Y = InvalidMarker.Z = NaN;
		GetParticleDataById(
			Particles, OutVelocities, InvalidMarker,
			[](const agx::Physics::GranularBodyPtr& Particle) { return Particle.velocity(); },
			[](const agx::Vec3& ValueAgx) { return ConvertDisplacement(ValueAgx); });
	}

	// Radii getters.

	void AppendRadii(
		const agx::Physics::GranularBodyPtrArray& GranularParticles, TArray<float>& OutRadii)
	{
		if (GranularParticles.size() > OutRadii.GetSlack())
		{
			OutRadii.Reserve(OutRadii.Num() + GranularParticles.size());
		}

		for (size_t i = 0; i < GranularParticles.size(); ++i)
		{
			const agx::Real RadiusAGX = GranularParticles[i].radius();
			const float Radius = ConvertDistanceToUnreal<float>(RadiusAGX);
			OutRadii.Add(Radius);
		}
	}

	void GetRadiiById(const FParticlesWithIdToIndex& Particles, TArray<float>& OutRadii)
	{
		constexpr float InvalidMarker = std::numeric_limits<float>::quiet_NaN();
		GetParticleDataById(
			Particles, OutRadii, InvalidMarker,
			[](const agx::Physics::GranularBodyPtr& Particle) { return Particle.radius(); },
			[](float ValueAgx) { return ConvertDistanceToUnreal<float>(ValueAgx); });
	}

	// Rotations getter.

	void AppendRotations(
		const agx::Physics::GranularBodyPtrArray& GranularParticles, TArray<FQuat>& OutRotations)
	{
		if (GranularParticles.size() > OutRotations.GetSlack())
		{
			OutRotations.Reserve(OutRotations.Num() + GranularParticles.size());
		}

		for (size_t i = 0; i < GranularParticles.size(); ++i)
		{
			const agx::Quat RotationAGX = GranularParticles[i].rotation();
			const FQuat Rotation = Convert(RotationAGX);
			OutRotations.Add(Rotation);
		}
	}

	void GetRotationsById(const FParticlesWithIdToIndex& Particles, TArray<FQuat>& OutRotations)
	{
		FQuat InvalidMarker;
		constexpr FQuat::FReal NaN = std::numeric_limits<FVector::FReal>::quiet_NaN();
		InvalidMarker.X = InvalidMarker.Y = InvalidMarker.Z = InvalidMarker.W = NaN;
		GetParticleDataById(
			Particles, OutRotations, InvalidMarker,
			[](const agx::Physics::GranularBodyPtr& Particle) { return Particle.rotation(); },
			[](const agx::Quat& ValueAgx) { return Convert(ValueAgx); });
	}
}

void FTerrainUtilities::AppendParticlePositions(
	const FTerrainBarrier& Terrain, TArray<FVector>& OutPositions)
{
	AGX_CHECK(Terrain.HasNative());
	if (!Terrain.HasNative())
		return;
	using namespace TerrainUtilities_helpers;
	const agx::Physics::GranularBodyPtrArray GranularParticles = GetGranularParticles(Terrain);
	AppendPositions(GranularParticles, OutPositions);
}

void FTerrainUtilities::AppendParticleVelocities(
	const FTerrainBarrier& Terrain, TArray<FVector>& OutVelocities)
{
	AGX_CHECK(Terrain.HasNative());
	if (!Terrain.HasNative())
		return;
	using namespace TerrainUtilities_helpers;
	const agx::Physics::GranularBodyPtrArray Particles = GetGranularParticles(Terrain);
	AppendVelocities(Particles, OutVelocities);
}

void FTerrainUtilities::AppendParticleRadii(const FTerrainBarrier& Terrain, TArray<float>& OutRadii)
{
	AGX_CHECK(Terrain.HasNative());
	if (!Terrain.HasNative())
		return;
	using namespace TerrainUtilities_helpers;
	const agx::Physics::GranularBodyPtrArray GranularParticles = GetGranularParticles(Terrain);
	AppendRadii(GranularParticles, OutRadii);
}

void FTerrainUtilities::AppendParticleRotations(
	const FTerrainBarrier& Terrain, TArray<FQuat>& OutRotations)
{
	AGX_CHECK(Terrain.HasNative());
	if (!Terrain.HasNative())
		return;
	using namespace TerrainUtilities_helpers;
	const agx::Physics::GranularBodyPtrArray GranularParticles = GetGranularParticles(Terrain);
	AppendRotations(GranularParticles, OutRotations);
}

void FTerrainUtilities::AppendParticleData(
	const FTerrainBarrier& Terrain, FParticleData& OutParticleData, EParticleDataFlags ToInclude)
{
	AGX_CHECK(Terrain.HasNative());
	if (!Terrain.HasNative())
		return;

	using namespace TerrainUtilities_helpers;
	const agx::Physics::GranularBodyPtrArray GranularParticles = GetGranularParticles(Terrain);
	if (ToInclude & EParticleDataFlags::Positions)
		AppendPositions(GranularParticles, OutParticleData.Positions);
	if (ToInclude & EParticleDataFlags::Velocities)
		AppendVelocities(GranularParticles, OutParticleData.Velocities);
	if (ToInclude & EParticleDataFlags::Radii)
		AppendRadii(GranularParticles, OutParticleData.Radii);
	if (ToInclude & EParticleDataFlags::Rotations)
		AppendRotations(GranularParticles, OutParticleData.Rotations);
}

void FTerrainUtilities::GetParticleExistsById(
	const FTerrainBarrier& Terrain, TArray<bool>& OutExists)
{
	AGX_CHECK(Terrain.HasNative());
	if (!Terrain.HasNative())
		return;
	using namespace TerrainUtilities_helpers;
	const FParticlesWithIdToIndex ParticlesWithIdToIndex = GetParticlesWithIdToIndex(Terrain);
	GetExistsById(ParticlesWithIdToIndex, OutExists);
}

void FTerrainUtilities::GetParticlePositionsById(
	const FTerrainBarrier& Terrain, TArray<FVector>& OutPositions)
{
	AGX_CHECK(Terrain.HasNative());
	if (!Terrain.HasNative())
		return;
	using namespace TerrainUtilities_helpers;
	const FParticlesWithIdToIndex ParticlesWithIdToIndex = GetParticlesWithIdToIndex(Terrain);
	GetPositionsById(ParticlesWithIdToIndex, OutPositions);
}

void FTerrainUtilities::GetParticleVelocitiesById(
	const FTerrainBarrier& Terrain, TArray<FVector>& OutVelocities)
{
	AGX_CHECK(Terrain.HasNative())
	if (!Terrain.HasNative())
		return;
	using namespace TerrainUtilities_helpers;
	const FParticlesWithIdToIndex ParticlesWithIdToIndex = GetParticlesWithIdToIndex(Terrain);
	GetVelocitiesById(ParticlesWithIdToIndex, OutVelocities);
}

void FTerrainUtilities::GetParticleRotationsById(
	const FTerrainBarrier& Terrain, TArray<FQuat>& OutRotations)
{
	AGX_CHECK(Terrain.HasNative())
	if (!Terrain.HasNative())
		return;
	using namespace TerrainUtilities_helpers;
	const FParticlesWithIdToIndex ParticlesWithIdToIndex = GetParticlesWithIdToIndex(Terrain);
	GetRotationsById(ParticlesWithIdToIndex, OutRotations);
}

void FTerrainUtilities::GetParticleRadiiById(
	const FTerrainBarrier& Terrain, TArray<float>& OutRadii)
{
	AGX_CHECK(Terrain.HasNative())
	if (!Terrain.HasNative())
		return;
	using namespace TerrainUtilities_helpers;
	const FParticlesWithIdToIndex ParticlesWithIdToIndex = GetParticlesWithIdToIndex(Terrain);
	GetRadiiById(ParticlesWithIdToIndex, OutRadii);
}

void FTerrainUtilities::GetParticleDataById(
	const FTerrainBarrier& Terrain, FParticleDataById& OutParticleData,
	EParticleDataFlags ToInclude)
{
	AGX_CHECK(Terrain.HasNative());
	if (!Terrain.HasNative())
		return;

	using namespace TerrainUtilities_helpers;
	const FParticlesWithIdToIndex ParticlesWithIdToIndex = GetParticlesWithIdToIndex(Terrain);
	GetExistsById(ParticlesWithIdToIndex, OutParticleData.Exists);
	if (ToInclude & EParticleDataFlags::Positions)
		GetPositionsById(ParticlesWithIdToIndex, OutParticleData.Positions);
	if (ToInclude & EParticleDataFlags::Velocities)
		GetVelocitiesById(ParticlesWithIdToIndex, OutParticleData.Velocities);
	if (ToInclude & EParticleDataFlags::Radii)
		GetRadiiById(ParticlesWithIdToIndex, OutParticleData.Radii);
	if (ToInclude & EParticleDataFlags::Rotations)
		GetRotationsById(ParticlesWithIdToIndex, OutParticleData.Rotations);
}

size_t FTerrainUtilities::GetNumParticles(const FTerrainBarrier& Terrain)
{
	AGX_CHECK(Terrain.HasNative());
	if (!Terrain.HasNative())
		return 0;

	return Terrain.GetNative()->Native->getSoilSimulationInterface()->getNumSoilParticles();
}
