// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/CameraBackendBarrier.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/CameraBarrier.h"
#include "Sensors/CameraLensSingleElementParametersBarrier.h"
#include "Sensors/SensorRef.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxSensor/Camera.h>
#include "EndAGXIncludes.h"

namespace CameraBackendBarrier_helpers
{
	uint64 GetCameraNativeAddress(const void* NativeCamera)
	{
		return reinterpret_cast<uint64>(NativeCamera);
	}

	uint64 GetCameraNativeAddress(FCameraBarrier& Camera)
	{
		check(Camera.HasNative());

		const agxSensor::Camera* NativeCamera =
			Camera.GetNative()->Native->asSafe<agxSensor::Camera>();
		check(NativeCamera != nullptr);
		return GetCameraNativeAddress(NativeCamera);
	}

	void SynchronizeGraphics(agxSensor::Camera* Camera, agxSensor::Matrix4x4*)
	{
		FCameraBackendBarrier::GetInstance().FindCamera(GetCameraNativeAddress(Camera));
		UE_LOG(LogTemp, Warning, TEXT("CameraBackendBarrier_helpers::SynchronizeGraphics"));
	}

	void SetCameraLensSingleElement(
		agxSensor::Camera* Camera, agxSensor::CameraLensSingleElement*,
		agxSensor::CameraLensSingleElementParameters* Parameters)
	{
		if (FCameraBarrier* CameraBarrier =
				FCameraBackendBarrier::GetInstance().FindCamera(GetCameraNativeAddress(Camera)))
		{
			FCameraLensSingleElementParametersBarrier ParametersBarrier(
				std::make_shared<FCameraLensSingleElementParametersRef>(Parameters));
			CameraBarrier->OnBackendSetCameraLensSingleElement(ParametersBarrier);
		}
	}

	void SetCameraCMOSSensor(
		agxSensor::Camera* Camera, agxSensor::CameraCMOSSensor*,
		agxSensor::CameraCMOSSensorParameters*)
	{
		FCameraBackendBarrier::GetInstance().FindCamera(GetCameraNativeAddress(Camera));
		UE_LOG(LogTemp, Warning, TEXT("CameraBackendBarrier_helpers::SetCameraCMOSSensor"));
	}

	void SetCameraLensDistortionNone(agxSensor::Camera* Camera, agxSensor::CameraLens*)
	{
		FCameraBackendBarrier::GetInstance().FindCamera(GetCameraNativeAddress(Camera));
		UE_LOG(LogTemp, Warning, TEXT("CameraBackendBarrier_helpers::SetCameraLensDistortionNone"));
	}

	void SetCameraLensDistortionBrownConrady(
		agxSensor::Camera* Camera, agxSensor::CameraLens*,
		agxSensor::LensDistortionBrownConradyCoefficients*)
	{
		FCameraBackendBarrier::GetInstance().FindCamera(GetCameraNativeAddress(Camera));
		UE_LOG(
			LogTemp, Warning,
			TEXT("CameraBackendBarrier_helpers::SetCameraLensDistortionBrownConrady"));
	}

	void SetCameraColorOutput(
		agxSensor::Camera* Camera, agxSensor::CameraColorOutput*,
		agxSensor::CameraColorOutputParameters*)
	{
		FCameraBackendBarrier::GetInstance().FindCamera(GetCameraNativeAddress(Camera));
		UE_LOG(LogTemp, Warning, TEXT("CameraBackendBarrier_helpers::SetCameraColorOutput"));
	}

	void SetCameraColorOutputAddress(
		agxSensor::Camera* Camera, agxSensor::CameraColorOutput*, void*)
	{
		FCameraBackendBarrier::GetInstance().FindCamera(GetCameraNativeAddress(Camera));
		UE_LOG(LogTemp, Warning, TEXT("CameraBackendBarrier_helpers::SetCameraColorOutputAddress"));
	}

	void CaptureCameraColorOutput(agxSensor::Camera* Camera, agxSensor::CameraColorOutput*)
	{
		FCameraBackendBarrier::GetInstance().FindCamera(GetCameraNativeAddress(Camera));
		UE_LOG(LogTemp, Warning, TEXT("CameraBackendBarrier_helpers::CaptureCameraColorOutput"));
	}

	bool HasCameraColorOutputUnreadData(
		agxSensor::Camera* Camera, agxSensor::CameraColorOutput*, bool)
	{
		FCameraBackendBarrier::GetInstance().FindCamera(GetCameraNativeAddress(Camera));
		UE_LOG(
			LogTemp, Warning, TEXT("CameraBackendBarrier_helpers::HasCameraColorOutputUnreadData"));
		return false;
	}
}

using namespace CameraBackendBarrier_helpers;

FCameraBackendBarrier& FCameraBackendBarrier::GetInstance()
{
	static FCameraBackendBarrier Instance;
	return Instance;
}

FCameraBackendBarrier::FCameraBackendBarrier()
{
	AllocateNative();
}

FCameraBackendBarrier::~FCameraBackendBarrier()
{
	ReleaseNative();
}

bool FCameraBackendBarrier::HasNative() const
{
	return NativeRef != nullptr;
}

void FCameraBackendBarrier::AllocateNative()
{
	check(!HasNative());
	NativeRef = std::make_shared<FCameraBackendRef>();
	NativeRef->Native.synchronizeGraphics = SynchronizeGraphics;
	NativeRef->Native.setCameraLensSingleElement = SetCameraLensSingleElement;
	NativeRef->Native.setCameraCMOSSensor = SetCameraCMOSSensor;
	NativeRef->Native.setCameraLensDistortionNone = SetCameraLensDistortionNone;
	NativeRef->Native.setCameraLensDistortionBrownConrady = SetCameraLensDistortionBrownConrady;
	NativeRef->Native.setCameraColorOutput = SetCameraColorOutput;
	NativeRef->Native.setCameraColorOutputAddress = SetCameraColorOutputAddress;
	NativeRef->Native.captureCameraColorOutput = CaptureCameraColorOutput;
	NativeRef->Native.hasCameraColorOutputUnreadData = HasCameraColorOutputUnreadData;
}

FCameraBackendRef* FCameraBackendBarrier::GetNative()
{
	check(HasNative());
	return NativeRef.get();
}

const FCameraBackendRef* FCameraBackendBarrier::GetNative() const
{
	check(HasNative());
	return NativeRef.get();
}

void FCameraBackendBarrier::Add(FCameraBarrier& Camera)
{
	check(Camera.HasNative());

	CameraBarriers.Add(GetCameraNativeAddress(Camera), &Camera);
}

bool FCameraBackendBarrier::Remove(FCameraBarrier& Camera)
{
	if (!Camera.HasNative())
		return false;

	return CameraBarriers.Remove(GetCameraNativeAddress(Camera)) > 0;
}

void FCameraBackendBarrier::ClearCameras()
{
	CameraBarriers.Empty();
}

void FCameraBackendBarrier::ReleaseNative()
{
	CameraBarriers.Empty();

	if (HasNative())
		NativeRef.reset();
}

FCameraBarrier* FCameraBackendBarrier::FindCamera(uint64 NativeCameraAddress)
{
	return const_cast<FCameraBarrier*>(
		static_cast<const FCameraBackendBarrier*>(this)->FindCamera(NativeCameraAddress));
}

const FCameraBarrier* FCameraBackendBarrier::FindCamera(uint64 NativeCameraAddress) const
{
	if (NativeCameraAddress == 0)
		return nullptr;

	if (FCameraBarrier* const* CameraBarrier = CameraBarriers.Find(NativeCameraAddress))
		return *CameraBarrier;

	return nullptr;
}
