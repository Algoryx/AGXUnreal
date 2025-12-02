// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "Misc/Guid.h"

// Standard library includes.
#include <memory>

struct FMergeSplitThresholdsRef;

class AGXUNREALBARRIER_API FMergeSplitThresholdsBarrier
{
public:
	FMergeSplitThresholdsBarrier();
	FMergeSplitThresholdsBarrier(std::unique_ptr<FMergeSplitThresholdsRef>&& Native);
	FMergeSplitThresholdsBarrier(FMergeSplitThresholdsBarrier&& Other);
	virtual ~FMergeSplitThresholdsBarrier();

	bool HasNative() const;
	FMergeSplitThresholdsRef* GetNative();
	const FMergeSplitThresholdsRef* GetNative() const;

	void ReleaseNative();

	FGuid GetGuid() const;

private:
	FMergeSplitThresholdsBarrier(const FMergeSplitThresholdsBarrier&) = delete;
	void operator=(const FMergeSplitThresholdsBarrier&) = delete;

protected:
	std::unique_ptr<FMergeSplitThresholdsRef> NativeRef;
};
