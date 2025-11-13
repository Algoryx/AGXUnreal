// Copyright 2025, Algoryx Simulation AB.

#include "Sensors/AGX_SensorComponentBase.h"

// AGX Dynamics for Unreal includes.
#include "AGX_AssetGetterSetterImpl.h"
#include "AGX_InternalDelegateAccessor.h"
#include "AGX_NativeOwnerInstanceData.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "AGX_Simulation.h"

UAGX_SensorComponentBase::UAGX_SensorComponentBase()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAGX_SensorComponentBase::SetEnabled(bool InEnabled)
{
	bEnabled = InEnabled;

	if (NativeBarrier != nullptr && NativeBarrier->HasNative())
		NativeBarrier->SetEnabled(InEnabled);
}

bool UAGX_SensorComponentBase::IsEnabled() const
{
	if (NativeBarrier != nullptr && NativeBarrier->HasNative())
		return NativeBarrier->GetEnabled();

	return bEnabled;
}

bool UAGX_SensorComponentBase::GetEnabled() const
{
	return IsEnabled();
}

void UAGX_SensorComponentBase::SetStepStride(int32 Stride)
{
	StepStride = Stride;

	if (NativeBarrier != nullptr && NativeBarrier->HasNative())
		NativeBarrier->SetStepStride(static_cast<uint32>(Stride));
}

int32 UAGX_SensorComponentBase::GetStepStride() const
{
	if (NativeBarrier != nullptr && NativeBarrier->HasNative())
		return static_cast<int32>(NativeBarrier->GetStepStride());

	return StepStride;
}

uint64 UAGX_SensorComponentBase::GetNativeAddress() const
{
	if (!HasNative())
		return 0;

	NativeBarrier->IncrementRefCount();
	return NativeBarrier->GetNativeAddress();
}

void UAGX_SensorComponentBase::SetNativeAddress(uint64 NativeAddress)
{
	check(!HasNative());
	NativeBarrier->SetNativeAddress(NativeAddress);
	NativeBarrier->DecrementRefCount();
}

bool UAGX_SensorComponentBase::HasNative() const
{
	return NativeBarrier != nullptr && NativeBarrier->HasNative();
}

FSensorBarrier* UAGX_SensorComponentBase::GetOrCreateNative()
{
	if (HasNative())
		return NativeBarrier.Get();

	return CreateNativeImpl();
}

FSensorBarrier* UAGX_SensorComponentBase::GetNative()
{
	if (!HasNative())
		return nullptr;

	return NativeBarrier.Get();
}

const FSensorBarrier* UAGX_SensorComponentBase::GetNative() const
{
	if (!HasNative())
		return nullptr;

	return NativeBarrier.Get();
}

void UAGX_SensorComponentBase::BeginPlay()
{
	Super::BeginPlay();
	if (!HasNative() && !GIsReconstructingBlueprintInstances)
		CreateNativeImpl();

	if (UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this))
	{
		PreStepForwardHandle =
			FAGX_InternalDelegateAccessor::GetOnPreStepForwardInternal(*Simulation)
				.AddLambda(
					[this](double)
					{
						// When using StepStride > 1, old Output data is still available in AGX, so
						// therefore we mark it as read in Pre so that the user gets no output
						// between strides.
						if (StepStride != 1)
							MarkOutputAsRead();
					});
	}
}

void UAGX_SensorComponentBase::EndPlay(const EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);

	if (HasNative() && Reason != EEndPlayReason::EndPlayInEditor &&
		Reason != EEndPlayReason::Quit && Reason != EEndPlayReason::LevelTransition)
	{
		if (UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this))
		{
			FAGX_InternalDelegateAccessor::GetOnPreStepForwardInternal(*Simulation)
				.Remove(PreStepForwardHandle);
		}
	}

	if (HasNative())
		NativeBarrier->ReleaseNative();
}

TStructOnScope<FActorComponentInstanceData> UAGX_SensorComponentBase::GetComponentInstanceData()
	const
{
	return MakeStructOnScope<FActorComponentInstanceData, FAGX_NativeOwnerInstanceData>(
		this, this,
		[](UActorComponent* Component)
		{
			ThisClass* AsThisClass = Cast<ThisClass>(Component);
			return static_cast<IAGX_NativeOwner*>(AsThisClass);
		});
}

#if WITH_EDITOR
void UAGX_SensorComponentBase::PostEditChangeChainProperty(FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);

	// If we are part of a Blueprint then this will trigger a RerunConstructionScript on the owning
	// Actor. That means that this object will be removed from the Actor and destroyed. We want to
	// apply all our changes before that so that they are carried over to the copy.
	Super::PostEditChangeChainProperty(Event);
}

void UAGX_SensorComponentBase::PostInitProperties()
{
	Super::PostInitProperties();
	InitPropertyDispatcher();
}

void UAGX_SensorComponentBase::InitPropertyDispatcher()
{
	FAGX_PropertyChangedDispatcher<ThisClass>& PropertyDispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (PropertyDispatcher.IsInitialized())
	{
		return;
	}

	AGX_COMPONENT_DEFAULT_DISPATCHER(StepStride);
	AGX_COMPONENT_DEFAULT_DISPATCHER_BOOL(Enabled);
}
#endif // WITH_EDITOR

void UAGX_SensorComponentBase::UpdateNativeProperties()
{
	AGX_CHECK(HasNative());
	NativeBarrier->SetEnabled(bEnabled);
	NativeBarrier->SetStepStride(static_cast<uint32>(StepStride));
}

FSensorBarrier* UAGX_SensorComponentBase::CreateNativeImpl()
{
	if (!NativeBarrier->HasStepStrideNative())
		NativeBarrier->AllocateStepStride();

	return GetNative();
}
