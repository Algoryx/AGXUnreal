// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Constraints/AGX_Constraint1DofComponent.h"
#include "Constraints/AGX_Constraint2DofComponent.h"
#include "Constraints/AGX_Constraint2DOFFreeDOF.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <type_traits>

class FBallJointBarrier;
class FConstraint1DOFBarrier;
class FConstraint2DOFBarrier;
class UAGX_BallConstraintComponent;
class UAGX_Constraint1DofComponent;
class UAGX_RigidBodyComponent;

struct FAGX_ConstraintElectricMotorController;
struct FAGX_ConstraintFrictionController;
struct FAGX_ConstraintLockController;
struct FAGX_ConstraintRangeController;
struct FAGX_ConstraintTargetSpeedController;
struct FAGX_TwistRangeController;

template <typename>
struct FAGX_PropertyChangedDispatcher;

class AGXUNREAL_API FAGX_ConstraintUtilities
{
public:
	/**
	 * Copy constraint controller properties, such as enabled, compliance, force range, voltage,
	 * from the AGX Dynamics constraint to the AGXUnreal constraint.
	 * @param Component The AGXUnreal constraint to copy properties to.
	 * @param Barrier The AGX Dynamics constraint to copy properties from.
	 */
	static void CopyControllersFrom(
		UAGX_Constraint1DofComponent& Component, const FConstraint1DOFBarrier& Barrier);

	/**
	 * Copy constraint controller properties, such as enabled, compliance, force range, voltage,
	 * from the AGX Dynamics constraint to the AGXUnreal constraint.
	 * @param Component The AGXUnreal constraint to copy properties to.
	 * @param Barrier The AGX Dynamics constraint to copy properties from.
	 */
	static void CopyControllersFrom(
		UAGX_Constraint2DofComponent& Component, const FConstraint2DOFBarrier& Barrier);

	static void CopyControllersFrom(
		UAGX_BallConstraintComponent& Component, const FBallJointBarrier& Barrier);

	static void StoreElectricMotorController(
		const FConstraint1DOFBarrier& Barrier, FAGX_ConstraintElectricMotorController& Controller);

	static void StoreElectricMotorController(
		const FConstraint2DOFBarrier& Barrier, FAGX_ConstraintElectricMotorController& Controller,
		EAGX_Constraint2DOFFreeDOF Dof);

	static void StoreFrictionController(
		const FConstraint1DOFBarrier& Barrier, FAGX_ConstraintFrictionController& Controller);

	static void StoreFrictionController(
		const FConstraint2DOFBarrier& Barrier, FAGX_ConstraintFrictionController& Controller,
		EAGX_Constraint2DOFFreeDOF Dof);

	static void StoreLockController(
		const FConstraint1DOFBarrier& Barrier, FAGX_ConstraintLockController& Controller);

	static void StoreLockController(
		const FConstraint2DOFBarrier& Barrier, FAGX_ConstraintLockController& Controller,
		EAGX_Constraint2DOFFreeDOF Dof);

	static void StoreRangeController(
		const FConstraint1DOFBarrier& Barrier, FAGX_ConstraintRangeController& Controller);

	static void StoreRangeController(
		const FConstraint2DOFBarrier& Barrier, FAGX_ConstraintRangeController& Controller,
		EAGX_Constraint2DOFFreeDOF Dof);

	static void StoreTargetSpeedController(
		const FConstraint1DOFBarrier& Barrier, FAGX_ConstraintTargetSpeedController& Controller);

	static void StoreTargetSpeedController(
		const FConstraint2DOFBarrier& Barrier, FAGX_ConstraintTargetSpeedController& Controller,
		EAGX_Constraint2DOFFreeDOF Dof);

	static void StoreScrewController(
		const FConstraint2DOFBarrier& Barrier, FAGX_ConstraintScrewController& Controller);

	static void StoreTwistRangeController(
		const FBallJointBarrier& Barrier, FAGX_TwistRangeController& Controller);

#if WITH_EDITOR
	template <typename UConstraintClass, typename FControllerClass>
	static void AddControllerPropertyCallbacks(
		FAGX_PropertyChangedDispatcher<UConstraintClass>& PropertyDispatcher,
		TFunction<FControllerClass*(UConstraintClass*)> GetController, const FName& Member);

	template <typename UConstraintClass>
	static void AddElectricMotorControllerPropertyCallbacks(
		FAGX_PropertyChangedDispatcher<UConstraintClass>& PropertyDispatcher,
		TFunction<FAGX_ConstraintElectricMotorController*(UConstraintClass*)> GetController,
		const FName& Member);

	template <typename UConstraintClass>
	static void AddFrictionControllerPropertyCallbacks(
		FAGX_PropertyChangedDispatcher<UConstraintClass>& PropertyDispatcher,
		TFunction<FAGX_ConstraintFrictionController*(UConstraintClass*)> GetController,
		const FName& Member);

	template <typename UConstraintClass>
	static void AddLockControllerPropertyCallbacks(
		FAGX_PropertyChangedDispatcher<UConstraintClass>& PropertyDispatcher,
		TFunction<FAGX_ConstraintLockController*(UConstraintClass*)> GetController,
		const FName& Member);

	template <typename UConstraintClass>
	static void AddRangeControllerPropertyCallbacks(
		FAGX_PropertyChangedDispatcher<UConstraintClass>& PropertyDispatcher,
		TFunction<FAGX_ConstraintRangeController*(UConstraintClass*)> GetController,
		const FName& Member);

	template <typename UConstraintClass>
	static void AddTargetSpeedControllerPropertyCallbacks(
		FAGX_PropertyChangedDispatcher<UConstraintClass>& PropertyDispatcher,
		TFunction<FAGX_ConstraintTargetSpeedController*(UConstraintClass*)> GetController,
		const FName& Member);

	template <typename UConstraintClass>
	static void AddScrewControllerPropertyCallbacks(
		FAGX_PropertyChangedDispatcher<UConstraintClass>& PropertyDispatcher,
		TFunction<FAGX_ConstraintScrewController*(UConstraintClass*)> GetController,
		const FName& Member);

	template <typename UConstraintClass>
	static void AddTwistRangeControllerPropertyCallbacks(
		FAGX_PropertyChangedDispatcher<UConstraintClass>& PropertyDispatcher,
		TFunction<FAGX_TwistRangeController*(UConstraintClass*)> GetController,
		const FName& Member);
#endif

	/**
	 * Sets up the constraint 'Component' and its BodyAttachments in accordance with
	 * FrameDefiningSource = Constraint, given an FConstraintBarrier and the constrained
	 * RigidBodies. Returns the new World Transform of the Constraint. If Component is a Component
	 * template, the caller of this function is responsible for setting the Component's world's
	 * transform and updating its archetype instances. The returned Transform holds this data.
	 */
	static FTransform SetupConstraintAsFrameDefiningSource(
		const FConstraintBarrier& Barrier, UAGX_ConstraintComponent& Component,
		UAGX_RigidBodyComponent* RigidBody1, UAGX_RigidBodyComponent* RigidBody2,
		bool ForceOverwriteInstances = false);

	static void CreateNative(
		FConstraintBarrier* Barrier, FAGX_ConstraintBodyAttachment& Attachment1,
		FAGX_ConstraintBodyAttachment& Attachment2, const FName& ConstraintName,
		const FString& ActorLabel);

	/**
	 * Get the Barrier object for the given Component.
	 *
	 * The Component must have a FBarrierType member type that declare the type
	 * of the Barrier that the Component contains.
	 *
	 * Will return nullptr if a native AGX Dynamics object hasn't been created yet.
	 *
	 * @tparam UComponent The type of the Component. Deduced from the parameter.
	 * @tparam bIsConst Whether or not the types are const. Deduced from the parameter.
	 * @param Component The Component to get the Barrier from.
	 * @return The Barrier object created for the given Component, if one has been created.
	 */
	template <typename UComponent, bool bIsConst = std::is_const<UComponent>::value>
	static typename std::conditional<
		bIsConst, const typename UComponent::FBarrierType*,
		typename UComponent::FBarrierType*>::type
	GetNativeCast(UComponent* Component);
};

template <typename UComponent, bool bIsConst>
typename std::conditional<
	bIsConst, const typename UComponent::FBarrierType*, typename UComponent::FBarrierType*>::type
FAGX_ConstraintUtilities::GetNativeCast(UComponent* Component)
{
	if (Component == nullptr)
	{
		return nullptr;
	}

	using FBarrierType = typename std::conditional<
		bIsConst, const typename UComponent::FBarrierType, typename UComponent::FBarrierType>::type;
	using FBaseBarrier =
		typename std::conditional<bIsConst, const FConstraintBarrier, FConstraintBarrier>::type;

	FBaseBarrier* BaseBarrier = Component->GetNative();
	if (BaseBarrier == nullptr)
	{
		return nullptr;
	}

	// We "know" that the Barrier can only ever be the correct type, but it would be nice to be able
	// to make sure. But we can't use dynamic_cast since Unreal Engine disables RTTI.
	FBarrierType* Barrier = static_cast<FBarrierType*>(BaseBarrier);
	if (Barrier == nullptr)
	{
		UE_LOG(LogAGX, Error, TEXT("Found Component with a mismatched Barrier type."));
		return nullptr;
	}

	return Barrier;
}
