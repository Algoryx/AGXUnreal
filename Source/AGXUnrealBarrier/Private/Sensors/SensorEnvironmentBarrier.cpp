// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/SensorEnvironmentBarrier.h"

// AGX Dynamics for Unreal includes.
#include "BarrierOnly/AGXRefs.h"
#include "BarrierOnly/Wire/WireRef.h"
#include "Sensors/LidarBarrier.h"
#include "Sensors/RtAmbientMaterialBarrier.h"
#include "Sensors/RtLambertianOpaqueMaterialBarrier.h"
#include "Sensors/SensorRef.h"
#include "Sensors/RtAmbientMaterialBarrier.h"
#include "Sensors/RtLambertianOpaqueMaterialBarrier.h"
#include "SimulationBarrier.h"
#include "Terrain/TerrainBarrier.h"
#include "Terrain/TerrainPagerBarrier.h"
#include "TypeConversions.h"
#include "Wire/WireBarrier.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxSensor/RaytraceAmbientMaterial.h>
#include <agxSensor/RaytraceSurfaceMaterial.h>
#include <agxSensor/RaytraceConfig.h>
#include <agxUtil/agxUtil.h>
#include "EndAGXIncludes.h"

// Standard Library includes.
#include <mutex>
#include <string>
#include <vector>

FSensorEnvironmentBarrier::FSensorEnvironmentBarrier()
	: NativeRef {new FSensorEnvironmentRef}
{
}

FSensorEnvironmentBarrier::FSensorEnvironmentBarrier(std::unique_ptr<FSensorEnvironmentRef> Native)
	: NativeRef(std::move(Native))
{
}

FSensorEnvironmentBarrier::FSensorEnvironmentBarrier(FSensorEnvironmentBarrier&& Other)
	: NativeRef {std::move(Other.NativeRef)}
{
	Other.NativeRef.reset(new FSensorEnvironmentRef);
}

FSensorEnvironmentBarrier::~FSensorEnvironmentBarrier()
{
}

bool FSensorEnvironmentBarrier::HasNative() const
{
	return NativeRef->Native != nullptr;
}

void FSensorEnvironmentBarrier::AllocateNative(FSimulationBarrier& Simulation)
{
	check(!HasNative());
	check(Simulation.HasNative());
	NativeRef->Native = agxSensor::Environment::getOrCreate(Simulation.GetNative()->Native);
}

FSensorEnvironmentRef* FSensorEnvironmentBarrier::GetNative()
{
	check(HasNative());
	return NativeRef.get();
}

const FSensorEnvironmentRef* FSensorEnvironmentBarrier::GetNative() const
{
	check(HasNative());
	return NativeRef.get();
}

void FSensorEnvironmentBarrier::ReleaseNative()
{
}

bool FSensorEnvironmentBarrier::Add(FLidarBarrier& Lidar)
{
	check(HasNative());
	check(Lidar.HasNative());
	return NativeRef->Native->add(Lidar.GetNative()->Native);
}

bool FSensorEnvironmentBarrier::Add(FTerrainBarrier& Terrain)
{
	check(HasNative());
	check(Terrain.HasNative());
	return NativeRef->Native->add(Terrain.GetNative()->Native);
}

bool FSensorEnvironmentBarrier::Add(FTerrainPagerBarrier& Pager)
{
	check(HasNative());
	check(Pager.HasNative());
	return NativeRef->Native->add(Pager.GetNative()->Native);
}

bool FSensorEnvironmentBarrier::Add(FWireBarrier& Wire)
{
	check(HasNative());
	check(Wire.HasNative());
	return NativeRef->Native->add(Wire.GetNative()->Native);
}

bool FSensorEnvironmentBarrier::Remove(FLidarBarrier& Lidar)
{
	check(HasNative());
	check(Lidar.HasNative());
	return NativeRef->Native->remove(Lidar.GetNative()->Native);
}

bool FSensorEnvironmentBarrier::Remove(FTerrainBarrier& Terrain)
{
	check(HasNative());
	check(Terrain.HasNative());
	return NativeRef->Native->remove(Terrain.GetNative()->Native);
}

bool FSensorEnvironmentBarrier::Remove(FTerrainPagerBarrier& Pager)
{
	check(HasNative());
	check(Pager.HasNative());
	return NativeRef->Native->remove(Pager.GetNative()->Native);
}

bool FSensorEnvironmentBarrier::Remove(FWireBarrier& Wire)
{
	check(HasNative());
	check(Wire.HasNative());
	return NativeRef->Native->remove(Wire.GetNative()->Native);
}

void FSensorEnvironmentBarrier::SetAmbientMaterial(FRtAmbientMaterialBarrier* Material)
{
	check(HasNative());
	if (Material == nullptr)
		NativeRef->Native->getScene()->setMaterial({nullptr});
	else
		NativeRef->Native->getScene()->setMaterial(*Material->GetNative()->Native);
}

namespace SensorEnvironmentBarrier_helpers
{
	template <typename TBarrier>
	void SetLidarSurfaceMaterialOrDefault(
		TBarrier& Barrier, FRtLambertianOpaqueMaterialBarrier* Material)
	{
		check(Barrier.HasNative());

		if (Material == nullptr)
		{
			// Assign default if setting nullptr Material.
			agxSensor::RtSurfaceMaterial::set(
				Barrier.GetNative()->Native, agxSensor::RtLambertianOpaqueMaterial::create());
		}
		else
		{
			check(Material->HasNative());
			agxSensor::RtSurfaceMaterial::set(
				Barrier.GetNative()->Native, *Material->GetNative()->Native);
		}
	}
}

void FSensorEnvironmentBarrier::SetLidarSurfaceMaterialOrDefault(
	FTerrainBarrier& Terrain, FRtLambertianOpaqueMaterialBarrier* Material)
{
	SensorEnvironmentBarrier_helpers::SetLidarSurfaceMaterialOrDefault(Terrain, Material);
}

void FSensorEnvironmentBarrier::SetLidarSurfaceMaterialOrDefault(
	FTerrainPagerBarrier& TerrainPager, FRtLambertianOpaqueMaterialBarrier* Material)
{
	SensorEnvironmentBarrier_helpers::SetLidarSurfaceMaterialOrDefault(TerrainPager, Material);
}

void FSensorEnvironmentBarrier::SetLidarSurfaceMaterialOrDefault(
	FWireBarrier& Wire, FRtLambertianOpaqueMaterialBarrier* Material)
{
	SensorEnvironmentBarrier_helpers::SetLidarSurfaceMaterialOrDefault(Wire, Material);
}

bool FSensorEnvironmentBarrier::IsRaytraceSupported()
{
	static bool bIsRaytraceSupported = true;
	static std::once_flag InitFlag; // Ensures initialization runs only once.

	std::call_once(
		InitFlag,
		[]()
		{
			bIsRaytraceSupported = agxSensor::RtConfig::isRaytraceSupported();
			if (!bIsRaytraceSupported)
				return;

			// We might get false positives from the isRaytraceSupported check above, try initialize
			// the library to ensure everything works as expected.
			FRtAmbientMaterialBarrier Barrier;
			try
			{
				UE_LOG(LogAGX, Log, TEXT("Trying to initialize a Lidar material."));
				Barrier.AllocateNative();
				bIsRaytraceSupported = Barrier.HasNative();
				Barrier.ReleaseNative();
			}
			catch (...)
			{
				UE_LOG(LogAGX, Log, TEXT("Caught exception after Lidar material initialization."));
				Barrier.ReleaseNative();
				bIsRaytraceSupported = false;
			}
		});

	return bIsRaytraceSupported;
}

TArray<FString> FSensorEnvironmentBarrier::GetRaytraceDevices()
{
	std::vector<std::string> DevicesAGX = agxSensor::RtConfig::listRaytraceDevices();
	TArray<FString> Devices = ToUnrealStringArray(DevicesAGX);

	// Must be called to avoid crash due to different allocators used by AGX Dynamics and
	// Unreal Engine.
	agxUtil::freeContainerMemory(DevicesAGX);

	return Devices;
}

int32 FSensorEnvironmentBarrier::GetCurrentRayraceDevice()
{
	return agxSensor::RtConfig::getRaytraceDevice();
}

bool FSensorEnvironmentBarrier::SetCurrentRaytraceDevice(int32 DeviceIndex)
{
	return agxSensor::RtConfig::setRaytraceDevice(DeviceIndex);
}
