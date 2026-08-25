// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/CameraBackendBarrier.h"

// AGX Dynamics for Unreal includes.
#include "BarrierOnly/AGXTypeConversions.h"
#include "Sensors/CameraBarrier.h"
#include "Sensors/CameraBackendParameters.h"
#include "Sensors/CameraOutputBarrier.h"
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

	uint64 GetOutputNativeAddress(const void* NativeOutput)
	{
		return reinterpret_cast<uint64>(NativeOutput);
	}

	uint64 GetOutputNativeAddress(FCameraOutputBarrier& Output)
	{
		check(Output.HasNative());

		const agxSensor::ICameraOutput* NativeOutput = Output.GetNative()->Native.get();
		check(NativeOutput != nullptr);
		return GetOutputNativeAddress(NativeOutput);
	}

	FCameraLensSingleElementParameters Convert(
		const agxSensor::CameraLensSingleElementParameters& Parameters)
	{
		FCameraLensSingleElementParameters Result;
		Result.focalLength = ConvertDistanceToUnreal<double>(Parameters.focalLength);
		Result.fStop = Parameters.fStop;
		Result.autofocus = Parameters.autofocus;
		if (Result.autofocus)
		{
			Result.focus.minimumDistance =
				ConvertDistanceToUnreal<double>(Parameters.focus.minimumDistance);
		}
		else
		{
			Result.focus.distance = ConvertDistanceToUnreal<double>(Parameters.focus.distance);
		}
		return Result;
	}

	void Synchronize(agxSensor::Camera* Camera, agx::Real dt)
	{
		auto& Backend = FCameraBackendBarrier::GetInstance();
		if (FCameraBarrier* CameraBarrier = Backend.FindCamera(GetCameraNativeAddress(Camera)))
		{
			if (TArray<FAGX_CameraCaptureState>* CaptureStates =
					Backend.FindCaptureStates(CameraBarrier))
			{
				CameraBarrier->OnBackendSynchronize(*CaptureStates, dt);
			}
		}
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
		check(Parameters != nullptr);

		if (FCameraBarrier* CameraBarrier =
				FCameraBackendBarrier::GetInstance().FindCamera(GetCameraNativeAddress(Camera)))
		{
			CameraBarrier->OnBackendSetCameraLensSingleElement(Convert(*Parameters));
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
	NativeRef->Native.synchronize = Synchronize;
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

void FCameraBackendBarrier::RegisterCamera(FCameraBarrier& Camera)
{
	check(Camera.HasNative());
	CameraBarriers.Add(GetCameraNativeAddress(Camera), &Camera);
}

void FCameraBackendBarrier::UnregisterCamera(FCameraBarrier& Camera)
{
	if (TArray<FAGX_CameraCaptureState>* CameraCaptureStates = CaptureStates.Find(&Camera))
	{
		FScopeLock Lock(&OutputRawDataMutex);
		for (const FAGX_CameraCaptureState& CaptureState : *CameraCaptureStates)
		{
			OutputRawData.Remove(CaptureState.OutputAddr);
		}
	}

	CaptureStates.Remove(&Camera);

	if (Camera.HasNative())
		CameraBarriers.Remove(GetCameraNativeAddress(Camera));
}

void FCameraBackendBarrier::RegisterOutput(FCameraBarrier& Camera, FCameraOutputBarrier& Output)
{
	check(Camera.HasNative());
	check(Output.HasNative());

	const uint64 OutputAddr = GetOutputNativeAddress(Output);
	TArray<FAGX_CameraCaptureState>& CameraCaptureStates = CaptureStates.FindOrAdd(&Camera);
	FAGX_CameraCaptureState* ExistingCaptureState = CameraCaptureStates.FindByPredicate(
		[OutputAddr](const FAGX_CameraCaptureState& CaptureState)
		{
			return CaptureState.OutputAddr == OutputAddr;
		});

	if (ExistingCaptureState == nullptr)
	{
		FAGX_CameraCaptureState& NewCaptureState = CameraCaptureStates.AddDefaulted_GetRef();
		NewCaptureState.OutputAddr = OutputAddr;
	}

	{
		FScopeLock Lock(&OutputRawDataMutex);
		OutputRawData.FindOrAdd(OutputAddr);
	}
}

void FCameraBackendBarrier::UnregisterOutput(FCameraBarrier& Camera, FCameraOutputBarrier& Output)
{
	if (!Output.HasNative())
		return;

	const uint64 OutputAddr = GetOutputNativeAddress(Output);
	if (TArray<FAGX_CameraCaptureState>* CameraCaptureStates = CaptureStates.Find(&Camera))
	{
		CameraCaptureStates->RemoveAll(
			[OutputAddr](const FAGX_CameraCaptureState& CaptureState)
			{
				return CaptureState.OutputAddr == OutputAddr;
			});

		if (CameraCaptureStates->IsEmpty())
			CaptureStates.Remove(&Camera);
	}

	{
		FScopeLock Lock(&OutputRawDataMutex);
		OutputRawData.Remove(OutputAddr);
	}
}

void FCameraBackendBarrier::Clear()
{
	CameraBarriers.Empty();
	CaptureStates.Empty();
	{
		FScopeLock Lock(&OutputRawDataMutex);
		OutputRawData.Empty();
	}
}

int32 FCameraBackendBarrier::GetNumCameras() const
{
	return CameraBarriers.Num();
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

TArray<FAGX_CameraCaptureState>* FCameraBackendBarrier::FindCaptureStates(FCameraBarrier* Camera)
{
	if (Camera == nullptr)
		return nullptr;

	return CaptureStates.Find(Camera);
}

const TArray<FAGX_CameraCaptureState>* FCameraBackendBarrier::FindCaptureStates(
	FCameraBarrier* Camera) const
{
	if (Camera == nullptr)
		return nullptr;

	return CaptureStates.Find(Camera);
}
