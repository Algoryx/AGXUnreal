// Copyright 2025, Algoryx Simulation AB.

#include "Wire/AGX_WireInstanceData.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Wire/AGX_WireComponent.h"

FAGX_WireInstanceData::FAGX_WireInstanceData(const UAGX_WireComponent* Wire)
	: FSceneComponentInstanceData(Wire)
{
	if (Wire == nullptr)
	{
		return;
	}

	NativeWireAddress = Wire->GetNativeAddress();
	NativeBeginWinchAddress = Wire->OwnedBeginWinch.GetNativeAddress();
	NativeEndWinchAddress = Wire->OwnedEndWinch.GetNativeAddress();
}

bool FAGX_WireInstanceData::ContainsData() const
{
	return Super::ContainsData() || HasNativeAddress();
}

void FAGX_WireInstanceData::ApplyToComponent(
	UActorComponent* Component, const ECacheApplyPhase CacheApplyPhase)
{
	FSceneComponentInstanceData::ApplyToComponent(Component, CacheApplyPhase);

	UAGX_WireComponent* Wire = Cast<UAGX_WireComponent>(Component);
	if (Wire == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("FAGX_WireInstanceData::ApplyToComponent called on something not a Wire "
				 "Component."));
		return;
	}

	if (Wire->GetNativeAddress() != NativeWireAddress)
	{
		Wire->SetNativeAddress(NativeWireAddress);
	}

	if (Wire->OwnedBeginWinch.GetNativeAddress() != NativeBeginWinchAddress)
	{
		Wire->OwnedBeginWinch.SetNativeAddress(NativeBeginWinchAddress);
	}

	if (Wire->OwnedEndWinch.GetNativeAddress() != NativeEndWinchAddress)
	{
		Wire->OwnedEndWinch.SetNativeAddress(NativeEndWinchAddress);
	}
}

void FAGX_WireInstanceData::AddReferencedObjects(FReferenceCollector& Collector)
{
	Super::AddReferencedObjects(Collector);

	/// @todo Do we need to do something here?
}

void FAGX_WireInstanceData::FindAndReplaceInstances(
	const TMap<UObject*, UObject*>& OldToNewInstanceMap)
{
	Super::FindAndReplaceInstances(OldToNewInstanceMap);

	/// @todo Do we need to do something here?
}

bool FAGX_WireInstanceData::HasNativeAddress() const
{
	return NativeWireAddress != 0 || NativeBeginWinchAddress != 0 || NativeEndWinchAddress != 0;
}
