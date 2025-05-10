// Copyright 2024, Algoryx Simulation AB.

#include "Constraints/AGX_SingleControllerConstraint1DofComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Constraints/SingleControllerConstraint1DOFBarrier.h"
#include "Import/AGX_ImportContext.h"
#include "Utilities/AGX_ConstraintUtilities.h"
#include "Utilities/AGX_StringUtilities.h"

class FRigidBodyBarrier;

UAGX_SingleControllerConstraint1DofComponent::UAGX_SingleControllerConstraint1DofComponent()
	: UAGX_Constraint1DofComponent(TArray<EDofFlag>())
{
	NativeBarrier.Reset(new FSingleControllerConstraint1DOFBarrier());
}

UAGX_SingleControllerConstraint1DofComponent::~UAGX_SingleControllerConstraint1DofComponent()
{
}

FSingleControllerConstraint1DOFBarrier*
UAGX_SingleControllerConstraint1DofComponent::GetNativeFingleControllerConstraint1DOF()
{
	return FAGX_ConstraintUtilities::GetNativeCast(this);
}

const FSingleControllerConstraint1DOFBarrier*
UAGX_SingleControllerConstraint1DofComponent::GetNativeSingleControllerConstraint1DOF() const
{
	return FAGX_ConstraintUtilities::GetNativeCast(this);
}

void UAGX_SingleControllerConstraint1DofComponent::CopyFrom(
	const FConstraintBarrier& Barrier, FAGX_ImportContext* Context)
{
	Super::CopyFrom(Barrier, Context);
	const auto BarrierSCC1DOF =
		static_cast<const FSingleControllerConstraint1DOFBarrier*>(&Barrier);

	ControllerType = BarrierSCC1DOF->GetControllerType();
	ControllerAngleType = BarrierSCC1DOF->GetControllerAngleType();
}

bool UAGX_SingleControllerConstraint1DofComponent::GetValid() const
{
	// The default implementation of GetValid() in UAGX_ConstraintComponent returns the
	// Native->GetValid(). This functions is not really intended to check correct configuration but
	// is more a way of stating "should this Constraint go to the solver or not". So the name is
	// bad. But we have nothing better, so we use it currently because it will catch a bunch of
	// cases we want it to catch.
	// But here, for SingleControllerConstraint1DofComponent, we have a problem that GetValid can
	// return false in situations where the Constraint is "valid". For example when using a
	// SingleControllerConstraint1DofComponent with RangeController, for unclear reasons.
	// So we handle this Constraint specially.
	return HasNative() && BodyAttachment1.GetRigidBody() != nullptr &&
		   BodyAttachment1.GetRigidBody() != BodyAttachment2.GetRigidBody();
}

void UAGX_SingleControllerConstraint1DofComponent::CreateNativeImpl()
{
	if (ControllerType == EAGX_ConstraintControllerType::Invalid)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Invalid Controller type for UAGX_SingleControllerConstraint1DofComponent '%s' in "
				 "'%s'."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return;
	}

	// Create Constraint Controller.
	FAGX_ConstraintController* Controller = nullptr;
	switch (ControllerType)
	{
		case EAGX_ConstraintControllerType::ConstraintTargetSpeedController:
			TargetSpeedController.InitializeBarrier(MakeUnique<FTargetSpeedControllerBarrier>());
			Controller = &TargetSpeedController;
			break;
		case EAGX_ConstraintControllerType::ConstraintLockController:
			LockController.InitializeBarrier(MakeUnique<FLockControllerBarrier>());
			Controller = &LockController;
			break;
		case EAGX_ConstraintControllerType::ConstraintRangeController:
			RangeController.InitializeBarrier(MakeUnique<FRangeControllerBarrier>());
			Controller = &RangeController;
			break;
		case EAGX_ConstraintControllerType::ConstraintFrictionController:
			FrictionController.InitializeBarrier(MakeUnique<FFrictionControllerBarrier>());
			Controller = &FrictionController;
			break;
		case EAGX_ConstraintControllerType::ConstraintElectricMotorController:
			ElectricMotorController.InitializeBarrier(
				MakeUnique<FElectricMotorControllerBarrier>());
			Controller = &ElectricMotorController;
			break;
	}

	if (Controller == nullptr || Controller->GetNative() == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Unable to create Controller for Constraint '%s' in '%s'. Native object will not "
				 "be created."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return;
	}

	// Based on FAGX_ConstraintUtilities::CreateNative but with the custom AllocateNative call
	// required for this constraint.

	if (!FAGX_ConstraintUtilities::EnsureValidConstraintAttachmentPair(
			BodyAttachment1, BodyAttachment2, GetFName()))
		return; // Error printed in EnsureValidConstraintAttachmentPair.

	FRigidBodyBarrier* Body1 = BodyAttachment1.GetRigidBodyBarrier();
	FRigidBodyBarrier* Body2 = BodyAttachment2.GetRigidBodyBarrier();

	const FString OwnerName = GetLabelSafe(GetOwner());
	FTransform Transform1 =
		FAGX_ConstraintUtilities::GetFrameTransform(BodyAttachment1, GetFName(), OwnerName);
	FTransform Transform2 =
		FAGX_ConstraintUtilities::GetFrameTransform(BodyAttachment2, GetFName(), OwnerName);

	auto BarrierSCC1DOF = static_cast<FSingleControllerConstraint1DOFBarrier*>(NativeBarrier.Get());
	BarrierSCC1DOF->AllocateNative(
		*Body1, Transform1.GetLocation(), Transform1.GetRotation(), Body2, Transform2.GetLocation(),
		Transform2.GetRotation(), Controller->GetNative(), ControllerType, ControllerAngleType);
}
