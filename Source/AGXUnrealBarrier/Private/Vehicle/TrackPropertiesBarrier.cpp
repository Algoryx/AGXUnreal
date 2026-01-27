// Copyright 2025, Algoryx Simulation AB.

#include "Vehicle/TrackPropertiesBarrier.h"

// AGX Dynamics for Unreal includes.
#include "BarrierOnly/AGXTypeConversions.h"
#include "BarrierOnly/Vehicle/TrackPropertiesRef.h"

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

void FTrackPropertiesBarrier::SetBendingStiffnessLateral(double Stiffness)
{
	check(HasNative());
	//NativeRef->Native->setBendingStiffness(Stiffness, agxVehicle::TrackProperties::Axis::LATERAL);
}

double FTrackPropertiesBarrier::GetBendingStiffnessLateral() const
{
	check(HasNative());
	return NativeRef->Native->getBendingStiffness(agxVehicle::TrackProperties::Axis::LATERAL);
}

void FTrackPropertiesBarrier::SetBendingAttenuationLateral(double Attenuation)
{
	check(HasNative());
	NativeRef->Native->setBendingAttenuation(
		Attenuation, agxVehicle::TrackProperties::Axis::LATERAL);
}

double FTrackPropertiesBarrier::GetBendingAttenuationLateral() const
{
	check(HasNative());
	return NativeRef->Native->getBendingAttenuation(agxVehicle::TrackProperties::Axis::LATERAL);
}

void FTrackPropertiesBarrier::SetBendingStiffnessVertical(double Stiffness)
{
	check(HasNative());
	//NativeRef->Native->setBendingStiffness(Stiffness, agxVehicle::TrackProperties::Axis::VERTICAL);
}

double FTrackPropertiesBarrier::GetBendingStiffnessVertical() const
{
	check(HasNative());
	return NativeRef->Native->getBendingStiffness(agxVehicle::TrackProperties::Axis::VERTICAL);
}

void FTrackPropertiesBarrier::SetBendingAttenuationVertical(double Attenuation)
{
	check(HasNative());
	NativeRef->Native->setBendingAttenuation(
		Attenuation, agxVehicle::TrackProperties::Axis::VERTICAL);
}

double FTrackPropertiesBarrier::GetBendingAttenuationVertical() const
{
	check(HasNative());
	return NativeRef->Native->getBendingAttenuation(agxVehicle::TrackProperties::Axis::VERTICAL);
}

void FTrackPropertiesBarrier::SetShearStiffnessLateral(double Stiffness)
{
	check(HasNative());
	//NativeRef->Native->setShearStiffness(Stiffness, agxVehicle::TrackProperties::Axis::LATERAL);
}

double FTrackPropertiesBarrier::GetShearStiffnessLateral() const
{
	check(HasNative());
	return NativeRef->Native->getShearStiffness(agxVehicle::TrackProperties::Axis::LATERAL);
}

void FTrackPropertiesBarrier::SetShearAttenuationLateral(double Attenuation)
{
	check(HasNative());
	NativeRef->Native->setShearAttenuation(Attenuation, agxVehicle::TrackProperties::Axis::LATERAL);
}

double FTrackPropertiesBarrier::GetShearAttenuationLateral() const
{
	check(HasNative());
	return NativeRef->Native->getShearAttenuation(agxVehicle::TrackProperties::Axis::LATERAL);
}

void FTrackPropertiesBarrier::SetShearStiffnessVertical(double Stiffness)
{
	check(HasNative());
	//NativeRef->Native->setShearStiffness(Stiffness, agxVehicle::TrackProperties::Axis::VERTICAL);
}

double FTrackPropertiesBarrier::GetShearStiffnessVertical() const
{
	check(HasNative());
	return NativeRef->Native->getShearStiffness(agxVehicle::TrackProperties::Axis::VERTICAL);
}

void FTrackPropertiesBarrier::SetShearAttenuationVertical(double Attenuation)
{
	check(HasNative());
	NativeRef->Native->setShearAttenuation(
		Attenuation, agxVehicle::TrackProperties::Axis::VERTICAL);
}

double FTrackPropertiesBarrier::GetShearAttenuationVertical() const
{
	check(HasNative());
	return NativeRef->Native->getShearAttenuation(agxVehicle::TrackProperties::Axis::VERTICAL);
}

void FTrackPropertiesBarrier::SetTensileStiffness(double Stiffness)
{
	check(HasNative());
	//NativeRef->Native->setTensileStiffness(Stiffness);
}

double FTrackPropertiesBarrier::GetTensileStiffness() const
{
	check(HasNative());
	return NativeRef->Native->getTensileStiffness();
}

void FTrackPropertiesBarrier::SetTensileAttenuation(double Attenuation)
{
	check(HasNative());
	NativeRef->Native->setTensileAttenuation(Attenuation);
}

double FTrackPropertiesBarrier::GetTensileAttenuation() const
{
	check(HasNative());
	return NativeRef->Native->getTensileAttenuation();
}

void FTrackPropertiesBarrier::SetTorsionalStiffness(double Stiffness)
{
	check(HasNative());
	//NativeRef->Native->setTorsionalStiffness(Stiffness);
}

double FTrackPropertiesBarrier::GetTorsionalStiffness() const
{
	check(HasNative());
	return NativeRef->Native->getTorsionalStiffness();
}

void FTrackPropertiesBarrier::SetTorsionalAttenuation(double Attenuation)
{
	check(HasNative());
	NativeRef->Native->setTorsionalAttenuation(Attenuation);
}

double FTrackPropertiesBarrier::GetTorsionalAttenuation() const
{
	check(HasNative());
	return NativeRef->Native->getTorsionalAttenuation();
}

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
	const agx::RangeReal RangeAGX = NativeRef->Native->getHingeRange();
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
