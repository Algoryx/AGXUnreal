// Copyright 2024, Algoryx Simulation AB.

#include "Wire/WireParameterControllerBarrier.h"

// AGX Dynamics for Unreal includes.
#include "BarrierOnly/Wire/WireParameterControllerPtr.h"
#include "TypeConversions.h"

FWireParameterControllerBarrier::FWireParameterControllerBarrier()
	: NativePtr {new FWireParameterControllerPtr()}
{
}

FWireParameterControllerBarrier::FWireParameterControllerBarrier(
	std::unique_ptr<FWireParameterControllerPtr> Native)
	: NativePtr {std::move(Native)}
{
}

FWireParameterControllerBarrier::FWireParameterControllerBarrier(
	const FWireParameterControllerBarrier& Other)
	: NativePtr {new FWireParameterControllerPtr(Other.NativePtr->NativeWire)}
{
}

FWireParameterControllerBarrier::FWireParameterControllerBarrier(
	FWireParameterControllerBarrier&& Other)
	: NativePtr {std::move(Other.NativePtr)}
{
}

FWireParameterControllerBarrier::~FWireParameterControllerBarrier()
{
	// Destructor definition must be in the .cpp file since the definition, not just the
	// declaration, of FWireParameterControllerPtr must be available.
}

FWireParameterControllerBarrier& FWireParameterControllerBarrier::operator=(
	const FWireParameterControllerBarrier& Other)
{
	*NativePtr = *Other.NativePtr;
	return *this;
}

void FWireParameterControllerBarrier::SetMaximumContactMovementOneTimestep(double MaxMovement)
{
	check(HasNative());
	const double MaxMovementAGX = ConvertDistanceToAGX(MaxMovement);
	NativePtr->GetNative()->setMaximumContactMovementOneTimestep(MaxMovementAGX);
}

double FWireParameterControllerBarrier::GetMaximumContactMovementOneTimestep() const
{
	check(HasNative());
	const double MaxMovementAGX = NativePtr->GetNative()->getMaximumContactMovementOneTimestep();
	const double MaxMovement = ConvertDistanceToUnreal<double>(MaxMovementAGX);
	return MaxMovement;
}

void FWireParameterControllerBarrier::SetMinimumDistanceBetweenNodes(double MinDistance)
{
	check(HasNative());
	const double MinDistanceAGX = ConvertDistanceToAGX(MinDistance);
	NativePtr->GetNative()->setMinimumDistanceBetweenNodes(MinDistanceAGX);
}

double FWireParameterControllerBarrier::GetMinimumDistanceBetweenNodes() const
{
	check(HasNative());
	const double MinDistanceAGX = NativePtr->GetNative()->getMinimumDistanceBetweenNodes();
	const double MinDistance = ConvertDistanceToUnreal<double>(MinDistanceAGX);
	return MinDistance;
}

void FWireParameterControllerBarrier::SetRadiusMultiplier(double RadiusMultiplier)
{
	check(HasNative());
	NativePtr->GetNative()->setRadiusMultiplier(RadiusMultiplier);
}

double FWireParameterControllerBarrier::GetRadiusMultiplier() const
{
	check(HasNative());
	return NativePtr->GetNative()->getNonScaledRadiusMultiplier();
}

double FWireParameterControllerBarrier::GetScaledRadiusMultiplier(double WireRadius) const
{
	check(HasNative());
	const double WireRadiusAGX = ConvertDistanceToAGX(WireRadius);
	return NativePtr->GetNative()->getRadiusMultiplier(WireRadius);
}

void FWireParameterControllerBarrier::SetScaleConstant(double ScaleConstant)
{
	check(HasNative());
	NativePtr->GetNative()->setScaleConstant(ScaleConstant);
}

double FWireParameterControllerBarrier::GetScaleConstant() const
{
	check(HasNative());
	return NativePtr->GetNative()->getScaleConstant();
}

void FWireParameterControllerBarrier::SetSplitTensionMultiplier(double Multiplier)
{
	check(HasNative());
	NativePtr->GetNative()->setSplitTensionMultiplier(Multiplier);
}

double FWireParameterControllerBarrier::GetSplitTensionMultiplier() const
{
	check(HasNative());
	return NativePtr->GetNative()->getSplitTensionMultiplier();
}

void FWireParameterControllerBarrier::SetStopNodeLumpMinDistanceFraction(double Fraction)
{
	check(HasNative());
	NativePtr->GetNative()->setStopNodeLumpMinDistanceFraction(Fraction);
}

double FWireParameterControllerBarrier::GetStopNodeLumpMinDistanceFraction() const
{
	check(HasNative());
	return NativePtr->GetNative()->getStopNodeLumpMinDistanceFraction();
}

void FWireParameterControllerBarrier::SetStopNodeReferenceDistance(double Distance)
{
	check(HasNative());
	const agx::Real DistanceAGX = ConvertDistanceToAGX(Distance);
	NativePtr->GetNative()->setStopNodeReferenceDistance(DistanceAGX);
}

double FWireParameterControllerBarrier::GetStopNodeReferenceDistance() const
{
	check(HasNative());
	const agx::Real DistanceAGX = NativePtr->GetNative()->getStopNodeReferenceDistance();
	const double Distance = ConvertToUnreal<double>(DistanceAGX);
	return Distance;
}

void FWireParameterControllerBarrier::SetWireContactDynamicsSolverDampingScale(double Scale)
{
	check(HasNative());
	NativePtr->GetNative()->setWireContactDynamicsSolverDampingScale(Scale);
}

double FWireParameterControllerBarrier::GetWireContactDynamicsSolverDampingScale() const
{
	check(HasNative());
	return NativePtr->GetNative()->getWireContactDynamicsSolverDampingScale();
}

bool FWireParameterControllerBarrier::HasNative() const
{
	return NativePtr->NativeWire != nullptr;
}
