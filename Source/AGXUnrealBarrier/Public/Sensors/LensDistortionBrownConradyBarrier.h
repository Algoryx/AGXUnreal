// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/LensDistortionBarrier.h"

struct AGXUNREALBARRIER_API FLensDistortionBrownConradyBarrier : public FLensDistortionBarrier
{
	FLensDistortionBrownConradyBarrier() = default;
	FLensDistortionBrownConradyBarrier(std::shared_ptr<FLensDistortionRef> Native);
	virtual ~FLensDistortionBrownConradyBarrier() override = default;

	virtual void AllocateNative() override;

	void SetK1(double InK1);
	double GetK1() const;

	void SetK2(double InK2);
	double GetK2() const;

	void SetK3(double InK3);
	double GetK3() const;

	void SetP1(double InP1);
	double GetP1() const;

	void SetP2(double InP2);
	double GetP2() const;

	static bool IsBrownConrady(const FLensDistortionBarrier& Distortion);
};
