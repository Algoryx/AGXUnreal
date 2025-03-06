// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_Real.h"
#include "AGX_RealInterval.h"
#include "Vehicle/TrackPropertiesBarrier.h"

// Unreal Engine includes.
#include "UObject/Object.h"

#include "AGX_TrackProperties.generated.h"

/**
 * Contains configuration properties of an AGX Track Component. It is an asset that is created from
 * the Content Browser. Several Tracks can share the same Track Properties asset. Default values for
 * all properties has been taken from AGX Dynamics.
 */
UCLASS(
	ClassGroup = "AGX", Category = "AGX", BlueprintType,
	AutoCollapseCategories = ("Hinge Compliance", "Hinge Spook Damping"))
class AGXUNREAL_API UAGX_TrackProperties : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Compliance of the hinges between track nodes, along the axis pointing vertically
	 * out from the track node [m/N].
	 */
	UPROPERTY(EditAnywhere, Category = "Hinge Compliance", Meta = (ClampMin = "0.0"))
	FAGX_Real HingeComplianceTranslational_X = DefaultHingeCompliance;

	/**
	 * Compliance of the hinges between track nodes, along the track direction [m/N].
	 */
	UPROPERTY(EditAnywhere, Category = "Hinge Compliance", Meta = (ClampMin = "0.0"))
	FAGX_Real HingeComplianceTranslational_Y = DefaultHingeCompliance;

	/**
	 * Compliance of the hinges between track nodes, along the axis pointing sideways
	 * (i.e. the rotation axis) [m/N].
	 */
	UPROPERTY(EditAnywhere, Category = "Hinge Compliance", Meta = (ClampMin = "0.0"))
	FAGX_Real HingeComplianceTranslational_Z = DefaultHingeCompliance;

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	void SetHingeComplianceTranslational(
		double ComplianceX, double ComplianceY, double ComplianceZ);
	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	void SetHingeComplianceTranslationalX(double ComplianceX);
	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	void SetHingeComplianceTranslationalY(double ComplianceY);
	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	void SetHingeComplianceTranslationalZ(double ComplianceZ);

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	void GetHingeComplianceTranslational(double& X, double& Y, double& Z) const;
	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	double GetHingeComplianceTranslationalX() const;
	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	double GetHingeComplianceTranslationalY() const;
	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	double GetHingeComplianceTranslationalZ() const;

	/**
	 * Compliance of the hinges between track nodes, around the axis pointing vertically
	 * out from the track node [rad/Nm].
	 */
	UPROPERTY(EditAnywhere, Category = "Hinge Compliance", Meta = (ClampMin = "0.0"))
	FAGX_Real HingeComplianceRotational_X = DefaultHingeCompliance;

	/**
	 * Compliance of the hinges between track nodes, around the track direction [rad/Nm].
	 */
	UPROPERTY(EditAnywhere, Category = "Hinge Compliance", Meta = (ClampMin = "0.0"))
	FAGX_Real HingeComplianceRotational_Y = DefaultHingeCompliance;

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	void SetHingeComplianceRotational(double ComplianceX, double ComplianceY);
	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	void SetHingeComplianceRotationalX(double Compliance);
	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	void SetHingeComplianceRotationalY(double Compliance);

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	void GetHingeComplianceRotational(double& X, double& Y) const;
	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	double GetHingeComplianceRotationalX() const;
	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	double GetHingeComplianceRotationalY() const;

	/**
	 * Spook damping of the hinges between track nodes, along the axis pointing vertically
	 * out from the track node [s].
	 */
	UPROPERTY(EditAnywhere, Category = "Hinge Spook Damping", Meta = (ClampMin = "0.0"))
	FAGX_Real HingeSpookDampingTranslational_X = DefaultHingeSpookDamping;

	/**
	 * Spook damping of the hinges between track nodes, along the track direction [s].
	 */
	UPROPERTY(EditAnywhere, Category = "Hinge Spook Damping", Meta = (ClampMin = "0.0"))
	FAGX_Real HingeSpookDampingTranslational_Y = DefaultHingeSpookDamping;

	/**
	 * Spook damping of the hinges between track nodes, along the axis pointing sideways
	 * (i.e. the rotation axis) [s].
	 */
	UPROPERTY(EditAnywhere, Category = "Hinge Spook Damping", Meta = (ClampMin = "0.0"))
	FAGX_Real HingeSpookDampingTranslational_Z = DefaultHingeSpookDamping;

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	void SetHingeSpookDampingTranslational(double DampingX, double DampingY, double DampingZ);
	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	void SetHingeSpookDampingTranslationalX(double Damping);
	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	void SetHingeSpookDampingTranslationalY(double Damping);
	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	void SetHingeSpookDampingTranslationalZ(double Damping);

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	void GetHingeSpookDampingTranslational(
		double& DampingX, double& DampingY, double& DampingZ) const;
	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	double GetHingeSpookDampingTranslationalX() const;
	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	double GetHingeSpookDampingTranslationalY() const;
	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	double GetHingeSpookDampingTranslationalZ() const;

	/**
	 * Spook damping of the hinges between track nodes, around the axis pointing vertically
	 * out from the track node [s].
	 */
	UPROPERTY(EditAnywhere, Category = "Hinge Spook Damping", Meta = (ClampMin = "0.0"))
	FAGX_Real HingeSpookDampingRotational_X = DefaultHingeSpookDamping;

	/**
	 * Spook damping of the hinges between track nodes, around the track direction [s].
	 */
	UPROPERTY(EditAnywhere, Category = "Hinge Spook Damping", Meta = (ClampMin = "0.0"))
	FAGX_Real HingeSpookDampingRotational_Y = DefaultHingeSpookDamping;

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	void SetHingeSpookDampingRotational(double DampingX, double DampingY);
	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	void SetHingeSpookDampingRotationalX(double Damping);
	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	void SetHingeSpookDampingRotationalY(double Damping);

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	void GetHingeSpookDampingRotational(double& X, double& Y) const;
	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	double GetHingeSpookDampingRotationalX() const;
	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	double GetHingeSpookDampingRotationalY() const;

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
	UPROPERTY(BlueprintReadOnly, Category = "AGX Dynamics Import Guid")
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
		Meta = (
			DeprecatedFunction,
			DeprecationMessage =
				"Use SetHingeComplianceTranslational instead of SetHingeComplianceTranslational_BP"))
	void SetHingeComplianceTranslational_BP(float X, float Y, float Z);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Track Properties",
		Meta = (
			DeprecatedFunction,
			DeprecationMessage =
				"Use GetHingeComplianceTranslational instead of GetHingeComplianceTranslational_BP"))
	void GetHingeComplianceTranslational_BP(float& X, float& Y, float& Z);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Track Properties",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage =
				 "Use SetHingeComplianceRotational instead of SetHingeComplianceRotational_BP"))
	void SetHingeComplianceRotational_BP(float X, float Y);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Track Properties",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage =
				 "Use GetHingeComplianceRotational instead of GetHingeComplianceRotational_BP"))
	void GetHingeComplianceRotational_BP(float& X, float& Y);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Track Properties",
		Meta = (
			DeprecatedFunction,
			DeprecationMessage =
				"Use SetHingeSpookDampingTranslational instead of SetHingeSpookDampingTranslational_BP"))
	void SetHingeSpookDampingTranslational_BP(float DampingX, float DampingY, float DampingZ);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Track Properties",
		Meta = (
			DeprecatedFunction,
			DeprecationMessage =
				"Use GetHingeSpookDampingTranslational instead of GetHingeSpookDampingTranslational_BP"))
	void GetHingeSpookDampingTranslational_BP(float& DampingX, float& DampingY, float& DampingZ);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Track Properties",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage =
				 "Use SetHingeSpookDampingRotational instead of SetHingeSpookDampingRotational_BP"))
	void SetHingeSpookDampingRotational_BP(float X, float Y);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Track Properties",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage =
				 "Use GetHingeSpookDampingRotational instead of GetHingeSpookDampingRotational_BP"))
	void GetHingeSpookDampingRotational_BP(float& X, float& Y);

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
	TWeakObjectPtr<UAGX_TrackProperties> Asset;
	TWeakObjectPtr<UAGX_TrackProperties> Instance;
	FTrackPropertiesBarrier NativeBarrier;
};
