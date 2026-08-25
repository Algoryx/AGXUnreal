// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "HAL/CriticalSection.h"

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

struct AGXUNREALBARRIER_API FAGX_CameraOutputRawData
{
	TArray<uint8> RawData;
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

private:
	FCameraBackendBarrier();
	~FCameraBackendBarrier();

	void AllocateNative();
	void ReleaseNative();

	// Key is the address of the native AGX Camera.
	TMap<uint64, FCameraBarrier*> CameraBarriers;

	// One Camera may have serveral Outputs, ergo TArray value.
	TMap<FCameraBarrier*, TArray<FAGX_CameraCaptureState>> CaptureStates;

	// Key is the address of the native AGX Output. Will be accessed from both main thread and
	// render thread.
	TMap<uint64, FAGX_CameraOutputRawData> OutputRawData;
	FCriticalSection OutputRawDataMutex;

	std::shared_ptr<FCameraBackendRef> NativeRef;
};
