// Copyright 2025, Algoryx Simulation AB.

#pragma once

#include "Terrain/AGX_TerrainProperties.h"

// AGX Dynamics for Unreal includes.
#include "AGX_AssetGetterSetterImpl.h"
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "Terrain/AGX_Terrain.h"

// Unreal Engine includes.
#include "Engine/World.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

void UAGX_TerrainProperties::SetCreateParticles(bool CreateParticles)
{
	AGX_ASSET_SETTER_IMPL_VALUE(bCreateParticles, CreateParticles, SetCreateParticles);
	CallOnTemplateTerrainChangedOnAll();
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
	CallOnTemplateTerrainChangedOnAll();
}

bool UAGX_TerrainProperties::GetDeleteParticlesOutsideBounds() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(bDeleteParticlesOutsideBounds, GetDeleteParticlesOutsideBounds);
}

void UAGX_TerrainProperties::SetSoilParticleGrowthRate(double InRate)
{
	AGX_ASSET_SETTER_IMPL_VALUE(SoilParticleGrowthRate, InRate, SetSoilParticleGrowthRate);
	CallOnTemplateTerrainChangedOnAll();
}

double UAGX_TerrainProperties::GetSoilParticleGrowthRate() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(SoilParticleGrowthRate, GetSoilParticleGrowthRate);
}

void UAGX_TerrainProperties::SetSoilParticleMergeRate(double InRate)
{
	AGX_ASSET_SETTER_IMPL_VALUE(SoilParticleMergeRate, InRate, SetSoilParticleMergeRate);
	CallOnTemplateTerrainChangedOnAll();
}

double UAGX_TerrainProperties::GetSoilParticleMergeRate() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(SoilParticleMergeRate, GetSoilParticleMergeRate);
}

void UAGX_TerrainProperties::SetSoilParticleMergeThreshold(double InThreshold)
{
	AGX_ASSET_SETTER_IMPL_VALUE(
		SoilParticleMergeThreshold, InThreshold, SetSoilParticleMergeThreshold);
	CallOnTemplateTerrainChangedOnAll();
}

double UAGX_TerrainProperties::GetSoilParticleMergeThreshold() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(SoilParticleMergeThreshold, GetSoilParticleMergeThreshold);
}

void UAGX_TerrainProperties::SetSoilMergeSpeedThreshold(double InThreshold)
{
	AGX_ASSET_SETTER_IMPL_VALUE(SoilMergeSpeedThreshold, InThreshold, SetSoilMergeSpeedThreshold);
	CallOnTemplateTerrainChangedOnAll();
}

double UAGX_TerrainProperties::GetSoilMergeSpeedThreshold() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(SoilMergeSpeedThreshold, GetSoilMergeSpeedThreshold);
}

void UAGX_TerrainProperties::SetSoilParticleLifeTime(double InLifeTime)
{
	AGX_ASSET_SETTER_IMPL_VALUE(SoilParticleLifeTime, InLifeTime, SetSoilParticleLifeTime);
	CallOnTemplateTerrainChangedOnAll();
}

double UAGX_TerrainProperties::GetSoilParticleLifeTime() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(SoilParticleLifeTime, GetSoilParticleLifeTime);
}

void UAGX_TerrainProperties::SetEnableAvalanching(bool Enable)
{
	AGX_ASSET_SETTER_IMPL_VALUE(bEnableAvalanching, Enable, SetEnableAvalanching);
	CallOnTemplateTerrainChangedOnAll();
}

bool UAGX_TerrainProperties::GetEnableAvalanching() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(bEnableAvalanching, GetEnableAvalanching);
}

void UAGX_TerrainProperties::SetAvalancheMaxHeightGrowth(double InValue)
{
	AGX_ASSET_SETTER_IMPL_VALUE(AvalancheMaxHeightGrowth, InValue, SetAvalancheMaxHeightGrowth);
	CallOnTemplateTerrainChangedOnAll();
}

double UAGX_TerrainProperties::GetAvalancheMaxHeightGrowth() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(AvalancheMaxHeightGrowth, GetAvalancheMaxHeightGrowth);
}

void UAGX_TerrainProperties::SetAvalancheDecayFraction(double InValue)
{
	AGX_ASSET_SETTER_IMPL_VALUE(AvalancheDecayFraction, InValue, SetAvalancheDecayFraction);
	CallOnTemplateTerrainChangedOnAll();
}

double UAGX_TerrainProperties::GetAvalancheDecayFraction() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(AvalancheDecayFraction, GetAvalancheDecayFraction);
}

void UAGX_TerrainProperties::SetAvalancheErrorThreshold(double InValue)
{
	AGX_ASSET_SETTER_IMPL_VALUE(AvalancheErrorThreshold, InValue, SetAvalancheErrorThreshold);
	CallOnTemplateTerrainChangedOnAll();
}

double UAGX_TerrainProperties::GetAvalancheErrorThreshold() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(AvalancheErrorThreshold, GetAvalancheErrorThreshold);
}

void UAGX_TerrainProperties::SetActivationSpeed(double InSpeed)
{
	AGX_ASSET_SETTER_IMPL_VALUE(ActivationSpeed, InSpeed, SetActivationSpeed);
	CallOnTemplateTerrainChangedOnAll();
}

double UAGX_TerrainProperties::GetActivationSpeed() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(ActivationSpeed, GetActivationSpeed);
}

void UAGX_TerrainProperties::SetEnableDeformation(bool bEnable)
{
	AGX_ASSET_SETTER_IMPL_VALUE(bEnableDeformation, bEnable, SetEnableDeformation);
	CallOnTemplateTerrainChangedOnAll();
}

bool UAGX_TerrainProperties::GetEnableDeformation() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(bEnableDeformation, GetEnableDeformation);
}

void UAGX_TerrainProperties::SetEnableLockedBorders(bool Enable)
{
	AGX_ASSET_SETTER_IMPL_VALUE(bEnableLockedBorders, Enable, SetEnableLockedBorders);
	CallOnTemplateTerrainChangedOnAll();
}

bool UAGX_TerrainProperties::GetEnableLockedBorders() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(bEnableLockedBorders, GetEnableLockedBorders);
}

void UAGX_TerrainProperties::SetEnableSoilCompaction(bool Enable)
{
	AGX_ASSET_SETTER_IMPL_VALUE(bEnableSoilCompaction, Enable, SetEnableSoilCompaction);
	CallOnTemplateTerrainChangedOnAll();
}

bool UAGX_TerrainProperties::GetEnableSoilCompaction() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(bEnableSoilCompaction, GetEnableSoilCompaction);
}

void UAGX_TerrainProperties::SetPenetrationForceVelocityScaling(
	double InPenetrationForceVelocityScaling)
{
	AGX_ASSET_SETTER_IMPL_VALUE(
		PenetrationForceVelocityScaling, InPenetrationForceVelocityScaling,
		SetPenetrationForceVelocityScaling);
	CallOnTemplateTerrainChangedOnAll();
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
	CallOnTemplateTerrainChangedOnAll();
}

double UAGX_TerrainProperties::GetMaximumParticleActivationVolume() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(
		MaximumParticleActivationVolume, GetMaximumParticleActivationVolume);
}

void UAGX_TerrainProperties::SetSoilParticleSizeScaling(float InScaling)
{
	AGX_ASSET_SETTER_IMPL_VALUE(SoilParticleSizeScaling, InScaling, SetSoilParticleSizeScaling);
	CallOnTemplateTerrainChangedOnAll();
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

	SoilParticleGrowthRate = Source->SoilParticleGrowthRate;
	SoilParticleMergeRate = Source->SoilParticleMergeRate;
	SoilParticleMergeThreshold = Source->SoilParticleMergeThreshold;
	SoilMergeSpeedThreshold = Source->SoilMergeSpeedThreshold;
	SoilParticleLifeTime = Source->SoilParticleLifeTime;

	bEnableAvalanching = Source->bEnableAvalanching;
	AvalancheMaxHeightGrowth = Source->AvalancheMaxHeightGrowth;
	AvalancheDecayFraction = Source->AvalancheDecayFraction;
	AvalancheErrorThreshold = Source->AvalancheErrorThreshold;

	ActivationSpeed = Source->ActivationSpeed;
	bEnableDeformation = Source->bEnableDeformation;
	bEnableLockedBorders = Source->bEnableLockedBorders;
	bEnableSoilCompaction = Source->bEnableSoilCompaction;
}

void UAGX_TerrainProperties::CopyFrom(const FTerrainPropertiesBarrier& Source)
{
	if (!Source.HasNative())
		return;

	bCreateParticles = Source.GetCreateParticles();
	bDeleteParticlesOutsideBounds = Source.GetDeleteParticlesOutsideBounds();
	PenetrationForceVelocityScaling = Source.GetPenetrationForceVelocityScaling();
	MaximumParticleActivationVolume = Source.GetMaximumParticleActivationVolume();
	SoilParticleSizeScaling = Source.GetSoilParticleSizeScaling();

	SoilParticleGrowthRate = Source.GetSoilParticleGrowthRate();
	SoilParticleMergeRate = Source.GetSoilParticleMergeRate();
	SoilParticleMergeThreshold = Source.GetSoilParticleMergeThreshold();
	SoilMergeSpeedThreshold = Source.GetSoilMergeSpeedThreshold();
	SoilParticleLifeTime = Source.GetSoilParticleLifeTime();

	bEnableAvalanching = Source.GetEnableAvalanching();
	AvalancheMaxHeightGrowth = Source.GetAvalancheMaxHeightGrowth();
	AvalancheDecayFraction = Source.GetAvalancheDecayFraction();
	AvalancheErrorThreshold = Source.GetAvalancheErrorThreshold();

	ActivationSpeed = Source.GetActivationSpeed();
	bEnableDeformation = Source.GetEnableDeformation();
	bEnableLockedBorders = Source.GetEnableLockedBorders();
	bEnableSoilCompaction = Source.GetEnableSoilCompaction();
}

UAGX_TerrainProperties* UAGX_TerrainProperties::CreateInstanceFromAsset(
	const UWorld* PlayingWorld, UAGX_TerrainProperties* Source)
{
	check(Source);
	check(!Source->IsInstance());
	check(PlayingWorld != nullptr);
	check(PlayingWorld->IsGameWorld());

	const FString InstanceName = Source->GetName() + "_Instance";

	auto NewInstance = NewObject<UAGX_TerrainProperties>(
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

	NativeBarrier.SetSoilParticleGrowthRate(SoilParticleGrowthRate);
	NativeBarrier.SetSoilParticleMergeRate(SoilParticleMergeRate);
	NativeBarrier.SetSoilParticleMergeThreshold(SoilParticleMergeThreshold);
	NativeBarrier.SetSoilMergeSpeedThreshold(SoilMergeSpeedThreshold);
	NativeBarrier.SetSoilParticleLifeTime(SoilParticleLifeTime);

	NativeBarrier.SetEnableAvalanching(bEnableAvalanching);
	NativeBarrier.SetAvalancheMaxHeightGrowth(AvalancheMaxHeightGrowth);
	NativeBarrier.SetAvalancheDecayFraction(AvalancheDecayFraction);
	NativeBarrier.SetAvalancheErrorThreshold(AvalancheErrorThreshold);

	NativeBarrier.SetActivationSpeed(ActivationSpeed);
	NativeBarrier.SetEnableDeformation(bEnableDeformation);
	NativeBarrier.SetEnableLockedBorders(bEnableLockedBorders);
	NativeBarrier.SetEnableSoilCompaction(bEnableSoilCompaction);
}

void UAGX_TerrainProperties::RegisterTerrainPager(AAGX_Terrain& Terrain)
{
	if (!IsInstance())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Terrain '%s' called Register Terrain Pager on non-instance Terrain "
				 "Properties '%s'."),
			*Terrain.GetName(), *GetPathName());
		return;
	}

	if (!Terrain.HasNativeTerrainPager())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Terrain '%s' called Register Terrain Pager on Terrain Properties '%s' but does "
				 "not have a Terrain Pager native ojbect."),
			*Terrain.GetName(), *GetPathName());
		return;
	}

	TerrainPagers.AddUnique(&Terrain);
}

void UAGX_TerrainProperties::UnregisterTerrainPager(AAGX_Terrain& Terrain)
{
	if (!IsInstance())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Terrain '%s' called Unregister Terrain Pager on non-instance Terrain "
				 "Properties '%s'."),
			*Terrain.GetName(), *GetPathName());
		return;
	}

	TerrainPagers.RemoveSwap(&Terrain);
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

	AGX_COMPONENT_DEFAULT_DISPATCHER_BOOL(CreateParticles);
	AGX_COMPONENT_DEFAULT_DISPATCHER_BOOL(DeleteParticlesOutsideBounds);
	AGX_COMPONENT_DEFAULT_DISPATCHER(PenetrationForceVelocityScaling);
	AGX_COMPONENT_DEFAULT_DISPATCHER(MaximumParticleActivationVolume);
	AGX_COMPONENT_DEFAULT_DISPATCHER(SoilParticleSizeScaling);

	AGX_COMPONENT_DEFAULT_DISPATCHER(SoilParticleGrowthRate);
	AGX_COMPONENT_DEFAULT_DISPATCHER(SoilParticleMergeRate);
	AGX_COMPONENT_DEFAULT_DISPATCHER(SoilParticleMergeThreshold);
	AGX_COMPONENT_DEFAULT_DISPATCHER(SoilMergeSpeedThreshold);
	AGX_COMPONENT_DEFAULT_DISPATCHER(SoilParticleLifeTime);

	AGX_COMPONENT_DEFAULT_DISPATCHER_BOOL(EnableAvalanching);
	AGX_COMPONENT_DEFAULT_DISPATCHER(AvalancheMaxHeightGrowth);
	AGX_COMPONENT_DEFAULT_DISPATCHER(AvalancheDecayFraction);
	AGX_COMPONENT_DEFAULT_DISPATCHER(AvalancheErrorThreshold);

	AGX_COMPONENT_DEFAULT_DISPATCHER(ActivationSpeed);
	AGX_COMPONENT_DEFAULT_DISPATCHER_BOOL(EnableDeformation);
	AGX_COMPONENT_DEFAULT_DISPATCHER_BOOL(EnableLockedBorders);
	AGX_COMPONENT_DEFAULT_DISPATCHER_BOOL(EnableSoilCompaction);
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

void UAGX_TerrainProperties::CallOnTemplateTerrainChangedOnAll()
{
	for (auto& Tp : TerrainPagers)
	{
		if (Tp == nullptr || !Tp->HasNativeTerrainPager())
			continue;

		Tp->GetNativeTerrainPager()->OnTemplateTerrainChanged();
	}
}
