// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/CameraLensBarrier.h"

struct AGXUNREALBARRIER_API FCameraLensSingleElementBarrier : public FCameraLensBarrier
{
	FCameraLensSingleElementBarrier() = default;
	FCameraLensSingleElementBarrier(std::shared_ptr<FCameraLensRef> Native);
	virtual ~FCameraLensSingleElementBarrier() override = default;

	virtual void AllocateNative() override;

	static bool IsSingleElement(const FCameraLensBarrier& Lens);
};
