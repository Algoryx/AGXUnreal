// Copyright 2024, Algoryx Simulation AB.

#include "AGX_RigidBodyComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "AGX_NativeOwnerInstanceData.h"
#include "AGX_Simulation.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "AMOR/MergeSplitPropertiesBarrier.h"
#include "Shapes/AGX_ShapeComponent.h"
#include "Utilities/AGX_ObjectUtilities.h"
#include "Utilities/AGX_StringUtilities.h"

// Unreal Engine includes.
#include "Engine/GameInstance.h"
#include "CoreGlobals.h"
#include "GameFramework/Actor.h"
#include "Math/Rotator.h"
#include "Math/Quat.h"
#include "Misc/EngineVersionComparison.h"

// Sets default values for this component's properties
UAGX_RigidBodyComponent::UAGX_RigidBodyComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	// We step the AGX Dynamics simulation in PrePhysics and read the new state
	// in PostPhysics. This doesn't really have anything to do with the PhysX
	// stepping, which is what the Physics tick groups really refer to, but the
	// names are instructive so we'll use them.
	PrimaryComponentTick.TickGroup = TG_PostPhysics;

	Mass = 1.0f;
	CenterOfMassOffset = FVector(0.0f, 0.0f, 0.0f);
	PrincipalInertia = FVector(1.f, 1.f, 1.f);
	MotionControl = EAGX_MotionControl::MC_DYNAMICS;
	TransformTarget = EAGX_TransformTarget::TT_SELF;
}

#if WITH_EDITOR

void UAGX_RigidBodyComponent::PostInitProperties()
{
	Super::PostInitProperties();
	InitPropertyDispatcher();
}

void UAGX_RigidBodyComponent::PostEditChangeChainProperty(FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);

	// If we are part of a Blueprint then this will trigger a RerunConstructionScript on the owning
	// Actor. That means that this object will be removed from the Actor and destroyed. We want to
	// apply all our changes before that so that they are carried over to the copy.
	Super::PostEditChangeChainProperty(Event);
}

void UAGX_RigidBodyComponent::PostEditComponentMove(bool bFinished)
{
	Super::PostEditComponentMove(bFinished);

	if (!NativeBarrier.HasNative())
	{
		return;
	}

	// Not using the Set-functions here because we aren't editing a raw Property and don't want
	// to trigger a bunch of Unreal Engine code since we currently are in an Unreal Engine callback.
	// So go straight to the Barrier.
	WriteTransformToNative();
}

void UAGX_RigidBodyComponent::OnChildDetached(USceneComponent* Child)
{
	Super::OnChildDetached(Child);

	if (!HasNative())
	{
		return;
	}

	// @todo This does not get triggered if a child deeper down in the hierarchy is detached.
	// This means that Shapes detached from this Rigid Body that are not direct children will not
	// be removed from the Rigid Body.
	// An alternative would be to use OnAttachmentChanged() in the Shape Component, but that
	// function has the drawback of not giving information about the old parent, so the Shape cannot
	// easily notify the previous parent Rigid Body that it has been detached from it, simply
	// because it cannot (easily) know which Rigid Body that was.
	if (UAGX_ShapeComponent* Shape = Cast<UAGX_ShapeComponent>(Child))
	{
		if (Shape->HasNative())
		{
			NativeBarrier.RemoveShape(Shape->GetNative());
		}
	}
}

void UAGX_RigidBodyComponent::OnChildAttached(USceneComponent* Child)
{
	Super::OnChildAttached(Child);

	if (!HasNative())
	{
		return;
	}

	// @todo This does not get triggered if a child deeper down in the hierarchy is attached.
	// See comment in UAGX_RigidBodyComponent::OnChildDetached.
	if (UAGX_ShapeComponent* Shape = Cast<UAGX_ShapeComponent>(Child))
	{
		if (Shape->HasNative())
		{
			NativeBarrier.AddShape(Shape->GetNative());
		}
	}
}

void UAGX_RigidBodyComponent::InitPropertyDispatcher()
{
	FAGX_PropertyChangedDispatcher<ThisClass>& PropertyDispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (PropertyDispatcher.IsInitialized())
	{
		return;
	}

	// Location and Rotation are not Properties, so they won't trigger PostEditChangeProperty. e.g.,
	// when moving the Component using the Widget in the Level Viewport. They are instead handled in
	// PostEditComponentMove. The local transformations, however, the ones at the top of the Details
	// Panel, are properties and do end up here.
	PropertyDispatcher.Add(
		this->GetRelativeLocationPropertyName(),
		[](ThisClass* This) { This->TryWriteTransformToNative(); });

	PropertyDispatcher.Add(
		this->GetRelativeRotationPropertyName(),
		[](ThisClass* This) { This->TryWriteTransformToNative(); });

	PropertyDispatcher.Add(
		this->GetAbsoluteLocationPropertyName(),
		[](ThisClass* This) { This->TryWriteTransformToNative(); });

	PropertyDispatcher.Add(
		this->GetAbsoluteRotationPropertyName(),
		[](ThisClass* This) { This->TryWriteTransformToNative(); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_RigidBodyComponent, bEnabled),
		[](ThisClass* This) { This->SetEnabled(This->bEnabled); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_RigidBodyComponent, Mass),
		[](ThisClass* This) { This->SetMass(This->Mass); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_RigidBodyComponent, bAutoGenerateMass),
		[](ThisClass* This) { This->SetAutoGenerateMass(This->bAutoGenerateMass); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_RigidBodyComponent, CenterOfMassOffset),
		[](ThisClass* This) { This->SetCenterOfMassOffset(This->CenterOfMassOffset); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_RigidBodyComponent, bAutoGenerateCenterOfMassOffset),
		[](ThisClass* This)
		{ This->SetAutoGenerateCenterOfMassOffset(This->bAutoGenerateCenterOfMassOffset); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_RigidBodyComponent, PrincipalInertia),
		[](ThisClass* This) { This->SetPrincipalInertia(This->PrincipalInertia); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_RigidBodyComponent, bAutoGeneratePrincipalInertia),
		[](ThisClass* This)
		{ This->SetAutoGeneratePrincipalInertia(This->bAutoGeneratePrincipalInertia); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_RigidBodyComponent, Velocity),
		[](ThisClass* This) { This->SetVelocity(This->Velocity); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_RigidBodyComponent, AngularVelocity),
		[](ThisClass* This) { This->SetAngularVelocity(This->AngularVelocity); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_RigidBodyComponent, LinearVelocityDamping),
		[](ThisClass* This) { This->SetLinearVelocityDamping(This->LinearVelocityDamping); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_RigidBodyComponent, AngularVelocityDamping),
		[](ThisClass* This) { This->SetAngularVelocityDamping(This->AngularVelocityDamping); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_RigidBodyComponent, MotionControl),
		[](ThisClass* This) { This->SetMotionControl(This->MotionControl); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_RigidBodyComponent, MergeSplitProperties),
		[](ThisClass* This) { This->MergeSplitProperties.OnPostEditChangeProperty(*This); });

/// @todo Enable once we get UAGX_RigidBodyComponent::SetTransformTarget.
#if 0
	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_RigidBodyComponent, TransformTarget),
		[](ThisClass* This) { This->SetTransformTarget(This->TransformTarget); });
#endif
}

#endif

FRigidBodyBarrier* UAGX_RigidBodyComponent::GetOrCreateNative()
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
				TEXT("A request for the AGX Dynamics instance for Rigid Body '%s' in '%s' was made "
					 "but we are in the middle of a Blueprint Reconstruction and the requested "
					 "instance has not yet been restored. The instance cannot be returned, which "
					 "may lead to incorrect scene configuration."),
				*GetName(), *GetLabelSafe(GetOwner()));
			return nullptr;
		}

		InitializeNative();
	}
	check(HasNative()); /// \todo Consider better error handling than 'check'.
	return &NativeBarrier;
}

bool UAGX_RigidBodyComponent::HasNative() const
{
	return NativeBarrier.HasNative();
}

uint64 UAGX_RigidBodyComponent::GetNativeAddress() const
{
	return static_cast<uint64>(NativeBarrier.GetNativeAddress());
}

void UAGX_RigidBodyComponent::SetNativeAddress(uint64 NativeAddress)
{
	check(!HasNative());
	NativeBarrier.SetNativeAddress(static_cast<uintptr_t>(NativeAddress));

	if (HasNative())
	{
		MergeSplitProperties.BindBarrierToOwner(*GetNative());
	}
}

FRigidBodyBarrier* UAGX_RigidBodyComponent::GetNative()
{
	if (!HasNative())
	{
		return nullptr;
	}
	return &NativeBarrier;
}

const FRigidBodyBarrier* UAGX_RigidBodyComponent::GetNative() const
{
	if (!HasNative())
	{
		return nullptr;
	}
	return &NativeBarrier;
}

void UAGX_RigidBodyComponent::BeginPlay()
{
	Super::BeginPlay();
	if (!HasNative() && !GIsReconstructingBlueprintInstances)
	{
		// Not initializing a new Native if currently reconstructing Blueprint instances because
		// if we should have a Native then one will be assigned to us by our Component Instance
		// Data.
		InitializeNative();
		check(HasNative()); /// \todo Consider better error handling than 'check'.

		MergeSplitProperties.OnBeginPlay(*this);
	}
}

/// \todo Split the UAGX_RigidBodyComponent::TickComponent callback into two
///       parts. One in PrePhysics that reads the Unreal state to AGX Dynamics
///       and one in PostPhysics that read the AGX Dynamics state to Unreal.
///       Read about tick splitting under Advanced Ticking Functionality at
///       https://docs.unrealengine.com/en-US/Programming/UnrealArchitecture/Actors/Ticking/index.html
///
///      Take care to synchronize this with the actual AGX Dynamics stepping
///      done by UAGX_Simulation, they must not happen concurrently.
void UAGX_RigidBodyComponent::TickComponent(
	float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (MotionControl != MC_STATIC)
	{
		// ReadTransformFromNative may trigger user callbacks, e.g. On Begin Overlap, which may
		// remove this Rigid Body from the simulation.
		ReadTransformFromNative();
		if (HasNative())
		{
			Velocity = NativeBarrier.GetVelocity();
			AngularVelocity = NativeBarrier.GetAngularVelocity();
		}
	}
}

void UAGX_RigidBodyComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);

	if (GIsReconstructingBlueprintInstances)
	{
		// Another UAGX_RigidBodyComponent will inherit this one's Native, so don't wreck it.
		// The call to NativeBarrier.ReleaseNative below is safe because the AGX Dynamics Simulation
		// will retain a reference counted pointer to the AGX Dynamics Rigid Body.
		//
		// But what if the Rigid Body isn't currently part of any Simulation? Can we guarantee that
		// something will keep the Rigid Body instance alive? Should we do explicit incref/decref
		// on the Rigid Body in GetNativeAddress / SetNativeAddress?
	}
	else if (
		HasNative() && Reason != EEndPlayReason::EndPlayInEditor &&
		Reason != EEndPlayReason::Quit && Reason != EEndPlayReason::LevelTransition)
	{
		if (UAGX_Simulation* Sim = UAGX_Simulation::GetFrom(this))
		{
			Sim->Remove(*this);
		}
	}

	if (HasNative())
	{
		NativeBarrier.ReleaseNative();
	}
}

void UAGX_RigidBodyComponent::CreateMergeSplitProperties()
{
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("UAGX_RigidBodyComponent::CreateMergeSplitProperties was called on Rigid Body "
				 "'%s' that does not have a Native AGX Dynamics object. Only call this function "
				 "during play."),
			*GetName());
		return;
	}

	if (!MergeSplitProperties.HasNative())
	{
		MergeSplitProperties.CreateNative(*this);
	}
}

bool UAGX_RigidBodyComponent::IsAutomaticallyMerged()
{
	if (!HasNative())
		return false;

	return NativeBarrier.IsAutomaticallyMerged();
}

bool UAGX_RigidBodyComponent::Split()
{
	if (!HasNative())
		return false;

	if (!MergeSplitProperties.HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("UAGX_RigidBodyComponent::Split was called on Rigid Body "
				 "'%s' that does not have Merge Split Properties. Either activate Merge/Split from "
				 "the Details Panel or call the CreateMergeSplitProperties function prior to "
				 "calling this function."),
			*GetName());
		return false;
	}

	return NativeBarrier.Split();
}

namespace
{
	// Searches recursively, but does not search further if a child component is another Rigid Body
	// Component.
	void GetShapesRecursive(
		const USceneComponent& Parent, TArray<UAGX_ShapeComponent*>& OutFoundShapes)
	{
		TArray<USceneComponent*> AttachChildren = Parent.GetAttachChildren();
		OutFoundShapes.Append(FAGX_ObjectUtilities::Filter<UAGX_ShapeComponent>(AttachChildren));

		for (const auto C : AttachChildren)
		{
			if (Cast<UAGX_RigidBodyComponent>(C) != nullptr)
			{
				continue;
			}

			GetShapesRecursive(*C, OutFoundShapes);
		}
	}
}

void UAGX_RigidBodyComponent::InitializeNative()
{
	check(!GIsReconstructingBlueprintInstances);
	check(!HasNative());
	NativeBarrier.AllocateNative();
	check(HasNative()); /// \todo Consider better error handling than 'check'.

	WritePropertiesToNative();
	WriteTransformToNative();

	SynchronizeShapes();

	UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this);
	if (Simulation == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Rigid Body '%s' in '%s' tried to get Simulation, but UAGX_Simulation::GetFrom "
				 "returned nullptr."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return;
	}

	Simulation->Add(*this);

	if (bAutoGenerateMass)
	{
		Mass = NativeBarrier.GetMassProperties().GetMass();
	}
	if (bAutoGenerateCenterOfMassOffset)
	{
		CenterOfMassOffset = NativeBarrier.GetCenterOfMassOffset();
	}
	if (bAutoGeneratePrincipalInertia)
	{
		PrincipalInertia = NativeBarrier.GetMassProperties().GetPrincipalInertia();
	}
}

void UAGX_RigidBodyComponent::WritePropertiesToNative()
{
	if (!HasNative())
	{
		return;
	}
	FMassPropertiesBarrier& MassProperties = NativeBarrier.GetMassProperties();
	MassProperties.SetAutoGenerateMass(bAutoGenerateMass);
	MassProperties.SetAutoGenerateCenterOfMassOffset(bAutoGenerateCenterOfMassOffset);
	MassProperties.SetAutoGeneratePrincipalInertia(bAutoGeneratePrincipalInertia);

	if (!bAutoGenerateMass)
	{
		MassProperties.SetMass(Mass);
	}
	if (!bAutoGenerateCenterOfMassOffset)
	{
		NativeBarrier.SetCenterOfMassOffset(CenterOfMassOffset);
	}
	if (!bAutoGeneratePrincipalInertia)
	{
		MassProperties.SetPrincipalInertia(PrincipalInertia);
	}

	NativeBarrier.UpdateMassProperties(); // make sure mass properties are really updated

	NativeBarrier.SetVelocity(Velocity);
	NativeBarrier.SetAngularVelocity(AngularVelocity);
	NativeBarrier.SetLinearVelocityDamping(LinearVelocityDamping);
	NativeBarrier.SetAngularVelocityDamping(AngularVelocityDamping);
	NativeBarrier.SetName(GetName());
	NativeBarrier.SetEnabled(bEnabled);
	InitializeMotionControl();
}

void UAGX_RigidBodyComponent::CopyFrom(
	const FRigidBodyBarrier& Barrier, bool ForceOverwriteInstances)
{
	const FMassPropertiesBarrier& MassProperties = Barrier.GetMassProperties();

	AGX_COPY_PROPERTY_FROM(ImportGuid, Barrier.GetGuid(), *this, ForceOverwriteInstances)
	AGX_COPY_PROPERTY_FROM(Mass, MassProperties.GetMass(), *this, ForceOverwriteInstances)
	AGX_COPY_PROPERTY_FROM(
		bAutoGenerateMass, MassProperties.GetAutoGenerateMass(), *this, ForceOverwriteInstances)
	AGX_COPY_PROPERTY_FROM(
		bAutoGenerateCenterOfMassOffset, MassProperties.GetAutoGenerateCenterOfMassOffset(), *this,
		ForceOverwriteInstances)
	AGX_COPY_PROPERTY_FROM(
		bAutoGeneratePrincipalInertia, MassProperties.GetAutoGeneratePrincipalInertia(), *this,
		ForceOverwriteInstances)
	AGX_COPY_PROPERTY_FROM(
		CenterOfMassOffset, Barrier.GetCenterOfMassOffset(), *this, ForceOverwriteInstances)
	AGX_COPY_PROPERTY_FROM(
		PrincipalInertia, MassProperties.GetPrincipalInertia(), *this, ForceOverwriteInstances)
	AGX_COPY_PROPERTY_FROM(Velocity, Barrier.GetVelocity(), *this, ForceOverwriteInstances)
	AGX_COPY_PROPERTY_FROM(
		AngularVelocity, Barrier.GetAngularVelocity(), *this, ForceOverwriteInstances)
	AGX_COPY_PROPERTY_FROM(
		MotionControl, Barrier.GetMotionControl(), *this, ForceOverwriteInstances)
	AGX_COPY_PROPERTY_FROM(bEnabled, Barrier.GetEnabled(), *this, ForceOverwriteInstances)
	AGX_COPY_PROPERTY_FROM(
		LinearVelocityDamping, Barrier.GetLinearVelocityDamping(), *this, ForceOverwriteInstances)
	AGX_COPY_PROPERTY_FROM(
		AngularVelocityDamping, Barrier.GetAngularVelocityDamping(), *this, ForceOverwriteInstances)

	// Manually update archetype instances for properties that the AGX_COPY_PROPERTY_FROM macro
	// cannot handle.
	const FMergeSplitPropertiesBarrier Msp =
		FMergeSplitPropertiesBarrier::CreateFrom(*const_cast<FRigidBodyBarrier*>(&Barrier));
	if (FAGX_ObjectUtilities::IsTemplateComponent(*this))
	{
		for (auto Instance : FAGX_ObjectUtilities::GetArchetypeInstances(*this))
		{
			// Merge Split Properties.
			if (Msp.HasNative())
			{
				if (ForceOverwriteInstances ||
					Instance->MergeSplitProperties == MergeSplitProperties)
				{
					Instance->MergeSplitProperties.CopyFrom(Msp);
				}
			}
		}
	}

	// Finally, update this component for properties that the AGX_COPY_PROPERTY_FROM macro
	// cannot handle.
	if (Msp.HasNative())
	{
		MergeSplitProperties.CopyFrom(Msp);
	}

	FAGX_ObjectUtilities::SetAnyComponentWorldTransform(
		*this, FTransform(Barrier.GetRotation(), Barrier.GetPosition()), ForceOverwriteInstances);
}

void UAGX_RigidBodyComponent::InitializeMotionControl()
{
	NativeBarrier.SetMotionControl(MotionControl);

	if (MotionControl == MC_DYNAMICS && Mobility != EComponentMobility::Movable)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("The Rigid Body Component \"%s\" \"%s\" has a RigidBody with Dynamic AGX "
				 "MotionControl but Non-Movable Unreal Mobility. Unreal Mobility will "
				 "automatically be changed to Movable this game session, but should also be "
				 "changed manually in the Editor to avoid future problems!"),
			*GetName(), *GetLabelSafe(GetOwner()));

		SetMobility(EComponentMobility::Type::Movable);
	}
}

bool UAGX_RigidBodyComponent::ReadTransformFromNative()
{
	AGX_CHECK(HasNative());
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Read Transform From Native called on Rigid Body Component '%s' in '%s' that does "
				 "not have a native. Doing nothing."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return false;
	}

	const FVector NewLocation = NativeBarrier.GetPosition();
	const FQuat NewRotation = NativeBarrier.GetRotation();

	auto TransformSelf = [this, &NewLocation, &NewRotation]()
	{
		const FVector OldLocation = GetComponentLocation();
		const FVector LocationDelta = NewLocation - OldLocation;
		// MoveComponent may trigger user callbacks, e.g. On Begin Overlap, which may remove this
		// Rigid Body from the simulation.
		MoveComponent(LocationDelta, NewRotation, false);
		if (HasNative())
		{
			ComponentVelocity = NativeBarrier.GetVelocity();
		}
		return true;
	};

	auto TransformAncestor = [this, &NewLocation, &NewRotation](USceneComponent& Ancestor)
	{
		// Where Ancestor is relative to RigidBodyComponent, i.e., how the AGX Dynamics
		// transformation should be changed in order to be applicable to Ancestor.
		const FTransform AncestorRelativeToBody =
			Ancestor.GetComponentTransform().GetRelativeTransform(GetComponentTransform());

		// The transform we got from AGX Dynamics. We should manipulate Ancestor's transformation
		// so that this RigidBodyComponent end up at this position. All other children of
		// Ancestor should follow.
		const FTransform TargetBodyLocation = FTransform(NewRotation, NewLocation);

		// Compute the transform that moves Ancestor so that the body end up where we want it. Do
		// not change the scale of the ancestor, we don't want do deform meshes and other stuff in
		// there.
		FTransform NewTransform;
		FTransform::Multiply(&NewTransform, &AncestorRelativeToBody, &TargetBodyLocation);
		NewTransform.SetScale3D(Ancestor.GetComponentScale());

		// SetWorldTransform may trigger user callbacks, e.g. On Begin Overlap, which may remove
		// this Rigid Body from the simulation.
		Ancestor.SetWorldTransform(NewTransform);
		if (HasNative())
		{
			Ancestor.ComponentVelocity = NativeBarrier.GetVelocity();
		}
	};

	auto TryTransformAncestor =
		[this, &NewLocation, &NewRotation, &TransformAncestor](USceneComponent* Ancestor)
	{
		if (Ancestor == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Cannot update transformation of ancestor of RigidBody '%s' because it "
					 "doesn't have an ancestor."),
				*GetName());
			return false;
		}
		TransformAncestor(*Ancestor);
		return true;
	};

	switch (TransformTarget)
	{
		case TT_SELF:
			return TransformSelf();
			break;
		case TT_PARENT:
			return TryTransformAncestor(GetAttachParent());
			break;
		case TT_ROOT:
			return TryTransformAncestor(GetAttachmentRoot());
			break;
	}

	return false;
}

bool UAGX_RigidBodyComponent::WriteTransformToNative()
{
	AGX_CHECK(HasNative());
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Write Transform To Native called on Rigid Body Component '%s' in '%s' that does "
				 "not have a native. Doing nothing."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return false;
	}

	NativeBarrier.SetPosition(GetComponentLocation());
	NativeBarrier.SetRotation(GetComponentQuat());
	return true;
}

void UAGX_RigidBodyComponent::TryWriteTransformToNative()
{
	if (!HasNative())
	{
		return;
	}
	WriteTransformToNative();
}

#if WITH_EDITOR
bool UAGX_RigidBodyComponent::CanEditChange(
#if UE_VERSION_OLDER_THAN(4, 25, 0)
	const UProperty* InProperty
#else
	const FProperty* InProperty
#endif
) const
{
// This code was used when we had a bool property for the transform target and it used to enable
// or disable the checkbox in the Details Panel. Now that we have a drop-down list instead doing
// something like this is more complicated. Leaving the code here both as a reminder and for future
// inspiration.
//
// In essence, we want to disable the TT_ROOT option when TransformRootComponentAllowed returns
// false, and disable the TT_PARENT option when TransformParentComponentAllowed (not written yet)
// returns false.
#if 0
	// bTransformRootComponent is only allowed when this is the only RigidBodyComponent owned by the
	// parent actor.
	if (InProperty->GetFName() ==
		GET_MEMBER_NAME_CHECKED(UAGX_RigidBodyComponent, bTransformRootComponent))
	{
		return TransformRootComponentAllowed();
	}
#endif
	return Super::CanEditChange(InProperty);
}

bool UAGX_RigidBodyComponent::TransformRootComponentAllowed() const
{
	if (GetOwner() == nullptr)
	{
		// Components don't have an owner while being built in a Blueprint. Not sure how to handle
		// this. Leaving it to the user for now, i.e., the user is responsible for not enabling
		// TransformRootComponent when there are multiple RigidBodyComponents in a Blueprint.
		return true;
	}
	return FAGX_ObjectUtilities::GetNumComponentsInActor<UAGX_RigidBodyComponent>(*GetOwner()) == 1;
}
#endif

/// \note Can use TInlineComponentArray<UAGX_RigidBodyComponent*> here, for performance.
TArray<UAGX_RigidBodyComponent*> UAGX_RigidBodyComponent::GetFromActor(const AActor* Actor)
{
	TArray<UAGX_RigidBodyComponent*> Bodies;
	if (Actor == nullptr)
	{
		return Bodies;
	}

	Actor->GetComponents<UAGX_RigidBodyComponent>(Bodies, false);
	return Bodies;
}

UAGX_RigidBodyComponent* UAGX_RigidBodyComponent::GetFirstFromActor(const AActor* Actor)
{
	if (Actor == nullptr)
	{
		return nullptr;
	}

	return Actor->FindComponentByClass<UAGX_RigidBodyComponent>();
}

TStructOnScope<FActorComponentInstanceData> UAGX_RigidBodyComponent::GetComponentInstanceData()
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
void UAGX_RigidBodyComponent::OnComponentView()
{
	/// @todo Here we should detect if TransformTarget has an illegal value for the current
	/// Component configuration in the Actor and if so set it back to TT_SELF.
	/// Or perhaps notify the user in some other way. Whatever gives the best UX.
	DisableTransformRootCompIfMultiple();
}

void UAGX_RigidBodyComponent::DisableTransformRootCompIfMultiple()
{
// This code was used when we had a bool property for the transform target and it used to forcibly
// disable root targeting when discovered that it was no longer legal due to multple bodies with
// the same Actor. This is a bit more complicated now that we have an enum instead of a bool, but
// the process should be similar.
//
// I'm leaving this code here both as a reminder and for future inspiration.
#if 0
	if (GetOwner() == nullptr)
	{
		// Components don't have an owner while being built in a Blueprint. This may actually be
		// a problem. Makes automatic bTransformRootComponent disabling impossible. Is there
		// another way to get a list of all sibling RigidBodyComponents?
		return;
	}

	/*
	 * TransformRootComponent is not allowed when the owning Actor has multiple RigidBodyComponents
	 * because the rigid bodies would then be fighting each others' transform synchronization from
	 * AGX Dynamics. This is true even if a single RigidBodyComponent has bTransformRootComponet set
	 * to true because that rigid body would wreck all the other bodies.
	 */
	TArray<UAGX_RigidBodyComponent*> Components;
	GetOwner()->GetComponents<UAGX_RigidBodyComponent>(Components, false);
	if (Components.Num() > 1)
	{
		// Disable the bTransformRootComponent flag for all UAGX_RigidBodyComponent in the owning
		// actor.
		for (auto C : Components)
		{
			if (C->bTransformRootComponent)
			{
				C->bTransformRootComponent = false;
			}
		}
	}
#endif
}
#endif

void UAGX_RigidBodyComponent::SetPosition(FVector Position)
{
	if (HasNative())
	{
		NativeBarrier.SetPosition(Position);
		// Not calling Unreal Engine's Set World Location because this Rigid Body may have a
		// Transform Target other than Self. Read Transform From Native will apply the AGX Dynamics
		// transformation to the correct Scene Component, with the correct local transform if
		// necessary.
		//
		// The semantics is that Set Position moves the Rigid Body Component as-if it had been
		// moved by AGX Dynamics. This is different from the semantics when there is no native.
		ReadTransformFromNative();
	}
	else
	{
		// Set the position of the Rigid Body Component, not the Transform Target.
		//
		// The semantics is that Set Position moves the Rigid Body Component to the given
		// position, leaving the Transform Target where it is. This is analogous to dragging
		// the Rigid Body Component in the editor.
		SetWorldLocation(Position);
	}
}

FVector UAGX_RigidBodyComponent::GetPosition() const
{
	if (HasNative())
	{
		return NativeBarrier.GetPosition();
	}

	return GetComponentLocation();
}

void UAGX_RigidBodyComponent::SetRotation(FQuat Rotation)
{
	if (HasNative())
	{
		NativeBarrier.SetRotation(Rotation);
	}

	SetWorldRotation(Rotation);
}

FQuat UAGX_RigidBodyComponent::GetRotation() const
{
	if (HasNative())
	{
		return NativeBarrier.GetRotation();
	}

	return GetComponentQuat();
}

void UAGX_RigidBodyComponent::SetRotator(FRotator Rotator)
{
	const FQuat Quat = Rotator.Quaternion();
	SetRotation(Quat);
}

FRotator UAGX_RigidBodyComponent::GetRotator() const
{
	const FQuat Quat = GetRotation();
	const FRotator Rotator = Quat.Rotator();
	return Rotator;
}

void UAGX_RigidBodyComponent::SetEnabled(bool InEnabled)
{
	if (HasNative())
	{
		NativeBarrier.SetEnabled(InEnabled);
	}

	bEnabled = InEnabled;
}

bool UAGX_RigidBodyComponent::IsEnabled() const
{
	if (HasNative())
	{
		return NativeBarrier.GetEnabled();
	}

	return bEnabled;
}

bool UAGX_RigidBodyComponent::GetEnabled() const
{
	return IsEnabled();
}

void UAGX_RigidBodyComponent::SetMass(float InMass)
{
	if (HasNative())
	{
		NativeBarrier.GetMassProperties().SetMass(InMass);
		if (bAutoGeneratePrincipalInertia)
		{
			// Principal inertia depend on the mass, so changing the mass on the native may have
			// changed the principal inertia as well. Read it back from the native so that the
			// Unreal Engine representation is kept in sync.
			PrincipalInertia = NativeBarrier.GetMassProperties().GetPrincipalInertia();
		}
	}
	Mass = InMass;
}

float UAGX_RigidBodyComponent::GetMass() const
{
	if (HasNative())
	{
		return NativeBarrier.GetMassProperties().GetMass();
	}
	else
	{
		return Mass;
	}
}

void UAGX_RigidBodyComponent::SetAutoGenerateMass(bool bInAuto)
{
	if (HasNative())
	{
		NativeBarrier.GetMassProperties().SetAutoGenerateMass(bInAuto);
		NativeBarrier.UpdateMassProperties(); // trigger an update of mass properties
	}
	bAutoGenerateMass = bInAuto;
}

bool UAGX_RigidBodyComponent::GetAutoGenerateMass() const
{
	if (HasNative())
	{
		return NativeBarrier.GetMassProperties().GetAutoGenerateMass();
	}
	else
	{
		return bAutoGenerateMass;
	}
}

void UAGX_RigidBodyComponent::SetCenterOfMassOffset(FVector InCoMOffset)
{
	if (HasNative())
	{
		NativeBarrier.SetCenterOfMassOffset(InCoMOffset);
	}
	CenterOfMassOffset = InCoMOffset;
}

FVector UAGX_RigidBodyComponent::GetCenterOfMassOffset() const
{
	if (HasNative())
	{
		return NativeBarrier.GetCenterOfMassOffset();
	}
	else
	{
		return CenterOfMassOffset;
	}
}

FVector UAGX_RigidBodyComponent::GetCenterOfMassPosition() const
{
	if (!HasNative())
		return FVector::ZeroVector;

	return NativeBarrier.GetCenterOfMassPosition();
}

void UAGX_RigidBodyComponent::SetAutoGenerateCenterOfMassOffset(bool bInAuto)
{
	if (HasNative())
	{
		NativeBarrier.GetMassProperties().SetAutoGenerateCenterOfMassOffset(bInAuto);
		NativeBarrier.UpdateMassProperties(); // trigger an update of mass properties
	}
	bAutoGenerateCenterOfMassOffset = bInAuto;
}

bool UAGX_RigidBodyComponent::GetAutoGenerateCenterOfMassOffset() const
{
	if (HasNative())
	{
		return NativeBarrier.GetMassProperties().GetAutoGenerateCenterOfMassOffset();
	}
	else
	{
		return bAutoGenerateCenterOfMassOffset;
	}
}

TArray<UAGX_ShapeComponent*> UAGX_RigidBodyComponent::GetShapes() const
{
	TArray<UAGX_ShapeComponent*> FoundShapes;
	GetShapesRecursive(*this, FoundShapes);
	return FoundShapes;
}

void UAGX_RigidBodyComponent::SetPrincipalInertia(FVector InPrincipalInertia)
{
	if (HasNative())
	{
		NativeBarrier.GetMassProperties().SetPrincipalInertia(InPrincipalInertia);
	}
	PrincipalInertia = InPrincipalInertia;
}

FVector UAGX_RigidBodyComponent::GetPrincipalInertia() const
{
	if (HasNative())
	{
		return NativeBarrier.GetMassProperties().GetPrincipalInertia();
	}
	else
	{
		return PrincipalInertia;
	}
}

void UAGX_RigidBodyComponent::SetAutoGeneratePrincipalInertia(bool bInAuto)
{
	if (HasNative())
	{
		NativeBarrier.GetMassProperties().SetAutoGeneratePrincipalInertia(bInAuto);
		NativeBarrier.UpdateMassProperties(); // trigger an update of mass properties
	}
	bAutoGeneratePrincipalInertia = bInAuto;
}

bool UAGX_RigidBodyComponent::GetAutoGeneratePrincipalInertia() const
{
	if (HasNative())
	{
		return NativeBarrier.GetMassProperties().GetAutoGeneratePrincipalInertia();
	}
	else
	{
		return bAutoGeneratePrincipalInertia;
	}
}

void UAGX_RigidBodyComponent::UpdateMassProperties()
{
	if (HasNative())
	{
		NativeBarrier.UpdateMassProperties();
	}
}

double UAGX_RigidBodyComponent::CalculateMass() const
{
	if (HasNative())
	{
		return NativeBarrier.CalculateMass();
	}

	return 0.0;
}

float UAGX_RigidBodyComponent::CalculateMass_BP() const
{
	return static_cast<float>(CalculateMass());
}

void UAGX_RigidBodyComponent::SetVelocity(FVector InVelocity)
{
	if (HasNative())
	{
		NativeBarrier.SetVelocity(InVelocity);
	}

	Velocity = InVelocity;
}

FVector UAGX_RigidBodyComponent::GetVelocity() const
{
	if (HasNative())
	{
		return NativeBarrier.GetVelocity();
	}

	return Velocity;
}

void UAGX_RigidBodyComponent::SetAngularVelocity(FVector InAngularVelocity)
{
	if (HasNative())
	{
		NativeBarrier.SetAngularVelocity(InAngularVelocity);
	}

	AngularVelocity = InAngularVelocity;
}

FVector UAGX_RigidBodyComponent::GetAngularVelocity() const
{
	if (HasNative())
	{
		return NativeBarrier.GetAngularVelocity();
	}

	return AngularVelocity;
}

void UAGX_RigidBodyComponent::SetLinearVelocityDamping(FVector InLinearVelocityDamping)
{
	if (HasNative())
	{
		NativeBarrier.SetLinearVelocityDamping(InLinearVelocityDamping);
	}

	LinearVelocityDamping = InLinearVelocityDamping;
}

FVector UAGX_RigidBodyComponent::GetLinearVelocityDamping() const
{
	if (HasNative())
	{
		return NativeBarrier.GetLinearVelocityDamping();
	}

	return LinearVelocityDamping;
}

void UAGX_RigidBodyComponent::SetAngularVelocityDamping(FVector InAngularVelocityDamping)
{
	if (HasNative())
	{
		NativeBarrier.SetAngularVelocityDamping(InAngularVelocityDamping);
	}

	AngularVelocityDamping = InAngularVelocityDamping;
}

FVector UAGX_RigidBodyComponent::GetAngularVelocityDamping() const
{
	if (HasNative())
	{
		return NativeBarrier.GetAngularVelocityDamping();
	}

	return AngularVelocityDamping;
}

void UAGX_RigidBodyComponent::SetMotionControl(TEnumAsByte<enum EAGX_MotionControl> InMotionControl)
{
	if (HasNative())
	{
		NativeBarrier.SetMotionControl(InMotionControl);
	}

	MotionControl = InMotionControl;
}

TEnumAsByte<enum EAGX_MotionControl> UAGX_RigidBodyComponent::GetMotionControl() const
{
	if (HasNative())
	{
		return NativeBarrier.GetMotionControl();
	}

	return MotionControl;
}

void UAGX_RigidBodyComponent::AddForceAtCenterOfMass(FVector Force)
{
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Must have a native AGX Dynamics representation of RigidBody '%s' to call "
				 "AddForce."),
			*GetName());
		return;
	}

	NativeBarrier.AddForceAtCenterOfMass(Force);
}

void UAGX_RigidBodyComponent::AddForceAtWorldLocation(FVector Force, FVector Location)
{
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Must have a native AGX Dynamics representation or RigidBody '%s' to call "
				 "AddForce."),
			*GetName());
		return;
	}

	NativeBarrier.AddForceAtWorldLocation(Force, Location);
}

void UAGX_RigidBodyComponent::AddForceAtLocalLocation(FVector Force, FVector Location)
{
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Must have a native AGX Dynamics representation of RigidBody '%s' to call "
				 "AddForce."),
			*GetName());
		return;
	}

	NativeBarrier.AddForceAtLocalLocation(Force, Location);
}

void UAGX_RigidBodyComponent::AddLocalForceAtLocalLocation(FVector LocalForce, FVector Location)
{
	const FVector GlobalForce = GetComponentTransform().TransformVectorNoScale(LocalForce);
	AddForceAtLocalLocation(GlobalForce, Location);
}

FVector UAGX_RigidBodyComponent::GetForce() const
{
	if (!HasNative())
	{
		return FVector::ZeroVector;
	}

	return NativeBarrier.GetForce();
}

void UAGX_RigidBodyComponent::AddTorqueLocal(FVector Torque)
{
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Must have a native AGX Dynamics representation of RigidBody '%s' to call "
				 "AddTorqueLocal."),
			*GetName());
		return;
	}

	NativeBarrier.AddTorqueLocal(Torque);
}

void UAGX_RigidBodyComponent::AddTorqueWorld(FVector Torque)
{
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Must have a native AGX Dynamics representation of RigidBody '%s' to call "
				 "AddTorqueWorld."),
			*GetName());
		return;
	}

	NativeBarrier.AddTorqueWorld(Torque);
}

FVector UAGX_RigidBodyComponent::GetTorque() const
{
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Must have a native AGX Dynamics representation of RigidBody '%s' to call "
				 "GetTorque."),
			*GetName());
		return FVector::ZeroVector;
	}

	return NativeBarrier.GetTorque();
}

void UAGX_RigidBodyComponent::MoveTo(FVector Position, FRotator Rotation, double Duration)
{
	MoveTo(Position, Rotation.Quaternion(), Duration);
}

void UAGX_RigidBodyComponent::MoveTo(FVector Position, FQuat Rotation, double Duration)
{
	if (!HasNative() || Duration < 0.0)
		return;

	NativeBarrier.MoveTo(Position, Rotation, Duration);
}

void UAGX_RigidBodyComponent::MoveTo_Rotator(FVector Position, FRotator Rotation, double Duration)
{
	MoveTo(Position, Rotation, Duration);
}

void UAGX_RigidBodyComponent::MoveTo_Quat(FVector Position, FQuat Rotation, double Duration)
{
	MoveTo(Position, Rotation, Duration);
}

void UAGX_RigidBodyComponent::MoveToLocal(
	FVector PositionLocal, FRotator RotationLocal, float Duration)
{
	if (!HasNative())
		return;

	const FTransform BodyTransformGlobal(NativeBarrier.GetRotation(), NativeBarrier.GetPosition());

	const FVector PositionGlobal = BodyTransformGlobal.TransformPositionNoScale(PositionLocal);
	const FQuat RotationGlobal = BodyTransformGlobal.TransformRotation(RotationLocal.Quaternion());

	MoveTo(PositionGlobal, FRotator(RotationGlobal), Duration);
}

void UAGX_RigidBodyComponent::SynchronizeShapes()
{
	for (UAGX_ShapeComponent* Shape : GetShapes())
	{
		FShapeBarrier* NativeShape = Shape->GetOrCreateNative();
		AGX_CHECK(NativeShape != nullptr && NativeShape->HasNative());
		if (NativeShape == nullptr || !NativeShape->HasNative())
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("When creating AGX Dynamics natives for Shapes in '%s' in '%s', Shape '%s' "
					 "did not get a native. This Shape will not be included in the simulation."),
				*GetName(), *GetLabelSafe(GetOwner()), *Shape->GetName());
			continue;
		}
		Shape->UpdateNativeLocalTransform();
		NativeBarrier.AddShape(NativeShape);
	}
}
