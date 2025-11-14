// Copyright 2025, Algoryx Simulation AB.

#include "Vehicle/AGX_WheelJointComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
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

	// SteeringBounds.
	SteeringBoundsLockController.CopyFrom(
		*WJBarrier->GetLockController(EAGX_WheelJointSecondaryConstraint::SteeringBounds));
	SteeringBoundsRangeController.CopyFrom(
		*WJBarrier->GetRangeController(EAGX_WheelJointSecondaryConstraint::SteeringBounds));
	SteeringBoundsTargetSpeedController.CopyFrom(
		*WJBarrier->GetTargetSpeedController(EAGX_WheelJointSecondaryConstraint::SteeringBounds));
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

		// SteeringBounds.
		Constraint.SteeringBoundsLockController.InitializeBarrier(
			Barrier->GetLockController(EAGX_WheelJointSecondaryConstraint::SteeringBounds));
		Constraint.SteeringBoundsRangeController.InitializeBarrier(
			Barrier->GetRangeController(EAGX_WheelJointSecondaryConstraint::SteeringBounds));
		Constraint.SteeringBoundsTargetSpeedController.InitializeBarrier(
			Barrier->GetTargetSpeedController(EAGX_WheelJointSecondaryConstraint::SteeringBounds));
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

	// SteeringBounds.
	SteeringBoundsLockController.UpdateNativeProperties();
	SteeringBoundsRangeController.UpdateNativeProperties();
	SteeringBoundsTargetSpeedController.UpdateNativeProperties();
}
