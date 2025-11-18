// Copyright 2025, Algoryx Simulation AB.

#include "Vehicle/AGX_WheelJointComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_AssetGetterSetterImpl.h"
#include "AGX_LogCategory.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "Constraints/ControllerConstraintBarriers.h"
#include "Vehicle/WheelJointBarrier.h"
#include "Utilities/AGX_ConstraintUtilities.h"
#include "Utilities/AGX_StringUtilities.h"

struct FRigidBodyBarrier;

UAGX_WheelJointComponent::UAGX_WheelJointComponent()
	: UAGX_ConstraintComponent(
		  {EDofFlag::DofFlagTranslational1, EDofFlag::DofFlagTranslational2,
		   EDofFlag::DofFlagRotational1})
{
	NativeBarrier.Reset(new FWheelJointBarrier());

	// In AGX Dynamics, the Suspension LockController is enabled by default, all other disabled.
	SuspensionLockController.bEnable = true;
}

UAGX_WheelJointComponent::~UAGX_WheelJointComponent()
{
}

double UAGX_WheelJointComponent::GetAngle() const
{
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("UAGX_WheelJointComponent::GetAngle called on Wheel Joint '%s' in '%s' that does "
				 "not have an AGX Native object. Returning 0.0."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return 0.0;
	}

	return GetNativeWheelJoint()->GetAngle();
}

FWheelJointBarrier* UAGX_WheelJointComponent::GetNativeWheelJoint()
{
	return FAGX_ConstraintUtilities::GetNativeCast(this);
}

const FWheelJointBarrier* UAGX_WheelJointComponent::GetNativeWheelJoint() const
{
	return FAGX_ConstraintUtilities::GetNativeCast(this);
}

void UAGX_WheelJointComponent::CopyFrom(
	const FConstraintBarrier& Barrier, FAGX_ImportContext* Context)
{
	Super::CopyFrom(Barrier, Context);
	const FWheelJointBarrier* WJBarrier = static_cast<const FWheelJointBarrier*>(&Barrier);

	// Steering.
	SteeringLockController.CopyFrom(
		*WJBarrier->GetLockController(EAGX_WheelJointSecondaryConstraint::Steering));
	SteeringRangeController.CopyFrom(
		*WJBarrier->GetRangeController(EAGX_WheelJointSecondaryConstraint::Steering));
	SteeringTargetSpeedController.CopyFrom(
		*WJBarrier->GetTargetSpeedController(EAGX_WheelJointSecondaryConstraint::Steering));

	// Wheel.
	WheelLockController.CopyFrom(
		*WJBarrier->GetLockController(EAGX_WheelJointSecondaryConstraint::Wheel));
	WheelRangeController.CopyFrom(
		*WJBarrier->GetRangeController(EAGX_WheelJointSecondaryConstraint::Wheel));
	WheelTargetSpeedController.CopyFrom(
		*WJBarrier->GetTargetSpeedController(EAGX_WheelJointSecondaryConstraint::Wheel));

	// Suspension.
	SuspensionLockController.CopyFrom(
		*WJBarrier->GetLockController(EAGX_WheelJointSecondaryConstraint::Suspension));
	SuspensionRangeController.CopyFrom(
		*WJBarrier->GetRangeController(EAGX_WheelJointSecondaryConstraint::Suspension));
	SuspensionTargetSpeedController.CopyFrom(
		*WJBarrier->GetTargetSpeedController(EAGX_WheelJointSecondaryConstraint::Suspension));
}

namespace AGX_WheelJointComponent_helpers
{
	void InitializeControllerBarriers(UAGX_WheelJointComponent& Constraint)
	{
		FWheelJointBarrier* Barrier = Constraint.GetNativeWheelJoint();

		// Steering.
		Constraint.SteeringLockController.InitializeBarrier(
			Barrier->GetLockController(EAGX_WheelJointSecondaryConstraint::Steering));
		Constraint.SteeringRangeController.InitializeBarrier(
			Barrier->GetRangeController(EAGX_WheelJointSecondaryConstraint::Steering));
		Constraint.SteeringTargetSpeedController.InitializeBarrier(
			Barrier->GetTargetSpeedController(EAGX_WheelJointSecondaryConstraint::Steering));

		// Wheel.
		Constraint.WheelLockController.InitializeBarrier(
			Barrier->GetLockController(EAGX_WheelJointSecondaryConstraint::Wheel));
		Constraint.WheelRangeController.InitializeBarrier(
			Barrier->GetRangeController(EAGX_WheelJointSecondaryConstraint::Wheel));
		Constraint.WheelTargetSpeedController.InitializeBarrier(
			Barrier->GetTargetSpeedController(EAGX_WheelJointSecondaryConstraint::Wheel));

		// Suspension.
		Constraint.SuspensionLockController.InitializeBarrier(
			Barrier->GetLockController(EAGX_WheelJointSecondaryConstraint::Suspension));
		Constraint.SuspensionRangeController.InitializeBarrier(
			Barrier->GetRangeController(EAGX_WheelJointSecondaryConstraint::Suspension));
		Constraint.SuspensionTargetSpeedController.InitializeBarrier(
			Barrier->GetTargetSpeedController(EAGX_WheelJointSecondaryConstraint::Suspension));
	}
}

void UAGX_WheelJointComponent::CreateNativeImpl()
{
	FAGX_ConstraintUtilities::CreateNative(
		NativeBarrier.Get(), BodyAttachment1, BodyAttachment2, GetFName(),
		GetLabelSafe(GetOwner()));

	AGX_WheelJointComponent_helpers::InitializeControllerBarriers(*this);
}

void UAGX_WheelJointComponent::UpdateNativeProperties()
{
	Super::UpdateNativeProperties();

	// Steering.
	SteeringLockController.UpdateNativeProperties();
	SteeringRangeController.UpdateNativeProperties();
	SteeringTargetSpeedController.UpdateNativeProperties();

	// Wheel.
	WheelLockController.UpdateNativeProperties();
	WheelRangeController.UpdateNativeProperties();
	WheelTargetSpeedController.UpdateNativeProperties();

	// Suspension.
	SuspensionLockController.UpdateNativeProperties();
	SuspensionRangeController.UpdateNativeProperties();
	SuspensionTargetSpeedController.UpdateNativeProperties();
}

#if WITH_EDITOR
void UAGX_WheelJointComponent::PostEditChangeChainProperty(FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);

	// If we are part of a Blueprint then this will trigger a RerunConstructionScript on the owning
	// Actor. That means that this object will be removed from the Actor and destroyed. We want to
	// apply all our changes before that so that they are carried over to the copy.
	Super::PostEditChangeChainProperty(Event);
}
void UAGX_WheelJointComponent::PostInitProperties()
{
	Super::PostInitProperties();
	InitPropertyDispatcher();
}

void UAGX_WheelJointComponent::InitPropertyDispatcher()
{
	FAGX_PropertyChangedDispatcher<ThisClass>& PropertyDispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (PropertyDispatcher.IsInitialized())
		return;

	// Steering.
	TFunction<FAGX_ConstraintLockController*(ThisClass*)> GetSteeringLockController =
		[](ThisClass* EditedObject) { return &EditedObject->SteeringLockController; };

	TFunction<FAGX_ConstraintRangeController*(ThisClass*)> GetSteeringRangeController =
		[](ThisClass* EditedObject) { return &EditedObject->SteeringRangeController; };

	TFunction<FAGX_ConstraintTargetSpeedController*(ThisClass*)> GetSteeringTargetSpeedController =
		[](ThisClass* EditedObject) { return &EditedObject->SteeringTargetSpeedController; };

	FAGX_ConstraintUtilities::AddLockControllerPropertyCallbacks(
		PropertyDispatcher, GetSteeringLockController,
		GET_MEMBER_NAME_CHECKED(ThisClass, SteeringLockController));

	FAGX_ConstraintUtilities::AddRangeControllerPropertyCallbacks(
		PropertyDispatcher, GetSteeringRangeController,
		GET_MEMBER_NAME_CHECKED(ThisClass, SteeringRangeController));

	FAGX_ConstraintUtilities::AddTargetSpeedControllerPropertyCallbacks(
		PropertyDispatcher, GetSteeringTargetSpeedController,
		GET_MEMBER_NAME_CHECKED(ThisClass, SteeringTargetSpeedController));

	// Wheel.
	TFunction<FAGX_ConstraintLockController*(ThisClass*)> GetWheelLockController =
		[](ThisClass* EditedObject) { return &EditedObject->WheelLockController; };

	TFunction<FAGX_ConstraintRangeController*(ThisClass*)> GetWheelRangeController =
		[](ThisClass* EditedObject) { return &EditedObject->WheelRangeController; };

	TFunction<FAGX_ConstraintTargetSpeedController*(ThisClass*)> GetWheelTargetSpeedController =
		[](ThisClass* EditedObject) { return &EditedObject->WheelTargetSpeedController; };

	FAGX_ConstraintUtilities::AddLockControllerPropertyCallbacks(
		PropertyDispatcher, GetWheelLockController,
		GET_MEMBER_NAME_CHECKED(ThisClass, WheelLockController));

	FAGX_ConstraintUtilities::AddRangeControllerPropertyCallbacks(
		PropertyDispatcher, GetWheelRangeController,
		GET_MEMBER_NAME_CHECKED(ThisClass, WheelRangeController));

	FAGX_ConstraintUtilities::AddTargetSpeedControllerPropertyCallbacks(
		PropertyDispatcher, GetWheelTargetSpeedController,
		GET_MEMBER_NAME_CHECKED(ThisClass, WheelTargetSpeedController));

	// Suspension.
	TFunction<FAGX_ConstraintLockController*(ThisClass*)> GetSuspensionLockController =
		[](ThisClass* EditedObject) { return &EditedObject->SuspensionLockController; };

	TFunction<FAGX_ConstraintRangeController*(ThisClass*)> GetSuspensionRangeController =
		[](ThisClass* EditedObject) { return &EditedObject->SuspensionRangeController; };

	TFunction<FAGX_ConstraintTargetSpeedController*(ThisClass*)> GetSuspensionTargetSpeedController =
		[](ThisClass* EditedObject) { return &EditedObject->SuspensionTargetSpeedController; };

	FAGX_ConstraintUtilities::AddLockControllerPropertyCallbacks(
		PropertyDispatcher, GetSuspensionLockController,
		GET_MEMBER_NAME_CHECKED(ThisClass, SuspensionLockController));

	FAGX_ConstraintUtilities::AddRangeControllerPropertyCallbacks(
		PropertyDispatcher, GetSuspensionRangeController,
		GET_MEMBER_NAME_CHECKED(ThisClass, SuspensionRangeController));

	FAGX_ConstraintUtilities::AddTargetSpeedControllerPropertyCallbacks(
		PropertyDispatcher, GetSuspensionTargetSpeedController,
		GET_MEMBER_NAME_CHECKED(ThisClass, SuspensionTargetSpeedController));
}
#endif // WITH_EDITOR
