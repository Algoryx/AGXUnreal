// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/CameraPhotodetectorBarrier.h"

struct AGXUNREALBARRIER_API FCameraCMOSSensorBarrier : public FCameraPhotodetectorBarrier
{
	FCameraCMOSSensorBarrier() = default;
	FCameraCMOSSensorBarrier(std::shared_ptr<FCameraPhotodetectorRef> Native);
	virtual ~FCameraCMOSSensorBarrier() override = default;

	virtual void AllocateNative() override;

	static bool IsCMOSSensor(const FCameraPhotodetectorBarrier& Photodetector);
};
