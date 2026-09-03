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

	void SetFocalLength(double InFocalLength);
	double GetFocalLength() const;
	void SetFStop(double InFStop);
	double GetFStop() const;
	void SetAutofocus(double InMinimumFocusDistance);
	bool GetUseAutofocus() const;
	void SetFocusDistance(double InFocusDistance);
	double GetFocusDistance() const;
	double GetMinimumFocusDistance() const;

	static bool IsSingleElement(const FCameraLensBarrier& Lens);
};
