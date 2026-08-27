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
#include <agxSensor/CameraOutput.h>
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
		UE_LOG(LogTemp, Warning, TEXT("CameraBackendBarrier_helpers::SetCameraCMOSSensor"));
	}

	void SetCameraLensDistortionNone(agxSensor::Camera* Camera, agxSensor::CameraLens*)
	{
		UE_LOG(LogTemp, Warning, TEXT("CameraBackendBarrier_helpers::SetCameraLensDistortionNone"));
	}

	void SetCameraLensDistortionBrownConrady(
		agxSensor::Camera* Camera, agxSensor::CameraLens*,
		agxSensor::LensDistortionBrownConradyCoefficients*)
	{
		UE_LOG(
			LogTemp, Warning,
			TEXT("CameraBackendBarrier_helpers::SetCameraLensDistortionBrownConrady"));
	}

	void SetCameraColorOutput(
		agxSensor::Camera* Camera, agxSensor::CameraColorOutput*,
		agxSensor::CameraColorOutputParameters*)
	{
		UE_LOG(LogTemp, Warning, TEXT("CameraBackendBarrier_helpers::SetCameraColorOutput"));
	}

	void SetCameraColorOutputAddress(
		agxSensor::Camera* Camera, agxSensor::CameraColorOutput* Output, void* OutputAddress)
	{
		FCameraBackendBarrier& Backend = FCameraBackendBarrier::GetInstance();
		if (FAGX_CameraOutputState* OutputState =
				Backend.FindOutputState(GetOutputNativeAddress(Output)))
		{
			OutputState->NativeBuffer = OutputAddress;
		}
	}

	void CaptureCameraColorOutput(agxSensor::Camera* Camera, agxSensor::CameraColorOutput* Output)
	{
		auto& Backend = FCameraBackendBarrier::GetInstance();
		if (FCameraBarrier* CameraBarrier = Backend.FindCamera(GetCameraNativeAddress(Camera)))
		{
			const auto& CaptureStates = Backend.FindCaptureStates(CameraBarrier);
			if (CaptureStates == nullptr)
				return;

			CameraBarrier->OnBackendRequestCapture(GetOutputNativeAddress(Output));
		}
	}

	bool HasCameraColorOutputUnreadData(
		agxSensor::Camera* Camera, agxSensor::CameraColorOutput* Output, bool bMarkAsRead)
	{
		FCameraBackendBarrier& Backend = FCameraBackendBarrier::GetInstance();
		FAGX_CameraOutputState* OutputState = Backend.FindOutputState(GetOutputNativeAddress(Output));
		if (OutputState == nullptr)
			return false;

		const bool bHasUnreadData = OutputState->IsUnread;
		if (bMarkAsRead)
			OutputState->IsUnread = false;

		return bHasUnreadData;
	}
}

using namespace CameraBackendBarrier_helpers;

namespace CameraBackendBarrier_internal_helpers
{
	using FCameraOutputRawDataSlotPtr =
		TSharedPtr<FAGX_CameraOutputRawDataSlot, ESPMode::ThreadSafe>;
	using FCameraOutputRawDataSlotMap = TMap<uint64, FCameraOutputRawDataSlotPtr>;

	FCameraOutputRawDataSlotPtr FindOutputRawDataSlot(
		FCriticalSection& Mutex, FCameraOutputRawDataSlotMap& OutputRawData, uint64 OutputAddr)
	{
		FScopeLock Lock(&Mutex);
		return OutputRawData.FindRef(OutputAddr);
	}

	FCameraOutputRawDataSlotPtr FindOrAddOutputRawDataSlot(
		FCriticalSection& Mutex, FCameraOutputRawDataSlotMap& OutputRawData, uint64 OutputAddr)
	{
		FScopeLock Lock(&Mutex);
		FCameraOutputRawDataSlotPtr& Slot = OutputRawData.FindOrAdd(OutputAddr);
		if (!Slot.IsValid())
			Slot = MakeShared<FAGX_CameraOutputRawDataSlot, ESPMode::ThreadSafe>();

		return Slot;
	}
}

FCameraOutputRawDataWriteAccess::FCameraOutputRawDataWriteAccess(
	TSharedPtr<FAGX_CameraOutputRawDataSlot, ESPMode::ThreadSafe> InSlot)
	: Slot(MoveTemp(InSlot))
{
	if (Slot.IsValid())
		Slot->Mutex.Lock();
}

FCameraOutputRawDataWriteAccess::~FCameraOutputRawDataWriteAccess()
{
	Release();
}

FCameraOutputRawDataWriteAccess::FCameraOutputRawDataWriteAccess(
	FCameraOutputRawDataWriteAccess&& Other) noexcept
	: Slot(MoveTemp(Other.Slot))
{
}

FCameraOutputRawDataWriteAccess& FCameraOutputRawDataWriteAccess::operator=(
	FCameraOutputRawDataWriteAccess&& Other) noexcept
{
	if (this == &Other)
		return *this;

	Release();
	Slot = MoveTemp(Other.Slot);
	return *this;
}

FAGX_CameraOutputRawData* FCameraOutputRawDataWriteAccess::Get()
{
	return Slot.IsValid() ? &Slot->RawData : nullptr;
}

const FAGX_CameraOutputRawData* FCameraOutputRawDataWriteAccess::Get() const
{
	return Slot.IsValid() ? &Slot->RawData : nullptr;
}

FAGX_CameraOutputRawData* FCameraOutputRawDataWriteAccess::operator->()
{
	return Get();
}

const FAGX_CameraOutputRawData* FCameraOutputRawDataWriteAccess::operator->() const
{
	return Get();
}

void FCameraOutputRawDataWriteAccess::Release()
{
	if (Slot.IsValid())
	{
		Slot->Mutex.Unlock();
		Slot.Reset();
	}
}

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
		for (const FAGX_CameraCaptureState& CaptureState : *CameraCaptureStates)
			OutputStates.Remove(CaptureState.OutputAddr);

		FScopeLock Lock(&OutputRawDataMutex);
		for (const FAGX_CameraCaptureState& CaptureState : *CameraCaptureStates)
			OutputRawData.Remove(CaptureState.OutputAddr);
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
		{ return CaptureState.OutputAddr == OutputAddr; });

	if (ExistingCaptureState == nullptr)
	{
		FAGX_CameraCaptureState& NewCaptureState = CameraCaptureStates.AddDefaulted_GetRef();
		NewCaptureState.OutputAddr = OutputAddr;
	}

	OutputStates.FindOrAdd(OutputAddr).IsUnread = false;

	using namespace CameraBackendBarrier_internal_helpers;
	FCameraOutputRawDataSlotPtr RawDataSlot =
		FindOrAddOutputRawDataSlot(OutputRawDataMutex, OutputRawData, OutputAddr);
	FScopeLock Lock(&RawDataSlot->Mutex);
	RawDataSlot->RawData.IsUnread = false;
}

void FCameraBackendBarrier::UnregisterOutput(FCameraBarrier& Camera, FCameraOutputBarrier& Output)
{
	if (!Output.HasNative())
		return;

	const uint64 OutputAddr = GetOutputNativeAddress(Output);
	if (TArray<FAGX_CameraCaptureState>* CameraCaptureStates = CaptureStates.Find(&Camera))
	{
		CameraCaptureStates->RemoveAll([OutputAddr](const FAGX_CameraCaptureState& CaptureState)
									   { return CaptureState.OutputAddr == OutputAddr; });

		if (CameraCaptureStates->IsEmpty())
			CaptureStates.Remove(&Camera);
	}

	OutputStates.Remove(OutputAddr);

	{
		FScopeLock Lock(&OutputRawDataMutex);
		OutputRawData.Remove(OutputAddr);
	}
}

void FCameraBackendBarrier::Clear()
{
	CameraBarriers.Empty();
	CaptureStates.Empty();
	OutputStates.Empty();

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

FAGX_CameraOutputState* FCameraBackendBarrier::FindOutputState(uint64 NativeOutputAddress)
{
	if (NativeOutputAddress == 0)
		return nullptr;

	return OutputStates.Find(NativeOutputAddress);
}

const FAGX_CameraOutputState* FCameraBackendBarrier::FindOutputState(
	uint64 NativeOutputAddress) const
{
	if (NativeOutputAddress == 0)
		return nullptr;

	return OutputStates.Find(NativeOutputAddress);
}

bool FCameraBackendBarrier::StageUnreadDataIfExists(uint64 NativeOutputAddress)
{
	using namespace CameraBackendBarrier_internal_helpers;

	FAGX_CameraOutputState* OutputState = FindOutputState(NativeOutputAddress);
	if (OutputState == nullptr || OutputState->NativeBuffer == nullptr)
		return false;
	void* NativeBuffer = OutputState->NativeBuffer;

	const agxSensor::ICameraOutput* Output =
		reinterpret_cast<const agxSensor::ICameraOutput*>(NativeOutputAddress);
	if (Output == nullptr)
		return false;

	const agx::Vec2i OutputResolution = Output->getResolution();
	if (OutputResolution.x() <= 0 || OutputResolution.y() <= 0 || Output->getElementSize() == 0)
		return false;

	const size_t ExpectedNumBytes =
		static_cast<size_t>(OutputResolution.x()) * static_cast<size_t>(OutputResolution.y()) *
		Output->getElementSize();

	bool bStagedData = false;
	FCameraOutputRawDataSlotPtr RawDataSlot =
		FindOutputRawDataSlot(OutputRawDataMutex, OutputRawData, NativeOutputAddress);
	if (!RawDataSlot.IsValid())
		return false;

	{
		FScopeLock Lock(&RawDataSlot->Mutex);
		FAGX_CameraOutputRawData& RawData = RawDataSlot->RawData;
		if (!RawData.IsUnread || RawData.RawData.Num() == 0)
			return false;

		if (ExpectedNumBytes != static_cast<size_t>(RawData.RawData.Num()))
		{
			UE_LOG(
				LogTemp, Warning,
				TEXT("FCameraBackendBarrier::StageUnreadDataIfExists cannot stage camera output "
					 "because the raw buffer size does not match the native AGX output buffer "
					 "size. Raw size: %d. Native size: %llu."),
				RawData.RawData.Num(), static_cast<uint64>(ExpectedNumBytes));
			return false;
		}

		FMemory::Memcpy(NativeBuffer, RawData.RawData.GetData(), RawData.RawData.Num());
		RawData.IsUnread = false;
		OutputState->IsUnread = true;
		bStagedData = true;
	}

	return bStagedData;
}

FCameraOutputRawDataWriteAccess FCameraBackendBarrier::LockOutputRawDataForWrite(
	uint64 OutputAddr)
{
	using namespace CameraBackendBarrier_internal_helpers;
	return FCameraOutputRawDataWriteAccess(
		FindOutputRawDataSlot(OutputRawDataMutex, OutputRawData, OutputAddr));
}
