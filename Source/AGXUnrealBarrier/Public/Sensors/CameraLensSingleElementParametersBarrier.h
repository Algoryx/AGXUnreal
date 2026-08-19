// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <memory>

struct FCameraLensSingleElementParametersRef;

struct AGXUNREALBARRIER_API FCameraLensSingleElementParametersBarrier
{
	FCameraLensSingleElementParametersBarrier();
	FCameraLensSingleElementParametersBarrier(
		std::shared_ptr<FCameraLensSingleElementParametersRef> Native);
	~FCameraLensSingleElementParametersBarrier() = default;

	bool HasNative() const;

	void SetFocalLength(double FocalLength);
	double GetFocalLength() const;

	void SetFStop(double FStop);
	double GetFStop() const;

	/// Also enables autofocus.
	void SetMinimumFocusDistance(double MinimumFocusDistance);

	/// Returns -1 if autofocus is disabled.
	double GetMinimumFocusDistance() const;

	/// Also disables autofocus.
	void SetFocusDistance(double FocusDistance);

	/// Returns -1 if autofocus is enabled.
	double GetFocusDistance() const;

	bool IsAutofocusEnabled() const;

private:
	std::shared_ptr<FCameraLensSingleElementParametersRef> NativeRef;
};
