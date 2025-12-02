// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <memory>

struct FDataSeriesRef;

class AGXUNREALBARRIER_API FPlotDataSeriesBarrier
{
public:
	FPlotDataSeriesBarrier();
	FPlotDataSeriesBarrier(std::unique_ptr<FDataSeriesRef> Native);
	FPlotDataSeriesBarrier(FPlotDataSeriesBarrier&& Other) noexcept;
	~FPlotDataSeriesBarrier();

	bool HasNative() const;
	FDataSeriesRef* GetNative();
	const FDataSeriesRef* GetNative() const;

	void AllocateNative(const FString& Name);
	void ReleaseNative();

	FString GetName() const;

	void Write(double Data);

private:
	FPlotDataSeriesBarrier(const FPlotDataSeriesBarrier&) = delete;
	void operator=(const FPlotDataSeriesBarrier&) = delete;

	std::unique_ptr<FDataSeriesRef> NativeRef;
};
