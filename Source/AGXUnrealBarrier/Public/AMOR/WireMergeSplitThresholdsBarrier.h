// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AMOR/MergeSplitThresholdsBarrier.h"

class FWireBarrier;

class AGXUNREALBARRIER_API FWireMergeSplitThresholdsBarrier : public FMergeSplitThresholdsBarrier
{
public:
	FWireMergeSplitThresholdsBarrier();
	FWireMergeSplitThresholdsBarrier(FWireMergeSplitThresholdsBarrier&& Other) = default;
	FWireMergeSplitThresholdsBarrier(std::unique_ptr<FMergeSplitThresholdsRef>&& Native);
	~FWireMergeSplitThresholdsBarrier();

	void AllocateNative();

	void SetForcePropagationDecayScale(double InForcePropagationDecayScale);
	double GetForcePropagationDecayScale() const;

	void SetMergeTensionScale(double InMergeTensionScale);
	double GetMergeTensionScale() const;

	static FWireMergeSplitThresholdsBarrier CreateFrom(const FWireBarrier& Barrier);

private:
	FWireMergeSplitThresholdsBarrier(const FWireMergeSplitThresholdsBarrier&) = delete;
	void operator=(const FWireMergeSplitThresholdsBarrier&) = delete;
};
