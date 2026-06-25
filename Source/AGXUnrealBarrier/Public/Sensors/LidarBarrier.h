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
class UAGX_CustomRayPatternParameters;
class UAGX_GenericHorizontalSweepParameters;
class UAGX_LidarModelParameters;
class UAGX_OusterOS0Parameters;
class UAGX_OusterOS1Parameters;
class UAGX_OusterOS2Parameters;

struct FAGX_RealInterval;
struct FRigidBodyBarrier;

USTRUCT(BlueprintType)
struct AGXUNREALBARRIER_API FLidarBarrier : public FSensorBarrier
{
	GENERATED_BODY()

	FLidarBarrier() = default;
	FLidarBarrier(
		std::shared_ptr<FSensorRef> Native, std::shared_ptr<FSensorGroupStepStrideRef> StepStride);
	virtual ~FLidarBarrier() override = default;

	EAGX_LidarModel GetModel() const;

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
	FAGX_DistanceGaussianNoiseSettings GetDistanceGaussianNoiseSettings() const;

	void EnableOrUpdateRayAngleGaussianNoise(const FAGX_RayAngleGaussianNoiseSettings& Settings);
	void DisableRayAngleGaussianNoise();
	bool GetEnableRayAngleGaussianNoise() const;
	FAGX_RayAngleGaussianNoiseSettings GetRayAngleGaussianNoiseSettings() const;

	bool ReadModelParameters(UAGX_CustomRayPatternParameters& Parameters) const;
	bool ReadModelParameters(UAGX_GenericHorizontalSweepParameters& Parameters) const;
	bool ReadModelParameters(UAGX_OusterOS0Parameters& Parameters) const;
	bool ReadModelParameters(UAGX_OusterOS1Parameters& Parameters) const;
	bool ReadModelParameters(UAGX_OusterOS2Parameters& Parameters) const;

	bool UsesHorizontalSweepRayPattern() const;

	/// Returns the Rigid Body this Lidar is attached to, if it exists.
	FRigidBodyBarrier GetRigidBody() const;

	void AddOutput(FLidarOutputBarrier& Output);

	void MarkOutputAsRead();

	static bool IsLidar(const FSensorBarrier& Sensor);
};
