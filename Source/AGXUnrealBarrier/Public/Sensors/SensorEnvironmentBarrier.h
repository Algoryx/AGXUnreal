// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <memory>

class FLidarBarrier;
class FRtAmbientMaterialBarrier;
class FRtLambertianOpaqueMaterialBarrier;
class FSimulationBarrier;
class FTerrainBarrier;
class FTerrainPagerBarrier;
class FWireBarrier;

struct FSensorEnvironmentRef;

class AGXUNREALBARRIER_API FSensorEnvironmentBarrier
{
public:
	FSensorEnvironmentBarrier();
	FSensorEnvironmentBarrier(std::unique_ptr<FSensorEnvironmentRef> Native);
	FSensorEnvironmentBarrier(FSensorEnvironmentBarrier&& Other);
	~FSensorEnvironmentBarrier();

	bool HasNative() const;
	void AllocateNative(FSimulationBarrier& Simulation);
	FSensorEnvironmentRef* GetNative();
	const FSensorEnvironmentRef* GetNative() const;
	void ReleaseNative();

	bool Add(FLidarBarrier& Lidar);
	bool Add(FTerrainBarrier& Terrain);
	bool Add(FTerrainPagerBarrier& Pager);
	bool Add(FWireBarrier& Wire);

	bool Remove(FLidarBarrier& Lidar);
	bool Remove(FTerrainBarrier& Terrain);
	bool Remove(FTerrainPagerBarrier& Pager);
	bool Remove(FWireBarrier& Wire);

	void SetAmbientMaterial(FRtAmbientMaterialBarrier* Material);

	void SetLidarSurfaceMaterialOrDefault(
		FTerrainBarrier& Terrain, FRtLambertianOpaqueMaterialBarrier* Material);
	void SetLidarSurfaceMaterialOrDefault(
		FTerrainPagerBarrier& TerrainPager, FRtLambertianOpaqueMaterialBarrier* Material);
	void SetLidarSurfaceMaterialOrDefault(
		FWireBarrier& Wire, FRtLambertianOpaqueMaterialBarrier* Material);

	/// Returns true if raytrace (RTX) is supported on this computer.
	static bool IsRaytraceSupported();

	/// Returns list of raytrace (RTX) supporting GPU devices.
	static TArray<FString> GetRaytraceDevices();

	/**
	 * Get index of the hardware device currently in use for raytracing, or -1 if raytracing is not
	 * supported.
	 */
	static int32 GetCurrentRayraceDevice();

	/**
	 * Attempts to set the hardware device to use to the one with the specified index.
	 * Returns true if the hardware device is successfully set.
	 */
	static bool SetCurrentRaytraceDevice(int32 DeviceIndex);

private:
	FSensorEnvironmentBarrier(const FSensorEnvironmentBarrier&) = delete;
	void operator=(const FSensorEnvironmentBarrier&) = delete;

private:
	std::unique_ptr<FSensorEnvironmentRef> NativeRef;
};
