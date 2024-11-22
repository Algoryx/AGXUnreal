// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_Real.h"
#include "Vehicle/AGX_TrackEnums.h"

// Unreal Engine include.s
#include "UObject/Object.h"

#include "AGX_TrackInternalMergeProperties.generated.h"

class UAGX_TrackComponent;
class FTrackBarrier;

/**
 * Properties and thresholds for internal merging of nodes in AGX Track Component.
 *
 * Does not own any AGX Native. Instead, it applies its data to the native Track of each
 * Track Component that uses this asset. This means that modifications done directly on a Track
 * will overwrite the state set by a Track Internal Merge Properties but it will not override
 * it, meaning that setting the property on the Track Internal Merge Properties again will overwrite
 * the value set on the Track directly. Reading will always read the Track Internal Merge Properties
 * value, which may be different from the Tracks' value(s) if it has been overwritten on a
 * particular Track.
 *
 * Objects of this class exists for two different purposes. One type of object represents an asset,
 * something that lives exists persistently on disk and is manipulated in the Content Browser. When
 * creating scenes and Blueprints this is the objects that is used. These objects do not have an AGX
 * Dynamics Native associated with them. The other type of object are instances. They only exists
 * during Play and do have an AGX Dynamics Native associated with them. On Begin Play every class
 * that has an UAGX_TrackInternalMergeProperties pointer should swap that pointer from the asset
 * object to the instance object by calling GetOrCreateInstance on the asset and overwriting the
 * asset pointer with the instance pointer.
 *
 * The reason for doing the switch is that it should be possible to make changes to the instance
 * object during Play without altering the persistent asset, while still being able to edit the
 * asset object from within Unreal Editor.
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", BlueprintType)
class AGXUNREAL_API UAGX_TrackInternalMergeProperties : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Whether to enable merging of internal nodes into segments in the track.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Track Internal Merge Properties")
	bool bEnableMerge {false};

	UFUNCTION(BlueprintCallable, Category = "AGX Track Internal Merge Properties")
	void SetMergeEnabled(bool bInEnabled);

	UFUNCTION(BlueprintCallable, Category = "AGX Track Internal Merge Properties")
	bool GetMergeEnabled() const;

	/**
	 * Maximum number of consecutive nodes that may be merged together into a segment.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Track Internal Merge Properties",
		Meta = (EditCondition = "bMergeEnabled", ClampMin = "1"))
	int32 NumNodesPerMergeSegment {3};

	UFUNCTION(BlueprintCallable, Category = "AGX Track Internal Merge Properties")
	void SetNumNodesPerMergeSegment(int32 InNumNodesPerMergeSegment);

	UFUNCTION(BlueprintCallable, Category = "AGX Track Internal Merge Properties")
	int32 GetNumNodesPerMergeSegment() const;

	/**
	 * Contact reduction level of merged nodes against other objects, such as ground.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Track Internal Merge Properties",
		Meta = (EditCondition = "bMergeEnabled"))
	EAGX_MergedTrackNodeContactReduction ContactReduction {
		EAGX_MergedTrackNodeContactReduction::Minimal};

	UFUNCTION(BlueprintCallable, Category = "AGX Track Internal Merge Properties")
	void SetContactReduction(EAGX_MergedTrackNodeContactReduction InContactReduction);

	UFUNCTION(BlueprintCallable, Category = "AGX Track Internal Merge Properties")
	EAGX_MergedTrackNodeContactReduction GetContactReduction() const;

	/**
	 * Whether to enable the usage of hinge lock to reach merge condition, i.e. angle close to zero.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Track Internal Merge Properties",
		Meta = (EditCondition = "bMergeEnabled"))
	bool bEnableLockToReachMergeCondition {true};

	UFUNCTION(BlueprintCallable, Category = "AGX Track Internal Merge Properties")
	void SetLockToReachMergeConditionEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "AGX Track Internal Merge Properties")
	bool GetLockToReachMergeConditionEnabled() const;

	/**
	 * Compliance of the hinge lock used to reach merge condition [rad/Nm].
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Track Internal Merge Properties",
		Meta =
			(EditCondition = "bMergeEnabled && bLockToReachMergeConditionEnabled",
			 ClampMin = "0.0"))
	FAGX_Real LockToReachMergeConditionCompliance {1.0e-11};

	UFUNCTION(BlueprintCallable, Category = "AGX Track Internal Merge Properties")
	void SetLockToReachMergeConditionCompliance(double Compliance);

	UFUNCTION(BlueprintCallable, Category = "AGX Track Internal Merge Properties")
	double GetLockToReachMergeConditionCompliance() const;

	/**
	 * Damping of the hinge lock used to reach merge condition [s].
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Track Internal Merge Properties",
		Meta =
			(EditCondition = "bMergeEnabled && bLockToReachMergeConditionEnabled",
			 ClampMin = "0.0"))
	FAGX_Real LockToReachMergeConditionSpookDamping {3.0 / 60.0};

	UFUNCTION(BlueprintCallable, Category = "AGX Track Internal Merge Properties")
	void SetLockToReachMergeConditionSpookDamping(double Damping);

	UFUNCTION(BlueprintCallable, Category = "AGX Track Internal Merge Properties")
	double GetLockToReachMergeConditionSpookDamping() const;

	/**
	 * Maximum angle to trigger merge between nodes [deg].
	 *
	 * I.e., when the angle between two nodes < maxAngleMergeCondition the nodes will merge.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Track Internal Merge Properties",
		Meta = (EditCondition = "bMergeEnabled"))
	FAGX_Real MaxAngleMergeCondition {FMath::RadiansToDegrees(1.0e-5)};

	UFUNCTION(BlueprintCallable, Category = "AGX Track Internal Merge Properties")
	void SetMaxAngleMergeCondition(double MaxAngleToMerge);

	UFUNCTION(BlueprintCallable, Category = "AGX Track Internal Merge Properties")
	double GetMaxAngleMergeCondition() const;

public:
	UAGX_TrackInternalMergeProperties() = default;
	void CommitToAsset();
	void CopyFrom(const UAGX_TrackInternalMergeProperties& Source);
	void CopyFrom(const FTrackBarrier& Source);
	static UAGX_TrackInternalMergeProperties* CreateInstanceFromAsset(
		const UWorld& PlayingWorld, UAGX_TrackInternalMergeProperties& Source);

	UAGX_TrackInternalMergeProperties* GetInstance();
	const UAGX_TrackInternalMergeProperties* GetInstance() const;
	UAGX_TrackInternalMergeProperties* GetOrCreateInstance(const UWorld& PlayingWorld);

	UAGX_TrackInternalMergeProperties* GetAsset();
	bool IsInstance() const;

	void UpdateNativeProperties();
	void UpdateNativeProperties(UAGX_TrackComponent* Track);

	const TArray<TWeakObjectPtr<UAGX_TrackComponent>>& GetTargetTracks() const;

	void RegisterTargetTrack(UAGX_TrackComponent* Track);
	void UnregisterTargetTrack(UAGX_TrackComponent* Track);

	// ~Begin UObject interface.
	virtual void PostInitProperties() override;
#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
#endif
	// ~End UObject interface.

public: // Deprecated functions.
	// clang-format off

	UFUNCTION(
		BlueprintCallable, Category = "AGX Track Internal Merge Properties",
		Meta = (DeprecatedFunction, DeprecationMessage = "Use SetLockToReachMergeConditionCompliance instead of SetLockToReachMergeConditionCompliance_BP"))
	void SetLockToReachMergeConditionCompliance_BP(float Compliance);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Track Internal Merge Properties",
		Meta = (
			DeprecatedFunction,
			DeprecationMessage = "Use GetLockToReachMergeConditionCompliance instead of GetLockToReachMergeConditionCompliance_BP"))
	float GetLockToReachMergeConditionCompliance_BP() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Track Internal Merge Properties",
		Meta = (DeprecatedFunction, DeprecationMessage = "Use SetLockToReachMergeConditionSpookDamping instead of SetLockToReachMergeConditionSpookDamping_BP"))
	void SetLockToReachMergeConditionSpookDamping_BP(float Damping);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Track Internal Merge Properties",
		Meta = (
			DeprecatedFunction,
			DeprecationMessage = "Use GetLockToReachMergeConditionSpookDamping instead of GetLockToReachMergeConditionSpookDamping_BP"))
	float GetLockToReachMergeConditionSpookDamping_BP() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Track Internal Merge Properties",
		Meta = (
			DeprecatedFunction,
			DeprecationMessage = "Use SetMaxAngleMergeCondition instead of SetMaxAngleMergeCondition_BP"))
	void SetMaxAngleMergeCondition_BP(float MaxAngleToMerge);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Track Internal Merge Properties",
		Meta = (
			DeprecatedFunction,
			DeprecationMessage = "Use GetMaxAngleMergeCondition instead of GetMaxAngleMergeCondition_BP"))
	float GetMaxAngleMergeCondition_BP() const;

	// clang-format on

private:
#if WITH_EDITOR
	void InitPropertyDispatcher();
#endif

private:
	TWeakObjectPtr<UAGX_TrackInternalMergeProperties> Asset;
	TWeakObjectPtr<UAGX_TrackInternalMergeProperties> Instance;

	// List of tracks that this InternalMergeProperties should write to.
	// \todo What happens if target track is in a BP Actor Instance that gets reconstructed?
	TArray<TWeakObjectPtr<UAGX_TrackComponent>> TargetTracks;
};
