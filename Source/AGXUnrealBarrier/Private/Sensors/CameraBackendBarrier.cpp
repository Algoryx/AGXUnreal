// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/CameraBackendBarrier.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/SensorRef.h"

// Standard library includes.
#include <utility>

namespace CameraBackendBarrier_helpers
{
	void Synchronize(agxSensor::Camera*, agx::Real)
	{
		UE_LOG(LogTemp, Log, TEXT("CameraBackendBarrier_helpers::Synchronize"));
	}

	void Execute(agxSensor::Camera*, agx::Real)
	{
		UE_LOG(LogTemp, Log, TEXT("CameraBackendBarrier_helpers::Execute"));
	}

	void Complete(agxSensor::Camera*)
	{
		UE_LOG(LogTemp, Log, TEXT("CameraBackendBarrier_helpers::Complete"));
	}

	void Result(agxSensor::Camera*, agx::Real)
	{
		UE_LOG(LogTemp, Log, TEXT("CameraBackendBarrier_helpers::Result"));
	}

	void SynchronizeGraphics(agxSensor::Camera*, agxSensor::Matrix4x4*)
	{
		UE_LOG(LogTemp, Log, TEXT("CameraBackendBarrier_helpers::SynchronizeGraphics"));
	}

	void Cleanup(agxSensor::Camera*)
	{
		UE_LOG(LogTemp, Log, TEXT("CameraBackendBarrier_helpers::Cleanup"));
	}

	void SetCameraLensSingleElement(
		agxSensor::Camera*, agxSensor::CameraLensSingleElement*,
		agxSensor::CameraLensSingleElementParameters*)
	{
		UE_LOG(LogTemp, Log, TEXT("CameraBackendBarrier_helpers::SetCameraLensSingleElement"));
	}

	void SetCameraCMOSSensor(
		agxSensor::Camera*, agxSensor::CameraCMOSSensor*, agxSensor::CameraCMOSSensorParameters*)
	{
		UE_LOG(LogTemp, Log, TEXT("CameraBackendBarrier_helpers::SetCameraCMOSSensor"));
	}

	void SetCameraLensDistortionNone(agxSensor::Camera*, agxSensor::CameraLens*)
	{
		UE_LOG(LogTemp, Log, TEXT("CameraBackendBarrier_helpers::SetCameraLensDistortionNone"));
	}

	void SetCameraLensDistortionBrownConrady(
		agxSensor::Camera*, agxSensor::CameraLens*,
		agxSensor::LensDistortionBrownConradyCoefficients*)
	{
		UE_LOG(
			LogTemp, Log,
			TEXT("CameraBackendBarrier_helpers::SetCameraLensDistortionBrownConrady"));
	}

	void SetCameraColorOutput(
		agxSensor::Camera*, agxSensor::CameraColorOutput*, agxSensor::CameraColorOutputParameters*)
	{
		UE_LOG(LogTemp, Log, TEXT("CameraBackendBarrier_helpers::SetCameraColorOutput"));
	}

	void SetCameraColorOutputAddress(agxSensor::Camera*, agxSensor::CameraColorOutput*, void*)
	{
		UE_LOG(LogTemp, Log, TEXT("CameraBackendBarrier_helpers::SetCameraColorOutputAddress"));
	}

	void CaptureCameraColorOutput(agxSensor::Camera*, agxSensor::CameraColorOutput*)
	{
		UE_LOG(LogTemp, Log, TEXT("CameraBackendBarrier_helpers::CaptureCameraColorOutput"));
	}

	bool HasCameraColorOutputUnreadData(agxSensor::Camera*, agxSensor::CameraColorOutput*, bool)
	{
		UE_LOG(LogTemp, Log, TEXT("CameraBackendBarrier_helpers::HasCameraColorOutputUnreadData"));
		return false;
	}
}

using namespace CameraBackendBarrier_helpers;

FCameraBackendBarrier::FCameraBackendBarrier()
{
}

FCameraBackendBarrier::FCameraBackendBarrier(std::shared_ptr<FCameraBackendRef> Native)
	: NativeRef(std::move(Native))
{
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
	NativeRef->Native.synchronize = Synchronize;
	NativeRef->Native.execute = Execute;
	NativeRef->Native.complete = Complete;
	NativeRef->Native.result = Result;
	NativeRef->Native.synchronizeGraphics = SynchronizeGraphics;
	NativeRef->Native.cleanup = Cleanup;
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

void FCameraBackendBarrier::ReleaseNative()
{
	if (HasNative())
		NativeRef.reset();
}
