// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_Real.h"
#include "AGX_RealInterval.h"
#include "Vehicle/TrackPropertiesBarrier.h"

// Unreal Engine includes.
#include "UObject/Object.h"

#include "AGX_TrackProperties.generated.h"

class UAGX_TrackComponent;

/**
 * Contains configuration properties of an AGX Track Component. It is an asset that is created from
 * the Content Browser. Several Tracks can share the same Track Properties asset. Default values for
 * all properties has been taken from AGX Dynamics.
 */
UCLASS(
	ClassGroup = "AGX_Vehicle", Category = "AGX", BlueprintType,
	AutoCollapseCategories = ("Hinge Compliance", "Hinge Spook Damping"))
class AGXUNREAL_API UAGX_TrackProperties : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Lateral bending stiffness (resists sagging).
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Track Properties")
	FAGX_Real BendingStiffnessLateral = 50.0;

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	void SetBendingStiffnessLateral(double Stiffness);

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	double GetBendingStiffnessLateral() const;

	/**
	 * Lateral bending attenuation (sagging).
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Track Properties")
	double BendingAttenuationLateral = 2.0;

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	void SetBendingAttenuationLateral(double Attenuation);

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	double GetBendingAttenuationLateral() const;

	/**
	 * Vertical bending stiffness (resists side-to-side bending).
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Track Properties")
	FAGX_Real BendingStiffnessVertical = 1.1e9;

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	void SetBendingStiffnessVertical(double Stiffness);

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	double GetBendingStiffnessVertical() const;

	/**
	 * Vertical bending attenuation (side-to-side bending).
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Track Properties")
	double BendingAttenuationVertical = 2.0;

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	void SetBendingAttenuationVertical(double Attenuation);

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	double GetBendingAttenuationVertical() const;

	/**
	 * Lateral shear stiffness (resists lateral shear deformation).
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Track Properties")
	FAGX_Real ShearStiffnessLateral = 1.1e9;

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	void SetShearStiffnessLateral(double Stiffness);

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	double GetShearStiffnessLateral() const;

	/**
	 * Lateral shear attenuation.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Track Properties")
	double ShearAttenuationLateral = 2.0;

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	void SetShearAttenuationLateral(double Attenuation);

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	double GetShearAttenuationLateral() const;

	/**
	 * Vertical shear stiffness (resists vertical shear deformation).
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Track Properties")
	FAGX_Real ShearStiffnessVertical = 1.1e9;

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	void SetShearStiffnessVertical(double Stiffness);

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	double GetShearStiffnessVertical() const;

	/**
	 * Vertical shear attenuation.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Track Properties")
	double ShearAttenuationVertical = 2.0;

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	void SetShearAttenuationVertical(double Attenuation);

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	double GetShearAttenuationVertical() const;

	/**
	 * Tensile stiffness (resists stretching).
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Track Properties")
	FAGX_Real TensileStiffness = 1.1e9;

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	void SetTensileStiffness(double Stiffness);

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	double GetTensileStiffness() const;

	/**
	 * Tensile attenuation (stretching).
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Track Properties")
	double TensileAttenuation = 2.0;

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	void SetTensileAttenuation(double Attenuation);

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	double GetTensileAttenuation() const;

	/**
	 * Torsional stiffness (resists twisting).
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Track Properties")
	FAGX_Real TorsionalStiffness = 1.1e9;

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	void SetTorsionalStiffness(double Stiffness);

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	double GetTorsionalStiffness() const;

	/**
	 * Torsional attenuation (twisting).
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Track Properties")
	double TorsionalAttenuation = 2.0;

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	void SetTorsionalAttenuation(double Attenuation);

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	double GetTorsionalAttenuation() const;

	/**
	 * Whether to enable the range in the hinges between the track nodes
	 * to define how far the track may bend.
	 */
	UPROPERTY(EditAnywhere, Category = "Hinge Range")
	bool bEnableHingeRange = true;

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	void SetHingeRangeEnabled(bool bEnable);

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	bool GetHingeRangeEnabled() const;

	/**
	 * Range used if the hinge range between the nodes is enabled [deg].
	 */
	UPROPERTY(EditAnywhere, Category = "Hinge Range", Meta = (EditCondition = "bEnableHingeRange"))
	FAGX_RealInterval HingeRange {-120, 20};

	void SetHingeRange(FAGX_RealInterval InHingeRange);

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	void SetHingeRange(double Min, double Max);

	FAGX_RealInterval GetHingeRange() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	void GetHingeRange(double& Min, double& Max) const;

	/**
	 * When the track has been initialized some nodes are in contact with the wheels.
	 *
	 * If this flag is true the interacting nodes will be merged to the wheel directly after
	 * initialize, if false the nodes will be merged during the first (or later) time step.
	 */
	UPROPERTY(EditAnywhere, Category = "Merge/Split Properties")
	bool bEnableOnInitializeMergeNodesToWheels = false;

	UFUNCTION(BlueprintCallable, Category = "Merge/Split Properties")
	void SetOnInitializeMergeNodesToWheelsEnabled(bool bEnable);

	UFUNCTION(BlueprintCallable, Category = "Merge/Split Properties")
	bool GetOnInitializeMergeNodesToWheelsEnabled() const;

	/**
	 * Whether to position/transform the track nodes to the surface of the wheels after the
	 * track has been initialized. If false, the routing algorithm positions are used as is.
	 */
	UPROPERTY(EditAnywhere, Category = "Merge/Split Properties")
	bool bEnableOnInitializeTransformNodesToWheels = true;

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	void SetOnInitializeTransformNodesToWheelsEnabled(bool bEnable);

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	bool GetOnInitializeTransformNodesToWheelsEnabled() const;

	/**
	 * When the nodes are transformed to the wheels, this is the final target overlap [cm].
	 */
	UPROPERTY(EditAnywhere, Category = "Merge/Split Properties")
	FAGX_Real TransformNodesToWheelsOverlap = 0.1;

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	void SetTransformNodesToWheelsOverlap(double Overlap);

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	double GetTransformNodesToWheelsOverlap() const;

	/**
	 * Threshold when to merge a node to a wheel.
	 *
	 * Given a reference direction in the track, this value is the projection of the deviation
	 * (from the reference direction) of the node direction onto the wheel radial direction vector.
	 *
	 * I.e., when the projection is negative the node can be considered "wrapped" on the wheel.
	 */
	UPROPERTY(EditAnywhere, Category = "Merge/Split Properties")
	FAGX_Real NodesToWheelsMergeThreshold = -0.1;

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	void SetNodesToWheelsMergeThreshold(double MergeThreshold);

	/**
	 * Threshold when to split a node from a wheel.
	 *
	 * Given a reference direction in the track, this value is the projection of the deviation
	 * (from the reference direction) of the node direction onto the wheel radial direction vector.
	 *
	 * I.e., when the projection is negative the node can be considered "wrapped" on the wheel.
	 */
	UPROPERTY(EditAnywhere, Category = "Merge/Split Properties")
	FAGX_Real NodesToWheelsSplitThreshold = -0.05;

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	void SetNodesToWheelsSplitThreshold(double SplitThreshold);

	/**
	 * Average direction of non-merged nodes entering or exiting a wheel is used as
	 * reference direction to split of a merged node.
	 *
	 * This is the number of nodes to include into this average direction.
	 */
	UPROPERTY(EditAnywhere, Category = "Merge/Split Properties", Meta = (ClampMin = "1"))
	uint32 NumNodesIncludedInAverageDirection = 3;

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	void SetNumNodesIncludedInAverageDirection(int32 NumIncludedNodes);

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	int32 GetNumNodesIncludedInAverageDirection() const;

	/**
	 * Minimum value of the normal force (the hinge force along the track) used in
	 * "internal" friction calculations [N].
	 *
	 * I.e., when the track is compressed, this value is used with the friction coefficient
	 * as a minimum stabilizing compliance. If this value is negative there will be
	 * stabilization when the track is compressed.
	 */
	UPROPERTY(EditAnywhere, Category = "Stabilizing Properties", Meta = (ClampMin = "0.0"))
	FAGX_Real MinStabilizingHingeNormalForce = 100.0;

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	void SetMinStabilizingHingeNormalForce(double MinNormalForce);

	/**
	 * Friction parameter of the internal friction in the node hinges.
	 *
	 * This parameter scales the normal force in the hinge.
	 *
	 * This parameter can not be identified as a real friction coefficient when it's used to
	 * stabilize tracks under tension.
	 */
	UPROPERTY(EditAnywhere, Category = "Stabilizing Properties", Meta = (ClampMin = "0.0"))
	FAGX_Real StabilizingHingeFrictionParameter = 1.0;

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	void SetStabilizingHingeFrictionParameter(double FrictionParameter);

	/*
	 * The import Guid of this Object. Only used by the AGX Dynamics for Unreal import system.
	 * Should never be assigned manually.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Dynamics Import")
	FGuid ImportGuid;

public:
	UAGX_TrackProperties() = default;
	virtual ~UAGX_TrackProperties() = default;

	/**
	 * Copy Property values from the native AGX Dynamics instance to the Track Properties asset
	 * the instance was created from. That may be this UAGX_TrackProperties, if IsAsset returns
	 * true, or the UAGX_TrackProperties that this was created from, if IsInstance returns true.
	 */
	void CommitToAsset();

	void CopyFrom(const UAGX_TrackProperties* Source);
	void CopyFrom(const FTrackPropertiesBarrier& Source);

	/**
	 * Create the Play instance for the given Source Track Properties, which should be an asset.
	 * The AGX Dynamics Native will be created immediately.
	 */
	static UAGX_TrackProperties* CreateInstanceFromAsset(
		const UWorld* PlayingWorld, UAGX_TrackProperties* Source);

	/**
	 * Get the instance, i.e. Play version, of this Track Properties.
	 *
	 * For an asset Track Properties GetInstance will return nullptr if we are not currently in Play
	 * or if an instance has not been created with GetOrCreateInstance yet.
	 *
	 * For an instance GetInstance will always return itself.
	 */
	UAGX_TrackProperties* GetInstance();

	/**
	 * If PlayingWorld is an in-game World and this TrackProperties is a UAGX_TrackPropertiesAsset,
	 * returns a UAGX_TrackPropertiesInstance representing the TrackProperties asset throughout the
	 * lifetime of the GameInstance. If this is already a UAGX_TrackPropertiesInstance it returns
	 * itself. Returns null if not in-game (invalid call).
	 */
	UAGX_TrackProperties* GetOrCreateInstance(const UWorld* PlayingWorld);

	/**
	 * If this TrackProperties is a UAGX_TrackPropertiesInstance, returns the
	 * UAGX_TrackPropertiesAsset it was created from (if it still exists). Else returns null.
	 */
	UAGX_TrackProperties* GetAsset();

	bool IsInstance() const;

	bool HasNative() const;
	FTrackPropertiesBarrier* GetNative();
	const FTrackPropertiesBarrier* GetNative() const;
	FTrackPropertiesBarrier* GetOrCreateNative();

	void UpdateNativeProperties();
	virtual void Serialize(FArchive& Archive) override;

	// ~Begin UObject interface.	
	virtual void PostInitProperties() override;
#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
#endif
	// ~End UObject interface.

public: // Deprecated functions.
	// clang-format off
	UFUNCTION(
		BlueprintCallable, Category = "AGX Track Properties",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage = "Use SetHingeRange instead of SetHingeRange_BP"))
	void SetHingeRange_BP(float Min, float Max);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Track Properties",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage = "Use GetHingeRange instead of GetHingeRange_BP"))
	void GetHingeRange_BP(float& Min, float& Max) const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Track Properties",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage =
				"Use SetTransformNodesToWheelsOverlap instead of SetTransformNodesToWheelsOverlap_BP"))
	void SetTransformNodesToWheelsOverlap_BP(float Overlap);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Track Properties",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage =
				"Use GetTransformNodesToWheelsOverlap instead of GetTransformNodesToWheelsOverlap_BP"))
	float GetTransformNodesToWheelsOverlap_BP() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Track Properties",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage =
				 "Use SetNodesToWheelsMergeThreshold instead of SetNodesToWheelsMergeThreshold_BP"))
	void SetNodesToWheelsMergeThreshold_BP(float MergeThreshold);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Track Properties",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage =
				 "Use SetNodesToWheelsSplitThreshold instead of SetNodesToWheelsSplitThreshold_BP"))
	void SetNodesToWheelsSplitThreshold_BP(float SplitThreshold);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Track Properties",
		Meta = (
			DeprecatedFunction,
			DeprecationMessage =
				"Use SetStabilizingHingeFrictionParameter instead of SetStabilizingHingeFrictionParameter_BP"))
	void SetStabilizingHingeFrictionParameter_BP(float FrictionParameter);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Track Properties",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage =
				"Use SetMinStabilizingHingeNormalForce instead of SetMinStabilizingHingeNormalForce_BP"))
	void SetMinStabilizingHingeNormalForce_BP(float MinNormalForce);

	// clang-format on

private:
#if WITH_EDITOR
	virtual void InitPropertyDispatcher();
#endif

	void CreateNative();

private:
	static constexpr double DefaultHingeCompliance = 1.0E-10;
	static constexpr double DefaultHingeSpookDamping = 2.0 / 60.0;

private:
	UPROPERTY()
	FAGX_Real HingeComplianceTranslational_X_DEPRECATED = DefaultHingeCompliance;

	UPROPERTY()
	FAGX_Real HingeComplianceTranslational_Y_DEPRECATED = DefaultHingeCompliance;

	UPROPERTY()
	FAGX_Real HingeComplianceTranslational_Z_DEPRECATED = DefaultHingeCompliance;

	UPROPERTY()
	FAGX_Real HingeComplianceRotational_X_DEPRECATED = DefaultHingeCompliance;

	UPROPERTY()
	FAGX_Real HingeComplianceRotational_Y_DEPRECATED = DefaultHingeCompliance;

	UPROPERTY()
	FAGX_Real HingeSpookDampingTranslational_X_DEPRECATED = DefaultHingeSpookDamping;

	UPROPERTY()
	FAGX_Real HingeSpookDampingTranslational_Y_DEPRECATED = DefaultHingeSpookDamping;

	UPROPERTY()
	FAGX_Real HingeSpookDampingTranslational_Z_DEPRECATED = DefaultHingeSpookDamping;

	UPROPERTY()
	FAGX_Real HingeSpookDampingRotational_X_DEPRECATED = DefaultHingeSpookDamping;

	UPROPERTY()
	FAGX_Real HingeSpookDampingRotational_Y_DEPRECATED = DefaultHingeSpookDamping;

	TWeakObjectPtr<UAGX_TrackProperties> Asset;
	TWeakObjectPtr<UAGX_TrackProperties> Instance;
	FTrackPropertiesBarrier NativeBarrier;
};
