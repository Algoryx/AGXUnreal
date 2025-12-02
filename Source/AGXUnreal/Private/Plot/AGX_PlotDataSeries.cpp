// Copyright 2025, Algoryx Simulation AB.

#include "Plot/AGX_PlotDataSeries.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"

void FAGX_PlotDataSeries::Write(float Data)
{
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Write was called on PlotDataSeries '%s' but it does not have an AGX Native. Make "
				 "sure this PlotDataSeries is part of a Plot. It is registered by calling the "
				 "AGX_PlotComponent::CreatePlot function. Also note that the Write function should "
				 "only be called during Play."),
			*Label);
		return;
	}

	NativeBarrier.Write(static_cast<double>(Data));
}

bool FAGX_PlotDataSeries::HasNative() const
{
	return NativeBarrier.HasNative();
}

FAGX_PlotDataSeries& FAGX_PlotDataSeries::operator=(const FAGX_PlotDataSeries& Other)
{
	Label = Other.Label;
	return *this;
}

FAGX_PlotDataSeries::FAGX_PlotDataSeries(const FAGX_PlotDataSeries& Other)
{
	Label = Other.Label;
}
