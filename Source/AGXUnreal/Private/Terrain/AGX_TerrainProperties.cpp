// Copyright 2025, Algoryx Simulation AB.

#pragma once

#include "Terrain/AGX_TerrainProperties.h"

// AGX Dynamics for Unreal includes.
#include "AGX_AssetGetterSetterImpl.h"
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "AGX_PropertyChangedDispatcher.h"

void UAGX_TerrainProperties::SetCreateParticles(bool CreateParticles)
{
	AGX_ASSET_SETTER_IMPL_VALUE(bCreateParticles, CreateParticles, SetCreateParticles);
}

bool UAGX_TerrainProperties::GetCreateParticles() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(bCreateParticles, GetCreateParticles);
}

void UAGX_TerrainProperties::SetDeleteParticlesOutsideBounds(bool DeleteParticlesOutsideBounds)
{
	AGX_ASSET_SETTER_IMPL_VALUE(
		bDeleteParticlesOutsideBounds, DeleteParticlesOutsideBounds,
		SetDeleteParticlesOutsideBounds);
}

bool UAGX_TerrainProperties::GetDeleteParticlesOutsideBounds() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(bDeleteParticlesOutsideBounds, GetDeleteParticlesOutsideBounds);
}

void UAGX_TerrainProperties::SetPenetrationForceVelocityScaling(
	double InPenetrationForceVelocityScaling)
{
	AGX_ASSET_SETTER_IMPL_VALUE(
		PenetrationForceVelocityScaling, InPenetrationForceVelocityScaling,
		SetPenetrationForceVelocityScaling);
}

double UAGX_TerrainProperties::GetPenetrationForceVelocityScaling() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(
		PenetrationForceVelocityScaling, GetPenetrationForceVelocityScaling);
}

void UAGX_TerrainProperties::SetMaximumParticleActivationVolume(
	double InMaximumParticleActivationVolume)
{
	AGX_ASSET_SETTER_IMPL_VALUE(
		MaximumParticleActivationVolume, InMaximumParticleActivationVolume,
		SetMaximumParticleActivationVolume);
}

double UAGX_TerrainProperties::GetMaximumParticleActivationVolume() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(
		MaximumParticleActivationVolume, GetMaximumParticleActivationVolume);
}

void UAGX_TerrainProperties::SetSoilParticleSizeScaling(float InScaling)
{
	AGX_ASSET_SETTER_IMPL_VALUE(SoilParticleSizeScaling, InScaling, SetSoilParticleSizeScaling);
}

float UAGX_TerrainProperties::GetSoilParticleSizeScaling() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(SoilParticleSizeScaling, GetSoilParticleSizeScaling);
}

void UAGX_TerrainProperties::CopyFrom(const UAGX_TerrainProperties* Source)
{
	if (Source == nullptr)
		return;

	bCreateParticles = Source->bCreateParticles;
	bDeleteParticlesOutsideBounds = Source->bDeleteParticlesOutsideBounds;
	PenetrationForceVelocityScaling = Source->PenetrationForceVelocityScaling;
	MaximumParticleActivationVolume = Source->MaximumParticleActivationVolume;
	SoilParticleSizeScaling = Source->SoilParticleSizeScaling;
}

void UAGX_TerrainProperties::CopyFrom(const FTerrainPropertiesBarrier& Source)
{
	if (!Source.HasNative())
		return;

	bCreateParticles = Source.GetCreateParticles();
	bCreateParticles = Source.GetCreateParticles();
	bDeleteParticlesOutsideBounds = Source.GetDeleteParticlesOutsideBounds();
	PenetrationForceVelocityScaling = Source.GetPenetrationForceVelocityScaling();
	MaximumParticleActivationVolume = Source.GetMaximumParticleActivationVolume();
	SoilParticleSizeScaling = Source.GetSoilParticleSizeScaling();
}

UAGX_TerrainProperties* UAGX_TerrainProperties::CreateInstanceFromAsset(
	const UWorld* PlayingWorld, UAGX_TerrainProperties* Source)
{
	check(Source);
	check(!Source->IsInstance());
	check(PlayingWorld != nullptr);
	check(PlayingWorld->IsGameWorld());

	const FString InstanceName = Source->GetName() + "_Instance";

	UAGX_TerrainProperties* NewInstance = NewObject<UAGX_TerrainProperties>(
		GetTransientPackage(), UAGX_TerrainProperties::StaticClass(), *InstanceName, RF_Transient);
	NewInstance->Asset = Source;
	NewInstance->CopyFrom(Source);
	NewInstance->CreateNative();

	return NewInstance;
}

void UAGX_TerrainProperties::CommitToAsset()
{
	if (IsInstance())
	{
		if (HasNative())
		{
			Asset->CopyFrom(*GetNative());
		}
	}
	else if (Instance != nullptr)
	{
		Instance->CommitToAsset();
	}
}

UAGX_TerrainProperties* UAGX_TerrainProperties::GetInstance()
{
	if (IsInstance())
		return this;
	else
		return Instance.Get();
}

UAGX_TerrainProperties* UAGX_TerrainProperties::GetOrCreateInstance(const UWorld* PlayingWorld)
{
	if (IsInstance())
		return this;

	UAGX_TerrainProperties* InstancePtr = Instance.Get();
	if (InstancePtr == nullptr && PlayingWorld != nullptr && PlayingWorld->IsGameWorld())
	{
		InstancePtr = UAGX_TerrainProperties::CreateInstanceFromAsset(PlayingWorld, this);
		Instance = InstancePtr;
	}

	return InstancePtr;
}

UAGX_TerrainProperties* UAGX_TerrainProperties::GetAsset()
{
	if (IsInstance())
		return Asset.Get();
	else
		return this;
}

bool UAGX_TerrainProperties::IsInstance() const
{
	if (GetOuter() == GetTransientPackage() || Cast<UWorld>(GetOuter()) != nullptr)
		return true;

	const bool bIsInstance = Asset != nullptr;

	AGX_CHECK(bIsInstance != IsAsset());

	return bIsInstance;
}

bool UAGX_TerrainProperties::HasNative() const
{
	if (IsInstance())
		return NativeBarrier.HasNative();
	else
		return Instance != nullptr && Instance->HasNative();
}

FTerrainPropertiesBarrier* UAGX_TerrainProperties::GetNative()
{
	return const_cast<FTerrainPropertiesBarrier*>(const_cast<const ThisClass*>(this)->GetNative());
}

const FTerrainPropertiesBarrier* UAGX_TerrainProperties::GetNative() const
{
	if (IsInstance())
		return NativeBarrier.HasNative() ? &NativeBarrier : nullptr;
	else
		return Instance != nullptr ? Instance->GetNative() : nullptr;
}

FTerrainPropertiesBarrier* UAGX_TerrainProperties::GetOrCreateNative()
{
	if (IsInstance())
	{
		if (!HasNative())
			CreateNative();

		return GetNative();
	}
	else
	{
		if (Instance == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("GetOrCreateNative() called on '%s' but Instance == nullptr. "
					 "Call GetOrCreateInstance() first."),
				*GetName());
			return nullptr;
		}

		return Instance->GetOrCreateNative();
	}
}

void UAGX_TerrainProperties::UpdateNativeProperties()
{
	if (!IsInstance() || !HasNative())
		return;

	NativeBarrier.SetCreateParticles(bCreateParticles);
	NativeBarrier.SetDeleteParticlesOutsideBounds(bDeleteParticlesOutsideBounds);
	NativeBarrier.SetPenetrationForceVelocityScaling(PenetrationForceVelocityScaling);
	NativeBarrier.SetMaximumParticleActivationVolume(MaximumParticleActivationVolume);
	NativeBarrier.SetSoilParticleSizeScaling(SoilParticleSizeScaling);
}

void UAGX_TerrainProperties::PostInitProperties()
{
	Super::PostInitProperties();

#if WITH_EDITOR
	InitPropertyDispatcher();
#endif
}

#if WITH_EDITOR
void UAGX_TerrainProperties::PostEditChangeChainProperty(FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);
	Super::PostEditChangeChainProperty(Event);
}

void UAGX_TerrainProperties::InitPropertyDispatcher()
{
	auto& PropertyDispatcher = FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (PropertyDispatcher.IsInitialized())
		return;

	AGX_COMPONENT_DEFAULT_DISPATCHER_BOOL(CreateParticles);
	AGX_COMPONENT_DEFAULT_DISPATCHER_BOOL(DeleteParticlesOutsideBounds);
	AGX_COMPONENT_DEFAULT_DISPATCHER(PenetrationForceVelocityScaling);
	AGX_COMPONENT_DEFAULT_DISPATCHER(MaximumParticleActivationVolume);
	AGX_COMPONENT_DEFAULT_DISPATCHER(SoilParticleSizeScaling);
}
#endif // WITH_EDITOR

void UAGX_TerrainProperties::CreateNative()
{
	if (IsInstance())
	{
		check(!HasNative());
		NativeBarrier.AllocateNative();

		if (!HasNative())
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("UAGX_TerrainProperties '%s' failed to allocate AGX native instance."),
				*GetName());
			return;
		}
		UpdateNativeProperties();
	}
	else
	{
		if (Instance == nullptr)
		{
			UE_LOG(
				LogAGX, Error, TEXT("CreateNative() called on asset '%s' but Instance == nullptr."),
				*GetName());
			return;
		}

		Instance->CreateNative();
	}
}
