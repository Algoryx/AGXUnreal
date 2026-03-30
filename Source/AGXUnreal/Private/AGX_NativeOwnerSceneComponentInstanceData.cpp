// Copyright 2026, Algoryx Simulation AB.

#include "AGX_NativeOwnerSceneComponentInstanceData.h"

// AGX Dynamics for Unreal includes.
#include "AGX_NativeOwner.h"
#include "AGX_LogCategory.h"

FAGX_NativeOwnerSceneComponentInstanceData::FAGX_NativeOwnerSceneComponentInstanceData(
	const IAGX_NativeOwner* NativeOwner, const USceneComponent* SourceComponent,
	TFunction<IAGX_NativeOwner*(UActorComponent*)> InDowncaster)
	: FSceneComponentInstanceData(SourceComponent)
	, Downcaster(InDowncaster)
{
	NativeAddress = NativeOwner->GetNativeAddress();
}

void FAGX_NativeOwnerSceneComponentInstanceData::ApplyToComponent(
	UActorComponent* Component, const ECacheApplyPhase CacheApplyPhase)
{
	FSceneComponentInstanceData::ApplyToComponent(Component, CacheApplyPhase);

	IAGX_NativeOwner* NativeOwner = Downcaster(Component);
	if (NativeOwner == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("FAGX_NativeOwnerSceneComponentInstanceData::ApplyToComponent called on something not a "
				 "IAGX_NativeOwner. This is a bug. The created Component may malfunction."));
		return;
	}

	// Unreal Engine calls ApplyToComponent twice, so detect that and do nothing the second time.
	// But be aware that the first call happens before deserialization and the latter happens after,
	// so if you need the actual Property values then wait for the second call.
	if (NativeOwner->GetNativeAddress() == NativeAddress)
	{
		return;
	}

	NativeOwner->SetNativeAddress(NativeAddress);
}

bool FAGX_NativeOwnerSceneComponentInstanceData::ContainsData() const
{
	// Extend with more tests once we store more data.
	return Super::ContainsData() || HasNativeAddress();
}

void FAGX_NativeOwnerSceneComponentInstanceData::AddReferencedObjects(FReferenceCollector& Collector)
{
	Super::AddReferencedObjects(Collector);

	/// \todo Do we need to do something here? What about Constraints outside of this Blueprint that
	/// point here?
}

void FAGX_NativeOwnerSceneComponentInstanceData::FindAndReplaceInstances(
	const TMap<UObject*, UObject*>& OldToNewInstanceMap)
{
	Super::FindAndReplaceInstances(OldToNewInstanceMap);

	/// \todo Do we need to do something here? What about Constraints outside of this Blueprint that
	/// point here?
}

bool FAGX_NativeOwnerSceneComponentInstanceData::HasNativeAddress() const
{
	return NativeAddress != 0;
}
