#include "Wire/AGX_WireWinchComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_UpropertyDispatcher.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_NativeOwnerInstanceData.h"
#include "Wire/AGX_WireUtilities.h"
#include "Wire/WireBarrier.h"

// Unreal Engine includes.
#include "CoreGlobals.h"

FAGX_WireWinchRef UAGX_WireWinchComponent::GetWinch_BP()
{
	return {&WireWinch};
}

FVector UAGX_WireWinchComponent::ComputeBodyRelativeLocation()
{
	FVector WorldLocation = GetComponentTransform().TransformPosition(WireWinch.Location);
	if (UAGX_RigidBodyComponent* Body = WireWinch.GetBodyAttachment())
	{
		return Body->GetComponentTransform().InverseTransformPosition(WorldLocation);
	}
	else
	{
		return WorldLocation;
	}
}

FRotator UAGX_WireWinchComponent::ComputeBodyRelativeRotation()
{
	FQuat WorldRotation =
		GetComponentTransform().TransformRotation(WireWinch.Rotation.Quaternion());

	if (UAGX_RigidBodyComponent* Body = WireWinch.GetBodyAttachment())
	{
		return Body->GetComponentTransform().InverseTransformRotation(WorldRotation).Rotator();
	}
	else
	{
		return WorldRotation.Rotator();
	}
}

bool UAGX_WireWinchComponent::HasNative() const
{
	return WireWinch.HasNative();
}

uint64 UAGX_WireWinchComponent::GetNativeAddress() const
{
	return static_cast<uint64>(WireWinch.GetNativeAddress());
}

void UAGX_WireWinchComponent::SetNativeAddress(uint64 NativeAddress)
{
	check(!HasNative());
	WireWinch.SetNativeAddress(static_cast<uintptr_t>(NativeAddress));
}

void UAGX_WireWinchComponent::BeginPlay()
{
	Super::BeginPlay();
	if (!HasNative() && !GIsReconstructingBlueprintInstances)
	{
		// Do not create a native AGX Dynamics object if GIsReconstructingBlueprintInstances is set.
		// That means that we're being created as part of a Blueprint Reconstruction and we will
		// soon be assigned the native that the reconstructed Wire Winch Component had, if any.
		CreateNative();
		check(HasNative()); /// @todo Consider better error handling that check.
	}
}

void UAGX_WireWinchComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);
	if (GIsReconstructingBlueprintInstances)
	{
		// Nothing to do in this case since another Wire Winch will inherit this one's Native as
		// part of the Blueprint Reconstruction. The AGX Dynamics object should remain in the
		// simulation.
	}
	else
	{
/// @todo Enable this once WireWinchBarrier::GetWire has been implemented.
#if 0
		// Normally we would remove the corresponding AGX Dynamics object from the simulation
		// here, but Wire Winches aren't simply added to the simulation like most objects. Instead
		// they are added to a Wire. So the best we can do is to remove it from the wire.
		//
		// This won't update the Begin/End Winch Type UProperty of the Unreal Engine representation
		// of the wire. I don't even know how we would find the Wire Component in question.
		if (HasNative())
		{
			FWireBarrier Wire = GetNative()->GetWire();
			if (Wire.HasNative())
			{
				Wire.Detach(GetNative());
			}
		}
#endif
	}

	// This Wire Winch Component is no longer the owner of the AGX Dynamics object.
	WireWinch.NativeBarrier.ReleaseNative();
}

TStructOnScope<FActorComponentInstanceData> UAGX_WireWinchComponent::GetComponentInstanceData()
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

void UAGX_WireWinchComponent::PostLoad()
{
	Super::PostLoad();
#if WITH_EDITOR
	FAGX_UpropertyDispatcher<ThisClass>& Dispatcher = FAGX_UpropertyDispatcher<ThisClass>::Get();
	if (Dispatcher.IsInitialized())
	{
		return;
	}

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_WireWinchComponent, WireWinch),
		GET_MEMBER_NAME_CHECKED(FAGX_WireWinch, PulledInLength),
		[](ThisClass* Winch)
		{ Winch->WireWinch.SetPulledInLength(Winch->WireWinch.PulledInLength); });

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_WireWinchComponent, WireWinch),
		GET_MEMBER_NAME_CHECKED(FAGX_WireWinch, bMotorEnabled),
		[](ThisClass* Winch) { Winch->WireWinch.SetMotorEnabled(Winch->WireWinch.bMotorEnabled); });

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_WireWinchComponent, WireWinch),
		GET_MEMBER_NAME_CHECKED(FAGX_WireWinch, TargetSpeed),
		[](ThisClass* Winch) { Winch->WireWinch.SetTargetSpeed(Winch->WireWinch.TargetSpeed); });

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_WireWinchComponent, WireWinch),
		GET_MEMBER_NAME_CHECKED(FAGX_WireWinch, MotorForceRange),
		[](ThisClass* Winch)
		{ Winch->WireWinch.SetMotorForceRange(Winch->WireWinch.MotorForceRange); });

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_WireWinchComponent, WireWinch),
		GET_MEMBER_NAME_CHECKED(FAGX_WireWinch, bBrakeEnabled),
		[](ThisClass* Winch) { Winch->WireWinch.SetBrakeEnabled(Winch->WireWinch.bBrakeEnabled); });

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_WireWinchComponent, WireWinch),
		GET_MEMBER_NAME_CHECKED(FAGX_WireWinch, BrakeForceRange),
		[](ThisClass* Winch)
		{ Winch->WireWinch.SetBrakeForceRange(Winch->WireWinch.BrakeForceRange); });
#endif
}

void UAGX_WireWinchComponent::PostInitProperties()
{
	Super::PostInitProperties();
	WireWinch.BodyAttachment.OwningActor = GetTypedOuter<AActor>();
}

#if WITH_EDITOR
void UAGX_WireWinchComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	FAGX_UpropertyDispatcher<ThisClass>::Get().Trigger(PropertyChangedEvent, this);

	// If we are part of a Blueprint then this will trigger a RerunConstructionScript on the owning
	// Actor. That means that his object will be removed from the Actor and destroyed. We want to
	// apply all our changes before that so that they are carried over to the copy.
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void UAGX_WireWinchComponent::PostEditChangeChainProperty(
	struct FPropertyChangedChainEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.PropertyChain.Num() > 2)
	{
		// The cases fewer chain elements are handled by PostEditChangeProperty, which is called by
		// UObject's PostEditChangeChainProperty.
		FAGX_UpropertyDispatcher<ThisClass>::Get().Trigger(PropertyChangedEvent, this);
	}

	// If we are part of a Blueprint then this will trigger a RerunConstructionScript on the owning
	// Actor. That means that his object will be removed from the Actor and destroyed. We want to
	// apply all our changes before that so that they are carried over to the copy.
	Super::PostEditChangeChainProperty(PropertyChangedEvent);
}
#endif

FWireWinchBarrier* UAGX_WireWinchComponent::GetNative()
{
	return WireWinch.GetNative();
}

const FWireWinchBarrier* UAGX_WireWinchComponent::GetNative() const
{
	return WireWinch.GetNative();
}

FWireWinchBarrier* UAGX_WireWinchComponent::GetOrCreateNative()
{
	if (HasNative())
	{
		return GetNative();
	}

	checkf(
		!GIsReconstructingBlueprintInstances,
		TEXT("Native instances should never be created while a Blueprint Reconstruction is in "
			 "progress, the instances should be inherited via a Actor Component Instance Data."));
	CreateNative();
	checkf(HasNative(), TEXT("Failed to create a native Wire Winch instance."));
	return GetNative();
}

void UAGX_WireWinchComponent::CreateNative()
{
	checkf(
		!HasNative(),
		TEXT("Create Native called on a Wire Winch Component that already has a native."));
	checkf(
		!GIsReconstructingBlueprintInstances,
		TEXT("Create Native called on a Wire Winch Component while a Blueprint Reconstruction is "
			 "in progress, the instances should be inherited via a Actor Component Instance Data"));

	FAGX_WireUtilities::ComputeSimulationPlacement(*this, WireWinch);
	WireWinch.CreateNative();
}