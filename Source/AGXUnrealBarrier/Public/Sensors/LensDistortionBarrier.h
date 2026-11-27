// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <memory>

struct FLensDistortionRef;

struct AGXUNREALBARRIER_API FLensDistortionBarrier
{
	FLensDistortionBarrier();
	FLensDistortionBarrier(std::shared_ptr<FLensDistortionRef> Native);
	virtual ~FLensDistortionBarrier() = default;

	virtual void AllocateNative() PURE_VIRTUAL(FLensDistortionBarrier::AllocateNative, );

	bool HasNative() const;
	FLensDistortionRef* GetNative();
	const FLensDistortionRef* GetNative() const;
	void ReleaseNative();

protected:
	std::shared_ptr<FLensDistortionRef> NativeRef;
};
