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
#include "Utilities/AGX_StringUtilities.h"

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
void UAGX_ObserverFrameComponent::SetPosition(FVector Position)
{
	if (HasNative())
		NativeBarrier.SetPosition(Position);

	SetWorldLocation(Position);
}

FVector UAGX_ObserverFrameComponent::GetPosition() const
{
	if (HasNative())
		return NativeBarrier.GetPosition();

	return GetComponentLocation();
}

void UAGX_ObserverFrameComponent::SetLocalPosition(FVector Position)
{
	if (HasNative())
		NativeBarrier.SetLocalPosition(Position);

	SetRelativeLocation(Position);
}

FVector UAGX_ObserverFrameComponent::GetLocalPosition() const
{
	if (HasNative())
		return NativeBarrier.GetLocalPosition();

	return GetRelativeLocation();
}

void UAGX_ObserverFrameComponent::SetRotation(FQuat Rotation)
{
	if (HasNative())
		NativeBarrier.SetRotation(Rotation);

	SetWorldRotation(Rotation);
}

FQuat UAGX_ObserverFrameComponent::GetRotation() const
{
	if (HasNative())
		return NativeBarrier.GetRotation();

	return GetComponentQuat();
}

void UAGX_ObserverFrameComponent::SetLocalRotation(FQuat Rotation)
{
	if (HasNative())
		NativeBarrier.SetLocalRotation(Rotation);

	SetRelativeRotation(Rotation);
}

FQuat UAGX_ObserverFrameComponent::GetLocalRotation() const
{
	if (HasNative())
		return NativeBarrier.GetLocalRotation();

	return GetRelativeRotation().Quaternion();
}

void UAGX_ObserverFrameComponent::SetRotator(FRotator Rotator)
{
	SetRotation(Rotator.Quaternion());
}

FRotator UAGX_ObserverFrameComponent::GetRotator() const
{
	return GetRotation().Rotator();
}

void UAGX_ObserverFrameComponent::SetLocalRotator(FRotator Rotator)
{
	SetLocalRotation(Rotator.Quaternion());
}

FRotator UAGX_ObserverFrameComponent::GetLocalRotator() const
{
	return GetLocalRotation().Rotator();
}

FVector UAGX_ObserverFrameComponent::GetVelocity() const
{
	if (HasNative())
		return NativeBarrier.GetVelocity();

	return FVector::ZeroVector;
}

FVector UAGX_ObserverFrameComponent::GetAngularVelocity() const
{
	if (HasNative())
		return NativeBarrier.GetAngularVelocity();

	return FVector::ZeroVector;
}

FVector UAGX_ObserverFrameComponent::GetAcceleration() const
{
	if (HasNative())
		return NativeBarrier.GetAcceleration();

	return FVector::ZeroVector;
}

FVector UAGX_ObserverFrameComponent::GetAngularAcceleration() const
{
	if (HasNative())
		return NativeBarrier.GetAngularAcceleration();

	return FVector::ZeroVector;
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
	const FObserverFrameBarrier& Barrier, FAGX_ImportContext* Context)
{
	const FString CleanBarrierName =
		FAGX_ImportRuntimeUtilities::RemoveModelNameFromBarrierName(Barrier.GetName(), Context);
	const FString Name = FAGX_ObjectUtilities::SanitizeAndMakeNameUnique(
		GetOwner(), CleanBarrierName, UAGX_ObserverFrameComponent::StaticClass());
	Rename(*Name);

	SetRelativeLocation(Barrier.GetLocalPosition());
	SetRelativeRotation(Barrier.GetLocalRotation());
	ImportGuid = Barrier.GetGuid();
	ImportName = Barrier.GetName();

	if (Context != nullptr && Context->ObserverFrames != nullptr)
	{
		AGX_CHECK(!Context->ObserverFrames->Contains(ImportGuid));
		Context->ObserverFrames->Add(ImportGuid, this);
	}
}

FObserverFrameBarrier* UAGX_ObserverFrameComponent::GetOrCreateNative()
{
	if (!HasNative())
	{
		if (GIsReconstructingBlueprintInstances)
		{
			// We're in a very bad situation. Someone need this Component's native but if we're in
			// the middle of a RerunConstructionScripts and this Component haven't been given its
			// Native yet then there isn't much we can do. We can't create a new one since we will
			// be given the actual Native soon, but we also can't return the actual Native right now
			// because it hasn't been restored from the Component Instance Data yet.
			//
			// For now we simply die in non-shipping (checkNoEntry is active) so unit tests will
			// detect this situation, and log error and return nullptr otherwise, so that the
			// application can at least keep running. It is unlikely that the simulation will behave
			// as intended.
			checkNoEntry();
			UE_LOG(
				LogAGX, Error,
				TEXT("A request for the AGX Dynamics instance for Observer Frame '%s' in '%s' was "
					 "made "
					 "but we are in the middle of a Blueprint Reconstruction and the requested "
					 "instance has not yet been restored. The instance cannot be returned, which "
					 "may lead to incorrect scene configuration."),
				*GetName(), *GetLabelSafe(GetOwner()));
			return nullptr;
		}

		CreateNative();
	}

	return GetNative();
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

	UAGX_RigidBodyComponent* Body = GetRigidBody();
	if (Body == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("CreateNative called on Observer Frame '%s' in '%s', but no parent Rigid Body was "
				 "found. Ensure the Observer Frame is child of a Rigid Body Component. No native "
				 "AGX Dynamics Observer Frame will be created."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return;
	}

	FRigidBodyBarrier* BodyBarrier = Body->GetOrCreateNative();
	if (BodyBarrier == nullptr || !BodyBarrier->HasNative())
		return;

	NativeBarrier.AllocateNative(*BodyBarrier);
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

	NativeBarrier.SetLocalPosition(GetRelativeLocation());
	NativeBarrier.SetLocalRotation(GetRelativeRotation().Quaternion());
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
