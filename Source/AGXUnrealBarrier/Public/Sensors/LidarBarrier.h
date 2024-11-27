// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_LidarEnums.h"
#include "Sensors/AGX_DistanceGaussianNoiseSettings.h"
#include "Sensors/AGX_RayAngleGaussianNoiseSettings.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <memory>

class FCustomPatternFetcherBase;
class FLidarOutputBarrier;
class UAGX_LidarModelParameters;

struct FAGX_RealInterval;
struct FDistanceGaussianNoiseRef;
struct FLidarRef;

class AGXUNREALBARRIER_API FLidarBarrier
{
public:
	FLidarBarrier();
	FLidarBarrier(std::unique_ptr<FLidarRef> Native);
	FLidarBarrier(FLidarBarrier&& Other);
	~FLidarBarrier();

	bool HasNative() const;
	void AllocateNative(EAGX_LidarModel Model, const UAGX_LidarModelParameters& Params);
	void AllocateNativeCustomRayPattern(FCustomPatternFetcherBase& PatternFetcher);
	FLidarRef* GetNative();
	const FLidarRef* GetNative() const;
	uint64 GetNativeAddress() const;
	void SetNativeAddress(uint64 Address);
	void ReleaseNative();

	void SetEnabled(bool Enabled);
	bool GetEnabled() const;

	void SetTransform(const FTransform& Transform);

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

	/**
	 * Increment the reference count of the AGX Dynamics object. This should always be paired with
	 * a call to DecrementRefCount, and the count should only be artificially incremented for a
	 * very well specified duration.
	 *
	 * One use-case is during a Blueprint Reconstruction, when the Unreal Engine objects are
	 * destroyed and then recreated. During this time the AGX Dynamics objects are retained and
	 * handed between the old and the new Unreal Engine objects through a Component Instance Data.
	 * This Component Instance Data instance is considered the owner of the AGX Dynamics object
	 * during this transition period and the reference count is therefore increment during its
	 * lifetime. We're lending out ownership of the AGX Dynamics object to the Component Instance
	 * Data instance for the duration of the Blueprint Reconstruction.
	 *
	 * These functions can be const even though they have observable side effects because the
	 * reference count is not a salient part of the AGX Dynamics objects, and they are thread-safe.
	 */
	void IncrementRefCount() const;
	void DecrementRefCount() const;

private:
	FLidarBarrier(const FLidarBarrier&) = delete;
	void operator=(const FLidarBarrier&) = delete;

private:
	std::unique_ptr<FLidarRef> NativeRef;
};
