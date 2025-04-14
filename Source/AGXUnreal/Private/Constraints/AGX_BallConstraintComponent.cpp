// Copyright 2024, Algoryx Simulation AB.

#include "Constraints/AGX_BallConstraintComponent.h"

// AGX Dynamics for Unreal includes.
#include "Constraints/ConstraintBarrier.h"
#include "Constraints/BallJointBarrier.h"
#include "Import/AGX_ImportContext.h"
#include "Utilities/AGX_ConstraintUtilities.h"
#include "Utilities/AGX_StringUtilities.h"

class FRigidBodyBarrier;

// Helper functions.

namespace AGX_BallConstraintComponent_helpers
{
	void InitializeControllerBarriers(UAGX_BallConstraintComponent& Constraint)
	{
		FBallJointBarrier* Barrier = Constraint.GetNativeBallJoint();
		Constraint.TwistRangeController.InitializeBarrier(Barrier->GetTwistRangeController());
	}
}

// Special member functions.

UAGX_BallConstraintComponent::UAGX_BallConstraintComponent()
	: UAGX_ConstraintComponent(
		  {EDofFlag::DofFlagTranslational1, EDofFlag::DofFlagTranslational2,
		   EDofFlag::DofFlagTranslational3})
{
	NativeBarrier.Reset(new FBallJointBarrier());
}

UAGX_BallConstraintComponent::~UAGX_BallConstraintComponent()
{
}

// Function overrides.

//~ Begin AGX Constraint Component interface.

void UAGX_BallConstraintComponent::UpdateNativeProperties()
{
	Super::UpdateNativeProperties();
	TwistRangeController.UpdateNativeProperties();
}

//~ End AGX Constraint Component interface.

//~ Begin IAGX_NativeOwner interface.

void UAGX_BallConstraintComponent::SetNativeAddress(uint64 NativeAddress)
{
	Super::SetNativeAddress(NativeAddress);
	if (!HasNative())
	{
		return;
	}

	AGX_BallConstraintComponent_helpers::InitializeControllerBarriers(*this);
}

//~ End IAGX_NativeOwner interface.

//~ Begin UObject interface.

#if WITH_EDITOR

void UAGX_BallConstraintComponent::PostInitProperties()
{
	Super::PostInitProperties();

	FAGX_PropertyChangedDispatcher<ThisClass>& PropertyDispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (PropertyDispatcher.IsInitialized())
	{
		return;
	}

	TFunction<FAGX_TwistRangeController*(ThisClass*)> GetTwistRangeController =
		[](ThisClass* EditedObject) { return &EditedObject->TwistRangeController; };

	FAGX_ConstraintUtilities::AddTwistRangeControllerPropertyCallbacks(
		PropertyDispatcher, GetTwistRangeController,
		GET_MEMBER_NAME_CHECKED(ThisClass, TwistRangeController));
}

void UAGX_BallConstraintComponent::PostEditChangeChainProperty(FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);

	// If we are part of a Blueprint then this will trigger a RerunConstructionScript on the owning
	// Actor. That means that this object will be removed from the Actor and destroyed. We want to
	// apply all our changes before that so that they are carried over to the copy.
	Super::PostEditChangeChainProperty(Event);
}

#endif

//~ End UObject interface.

// Native management.

void UAGX_BallConstraintComponent::CopyFrom(
	const FConstraintBarrier& Barrier, FAGX_ImportContext* Context)
{
	UAGX_ConstraintComponent::CopyFrom(Barrier, Context);
	FAGX_ConstraintUtilities::CopyControllersFrom(
		*this, *static_cast<const FBallJointBarrier*>(&Barrier));
}

FBallJointBarrier* UAGX_BallConstraintComponent::GetNativeBallJoint()
{
	return FAGX_ConstraintUtilities::GetNativeCast(this);
}

const FBallJointBarrier* UAGX_BallConstraintComponent::GetNativeBallJoint() const
{
	return FAGX_ConstraintUtilities::GetNativeCast(this);
}

void UAGX_BallConstraintComponent::CreateNativeImpl()
{
	FAGX_ConstraintUtilities::CreateNative(
		NativeBarrier.Get(), BodyAttachment1, BodyAttachment2, GetFName(),
		GetLabelSafe(GetOwner()));
	if (!HasNative())
	{
		return;
	}

	AGX_BallConstraintComponent_helpers::InitializeControllerBarriers(*this);
}
