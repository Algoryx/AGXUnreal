// Copyright 2025, Algoryx Simulation AB.

#include "AGX_ObserverFrameComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_AssetGetterSetterImpl.h"
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "AGX_NativeOwnerInstanceData.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_Simulation.h"
#include "Import/AGX_ImportContext.h"
#include "Import/SimulationObjectCollection.h"
#include "Utilities/AGX_ImportRuntimeUtilities.h"
#include "Utilities/AGX_ObjectUtilities.h"

void UAGX_ObserverFrameComponent::SetEnabled(bool InEnabled)
{
	if (HasNative())
	{
		NativeBarrier.SetEnabled(InEnabled);
	}

	bEnabled = InEnabled;
}

bool UAGX_ObserverFrameComponent::IsEnabled() const
{
	if (HasNative())
	{
		return NativeBarrier.GetEnabled();
	}

	return bEnabled;
}

bool UAGX_ObserverFrameComponent::GetEnabled() const
{
	return IsEnabled();
}

UAGX_RigidBodyComponent* UAGX_ObserverFrameComponent::GetRigidBody() const
{
	return FAGX_ObjectUtilities::FindFirstAncestorOfType<UAGX_RigidBodyComponent>(*this);
}

void UAGX_ObserverFrameComponent::CopyFrom(
	const FObserverFrameData& Data, FAGX_ImportContext* Context)
{
	const FString CleanBarrierName =
		FAGX_ImportRuntimeUtilities::RemoveModelNameFromBarrierName(Data.Name, Context);
	const FString Name = FAGX_ObjectUtilities::SanitizeAndMakeNameUnique(
		GetOwner(), CleanBarrierName, UAGX_ObserverFrameComponent::StaticClass());
	Rename(*Name);

	SetRelativeTransform(Data.Transform);
	ImportGuid = Data.ObserverGuid;

	if (Context != nullptr && Context->ObserverFrames != nullptr)
	{
		AGX_CHECK(!Context->ObserverFrames->Contains(ImportGuid));
		Context->ObserverFrames->Add(ImportGuid, this);
	}
}

FObserverFrameBarrier* UAGX_ObserverFrameComponent::GetNative()
{
	if (!HasNative())
		return nullptr;

	return &NativeBarrier;
}

const FObserverFrameBarrier* UAGX_ObserverFrameComponent::GetNative() const
{
	if (!HasNative())
		return nullptr;

	return &NativeBarrier;
}

bool UAGX_ObserverFrameComponent::HasNative() const
{
	return NativeBarrier.HasNative();
}

uint64 UAGX_ObserverFrameComponent::GetNativeAddress() const
{
	return static_cast<uint64>(NativeBarrier.GetNativeAddress());
}

void UAGX_ObserverFrameComponent::SetNativeAddress(uint64 NativeAddress)
{
	check(!HasNative());
	NativeBarrier.SetNativeAddress(static_cast<uintptr_t>(NativeAddress));
}

void UAGX_ObserverFrameComponent::BeginPlay()
{
	Super::BeginPlay();
	if (!HasNative() && !GIsReconstructingBlueprintInstances)
		CreateNative();
}

void UAGX_ObserverFrameComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);
	if (GIsReconstructingBlueprintInstances)
	{
		// Another UAGX_ObserverFrameComponent will inherit this one's Native, so don't wreck it.
		// The call to NativeBarrier.ReleaseNative below is safe because the AGX Dynamics Simulation
		// will retain a reference counted pointer to the AGX Dynamics Observer Frame.
	}
	else if (
		HasNative() && Reason != EEndPlayReason::EndPlayInEditor &&
		Reason != EEndPlayReason::Quit && Reason != EEndPlayReason::LevelTransition)
	{
		if (UAGX_Simulation* Sim = UAGX_Simulation::GetFrom(this))
			Sim->Remove(*this);
	}

	if (HasNative())
		NativeBarrier.ReleaseNative();
}

TStructOnScope<FActorComponentInstanceData> UAGX_ObserverFrameComponent::GetComponentInstanceData()
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

void UAGX_ObserverFrameComponent::CreateNative()
{
	if (HasNative())
		return;

	check(!GIsReconstructingBlueprintInstances);
	NativeBarrier.AllocateNative();
	check(HasNative());

	UpdateNativeProperties();

	UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this);
	if (Simulation == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Observer Frame Component '%s' in '%s' tried to get Simulation, but "
				 "UAGX_Simulation::GetFrom "
				 "returned nullptr."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return;
	}

	Simulation->Add(*this);
}

#if WITH_EDITOR
void UAGX_ObserverFrameComponent::InitPropertyDispatcher()
{
	FAGX_PropertyChangedDispatcher<ThisClass>& PropertyDispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (PropertyDispatcher.IsInitialized())
		return;

	AGX_COMPONENT_DEFAULT_DISPATCHER_BOOL(Enabled);
}
#endif // WITH_EDITOR

void UAGX_ObserverFrameComponent::UpdateNativeProperties()
{
	if (!HasNative())
		return;

	NativeBarrier.SetEnabled(bEnabled);
	NativeBarrier.SetName(!ImportName.IsEmpty() ? ImportName : GetName());
}

#if WITH_EDITOR
void UAGX_ObserverFrameComponent::PostInitProperties()
{
	Super::PostInitProperties();
	InitPropertyDispatcher();
}

void UAGX_ObserverFrameComponent::PostEditChangeChainProperty(FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);
	Super::PostEditChangeChainProperty(Event);
}
#endif // WITH_EDITOR
