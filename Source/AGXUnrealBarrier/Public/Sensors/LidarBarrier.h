// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_LidarEnums.h"
#include "Sensors/AGX_DistanceGaussianNoiseSettings.h"
#include "Sensors/AGX_RayAngleGaussianNoiseSettings.h"
#include "Sensors/SensorBarrier.h"

// Standard Library includes.
#include <vector>

#include "LidarBarrier.generated.h"

class FCustomPatternFetcherBase;
class FLidarOutputBarrier;
class UAGX_LidarModelParameters;

struct FAGX_RealInterval;

USTRUCT(BlueprintType)
struct AGXUNREALBARRIER_API FLidarBarrier : public FSensorBarrier
{
	GENERATED_BODY()

	virtual ~FLidarBarrier() override = default;

	void AllocateNative(EAGX_LidarModel Model, const UAGX_LidarModelParameters& Params);
	void AllocateNativeCustomRayPattern(FCustomPatternFetcherBase& PatternFetcher);

	void SetTransform(const FTransform& Transform);
	FTransform GetTransform() const;

	void SetRange(FAGX_RealInterval Range);
	FAGX_RealInterval GetRange() const;

	void SetBeamDivergence(double BeamDivergence);
	double GetBeamDivergence() const;

	void SetBeamExitRadius(double BeamExitRadius);
	double GetBeamExitRadius() const;

	void SetEnableRemoveRayMisses(bool bEnable);
	bool GetEnableRemoveRayMisses() const;

	void SetRaytraceDepth(size_t Depth);
	size_t GetRaytraceDepth() const;

	void EnableOrUpdateDistanceGaussianNoise(const FAGX_DistanceGaussianNoiseSettings& Settings);
	void DisableDistanceGaussianNoise();
	bool GetEnableDistanceGaussianNoise() const;

	void EnableOrUpdateRayAngleGaussianNoise(const FAGX_RayAngleGaussianNoiseSettings& Settings);
	void DisableRayAngleGaussianNoise();
	bool GetEnableRayAngleGaussianNoise() const;

	void AddOutput(FLidarOutputBarrier& Output);

	void MarkOutputAsRead();
};
