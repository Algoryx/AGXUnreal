// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_CameraEnums.h"
#include "Sensors/AGX_ColorMappingMatrix.h"
#include "Sensors/CameraOutputBarrier.h"

struct AGXUNREALBARRIER_API FCameraOutputColorBarrier : public FCameraOutputBarrier
{
	FCameraOutputColorBarrier() = default;
	FCameraOutputColorBarrier(std::shared_ptr<FCameraOutputRef> Native);
	virtual ~FCameraOutputColorBarrier() override = default;

	virtual void AllocateNative() override;

	void GetDataBytes(TArray<uint8>& OutData) const;
	void GetDataU8(TArray<uint8>& OutData) const;
	void GetDataF32(TArray<float>& OutData) const;

	void SetChannelType(EAGX_CameraOutputChannelType InChannelType);
	EAGX_CameraOutputChannelType GetChannelType() const;

	void SetGamma(double InGamma);
	double GetGamma() const;

	void SetColorMappingMatrix(const FAGX_ColorMappingMatrix& InColorMappingMatrix);
	FAGX_ColorMappingMatrix GetColorMappingMatrix() const;

	void SetChannelCount(uint8 InChannelCount);
	uint8 GetChannelCount() const;

	static FCameraOutputColorBarrier CreateFrom(FCameraOutputBarrier& Output);
	static bool IsColorOutput(const FCameraOutputBarrier& Output);
};
