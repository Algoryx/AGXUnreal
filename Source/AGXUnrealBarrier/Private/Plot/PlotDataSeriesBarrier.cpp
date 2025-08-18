// Copyright 2025, Algoryx Simulation AB.

#include "Plot/PlotDataSeriesBarrier.h"

// AGX Dynamics for Unreal includes.
#include "BarrierOnly/AGXRefs.h"
#include "TypeConversions.h"

FPlotDataSeriesBarrier::FPlotDataSeriesBarrier()
	: NativeRef {new FDataSeriesRef}
{
}

FPlotDataSeriesBarrier::FPlotDataSeriesBarrier(std::unique_ptr<FDataSeriesRef> Native)
	: NativeRef(std::move(Native))
{
	check(NativeRef);
}

FPlotDataSeriesBarrier::FPlotDataSeriesBarrier(FPlotDataSeriesBarrier&& Other) noexcept
	: NativeRef {std::move(Other.NativeRef)}
{
	Other.NativeRef.reset(new FDataSeriesRef);
}

FPlotDataSeriesBarrier::~FPlotDataSeriesBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the
	// std::unique_ptr NativeRef's destructor must be able to see the definition,
	// not just the forward declaration, of FDataSeriesRef.
}

bool FPlotDataSeriesBarrier::HasNative() const
{
	return NativeRef->Native != nullptr;
}

void FPlotDataSeriesBarrier::AllocateNative(const FString& Name)
{
	NativeRef->Native = new agxPlot::DataSeries(Convert(Name));
}

FDataSeriesRef* FPlotDataSeriesBarrier::GetNative()
{
	check(HasNative());
	return NativeRef.get();
}

const FDataSeriesRef* FPlotDataSeriesBarrier::GetNative() const
{
	check(HasNative());
	return NativeRef.get();
}

void FPlotDataSeriesBarrier::ReleaseNative()
{
	NativeRef->Native = nullptr;
}

FString FPlotDataSeriesBarrier::GetName() const
{
	check(HasNative());

	// getName returns agx::String by reference, so no need to do the freeContainerMemory dance
	// here.
	return Convert(NativeRef->Native->getName());
}

void FPlotDataSeriesBarrier::Write(double Data)
{
	check(HasNative());
	NativeRef->Native->push(Data);
}
