// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Utilities/BarrierBase.h"

// Standard library includes.
#include <memory>

struct FUnitRef;

class AGXUNREALBARRIER_API FPowerLineUnitBarrier : public FBarrierBase<FPowerLineUnitBarrier>
{
public:
	FPowerLineUnitBarrier(std::unique_ptr<FUnitRef>&& Native);
	virtual ~FPowerLineUnitBarrier();
	FUnitRef* GetNative();
	const FUnitRef* GetNative() const;

protected:
	std::unique_ptr<FUnitRef> NativeRef;
};
