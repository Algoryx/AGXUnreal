// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_RealInterval.h"

// Unreal Engine includes.
#include "Containers/UnrealString.h"
#include "Math/Vector.h"
#include "Math/Quat.h"

// Standard library includes.
#include <memory>

struct FTrackPropertiesRef;

/**
 * Acts as an interface to a native AGX TrackProperties, and encapsulates it so that it is
 * completely hidden from code that includes this file.
 */
class AGXUNREALBARRIER_API FTrackPropertiesBarrier
{
public:
	FTrackPropertiesBarrier();
	FTrackPropertiesBarrier(FTrackPropertiesBarrier&& Other);
	FTrackPropertiesBarrier(std::unique_ptr<FTrackPropertiesRef> Native);
	virtual ~FTrackPropertiesBarrier();

	bool HasNative() const;
	FTrackPropertiesRef* GetNative();
	const FTrackPropertiesRef* GetNative() const;

	void AllocateNative();
	void ReleaseNative();

	FGuid GetGuid() const;

	void SetBendingStiffnessLateral(double Stiffness);
	double GetBendingStiffnessLateral() const;

	void SetBendingAttenuationLateral(double Attenuation);
	double GetBendingAttenuationLateral() const;

	void SetBendingStiffnessVertical(double Stiffness);
	double GetBendingStiffnessVertical() const;

	void SetBendingAttenuationVertical(double Attenuation);
	double GetBendingAttenuationVertical() const;

	void SetShearStiffnessLateral(double Stiffness);
	double GetShearStiffnessLateral() const;

	void SetShearAttenuationLateral(double Attenuation);
	double GetShearAttenuationLateral() const;

	void SetShearStiffnessVertical(double Stiffness);
	double GetShearStiffnessVertical() const;

	void SetShearAttenuationVertical(double Attenuation);
	double GetShearAttenuationVertical() const;

	void SetTensileStiffness(double Stiffness);
	double GetTensileStiffness() const;

	void SetTensileAttenuation(double Attenuation);
	double GetTensileAttenuation() const;

	void SetTorsionalStiffness(double Stiffness);
	double GetTorsionalStiffness() const;

	void SetTorsionalAttenuation(double Attenuation);
	double GetTorsionalAttenuation() const;

	void SetHingeRangeEnabled(bool bEnable);
	bool GetHingeRangeEnabled() const;

	void SetHingeRangeRange(FAGX_RealInterval MinMaxAngles);
	void SetHingeRange(FAGX_RealInterval MinMaxAngles);

	FAGX_RealInterval GetHingeRangeRange() const;
	FAGX_RealInterval GetHingeRange() const;

	// Merge/Split properties.

	void SetOnInitializeMergeNodesToWheelsEnabled(bool bEnable);
	bool GetOnInitializeMergeNodesToWheelsEnabled() const;

	void SetOnInitializeTransformNodesToWheelsEnabled(bool bEnable);
	bool GetOnInitializeTransformNodesToWheelsEnabled() const;

	void SetTransformNodesToWheelsOverlap(double Overlap);
	double GetTransformNodesToWheelsOverlap() const;

	void SetNodesToWheelsMergeThreshold(double MergeThreshold);
	double GetNodesToWheelsMergeThreshold() const;

	void SetNodesToWheelsSplitThreshold(double SplitThreshold);
	double GetNodesToWheelsSplitThreshold() const;

	void SetNumNodesIncludedInAverageDirection(uint32 NumIncludedNodes);
	uint32 GetNumNodesIncludedInAverageDirection() const;

	// Stabilizing properties.

	void SetMinStabilizingHingeNormalForce(double MinNormalForce);
	double GetMinStabilizingHingeNormalForce() const;

	void SetStabilizingHingeFrictionParameter(double FrictionParameter);
	double GetStabilizingHingeFrictionParameter() const;

	/**
	 * Helper function for creating TrackProperites using the old (deprecated) Hinge data
	 * (compliance/damping), which will then enable reading out the new, migrated data such as
	 * stiffness/attenuation values.
	 * Compliance - this must be a vector of length 5 where the compliance is defined in this order:
	 *      TRANSLATIONAL_1, lateral shear stiffness
	 *      TRANSLATIONAL_2, tensile stiffness
	 *      TRANSLATIONAL_3, vertical shear stiffness
	 *      ROTATIONAL_1, vertical bending stiffness
	 *      ROTATIONAL_2, torsional stiffness
	 * Damping - this must be a vector of length 5 with damping. Same order as stiffness.
	 */
	FTrackPropertiesBarrier CreateFromComplianceAndDamping(
		const TArray<double>& Compliance, const TArray<double>& Damping, double NodeLength, double SimulationTimeStep);

private:
	FTrackPropertiesBarrier(const FTrackPropertiesBarrier&) = delete;
	void operator=(const FTrackPropertiesBarrier&) = delete;

	// NativeRef has the same lifetime as this object, so it should never be null.
	// NativeRef->Native is created by AllocateNative(), released by ReleaseNative(), and can be
	// null.
	std::unique_ptr<FTrackPropertiesRef> NativeRef;
};
