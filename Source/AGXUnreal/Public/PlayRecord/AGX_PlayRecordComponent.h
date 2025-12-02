// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_MotionControl.h"

// Unreal Engine includes.
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "AGX_PlayRecordComponent.generated.h"

class UAGX_ConstraintComponent;
class UAGX_PlayRecord;
class UAGX_RigidBodyComponent;

/**
 * EXPERIMENTAL
 *
 * This Component enables simple Constraint position recording and playback functionality.
 *
 * When recording or playing back Constraint positions, it is recommended to use a
 * deterministic/stable tick event such as the Pre or Post Step Forward events exposed by the AGX
 * Simulation.
 *
 * This Component does not guarantee exact constraint forces, torques or trajectories during
 * playback and uses position control internally.
 */
UCLASS(
	ClassGroup = "AGX", Category = "AGX", Experimental, Meta = (BlueprintSpawnableComponent),
	Hidecategories = (Cooking, Collision, LOD, Physics, Rendering, Replication))
class AGXUNREAL_API UAGX_PlayRecordComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAGX_PlayRecordComponent();

	//~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type Reason) override;
	//~ End UActorComponent Interface

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Play Record")
	UAGX_PlayRecord* PlayRecord = nullptr;

	/**
	 * Writes the positions of the given Constraints to the PlayRecord Asset.
	 * The written data is permanently stored in the asset, even after Play.
	 * Note that old recorded data will be permanently removed from the Asset at the start of a new
	 * recording.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Play Record")
	void RecordConstraintPositions(const TArray<UAGX_ConstraintComponent*>& Constraints);

	/**
	 * Writes the positions and rotations of the given Rigid Bodies to the Play Record asset.
	 * The written data is permanently stored in the asset, even after Play ends.
	 * Note that old recorded data will be pemanently removed from the asset at the start of a new
	 * recording.
	 * @param RigidBodies The Rigid Bodies that shold be recorded.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Play Record")
	void RecordRigidBodyPositions(const TArray<UAGX_RigidBodyComponent*>& RigidBodies);

	/**
	 * Apply the next Constraint positions stored in the PlayRecord Asset to the given Constraints.
	 * The number of Constraints and their order should match those used to create the PlayRecord
	 * Asset if matching behavior is wanted. Note that this function uses purely position control
	 * internally and does not guarantee same Constraint force/torques.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Play Record")
	void PlayBackConstraintPositions(const TArray<UAGX_ConstraintComponent*>& Constraints);

	/**
	 * Apply the next Rigid Body positions and rotations stored in the PlayRecord asset to the given
	 * Rigid Bodies. The number of Rigid Bodies and their order must match those used to create
	 * the PlayRecord asset if matching behavior is wanted. Notes that this function sets the Motion
	 * Control to Kinematics on the bodies and uses Move To to move them to the next position and
	 * rotation. At the end of the playback the bodies will be restored to their original Motion
	 * Control.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Play Record")
	void PlayBackRigidBodyPositions(const TArray<UAGX_RigidBodyComponent*>& RigidBodies);

	/**
	 * Resets the internal counter such that e.g. a subsequent playback will start from the
	 * beginning and any subsequent recording will be written at the beginning of the PlayRecord
	 * Asset.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Play Record")
	void Reset();

	/**
	 * Advanced: Initial allocation size for the number of States stored in the given PlayRecord
	 * Asset. This is purely related to performance. Example usage: given a 60Hz Simulation and 100
	 * second long recording, at least 6000 should be set for this property if re-allocation of the
	 * internal States array should be avoided.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Play Record Advanced")
	int32 InitialStatesAllocationSize {3600};

private:
	int32 CurrentIndex {0};

	TMap<UAGX_RigidBodyComponent*, EAGX_MotionControl> OriginalMotionControl;
};
