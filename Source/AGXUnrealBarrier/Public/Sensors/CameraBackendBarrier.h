// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <memory>

struct FCameraBarrier;
struct FCameraBackendRef;

struct AGXUNREALBARRIER_API FCameraBackendBarrier
{
	static FCameraBackendBarrier& GetInstance();

	FCameraBackendBarrier(const FCameraBackendBarrier&) = delete;
	FCameraBackendBarrier& operator=(const FCameraBackendBarrier&) = delete;
	FCameraBackendBarrier(FCameraBackendBarrier&&) = delete;
	FCameraBackendBarrier& operator=(FCameraBackendBarrier&&) = delete;

	bool HasNative() const;
	FCameraBackendRef* GetNative();
	const FCameraBackendRef* GetNative() const;

	void Add(FCameraBarrier& Camera);
	bool Remove(FCameraBarrier& Camera);

	FCameraBarrier* FindCamera(uint64 NativeCameraAddress);
	const FCameraBarrier* FindCamera(uint64 NativeCameraAddress) const;

private:
	FCameraBackendBarrier();
	~FCameraBackendBarrier();

	void AllocateNative();
	void ReleaseNative();

	// Key is the address of the native AGX Camera.
	TMap<uint64, FCameraBarrier*> CameraBarriers;
	std::shared_ptr<FCameraBackendRef> NativeRef;
};
