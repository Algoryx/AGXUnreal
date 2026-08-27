// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "HAL/CriticalSection.h"
#include "PixelFormat.h"

// Standard library includes.
#include <memory>

struct FCameraBarrier;
struct FCameraBackendRef;
struct FCameraOutputBarrier;

struct AGXUNREALBARRIER_API FAGX_CameraCaptureState
{
	uint64 OutputAddr {0}; // Address to the native agxSensor::Output.
	double AccumulatedTime {0.0};
	double LastCaptureTime {0.0};
};

struct AGXUNREALBARRIER_API FAGX_CameraOutputState
{
	// Writable AGX-owned output buffer received from the camera backend address callback.
	void* NativeBuffer {nullptr};

	// Corresponds to the user-facing Output->hasUnreadData().
	bool IsUnread {false};
};

// Raw data normally written from the RenderThread when a Camera readout has been done.
// This acts as a staging memory before being passed to the Native Output buffer when the user reads
// the data.
struct AGXUNREALBARRIER_API FAGX_CameraOutputRawData
{
	TArray<uint8> RawData;
	FIntPoint Resolution {0, 0};
	EPixelFormat PixelFormat {PF_Unknown};
	bool IsUnread {false};
};

struct AGXUNREALBARRIER_API FCameraOutputRawDataWriteAccess
{
	FCameraOutputRawDataWriteAccess(
		FCriticalSection& InMutex, TMap<uint64, FAGX_CameraOutputRawData>& InOutputRawData,
		uint64 OutputAddr);
	~FCameraOutputRawDataWriteAccess();

	FCameraOutputRawDataWriteAccess(const FCameraOutputRawDataWriteAccess&) = delete;
	FCameraOutputRawDataWriteAccess& operator=(const FCameraOutputRawDataWriteAccess&) = delete;

	FCameraOutputRawDataWriteAccess(FCameraOutputRawDataWriteAccess&& Other) noexcept;
	FCameraOutputRawDataWriteAccess& operator=(FCameraOutputRawDataWriteAccess&& Other) noexcept;

	FAGX_CameraOutputRawData* Get();
	const FAGX_CameraOutputRawData* Get() const;

	FAGX_CameraOutputRawData* operator->();
	const FAGX_CameraOutputRawData* operator->() const;

private:
	void Release();

	FCriticalSection* Mutex {nullptr};
	FAGX_CameraOutputRawData* Data {nullptr};
};

struct AGXUNREALBARRIER_API FCameraBackendBarrier
{
	static FCameraBackendBarrier& GetInstance();

	FCameraBackendBarrier(const FCameraBackendBarrier&) = delete;
	FCameraBackendBarrier& operator=(const FCameraBackendBarrier&) = delete;
	FCameraBackendBarrier(FCameraBackendBarrier&&) = delete;
	FCameraBackendBarrier& operator=(FCameraBackendBarrier&&) = delete;

	bool HasNative() const;
	FCameraBackendRef* GetNative();
	const FCameraBackendRef* GetNative() const;

	void RegisterCamera(FCameraBarrier& Camera);
	void UnregisterCamera(FCameraBarrier& Camera);

	void RegisterOutput(FCameraBarrier& Camera, FCameraOutputBarrier& Output);
	void UnregisterOutput(FCameraBarrier& Camera, FCameraOutputBarrier& Output);

	void Clear();

	int32 GetNumCameras() const;

	FCameraBarrier* FindCamera(uint64 NativeCameraAddress);
	const FCameraBarrier* FindCamera(uint64 NativeCameraAddress) const;

	TArray<FAGX_CameraCaptureState>* FindCaptureStates(FCameraBarrier* Camera);
	const TArray<FAGX_CameraCaptureState>* FindCaptureStates(FCameraBarrier* Camera) const;

	FAGX_CameraOutputState* FindOutputState(uint64 NativeOutputAddress);
	const FAGX_CameraOutputState* FindOutputState(uint64 NativeOutputAddress) const;

	/// Copies pixel data from OutputRawData to the Native Output buffer if OutputRawData is unread.
	bool StageUnreadDataIfExists(uint64 NativeOutputAddress);

	FCameraOutputRawDataWriteAccess LockOutputRawDataForWrite(uint64 OutputAddr);

private:
	FCameraBackendBarrier();
	~FCameraBackendBarrier();

	void AllocateNative();
	void ReleaseNative();

	// Key is the address of the native AGX Camera.
	TMap<uint64, FCameraBarrier*> CameraBarriers;

	// One Camera may have serveral Outputs, ergo TArray value.
	TMap<FCameraBarrier*, TArray<FAGX_CameraCaptureState>> CaptureStates;

	// Key is the address of the native AGX Output.
	TMap<uint64, FAGX_CameraOutputState> OutputStates;

	// Key is the address of the native AGX Output. Will be accessed from both main thread and
	// render thread. If performance suffers due to waiting for the mutex when many outputs/cameras
	// are used, we might redesign this to have a mutex for each output, but then we should not
	// store the data in a common TMap.
	TMap<uint64, FAGX_CameraOutputRawData> OutputRawData;
	FCriticalSection OutputRawDataMutex;

	std::shared_ptr<FCameraBackendRef> NativeRef;
};
