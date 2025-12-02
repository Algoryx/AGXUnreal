// Copyright 2025, Algoryx Simulation AB.

#include "AMOR/MergeSplitThresholdsBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "BarrierOnly/AGXRefs.h"
#include "TypeConversions.h"

FMergeSplitThresholdsBarrier::FMergeSplitThresholdsBarrier()
	: NativeRef {new FMergeSplitThresholdsRef}
{
}

FMergeSplitThresholdsBarrier::FMergeSplitThresholdsBarrier(
	std::unique_ptr<FMergeSplitThresholdsRef>&& Native)
	: NativeRef(std::move(Native))
{
}

FMergeSplitThresholdsBarrier::FMergeSplitThresholdsBarrier(FMergeSplitThresholdsBarrier&& Other)
	: NativeRef(std::move(Other.NativeRef))
{
	Other.NativeRef = std::make_unique<FMergeSplitThresholdsRef>();
}

FMergeSplitThresholdsBarrier::~FMergeSplitThresholdsBarrier()
{
}

bool FMergeSplitThresholdsBarrier::HasNative() const
{
	return NativeRef && NativeRef->Native;
}

FMergeSplitThresholdsRef* FMergeSplitThresholdsBarrier::GetNative()
{
	return NativeRef.get();
}

const FMergeSplitThresholdsRef* FMergeSplitThresholdsBarrier::GetNative() const
{
	return NativeRef.get();
}

void FMergeSplitThresholdsBarrier::ReleaseNative()
{
	check(HasNative());
	NativeRef->Native = nullptr;
}

FGuid FMergeSplitThresholdsBarrier::GetGuid() const
{
	check(HasNative());
	return Convert(NativeRef->Native->getUuid());
}
