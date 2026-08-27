// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <memory>

struct FCameraOutputRef;

struct AGXUNREALBARRIER_API FCameraOutputBarrier
{
	FCameraOutputBarrier();
	FCameraOutputBarrier(std::shared_ptr<FCameraOutputRef> Native);
	virtual ~FCameraOutputBarrier() = default;

	virtual void AllocateNative() PURE_VIRTUAL(FCameraOutputBarrier::AllocateNative, );

	bool HasNative() const;
	FCameraOutputRef* GetNative();
	const FCameraOutputRef* GetNative() const;
	void ReleaseNative();

	void SetResolution(FIntPoint InResolution);
	FIntPoint GetResolution() const;

	void SetConstantCapture(double InFrameRate);
	void SetManualCapture();
	bool GetConstantCapture() const;
	double GetFrameRate() const;

protected:
	std::shared_ptr<FCameraOutputRef> NativeRef;
};
