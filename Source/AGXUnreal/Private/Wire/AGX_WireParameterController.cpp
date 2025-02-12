// Copyright 2024, Algoryx Simulation AB.

#include "Wire/AGX_WireParameterController.h"

// AGX Dynamics for Unreal includes.
#include "Utilities/AGX_BarrierUtilities.h"
#include "Wire/WireParameterControllerBarrier.h"


AGX_BARRIER_SET_GET_PROPERTY(
	FAGX_WireParameterController, double, MaximumContactMovementOneTimestep)
AGX_BARRIER_SET_GET_PROPERTY(FAGX_WireParameterController, double, MinimumDistanceBetweenNodes)
AGX_BARRIER_SET_GET_PROPERTY(FAGX_WireParameterController, double, RadiusMultiplier)
AGX_BARRIER_SET_GET_PROPERTY(FAGX_WireParameterController, double, ScaleConstant)
AGX_BARRIER_SET_GET_PROPERTY(FAGX_WireParameterController, double, SplitTensionMultiplier)
AGX_BARRIER_SET_GET_PROPERTY(FAGX_WireParameterController, double, StopNodeLumpMinDistanceFraction)
AGX_BARRIER_SET_GET_PROPERTY(FAGX_WireParameterController, double, StopNodeReferenceDistance)
AGX_BARRIER_SET_GET_PROPERTY(FAGX_WireParameterController, double, WireContactDynamicsSolverDampingScale);

double FAGX_WireParameterController::GetScaledRadiusMultiplier(double WireRadius) const
{
	if (!HasNative())
	{
		return 0.0;
	}
	return NativeBarrier.GetScaledRadiusMultiplier(WireRadius);
}

void FAGX_WireParameterController::SetBarrier(const FWireParameterControllerBarrier& InBarrier)
{
	NativeBarrier = InBarrier;
}

void FAGX_WireParameterController::ClearBarrier(){
	NativeBarrier = FWireParameterControllerBarrier();
}


void FAGX_WireParameterController::CopyFrom(const FWireParameterControllerBarrier& Barrier)
{
	MaximumContactMovementOneTimestep = Barrier.GetMaximumContactMovementOneTimestep();
	MinimumDistanceBetweenNodes = Barrier.GetMinimumDistanceBetweenNodes();
	RadiusMultiplier = Barrier.GetRadiusMultiplier();
	ScaleConstant = Barrier.GetScaleConstant();
	SplitTensionMultiplier = Barrier.GetSplitTensionMultiplier();
	StopNodeLumpMinDistanceFraction = Barrier.GetStopNodeLumpMinDistanceFraction();
	StopNodeReferenceDistance = Barrier.GetStopNodeReferenceDistance();
	WireContactDynamicsSolverDampingScale = Barrier.GetWireContactDynamicsSolverDampingScale();
}

void FAGX_WireParameterController::WritePropertiesToNative()
{
	NativeBarrier.SetMaximumContactMovementOneTimestep(MaximumContactMovementOneTimestep);
	NativeBarrier.SetMinimumDistanceBetweenNodes(MinimumDistanceBetweenNodes);
	NativeBarrier.SetRadiusMultiplier(RadiusMultiplier);
	NativeBarrier.SetScaleConstant(ScaleConstant);
	NativeBarrier.SetSplitTensionMultiplier(SplitTensionMultiplier);
	NativeBarrier.SetStopNodeLumpMinDistanceFraction(StopNodeLumpMinDistanceFraction);
	NativeBarrier.SetStopNodeReferenceDistance(StopNodeReferenceDistance);
	NativeBarrier.SetWireContactDynamicsSolverDampingScale(WireContactDynamicsSolverDampingScale);
}

bool FAGX_WireParameterController::HasNative() const
{
	return NativeBarrier.HasNative();
}
