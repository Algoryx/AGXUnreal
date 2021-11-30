#include "Constraints/AGX_Constraint2DofComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_CustomVersion.h"
#include "AGX_LogCategory.h"
#include "Constraints/ControllerConstraintBarriers.h"
#include "Constraints/Constraint2DOFBarrier.h"
#include "Utilities/AGX_ConstraintUtilities.h"

UAGX_Constraint2DofComponent::UAGX_Constraint2DofComponent()
{
}

UAGX_Constraint2DofComponent::UAGX_Constraint2DofComponent(
	const TArray<EDofFlag>& LockedDofsOrdered, bool bIsSecondaryConstraint1Rotational,
	bool bIsSecondaryConstraint2Rotational)
	: UAGX_ConstraintComponent(LockedDofsOrdered)
	, ElectricMotorController1(bIsSecondaryConstraint1Rotational)
	, ElectricMotorController2(bIsSecondaryConstraint2Rotational)
	, FrictionController1(bIsSecondaryConstraint1Rotational)
	, FrictionController2(bIsSecondaryConstraint2Rotational)
	, LockController1(bIsSecondaryConstraint1Rotational)
	, LockController2(bIsSecondaryConstraint2Rotational)
	, RangeController1(bIsSecondaryConstraint1Rotational)
	, RangeController2(bIsSecondaryConstraint2Rotational)
	, TargetSpeedController1(bIsSecondaryConstraint1Rotational)
	, TargetSpeedController2(bIsSecondaryConstraint2Rotational)
	, ScrewController()
{
}

UAGX_Constraint2DofComponent::~UAGX_Constraint2DofComponent()
{
}

namespace
{
	FConstraint2DOFBarrier* Get2DofBarrier(UAGX_Constraint2DofComponent& Constraint)
	{
		return static_cast<FConstraint2DOFBarrier*>(Constraint.GetNative());
	}
}

namespace AGX_Constraint2DofComponent_helpers
{
	void InitializeControllerBarriers(UAGX_Constraint2DofComponent& Constraint)
	{
		FConstraint2DOFBarrier* Barrier = Get2DofBarrier(Constraint);

		EAGX_Constraint2DOFFreeDOF FIRST = EAGX_Constraint2DOFFreeDOF::FIRST;
		EAGX_Constraint2DOFFreeDOF SECOND = EAGX_Constraint2DOFFreeDOF::SECOND;

		Constraint.ElectricMotorController1.InitializeBarrier(
			Barrier->GetElectricMotorController(FIRST));
		Constraint.FrictionController1.InitializeBarrier(Barrier->GetFrictionController(FIRST));
		Constraint.LockController1.InitializeBarrier(Barrier->GetLockController(FIRST));
		Constraint.RangeController1.InitializeBarrier(Barrier->GetRangeController(FIRST));
		Constraint.TargetSpeedController1.InitializeBarrier(
			Barrier->GetTargetSpeedController(FIRST));

		Constraint.ElectricMotorController2.InitializeBarrier(
			Barrier->GetElectricMotorController(SECOND));
		Constraint.FrictionController2.InitializeBarrier(Barrier->GetFrictionController(SECOND));
		Constraint.LockController2.InitializeBarrier(Barrier->GetLockController(SECOND));
		Constraint.RangeController2.InitializeBarrier(Barrier->GetRangeController(SECOND));
		Constraint.TargetSpeedController2.InitializeBarrier(
			Barrier->GetTargetSpeedController(SECOND));

		Constraint.ScrewController.InitializeBarrier(Barrier->GetScrewController());
	}
}

void UAGX_Constraint2DofComponent::CreateNativeImpl()
{
	AllocateNative();
	if (!HasNative())
	{
		return;
	}

	AGX_Constraint2DofComponent_helpers::InitializeControllerBarriers(*this);
}

void UAGX_Constraint2DofComponent::UpdateNativeProperties()
{
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("AGX Constraint '%s' is trying to update native properties while not having a "
				 "native handle."),
			*GetName());
		return;
	}

	Super::UpdateNativeProperties();

	/// \todo Perhaps add a function that returns a list of all the controllers.
	ElectricMotorController1.UpdateNativeProperties();
	ElectricMotorController2.UpdateNativeProperties();
	FrictionController1.UpdateNativeProperties();
	FrictionController2.UpdateNativeProperties();
	LockController1.UpdateNativeProperties();
	LockController2.UpdateNativeProperties();
	RangeController1.UpdateNativeProperties();
	RangeController2.UpdateNativeProperties();
	TargetSpeedController1.UpdateNativeProperties();
	TargetSpeedController2.UpdateNativeProperties();
	ScrewController.UpdateNativeProperties();
}

TStaticArray<FAGX_ConstraintController*, 11> UAGX_Constraint2DofComponent::GetAllControllers()
{
	TStaticArray<FAGX_ConstraintController*, 11> Controllers;
	int32 I = 0;
	Controllers[I++] = &ElectricMotorController1;
	Controllers[I++] = &ElectricMotorController2;
	Controllers[I++] = &FrictionController1;
	Controllers[I++] = &FrictionController2;
	Controllers[I++] = &LockController1;
	Controllers[I++] = &LockController2;
	Controllers[I++] = &RangeController1;
	Controllers[I++] = &RangeController2;
	Controllers[I++] = &TargetSpeedController1;
	Controllers[I++] = &TargetSpeedController2;
	Controllers[I++] = &ScrewController;
	AGX_CHECK(I == Controllers.Num());
	return Controllers;
}

void UAGX_Constraint2DofComponent::SetNativeAddress(uint64 NativeAddress)
{
	Super::SetNativeAddress(NativeAddress);
	if (!HasNative())
	{
		return;
	}

	AGX_Constraint2DofComponent_helpers::InitializeControllerBarriers(*this);
}

#if WITH_EDITOR
void UAGX_Constraint2DofComponent::PostInitProperties()
{
	Super::PostInitProperties();

	FAGX_ConstraintUtilities::AddElectricMotorControllerPropertyCallbacks(
		PropertyDispatcher, &ElectricMotorController1,
		GET_MEMBER_NAME_CHECKED(ThisClass, ElectricMotorController1));

	FAGX_ConstraintUtilities::AddElectricMotorControllerPropertyCallbacks(
		PropertyDispatcher, &ElectricMotorController2,
		GET_MEMBER_NAME_CHECKED(ThisClass, ElectricMotorController2));

	FAGX_ConstraintUtilities::AddFrictionControllerPropertyCallbacks(
		PropertyDispatcher, &FrictionController1,
		GET_MEMBER_NAME_CHECKED(ThisClass, FrictionController1));

	FAGX_ConstraintUtilities::AddFrictionControllerPropertyCallbacks(
		PropertyDispatcher, &FrictionController2,
		GET_MEMBER_NAME_CHECKED(ThisClass, FrictionController2));

	FAGX_ConstraintUtilities::AddLockControllerPropertyCallbacks(
		PropertyDispatcher, &LockController1, GET_MEMBER_NAME_CHECKED(ThisClass, LockController1));

	FAGX_ConstraintUtilities::AddLockControllerPropertyCallbacks(
		PropertyDispatcher, &LockController2, GET_MEMBER_NAME_CHECKED(ThisClass, LockController2));

	FAGX_ConstraintUtilities::AddRangeControllerPropertyCallbacks(
		PropertyDispatcher, &RangeController1,
		GET_MEMBER_NAME_CHECKED(ThisClass, RangeController1));

	FAGX_ConstraintUtilities::AddRangeControllerPropertyCallbacks(
		PropertyDispatcher, &RangeController2,
		GET_MEMBER_NAME_CHECKED(ThisClass, RangeController2));

	FAGX_ConstraintUtilities::AddTargetSpeedControllerPropertyCallbacks(
		PropertyDispatcher, &TargetSpeedController1,
		GET_MEMBER_NAME_CHECKED(ThisClass, TargetSpeedController1));

	FAGX_ConstraintUtilities::AddTargetSpeedControllerPropertyCallbacks(
		PropertyDispatcher, &TargetSpeedController2,
		GET_MEMBER_NAME_CHECKED(ThisClass, TargetSpeedController2));

	FAGX_ConstraintUtilities::AddScrewControllerPropertyCallbacks(
		PropertyDispatcher, &ScrewController, GET_MEMBER_NAME_CHECKED(ThisClass, ScrewController));
}

void UAGX_Constraint2DofComponent::PostEditChangeChainProperty(
	struct FPropertyChangedChainEvent& Event)
{
	PropertyDispatcher.Trigger(Event, this);

	// If we are part of a Blueprint then this will trigger a RerunConstructionScript on the owning
	// Actor. That means that this object will be removed from the Actor and destroyed. We want to
	// apply all our changes before that so that they are carried over to the copy.
	Super::PostEditChangeChainProperty(Event);
}
#endif

void UAGX_Constraint2DofComponent::Serialize(FArchive& Archive)
{
	Super::Serialize(Archive);
	for (FAGX_ConstraintController* Controller : GetAllControllers())
	{
		Controller->Serialize(Archive);
	}
}
