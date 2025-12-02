// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <memory>

class FPlotDataSeriesBarrier;
class FSimulationBarrier;

struct FPlotRef;

class AGXUNREALBARRIER_API FPlotBarrier
{
public:
	FPlotBarrier();
	FPlotBarrier(std::unique_ptr<FPlotRef> Native);
	FPlotBarrier(FPlotBarrier&& Other);
	~FPlotBarrier();

	bool HasNative() const;
	FPlotRef* GetNative();
	const FPlotRef* GetNative() const;

	/**
	 * The OutputFileName can be a file name with or without extension, or a relative (to the
	 * project root) or absolute file path. If no file extension is given, a .csv extension will be
	 * appended to the final output path.
	 */
	void AllocateNative(
		const FSimulationBarrier& Simulation, const FString* OutputFileName, bool bOpenWebPlot);
	void ReleaseNative();

	void CreatePlot(
		const FString& Name, FPlotDataSeriesBarrier& SeriesX, FPlotDataSeriesBarrier& SeriesY);

	void OpenWebPlot();

private:
	FPlotBarrier(const FPlotBarrier&) = delete;
	void operator=(const FPlotBarrier&) = delete;

private:
	std::unique_ptr<FPlotRef> NativeRef;
};
