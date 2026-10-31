// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <memory>

struct FCameraLensRef;

struct AGXUNREALBARRIER_API FCameraLensBarrier
{
	FCameraLensBarrier();
	FCameraLensBarrier(std::shared_ptr<FCameraLensRef> Native);
	virtual ~FCameraLensBarrier() = default;

	virtual void AllocateNative() PURE_VIRTUAL(FCameraLensBarrier::AllocateNative, );

	bool HasNative() const;
	FCameraLensRef* GetNative();
	const FCameraLensRef* GetNative() const;
	void ReleaseNative();

protected:
	std::shared_ptr<FCameraLensRef> NativeRef;
};
