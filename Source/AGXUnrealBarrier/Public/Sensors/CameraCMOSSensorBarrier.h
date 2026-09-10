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

	void SetSize(const FVector2D& InSize);
	FVector2D GetSize() const;

	void SetISO(double InISO);
	double GetISO() const;

	void SetShutterSpeed(double InShutterSpeed);
	double GetShutterSpeed() const;

	void SetAutoExposure(double InDynamicRange);
	bool GetUseAutoExposure() const;
	double GetDynamicRange() const;

	void SetManualExposureCompensation(double InExposureCompensation);
	double GetExposureCompensation() const;

	static bool IsCMOSSensor(const FCameraPhotodetectorBarrier& Photodetector);
};
