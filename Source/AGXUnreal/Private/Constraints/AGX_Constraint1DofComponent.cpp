// Copyright 2025, Algoryx Simulation AB.

#include "Constraints/AGX_Constraint1DofComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_CustomVersion.h"
#include "AGX_LogCategory.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "Constraints/ControllerConstraintBarriers.h"
#include "Constraints/Controllers/AGX_LockController.h"
#include "Constraints/Constraint1DOFBarrier.h"
#include "Import/AGX_ImportContext.h"
#include "Utilities/AGX_ConstraintUtilities.h"

UAGX_Constraint1DofComponent::UAGX_Constraint1DofComponent(
	const TArray<EDofFlag>& LockedDofsOrdered, bool bInIsLockControllerEditable)
	: UAGX_ConstraintComponent(LockedDofsOrdered)
	, bIsLockControllerEditable(bInIsLockControllerEditable)
{
}

UAGX_Constraint1DofComponent::~UAGX_Constraint1DofComponent()
{
}

namespace
{
	FConstraint1DOFBarrier* Get1DOFBarrier(UAGX_Constraint1DofComponent& Constraint)
	{
		// See comment in GetElectricMotorController.
		return static_cast<FConstraint1DOFBarrier*>(Constraint.GetNative());
	}

	const FConstraint1DOFBarrier* Get1DOFBarrier(const UAGX_Constraint1DofComponent& Constraint)
	{
		return static_cast<const FConstraint1DOFBarrier*>(Constraint.GetNative());
	}
}

#if WITH_EDITOR
void UAGX_Constraint1DofComponent::PostInitProperties()
{
	Super::PostInitProperties();

	FAGX_PropertyChangedDispatcher<ThisClass>& PropertyDispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (PropertyDispatcher.IsInitialized())
	{
		return;
	}

	// The compiler doesn't let me pass in a lambda directly to the
	// Add.+PropertyCallbacks functions, so here I pre-create the callbacks with
	// the exact type that the parameter has. I believe the problem is that
	// templates doesn't consider inheritance when matching template type
	// parameters, i.e. a template type T already matched to TBase* isn't
	// compatible with a TDerived* template type parameter even if TDerived
	// inherit from TBase.
	//
	// Example error message:
	//
	// Could not match 'TFunction<FAGX_ConstraintElectricMotorController *(type-parameter-0-0 *)>'
	// against lambda.
	//
	// The assignments below are accepted, and I don't understand what the
	// difference is.

	TFunction<FAGX_ConstraintElectricMotorController*(ThisClass*)> GetElectricMotorController =
		[](ThisClass* EditedObject) { return &EditedObject->ElectricMotorController; };

	TFunction<FAGX_ConstraintFrictionController*(ThisClass*)> GetFrictionController =
		[](ThisClass* EditedObject) { return &EditedObject->FrictionController; };

	TFunction<FAGX_ConstraintLockController*(ThisClass*)> GetLockController =
		[](ThisClass* EditedObject) { return &EditedObject->LockController; };

	TFunction<FAGX_ConstraintRangeController*(ThisClass*)> GetRangeController =
		[](ThisClass* EditedObject) { return &EditedObject->RangeController; };

	TFunction<FAGX_ConstraintTargetSpeedController*(ThisClass*)> GetTargetSpeedController =
		[](ThisClass* EditedObject) { return &EditedObject->TargetSpeedController; };

	FAGX_ConstraintUtilities::AddElectricMotorControllerPropertyCallbacks(
		PropertyDispatcher, GetElectricMotorController,
		GET_MEMBER_NAME_CHECKED(ThisClass, ElectricMotorController));

	FAGX_ConstraintUtilities::AddFrictionControllerPropertyCallbacks(
		PropertyDispatcher, GetFrictionController,
		GET_MEMBER_NAME_CHECKED(ThisClass, FrictionController));

	FAGX_ConstraintUtilities::AddLockControllerPropertyCallbacks(
		PropertyDispatcher, GetLockController, GET_MEMBER_NAME_CHECKED(ThisClass, LockController));

	FAGX_ConstraintUtilities::AddRangeControllerPropertyCallbacks(
		PropertyDispatcher, GetRangeController,
		GET_MEMBER_NAME_CHECKED(ThisClass, RangeController));

	FAGX_ConstraintUtilities::AddTargetSpeedControllerPropertyCallbacks(
		PropertyDispatcher, GetTargetSpeedController,
		GET_MEMBER_NAME_CHECKED(ThisClass, TargetSpeedController));
}

void UAGX_Constraint1DofComponent::PostEditChangeChainProperty(
	struct FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);

	// If we are part of a Blueprint then this will trigger a RerunConstructionScript on the owning
	// Actor. That means that this object will be removed from the Actor and destroyed. We want to
	// apply all our changes before that so that they are carried over to the copy.
	Super::PostEditChangeChainProperty(Event);
}
#endif

void UAGX_Constraint1DofComponent::Serialize(FArchive& Archive)
{
	Super::Serialize(Archive);
	for (FAGX_ConstraintController* Controller : GetAllControllers())
	{
		Controller->Serialize(Archive);
	}
}

double UAGX_Constraint1DofComponent::GetAngle() const
{
	if (!HasNative())
		return 0.0;
	return Get1DOFBarrier(*this)->GetAngle();
}

float UAGX_Constraint1DofComponent::GetAngle_BP() const
{
	return static_cast<float>(GetAngle());
}

double UAGX_Constraint1DofComponent::GetSpeed() const
{
	if (!HasNative())
		return 0.0;
	return Get1DOFBarrier(*this)->GetSpeed();
}

float UAGX_Constraint1DofComponent::GetSpeed_BP() const
{
	return static_cast<float>(GetSpeed());
}

namespace AGX_Constraint1DofComponent_helpers
{
	void InitializeControllerBarriers(UAGX_Constraint1DofComponent& Constraint)
	{
		FConstraint1DOFBarrier* Barrier = Get1DOFBarrier(Constraint);
		Constraint.ElectricMotorController.InitializeBarrier(Barrier->GetElectricMotorController());
		Constraint.FrictionController.InitializeBarrier(Barrier->GetFrictionController());
		Constraint.LockController.InitializeBarrier(Barrier->GetLockController());
		Constraint.RangeController.InitializeBarrier(Barrier->GetRangeController());
		Constraint.TargetSpeedController.InitializeBarrier(Barrier->GetTargetSpeedController());
	}
}

void UAGX_Constraint1DofComponent::CreateNativeImpl()
{
	AllocateNative();
	if (!HasNative())
	{
		return;
	}

	AGX_Constraint1DofComponent_helpers::InitializeControllerBarriers(*this);
}

void UAGX_Constraint1DofComponent::UpdateNativeProperties()
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

	if (ElectricMotorController.HasNative())
		ElectricMotorController.UpdateNativeProperties();
	if (FrictionController.HasNative())
		FrictionController.UpdateNativeProperties();
	if (LockController.HasNative())
		LockController.UpdateNativeProperties();
	if (RangeController.HasNative())
		RangeController.UpdateNativeProperties();
	if (TargetSpeedController.HasNative())
		TargetSpeedController.UpdateNativeProperties();
}

void UAGX_Constraint1DofComponent::CopyFrom(
	const FConstraintBarrier& Barrier, FAGX_ImportContext* Context)
{
	UAGX_ConstraintComponent::CopyFrom(Barrier, Context);
	FAGX_ConstraintUtilities::CopyControllersFrom(
		*this, *static_cast<const FConstraint1DOFBarrier*>(&Barrier));
}

TStaticArray<FAGX_ConstraintController*, 5> UAGX_Constraint1DofComponent::GetAllControllers()
{
	TStaticArray<FAGX_ConstraintController*, 5> Controllers;
	int32 I = 0;
	Controllers[I++] = &ElectricMotorController;
	Controllers[I++] = &FrictionController;
	Controllers[I++] = &LockController;
	Controllers[I++] = &RangeController;
	Controllers[I++] = &TargetSpeedController;
	AGX_CHECK(I == Controllers.Num());
	return Controllers;
}

void UAGX_Constraint1DofComponent::SetNativeAddress(uint64 NativeAddress)
{
	Super::SetNativeAddress(NativeAddress);
	if (!HasNative())
	{
		return;
	}

	AGX_Constraint1DofComponent_helpers::InitializeControllerBarriers(*this);
}
