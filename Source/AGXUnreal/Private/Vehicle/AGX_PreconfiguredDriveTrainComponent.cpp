// Copyright 2024, Algoryx Simulation AB.

#include "Vehicle/AGX_PreconfiguredDriveTrainComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_Simulation.h"
#include "Utilities/AGX_ObjectUtilities.h"
#include "Utilities/AGX_StringUtilities.h"

// Unreal Engine includes.
#include "CoreGlobals.h"
#include "GameFramework/Actor.h"

namespace AGX_PreconfiguredDriveTrainBarriers_helpers
{
	// Update the Local Scope of all Component References in the drive-train.
	void SetLocalScope(UAGX_PreconfiguredDriveTrainComponent& DriveTrain)
	{
		AActor* LocalScope = FAGX_ComponentReference::FindLocalScope(DriveTrain);
		DriveTrain.FrontLeftHinge.SetLocalScope(LocalScope);
		DriveTrain.FrontRightHinge.SetLocalScope(LocalScope);
	}
}

UAGX_PreconfiguredDriveTrainComponent::UAGX_PreconfiguredDriveTrainComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	AGX_PreconfiguredDriveTrainBarriers_helpers::SetLocalScope(*this);
}

void UAGX_PreconfiguredDriveTrainComponent::SetThrottle(double InThrottle)
{
	if (HasNativeCombustionEngine())
	{
		NativeBarriers.CombustionEngine.SetThrottle(InThrottle);
	}
	Throttle = InThrottle;
}

double UAGX_PreconfiguredDriveTrainComponent::GetThrottle() const
{
	if (HasNativeCombustionEngine())
	{
		return NativeBarriers.CombustionEngine.GetThrottle();
	}
	else
	{
		return Throttle;
	}
}

//
// Begin Native Owner interface.
//

void UAGX_PreconfiguredDriveTrainComponent::UpdateNativeProperties()
{
	if (HasNativeCombustionEngine())
	{
		NativeBarriers.CombustionEngine.SetCombustionEngineParameters(CombustionEngineParameters);
		NativeBarriers.CombustionEngine.SetThrottle(Throttle);
	}
}

bool UAGX_PreconfiguredDriveTrainComponent::HasNative() const
{
	return NativeBarriers.PowerLine.HasNative();
}

uint64 UAGX_PreconfiguredDriveTrainComponent::GetNativeAddress() const
{
	return static_cast<uint64>(NativeBarriers.PowerLine.GetNativeAddress());
}

void UAGX_PreconfiguredDriveTrainComponent::SetNativeAddress(uint64 NativeAddress)
{
	check(!HasNative());
	NativeBarriers.PowerLine.SetNativeAddress(static_cast<uintptr_t>(NativeAddress));
}

FPowerLineBarrier* UAGX_PreconfiguredDriveTrainComponent::GetNative()
{
	if (!HasNative())
	{
		return nullptr;
	}
	return &NativeBarriers.PowerLine;
}

const FPowerLineBarrier* UAGX_PreconfiguredDriveTrainComponent::GetNative() const
{
	if (!HasNative())
	{
		return nullptr;
	}
	return &NativeBarriers.PowerLine;
}

//
// End Native Owner interface.
//

FPowerLineBarrier* UAGX_PreconfiguredDriveTrainComponent::GetOrCreateNative()
{
	if (!HasNative())
	{
		CreateNative();
	}
	return GetNative();
}

bool UAGX_PreconfiguredDriveTrainComponent::HasNativeCombustionEngine() const
{
	return NativeBarriers.CombustionEngine.HasNative();
}

//
// Begin Actor Component interface.
//

void UAGX_PreconfiguredDriveTrainComponent::OnRegister()
{
	Super::OnRegister();
	AGX_PreconfiguredDriveTrainBarriers_helpers::SetLocalScope(*this);
}

void UAGX_PreconfiguredDriveTrainComponent::BeginPlay()
{
	Super::BeginPlay();
	if (!HasNative() && !GIsReconstructingBlueprintInstances)
	{
		GetOrCreateNative();
	}
}

//
// End Actor Component interface.
//

bool UAGX_PreconfiguredDriveTrainComponent::CreateNative()
{
	check(!HasNative());
	check(!GIsReconstructingBlueprintInstances);

	NativeBarriers.PowerLine.AllocateNative();
	if (!NativeBarriers.PowerLine.HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Preconfigured Drive Train Component could not create native power-line. Aborting "
				 "drive-train initialization."));
		return false;
	}

	NativeBarriers.CombustionEngine.AllocateNative(CombustionEngineParameters);
	if (HasNativeCombustionEngine())
	{
		NativeBarriers.PowerLine.Add(NativeBarriers.CombustionEngine);
	}
	else
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Preconfigured Drive-Train '%s' in '%s' tried to allocate native AGX Dynamics "
				 "Combustion Engine, but the allocation failed."),
			*GetName(), *GetLabelSafe(GetOwner()));
	}
	UpdateNativeProperties();

	UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this);
	if (!Simulation)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Preconfigured Drive-Train '%s' in '%s' tried to get Simulation, but "
				 "UAGX_Simulation::GetFrom returned nullptr."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return false;
	}

	if (!Simulation->HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT(
				"Preconfigured Drive-Train '%s' in '%s' tried to add itself to the Simulation, but "
				"UAGX_Simulation::GetFrom returned a Simulation without a native AGX Dynamics "
				"representation."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return false;
	}

	if (!NativeBarriers.PowerLine.AddTo(*Simulation->GetNative()))
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Preconfigured Drive-Train '%s' in '%s' tried to add itself to the Simulation, "
				 "but the Add call failed on the AGX Dynamics side."));
		return false;
	}

	return true;
}
