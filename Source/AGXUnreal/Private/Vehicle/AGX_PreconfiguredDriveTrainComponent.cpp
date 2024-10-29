// Copyright 2024, Algoryx Simulation AB.

#include "Vehicle/AGX_PreconfiguredDriveTrainComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"

// Unreal Engine includes.
#include "CoreGlobals.h"
#include "GameFramework/Actor.h"

namespace AGX_PreconfiguredDriveTrainBarriers_helpers
{
	void SetLocalScope(UAGX_PreconfiguredDriveTrainComponent& DriveTrain)
	{
		AActor* LocalScope = DriveTrain.GetTypedOuter<AActor>();
		DriveTrain.FrontLeftHinge.SetLocalScope(LocalScope);
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

//
// End Native Owner interface.
//

void UAGX_PreconfiguredDriveTrainComponent::CreateNative()
{
	check(!HasNative());
	check(!GIsReconstructingBlueprintInstances);

	NativeBarriers.PowerLine.AllocateNative();
	NativeBarriers.CombustionEngine.AllocateNative(CombustionEngineParameters);
	NativeBarriers.PowerLine.Add(NativeBarriers.CombustionEngine);
	UpdateNativeProperties();
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
		CreateNative();
	}
}

//
// End Actor Component interface.
//

void UAGX_PreconfiguredDriveTrainComponent::AllocateNative()
{
}
