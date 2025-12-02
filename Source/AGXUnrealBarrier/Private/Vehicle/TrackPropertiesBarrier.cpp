// Copyright 2025, Algoryx Simulation AB.

#include "Vehicle/TrackPropertiesBarrier.h"

// AGX Dynamics for Unreal includes.
#include "BarrierOnly/Vehicle/TrackPropertiesRef.h"
#include "TypeConversions.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agx/Hinge.h>
#include <agxVehicle/TrackProperties.h>
#include "EndAGXIncludes.h"

// Unreal Engine includes.
#include <Misc/AssertionMacros.h>

FTrackPropertiesBarrier::FTrackPropertiesBarrier()
	: NativeRef {new FTrackPropertiesRef}
{
}

FTrackPropertiesBarrier::FTrackPropertiesBarrier(FTrackPropertiesBarrier&& Other)
	: NativeRef {std::move(Other.NativeRef)}
{
}

FTrackPropertiesBarrier::FTrackPropertiesBarrier(std::unique_ptr<FTrackPropertiesRef> Native)
	: NativeRef(std::move(Native))
{
}

FTrackPropertiesBarrier::~FTrackPropertiesBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the
	// std::unique_ptr NativeRef's destructor must be able to see the definition,
	// not just the forward declaration, of FTrackPropertiesRef.
}

bool FTrackPropertiesBarrier::HasNative() const
{
	return NativeRef && NativeRef->Native;
}

FTrackPropertiesRef* FTrackPropertiesBarrier::GetNative()
{
	return NativeRef.get();
}

const FTrackPropertiesRef* FTrackPropertiesBarrier::GetNative() const
{
	return NativeRef.get();
}

void FTrackPropertiesBarrier::AllocateNative()
{
	check(!HasNative());
	NativeRef->Native = new agxVehicle::TrackProperties();
}

void FTrackPropertiesBarrier::ReleaseNative()
{
	check(HasNative());
	NativeRef->Native = nullptr;
}

FGuid FTrackPropertiesBarrier::GetGuid() const
{
	check(HasNative());
	FGuid Guid = Convert(NativeRef->Native->getUuid());
	return Guid;
}

//
// Hinge compliance. Set.
//

void FTrackPropertiesBarrier::SetHingeCompliance(double Compliance, int32 DOF)
{
	check(HasNative());
	NativeRef->Native->setHingeCompliance(Compliance, static_cast<agx::Hinge::DOF>(DOF));
}

void FTrackPropertiesBarrier::SetHingeComplianceTranslational(double Compliance)
{
	check(HasNative());
	NativeRef->Native->setHingeComplianceTranslational(Compliance);
}

void FTrackPropertiesBarrier::SetHingeComplianceTranslationalX(double Compliance)
{
	check(HasNative());
	NativeRef->Native->setHingeCompliance(Compliance, agx::Hinge::TRANSLATIONAL_1);
}

void FTrackPropertiesBarrier::SetHingeComplianceTranslationalY(double Compliance)
{
	check(HasNative());
	NativeRef->Native->setHingeCompliance(Compliance, agx::Hinge::TRANSLATIONAL_2);
}

void FTrackPropertiesBarrier::SetHingeComplianceTranslationalZ(double Compliance)
{
	check(HasNative());
	NativeRef->Native->setHingeCompliance(Compliance, agx::Hinge::TRANSLATIONAL_3);
}

void FTrackPropertiesBarrier::SetHingeComplianceRotational(double Compliance)
{
	check(HasNative());
	NativeRef->Native->setHingeComplianceRotational(Compliance);
}

void FTrackPropertiesBarrier::SetHingeComplianceRotationalX(double Compliance)
{
	check(HasNative());
	NativeRef->Native->setHingeCompliance(Compliance, agx::Hinge::ROTATIONAL_1);
}

void FTrackPropertiesBarrier::SetHingeComplianceRotationalY(double Compliance)
{
	check(HasNative());
	NativeRef->Native->setHingeCompliance(Compliance, agx::Hinge::ROTATIONAL_2);
}

//
// Hinge compliance. Get.
//

double FTrackPropertiesBarrier::GetHingeCompliance(int32 DOF) const
{
	check(HasNative());
	return NativeRef->Native->getHingeCompliance(static_cast<agx::Hinge::DOF>(DOF));
}

double FTrackPropertiesBarrier::GetHingeComplianceTranslationalX() const
{
	check(HasNative());
	return NativeRef->Native->getHingeCompliance(agx::Hinge::TRANSLATIONAL_1);
}

double FTrackPropertiesBarrier::GetHingeComplianceTranslationalY() const
{
	check(HasNative());
	return NativeRef->Native->getHingeCompliance(agx::Hinge::TRANSLATIONAL_2);
}

double FTrackPropertiesBarrier::GetHingeComplianceTranslationalZ() const
{
	check(HasNative());
	return NativeRef->Native->getHingeCompliance(agx::Hinge::TRANSLATIONAL_3);
}

double FTrackPropertiesBarrier::GetHingeComplianceRotationalX() const
{
	check(HasNative());
	return NativeRef->Native->getHingeCompliance(agx::Hinge::ROTATIONAL_1);
}

double FTrackPropertiesBarrier::GetHingeComplianceRotationalY() const
{
	check(HasNative());
	return NativeRef->Native->getHingeCompliance(agx::Hinge::ROTATIONAL_2);
}

// Hinge spook damping. Set

void FTrackPropertiesBarrier::SetHingeSpookDamping(double Damping, int32 DOF)
{
	check(HasNative());
	NativeRef->Native->setHingeDamping(Damping, static_cast<agx::Hinge::DOF>(DOF));
}

void FTrackPropertiesBarrier::SetHingeSpookDampingTranslational(double Damping)
{
	check(HasNative());
	NativeRef->Native->setHingeDampingTranslational(Damping);
}

void FTrackPropertiesBarrier::SetHingeSpookDampingTranslationalX(double Damping)
{
	check(HasNative());
	NativeRef->Native->setHingeDamping(Damping, agx::Hinge::TRANSLATIONAL_1);
}

void FTrackPropertiesBarrier::SetHingeSpookDampingTranslationalY(double Damping)
{
	check(HasNative());
	NativeRef->Native->setHingeDamping(Damping, agx::Hinge::TRANSLATIONAL_2);
}

void FTrackPropertiesBarrier::SetHingeSpookDampingTranslationalZ(double Damping)
{
	check(HasNative());
	NativeRef->Native->setHingeDamping(Damping, agx::Hinge::TRANSLATIONAL_3);
}

void FTrackPropertiesBarrier::SetHingeSpookDampingRotational(double Damping)
{
	check(HasNative());
	NativeRef->Native->setHingeDampingRotational(Damping);
}

void FTrackPropertiesBarrier::SetHingeSpookDampingRotationalX(double Damping)
{
	check(HasNative());
	NativeRef->Native->setHingeDamping(agx::Hinge::ROTATIONAL_1);
}

void FTrackPropertiesBarrier::SetHingeSpookDampingRotationalY(double Damping)
{
	check(HasNative());
	NativeRef->Native->setHingeDamping(agx::Hinge::ROTATIONAL_2);
}

//
// Hinge spook damping. Get.

double FTrackPropertiesBarrier::GetHingeSpookDamping(int32 DOF) const
{
	check(HasNative());
	return NativeRef->Native->getHingeDamping(static_cast<agx::Hinge::DOF>(DOF));
}

double FTrackPropertiesBarrier::GetHingeSpookDampingTranslationalX() const
{
	check(HasNative());
	return NativeRef->Native->getHingeDamping(agx::Hinge::TRANSLATIONAL_1);
}

double FTrackPropertiesBarrier::GetHingeSpookDampingTranslationalY() const
{
	check(HasNative());
	return NativeRef->Native->getHingeDamping(agx::Hinge::TRANSLATIONAL_2);
}

double FTrackPropertiesBarrier::GetHingeSpookDampingTranslationalZ() const
{
	check(HasNative());
	return NativeRef->Native->getHingeDamping(agx::Hinge::TRANSLATIONAL_3);
}

double FTrackPropertiesBarrier::GetHingeSpookDampingRotationalX() const
{
	check(HasNative());
	return NativeRef->Native->getHingeDamping(agx::Hinge::ROTATIONAL_1);
}

double FTrackPropertiesBarrier::GetHingeSpookDampingRotationalY() const
{
	check(HasNative());
	return NativeRef->Native->getHingeDamping(agx::Hinge::ROTATIONAL_2);
}

// Hinge range.

void FTrackPropertiesBarrier::SetHingeRangeEnabled(bool bEnable)
{
	check(HasNative());
	NativeRef->Native->setEnableHingeRange(bEnable);
}

bool FTrackPropertiesBarrier::GetHingeRangeEnabled() const
{
	check(HasNative());
	return NativeRef->Native->getEnableHingeRange();
}

void FTrackPropertiesBarrier::SetHingeRangeRange(FAGX_RealInterval MinMaxAngles)
{
	check(HasNative());
	agx::RangeReal RangeAGX = ConvertAngle(MinMaxAngles);
	NativeRef->Native->setHingeRangeRange(RangeAGX.lower(), RangeAGX.upper());
}

void FTrackPropertiesBarrier::SetHingeRange(FAGX_RealInterval MinMaxAngles)
{
	return SetHingeRangeRange(MinMaxAngles);
}

FAGX_RealInterval FTrackPropertiesBarrier::GetHingeRangeRange() const
{
	check(HasNative());
	const agx::RangeReal RangeAGX = NativeRef->Native->getHingeRangeRange();
	const FAGX_RealInterval RangeUnreal = ConvertAngle(RangeAGX);
	return RangeUnreal;
}

FAGX_RealInterval FTrackPropertiesBarrier::GetHingeRange() const
{
	return GetHingeRangeRange();
}

void FTrackPropertiesBarrier::SetOnInitializeMergeNodesToWheelsEnabled(bool bEnable)
{
	check(HasNative());
	NativeRef->Native->setEnableOnInitializeMergeNodesToWheels(bEnable);
}

bool FTrackPropertiesBarrier::GetOnInitializeMergeNodesToWheelsEnabled() const
{
	check(HasNative());
	return NativeRef->Native->getEnableOnInitializeMergeNodesToWheels();
}

void FTrackPropertiesBarrier::SetOnInitializeTransformNodesToWheelsEnabled(bool bEnable)
{
	check(HasNative());
	NativeRef->Native->setEnableOnInitializeTransformNodesToWheels(bEnable);
}

bool FTrackPropertiesBarrier::GetOnInitializeTransformNodesToWheelsEnabled() const
{
	check(HasNative());
	return NativeRef->Native->getEnableOnInitializeTransformNodesToWheels();
}

void FTrackPropertiesBarrier::SetTransformNodesToWheelsOverlap(double Overlap)
{
	check(HasNative());
	NativeRef->Native->setTransformNodesToWheelsOverlap(ConvertDistanceToAGX(Overlap));
}

double FTrackPropertiesBarrier::GetTransformNodesToWheelsOverlap() const
{
	check(HasNative());
	return ConvertDistanceToUnreal<double>(NativeRef->Native->getTransformNodesToWheelsOverlap());
}

void FTrackPropertiesBarrier::SetNodesToWheelsMergeThreshold(double MergeThreshold)
{
	check(HasNative());
	NativeRef->Native->setNodesToWheelsMergeThreshold(MergeThreshold);
}

double FTrackPropertiesBarrier::GetNodesToWheelsMergeThreshold() const
{
	check(HasNative());
	return NativeRef->Native->getNodesToWheelsMergeThreshold();
}

void FTrackPropertiesBarrier::SetNodesToWheelsSplitThreshold(double SplitThreshold)
{
	check(HasNative());
	NativeRef->Native->setNodesToWheelsSplitThreshold(SplitThreshold);
}

double FTrackPropertiesBarrier::GetNodesToWheelsSplitThreshold() const
{
	check(HasNative());
	return NativeRef->Native->getNodesToWheelsSplitThreshold();
}

void FTrackPropertiesBarrier::SetNumNodesIncludedInAverageDirection(uint32 NumIncludedNodes)
{
	check(HasNative());
	NativeRef->Native->setNumNodesIncludedInAverageDirection(NumIncludedNodes);
}

uint32 FTrackPropertiesBarrier::GetNumNodesIncludedInAverageDirection() const
{
	check(HasNative());
	return NativeRef->Native->getNumNodesIncludedInAverageDirection();
}

void FTrackPropertiesBarrier::SetMinStabilizingHingeNormalForce(double MinNormalForce)
{
	check(HasNative());
	NativeRef->Native->setMinStabilizingHingeNormalForce(MinNormalForce);
}

double FTrackPropertiesBarrier::GetMinStabilizingHingeNormalForce() const
{
	check(HasNative());
	return NativeRef->Native->getMinStabilizingHingeNormalForce();
}

void FTrackPropertiesBarrier::SetStabilizingHingeFrictionParameter(double FrictionParameter)
{
	check(HasNative());
	NativeRef->Native->setStabilizingHingeFrictionParameter(FrictionParameter);
}

double FTrackPropertiesBarrier::GetStabilizingHingeFrictionParameter() const
{
	check(HasNative());
	return NativeRef->Native->getStabilizingHingeFrictionParameter();
}
