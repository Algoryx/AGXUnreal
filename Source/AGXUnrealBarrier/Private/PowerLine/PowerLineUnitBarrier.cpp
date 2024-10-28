// Copyright 2024, Algoryx Simulation AB.

#include "PowerLine/PowerLineUnitBarrier.h"

// AGX Dynamics for Unreal includes.
#include "BarrierOnly/PowerLine/PowerLineRefs.h"
#include "BarrierOnly/BarrierUtilities.h"
#include "Utilities/BarrierBase.inl.h"

FPowerLineUnitBarrier::FPowerLineUnitBarrier(std::unique_ptr<FUnitRef>&& Native)
	: NativeRef {std::move(Native)}
{
}

FPowerLineUnitBarrier::~FPowerLineUnitBarrier()
{
}

FUnitRef* FPowerLineUnitBarrier::GetNative()
{
	return NativeRef.get();
}

const FUnitRef* FPowerLineUnitBarrier::GetNative() const
{
	return NativeRef.get();
}

template class AGXUNREALBARRIER_API FBarrierBase<FPowerLineUnitBarrier>;
