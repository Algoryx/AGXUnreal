// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <memory>

struct FCameraBackendRef;

struct AGXUNREALBARRIER_API FCameraBackendBarrier
{
	FCameraBackendBarrier();
	FCameraBackendBarrier(std::shared_ptr<FCameraBackendRef> Native);
	~FCameraBackendBarrier();

	bool HasNative() const;
	void AllocateNative();
	FCameraBackendRef* GetNative();
	const FCameraBackendRef* GetNative() const;
	void ReleaseNative();

private:
	std::shared_ptr<FCameraBackendRef> NativeRef;
};
