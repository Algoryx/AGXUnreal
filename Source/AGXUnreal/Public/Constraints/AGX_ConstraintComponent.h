// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AMOR/AGX_ConstraintMergeSplitProperties.h"
#include "AGX_NativeOwner.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "AGX_RealInterval.h"
#include "Constraints/AGX_ConstraintBodyAttachment.h"
#include "Constraints/AGX_ConstraintEnums.h"
#include "Constraints/AGX_ConstraintPropertyPerDof.h"
#include "Constraints/ConstraintBarrier.h" // TODO: Shouldn't be necessary here!

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Components/SceneComponent.h"

#include "AGX_ConstraintComponent.generated.h"

struct FAGX_ConstraintController;
struct FAGX_ImportContext;
class FConstraintBarrier;

/**
 * Component owned by every Constraint Actor so that component features can be used.
 * For example, enables the usage of a Component Visualizer, so that helpful graphics
 * can be shown in the Level Editor Viewport when editing the constraint.
 *
 * @see FAGX_ConstraintComponentVisualizer
 *
 */
UCLASS(Category = "AGX", ClassGroup = "AGX_Constraint", NotPlaceable)
class AGXUNREAL_API UAGX_ConstraintComponent : public USceneComponent, public IAGX_NativeOwner
{
	GENERATED_BODY()

public:
	UAGX_ConstraintComponent();

protected:
	UAGX_ConstraintComponent(const TArray<EDofFlag>& LockedDofsOrdered);

public:
	/**
	 * The first Rigid Body bound by this constraint, and its Attachment Frame definition.
	 * Rigid Body Actor must be set.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX Constraint Bodies", Meta = (ExposeOnSpawn))
	FAGX_ConstraintBodyAttachment BodyAttachment1;

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Attachment")
	bool SetBody1(UAGX_RigidBodyComponent* Body);

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Attachment")
	void SetConstraintAttachmentLocation1(FVector BodyLocalLocation);

	/**
	 * The second Rigid Body bound by this constraint, and its Attachment Frame definition.
	 * If second Rigid Body is null, the first Rigid Body will be constrained to the World.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX Constraint Bodies", Meta = (ExposeOnSpawn))
	FAGX_ConstraintBodyAttachment BodyAttachment2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX AMOR")
	FAGX_ConstraintMergeSplitProperties MergeSplitProperties;

	UFUNCTION(BlueprintCallable, Category = "AGX AMOR")
	void CreateMergeSplitProperties();

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Attachment")
	bool SetBody2(UAGX_RigidBodyComponent* Body);

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Attachment")
	void SetConstraintAttachmentLocation2(FVector BodyLocalLocation);

	UPROPERTY(EditAnywhere, Category = "AGX Constraint")
	bool bEnable = true;

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint")
	void SetEnable(bool InEnable);

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint")
	bool GetEnable() const;

	/**
	 * Specifies whether any contacts can be generated between the attached Rigid Bodies.
	 * If less than two Rigid Bodies are attached to this Constraint, this has no effect.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Constraint")
	bool bSelfCollision = true;

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint")
	void SetEnableSelfCollision(bool InEnable);

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint")
	bool GetEnableSelfCollision() const;

	/**
	 * Solve type for this constraint. Valid is DIRECT (default for non-iterative solvers),
	 * ITERATIVE or DIRECT_AND_ITERATIVE where DIRECT_AND_ITERATIVE means that this constraint
	 * will be solved both direct and iterative.
	 *
	 * Note that solve type is ignored by iterative solvers.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Constraint")
	TEnumAsByte<enum EAGX_SolveType> SolveType;

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint")
	void SetSolveType(EAGX_SolveType InSolveType);

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint")
	EAGX_SolveType GetSolveType() const;

	/**
	 * The compliance in a certain DOF. Measured in [m/N] for translational DOFs and [rad/Nm] for
	 * rotational DOFs.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Constraint",
		Meta = (SliderMin = "0", SliderMax = "1", SliderExponent = "10000"))
	FAGX_ConstraintDoublePropertyPerDof Compliance;

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint")
	void SetCompliance(EGenericDofIndex Index, double InCompliance);

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint")
	double GetCompliance(EGenericDofIndex Index) const;

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint")
	void SetElasticity(EGenericDofIndex Index, double InElasticity);

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint")
	double GetElasticity(EGenericDofIndex Index) const;

	/**
	 * The damping (spook) of the Constraint [s].
	 * The value is the time the constraint has to fulfill its violation.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Constraint",
		Meta = (SliderMin = "1e-2", SliderMax = "5.0", SliderExponent = "2"))
	FAGX_ConstraintDoublePropertyPerDof SpookDamping;

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint")
	void SetSpookDamping(EGenericDofIndex Index, double InSpookDamping);

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint")
	double GetSpookDamping(EGenericDofIndex Index) const;

	/**
	 * The force range in a certain DOF. Measured in [N] for translational DOFs and [Nm] for
	 * rotational DOFs.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Constraint")
	FAGX_ConstraintRangePropertyPerDof ForceRange;

	void SetForceRange(EGenericDofIndex Index, const FAGX_RealInterval& InForceRange);

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint")
	void SetForceRange(EGenericDofIndex Index, double Min, double Max);

	FAGX_RealInterval GetForceRange(EGenericDofIndex Index) const;

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint")
	void GetForceRange(EGenericDofIndex Index, double& Min, double& Max) const;

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint")
	double GetForceRangeMin(EGenericDofIndex Index) const;

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint")
	double GetForceRangeMax(EGenericDofIndex Index) const;

	/**
	 * Enable or disable computation of the forces applied to the dynamic bodies in this constraint.
	 * This adds the cost of a matrix-vector operation to compute the forces after solve.
	 * @see GetLastForce
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Constraint")
	bool bComputeForces = false;

	/**
	 * Enable or disable computation of the forces applied to the dynamic bodies in this constraint.
	 * This adds the cost of a matrix-vector operation to compute the forces after solve.
	 * @see GetLastForce
	 * @param bInComputeForces True to enable force computation, false to disable it.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Constraint")
	void SetComputeForces(bool bInComputeForces);

	/**
	 * @return True if this constraint has been enabled to compute the forces applied to its bodies,
	 * false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Constraint")
	bool GetComputeForces() const;

	/**
	 * Enable or disable computation of the forces applied to the dynamic bodies in this constraint.
	 * This adds the cost of a matrix-vector operation to compute the forces after solve.
	 * Alias provided for API compatibility with AGX Dynamics.
	 * @see SetComputeForces.
	 * @see GetLastForce
	 * @param bInComputeForces True to enable force computation, false to disable it.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Constraint")
	void SetEnableComputeForces(bool bInEnable);

	/**
	 * @return True if this constraint has been enabled to compute the forces applied to its bodies,
	 * false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Constraint")
	bool GetEnableComputeForces() const;

	/**
	 * Check if the native AGX Dynamics constraint has been successfully created and initialized.
	 *
	 * Not to be confused with Unreal Engine's Is Valid. A Constraint Component can be a valid
	 * Unreal Engine Component without having a valid native AGX Dynamics constraint.
	 *
	 * @return True if the native AGX Dynamics constraint has been successfully created and
	 * initialized.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Constraint")
	bool GetValid() const;

	/**
	 * Get the current force [N] or torque [Nm] of the constraint along a particular degree of freedom.
	 *
	 * Only degrees of freedoms listed in the return value of GetLockedDofsBitmask may be passed.
	 *
	 * Consider using one of the Get Last Force functions instead.
	 *
	 * @param Dof Degree of freedom to get current force for.
	 * @return Constraint force in the given degree of freedom.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Constraint")
	double GetCurrentForce(EGenericDofIndex Dof) const;

	/**
	 * If Compute Forces is enabled, returns the last force [N] and torque [Nm] applied by this
	 * constraint on the body at \p BodyIndex. The force is given in world coordinates and is the
	 * one applied at the anchor position of this constraint.
	 *
	 * The result includes force and torque from this constraint including all enabled controllers
	 * such as motors, locks, and ranges.
	 *
	 * \see SetComputeForces
	 * \param BodyIndex Index of body, if number of bodies = 1 and bodyIndex = 1, the force and
	 * torque applied to "the world body" is returned.
	 * \param OutForce The force applied by this constraint on body at \p bodyIndex at the last
	 * solve.
	 * \param OutTorque The torque applied by this constraint on body at \p bodyIndex last
	 * solve.
	 * \param bForceAtCm This parameter affects the resulting torque. The default behavior
	 * (bForceAtCm = false) calculates the force applied by this constraint at the anchor
	 * position. Setting bForceAtCm = true, the force will be applied at center of mass,
	 * affecting the torque as \f$ T_{new} = T - r \times F \f$ where r is the vector from the
	 * anchor point to the center of mass of the body.
	 * \return True if resulting force and torque was written to \p OutForce and \p OutTorque, false
	 * otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Constraint")
	bool GetLastForceIndex(
		int32 BodyIndex, FVector& OutForce, FVector& OutTorque, bool bForceAtCm = false) const;

	/**
	 * If Compute Forces is enabled, returns the last force [N] and torque [Nm] applied by this
	 * constraint on the body \p Body. The force is given in world coordinates and is the one
	 * applied at the anchor position of this constraint.
	 *
	 * The result includes force and torque from this constraint including all enabled controllers
	 * such as motors, locks, and ranges.
	 *
	 * \see SetComputeForces
	 * \param Body Body in this constraint. If this constraint is attached in world, nullptr can be
	 * used.
	 * \param OutForce The force applied by this constraint on body \p Body at the last solve.
	 * \param OutTorque The torque applied by this constraint on body \p Body at the last solve.
	 * \param bForceAtCm This parameter affects the resulting torque. The default behavior
	 * (bForceAtCm = false) calculates the force applied by this constraint at the anchor
	 * position. Setting bForceAtCm = true, the force will be applied at center of mass,
	 * affecting the torque as \f$T_{new} = T - r \times F\f$ where r is the vector from the anchor
	 * point to the center of mass of the body.
	 * \return true if resulting force and torque was written to \p OutForce and \p OutTorque, false
	 * otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Constraint")
	bool GetLastForceBody(
		const UAGX_RigidBodyComponent* Body, FVector& OutForce, FVector& OutTorque,
		bool bForceAtCm = false) const;

	/**
	 * If Compute Forces is enabled, returns the last force [N] and torque [Nm] applied by this
	 * constraint on the body at \p BodyIndex. The force is given in the frame of the constraint.
	 *
	 * The result includes force and torque from this constraint including all enabled controllers
	 * such as motors, locks, and ranges.
	 *
	 * \see setComputeForces
	 * \param BodyIndex Index of body, if number of bodies = 1 and bodyIndex = 1, the force and
	 * torque applied to "the world body" is returned.
	 * \param OutForce The force applied by this constraint on body at \p BodyIndex last solve.
	 * \param OutTorque The torque applied by this constraint on body at \p BodyIndex last solve.
	 * \param bForceAtCm This parameter affects the resulting torque. The default behavior
	 * (bForceAtCm = false) calculates the force applied by this constraint at the anchor
	 * position. Setting bForceAtCm = true, the force will be applied at center of mass,
	 * affecting the torque as \f$T_{new} = T - r \times F\f$ where r is the vector from the anchor
	 * point to the center of mass of the body.
	 * \return True if resulting force and torque was written to \p OutForce and \p OutTorque, false
	 * otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Constraint")
	bool GetLastLocalForceIndex(
		int32 BodyIndex, FVector& OutForce, FVector& OutTorque, bool bForceAtCm = false) const;

	/**
	 * If Compute Forces is enabled, returns the last force [N] and torque [Nm] applied by this
	 * constraint on the body \p Body. The force is given in the frame of the constraint.
	 *
	 * The result includes force and torque from this constraint including all enabled controllers
	 * such as motors, locks, and ranges.
	 *
	 * \see setEnableComputeForces
	 *
	 * \param Body Body in this constraint. If this constraint is attached in world, nullptr can be
	 * used.
	 * \param OutForce The force applied by this constraint on body \p Body at the last solve.
	 * \param OutTorque The torque applied by this constraint on body \p Body at the last solve.
	 * \param bForceAtCm This parameter affects the resulting torque. The
	 * default behavior (bForceAtCm = false) calculates the force applied by this constraint at
	 * the anchor position. Setting bForceAtCm = true, the force will be applied at center of
	 * mass, affecting the torque as \f$T_{new} = T - r \times F\f$ where r is the vector from the
	 * anchor point to the center of mass of the body.
	 * \return True if resulting force and torque was written to \p OutForce and \p OutTorque, false
	 * otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Constraint")
	bool GetLastLocalForceBody(
		const UAGX_RigidBodyComponent* Body, FVector& OutForce, FVector& OutTorque,
		bool bForceAtCm = false) const;

	virtual void CopyFrom(const FConstraintBarrier& Barrier, FAGX_ImportContext* Context);

	/**
	 * Returns true if for any of the locked DOFs both the global attachment frame transforms do no
	 * match.
	 *
	 * This function should never be used after the constraint has begun play.
	 *
	 * Can be overriden for specialized constraint checks.
	 */
	virtual bool AreFramesInViolatedState(
		float Tolerance = KINDA_SMALL_NUMBER, FString* OutMessage = nullptr) const;

	EDofFlag GetLockedDofsBitmask() const;

	bool IsDofLocked(EDofFlag Dof) const;

	bool IsRotational() const;

	// ~Begin IAGX_NativeOwner interface.
	virtual bool HasNative() const override;
	virtual uint64 GetNativeAddress() const override;
	virtual void SetNativeAddress(uint64 NativeAddress) override;
	// ~End IAGX_NativeOwner interface.

	/** Get the native AGX Dynamics representation of this constraint. Create it if necessary. */
	FConstraintBarrier* GetOrCreateNative();

	/** Get the native AGX Dynamics representation of this constraint. May return nullptr. */
	FConstraintBarrier* GetNative();

	/** Get the native AGX Dynamics representation of this constraint. May return nullptr. */
	const FConstraintBarrier* GetNative() const;

	/** Subclasses that overrides this MUST invoke the parents version! */
	virtual void UpdateNativeProperties();

	void UpdateNativeCompliance();

	void UpdateNativeSpookDamping();

	// ~Begin UActorComponent interface.
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	// ~End UActorComponent interface.

	//~ Begin UObject interface.
	// Shortly after construction, before deserialization.
	virtual void PostInitProperties() override;

#if WITH_EDITOR
	// When loaded in Editor/Game. Not called when creating a new object from the Place Mode.
	virtual void PostLoad() override;

	// When copied in Editor.
	virtual void PostDuplicate(bool bDuplicateForPIE) override;

	// When a property is changed from the Details Panel, and possibly other situations.
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditChangeChainProperty(
		struct FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif

	virtual void OnRegister() override;
	// Called when deleted in editor, in game, if owning actor is deleted or when switching level.
	virtual void OnUnregister() override;

	virtual void Serialize(FArchive& Archive) override;

	//~ End UObject interface.

	bool ToNativeDof(EGenericDofIndex GenericDof, int32& NativeDof) const;

	//~ Begin UActorComponent Interface
	virtual TStructOnScope<FActorComponentInstanceData> GetComponentInstanceData() const override;
	//~ End UActorComponent Interface

	/*
	 * The import Guid of this Component. Only used by the AGX Dynamics for Unreal import system.
	 * Should never be assigned manually.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "AGX Dynamics Import Guid")
	FGuid ImportGuid;

public: // Deprecated functions.
	UFUNCTION(
		BlueprintCallable, Category = "AGX Constraint",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage = "Use SetCompliance instead of SetComplianceFloat"))
	void SetComplianceFloat(EGenericDofIndex Index, float InCompliance);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Constraint",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage = "Use GetCompliance instead of GetComplianceFloat"))
	float GetComplianceFloat(EGenericDofIndex Index) const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Constraint",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage = "Use SetElasticity instead of SetElasticityFloat"))
	void SetElasticityFloat(EGenericDofIndex Index, float InElasticity);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Constraint",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage = "Use GetElasticity instead of GetElasticityFloat"))
	float GetElasticityFloat(EGenericDofIndex Index) const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Constraint",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage = "Use SetSpookDamping instead of SetSpookDampinFloat"))
	void SetSpookDampingFloat(EGenericDofIndex Index, float InSpookDamping);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Constraint",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage = "Use GetSpookDamping instead of GetSpookDampingFloat"))
	float GetSpookDampingFloat(EGenericDofIndex Index) const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Constraint",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage = "Use SetForceRange instead of SetForceRangeFloat"))
	void SetForceRangeFloat(EGenericDofIndex Index, float RangeMin, float RangeMax);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Constraint",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage = "Use GetForceRangeMin instead of GetForceRangeMinFloat"))
	float GetForceRangeMinFloat(EGenericDofIndex Index) const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Constraint",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage = "Use GetForceRangeMax instead of GetForceRangeMaxFloat"))
	float GetForceRangeMaxFloat(EGenericDofIndex Index) const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Constraint",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage = "Use GetForceRange instead of GetForceRangeFloat"))
	void GetForceRangeFloat(EGenericDofIndex Index, float& Min, float& Max) const;

protected:
	/**
	 * Allocate a type-specific native constraint and point NativeBarrier to it. Perform any
	 * constraint-specific configuration that may be necessary, such as binding secondary constraint
	 * barriers to their respective native objects within the native constraint.
	 */
	virtual void CreateNativeImpl() PURE_VIRTUAL(AAGX_Constraint::CreateNativeImpl, );

private:
	void InitPropertyDispatcher();

	/**
	 * Invokes CreateNativeImpl, then adds the native to the simulation.
	 */
	void CreateNative();

protected:
	/**
	 * Pointer to the Barrier object that holds the FConstraintRef that holds the agx::ConstraintRef
	 * that holds the agx::Constraint.
	 *
	 * Most Components hold the Barrier by-value but here we use a pointer because we need virtual
	 * dispatch for AllocateNativeImpl, the member function that does the allocation of the AGX
	 * Dynamics constraint, i.e., agx::Hinge, etc. There may be other ways to achieve the same goal
	 * that doesn't require an extra indirection.
	 *
	 * NativeBarrier pointer is initialized in each specific Constraint Component subclass so a
	 * Barrier object is always available, just like for the Components that store the Barrier
	 * by-value, and just like them the Barrier may be empty, i.e., not had the AGX Dynamics object
	 * created yet.
	 */
	TUniquePtr<FConstraintBarrier> NativeBarrier;

private:
	const EDofFlag LockedDofsBitmask = static_cast<EDofFlag>(0);

	// The Degrees of Freedom (DOF) that are locked by the specific constraint type,
	// ordered the way they are indexed by in the native AGX api (except for ALL_DOF and NUM_DOF).
	const TArray<EDofFlag> LockedDofs;

	// Mapping from EGenericDofIndex to native AGX constraint specific DOF index.
	// This list can change with each constraint type, and should exactly reflect
	// the DOF enum in the native header for each constraint.
	const TMap<EGenericDofIndex, int32> NativeDofIndexMap;

private: // Deprecated functionality.
	/**
	 * The elasticity in a certain DOF. Measured in [N/m] for translational DOFs and [Nm/rad] for
	 * rotational DOFs.
	 */
	UPROPERTY()
	FAGX_ConstraintDoublePropertyPerDof Elasticity_DEPRECATED;
	// Deprecated because AGX Dynamics uses Compliance, which is the inverse of Elasticity. We
	// switched to Elasticity in AGX Dynamics for Unreal because Unreal Engine can't handle small
	// numbers and Compliance is typically small. Now that we have FAGX_Real to help with small
	// numbers we can switch back to Compliance again.
};
