// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "DriveTrain/CombustionEngineParameters.h"
#include "PowerLine/PowerLineUnitBarrier.h"

// Standard library includes.
#include <memory>


class AGXUNREALBARRIER_API FCombustionEngineBarrier : public FPowerLineUnitBarrier
{
public:
	FCombustionEngineBarrier();
	FCombustionEngineBarrier(std::unique_ptr<FUnitRef> Native);
	FCombustionEngineBarrier(const FCombustionEngineBarrier& Other);
	FCombustionEngineBarrier(FCombustionEngineBarrier&& Other);
	FCombustionEngineBarrier& operator=(const FCombustionEngineBarrier& Other);
	virtual ~FCombustionEngineBarrier() = default;

	void AllocateNative(FAGX_CombustionEngineParameters Parameters);

	void SetCombustionEngineParameters(const FAGX_CombustionEngineParameters& Parameters);
	void SetThrottle(double Throttle);
	double GetThrottle() const;
};
