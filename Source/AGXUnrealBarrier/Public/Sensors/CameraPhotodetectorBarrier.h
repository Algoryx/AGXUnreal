// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <memory>

struct FCameraPhotodetectorRef;

struct AGXUNREALBARRIER_API FCameraPhotodetectorBarrier
{
	FCameraPhotodetectorBarrier();
	FCameraPhotodetectorBarrier(std::shared_ptr<FCameraPhotodetectorRef> Native);
	virtual ~FCameraPhotodetectorBarrier() = default;

	virtual void AllocateNative() PURE_VIRTUAL(FCameraPhotodetectorBarrier::AllocateNative, );

	bool HasNative() const;
	FCameraPhotodetectorRef* GetNative();
	const FCameraPhotodetectorRef* GetNative() const;
	void ReleaseNative();

protected:
	std::shared_ptr<FCameraPhotodetectorRef> NativeRef;
};
