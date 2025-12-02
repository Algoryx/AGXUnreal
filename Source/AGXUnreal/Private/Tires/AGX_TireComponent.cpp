// Copyright 2025, Algoryx Simulation AB.

#include "Tires/AGX_TireComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Simulation.h"
#include "AGX_LogCategory.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_StringUtilities.h"

UAGX_TireComponent::UAGX_TireComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UAGX_TireComponent::HasNative() const
{
	return NativeBarrier != nullptr && NativeBarrier->HasNative();
}

FTireBarrier* UAGX_TireComponent::GetOrCreateNative()
{
	if (!HasNative())
	{
		CreateNative();
	}
	return GetNative();
}

FTireBarrier* UAGX_TireComponent::GetNative()
{
	return HasNative() ? NativeBarrier.Get() : nullptr;
}

const FTireBarrier* UAGX_TireComponent::GetNative() const
{
	return HasNative() ? NativeBarrier.Get() : nullptr;
}

void UAGX_TireComponent::BeginPlay()
{
	Super::BeginPlay();
	if (!HasNative())
	{
		CreateNative();
	}
}

void UAGX_TireComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);

	if (GIsReconstructingBlueprintInstances)
	{
		// Another Tire will inherit this one's Native, so don't wreck it.
		// It's still safe to release the native since the Simulation will hold a reference if
		// necessary.
	}
	else if (
		HasNative() && Reason != EEndPlayReason::EndPlayInEditor &&
		Reason != EEndPlayReason::Quit && Reason != EEndPlayReason::LevelTransition)
	{
		if (UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this))
		{
			Simulation->Remove(*this);
		}
	}

	if (HasNative())
	{
		NativeBarrier->ReleaseNative();
	}
}

void UAGX_TireComponent::CreateNative()
{
	check(!HasNative());

	AllocateNative();

	if (!HasNative())
	{
		FAGX_NotificationUtilities::ShowNotification(
			FString::Printf(
				TEXT("Tire %s in %s: Unable to create Tire."), *GetFName().ToString(),
				*GetOwner()->GetName()),
			SNotificationItem::CS_Fail);
		return;
	}

	UpdateNativeProperties();
	UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this);
	if (Simulation == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Tire '%s' in '%s' tried to get Simulation, but UAGX_Simulation::GetFrom "
				 "returned nullptr."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return;
	}
	Simulation->Add(*this);
}
