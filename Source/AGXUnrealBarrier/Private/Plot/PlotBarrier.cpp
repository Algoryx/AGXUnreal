// Copyright 2025, Algoryx Simulation AB.

#include "Plot/PlotBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "BarrierOnly/AGXRefs.h"
#include "Plot/PlotDataSeriesBarrier.h"
#include "SimulationBarrier.h"
#include "TypeConversions.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxPlot/FilePlot.h>
#include "EndAGXIncludes.h"

// Unreal Engine includes.
#include "HAL/FileManager.h"
#include "Misc/Paths.h"

FPlotBarrier::FPlotBarrier()
	: NativeRef {new FPlotRef}
{
}

FPlotBarrier::FPlotBarrier(std::unique_ptr<FPlotRef> Native)
	: NativeRef(std::move(Native))
{
	check(NativeRef);
}

FPlotBarrier::FPlotBarrier(FPlotBarrier&& Other)
	: NativeRef {std::move(Other.NativeRef)}
{
	Other.NativeRef.reset(new FPlotRef);
}

FPlotBarrier::~FPlotBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the
	// std::unique_ptr NativeRef's destructor must be able to see the definition,
	// not just the forward declaration, of FPlotRef.
}

bool FPlotBarrier::HasNative() const
{
	return NativeRef->Native != nullptr;
}

void FPlotBarrier::AllocateNative(
	const FSimulationBarrier& Simulation, const FString* OutputFileName, bool bOpenWebPlot)
{
	check(!HasNative());
	check(Simulation.HasNative());
	NativeRef->Native = Simulation.GetNative()->Native->getPlotSystem();

	if (bOpenWebPlot)
	{
		OpenWebPlot();
	}

	if (OutputFileName == nullptr)
	{
		return; // We are done.
	}

	const FString OutputFileNameWExtension = [OutputFileName]()
	{
		if (FPaths::GetExtension(*OutputFileName).IsEmpty())
			return *OutputFileName + FString(".csv");
		else
			return *OutputFileName;
	}();

	const FString OutputPath = [&OutputFileNameWExtension]()
	{
		if (FPaths::IsRelative(OutputFileNameWExtension))
		{
			const FString RelOutputDir = FPaths::GetPath(FPaths::GetProjectFilePath());
			const FString OutputDir =
				IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*RelOutputDir);
			return FPaths::Combine(OutputDir, OutputFileNameWExtension);
		}
		else
		{
			return OutputFileNameWExtension;
		}
	}();

	if (!FPaths::DirectoryExists(FPaths::GetPath(OutputPath)))
	{
		UE_LOG(
			LogAGX, Error, TEXT("Unable to write file '%s' because directory '%s' does not exist."),
			**OutputFileName, *FPaths::GetPath(OutputPath));
	}
	else
	{
		NativeRef->Native->add(new agxPlot::FilePlot(Convert(OutputPath)));
	}
}

FPlotRef* FPlotBarrier::GetNative()
{
	check(HasNative());
	return NativeRef.get();
}

const FPlotRef* FPlotBarrier::GetNative() const
{
	check(HasNative());
	return NativeRef.get();
}

void FPlotBarrier::ReleaseNative()
{
	NativeRef->Native = nullptr;
}

void FPlotBarrier::CreatePlot(
	const FString& Name, FPlotDataSeriesBarrier& SeriesX, FPlotDataSeriesBarrier& SeriesY)
{
	check(HasNative());
	check(SeriesX.HasNative());
	check(SeriesY.HasNative());

	agxPlot::Window* plotWindow = NativeRef->Native->getOrCreateWindow(Convert(Name));
	agxPlot::DataSeries* X = SeriesX.GetNative()->Native;
	agxPlot::DataSeries* Y = SeriesY.GetNative()->Native;
	plotWindow->add(new agxPlot::Curve(X, Y, Y->getName()));
}

void FPlotBarrier::OpenWebPlot()
{
	check(HasNative());
	NativeRef->Native->add(new agxPlot::WebPlot(true));
}
