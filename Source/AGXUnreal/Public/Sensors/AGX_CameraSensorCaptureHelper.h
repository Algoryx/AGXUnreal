// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "RHIResources.h"

// Standard library includes.
#include <array>

#include "AGX_CameraSensorCaptureHelper.generated.h"

enum class EAGX_CameraSensorSlotState : int32
{
	Free,
	CaptureRequested,
	AwaitingCopyFence,
	PollCopyFence
};

USTRUCT()
struct AGXUNREAL_API FAGX_CameraSensorCaptureData
{
	GENERATED_BODY()

	FAGX_CameraSensorCaptureData() = default;
	FAGX_CameraSensorCaptureData(const FAGX_CameraSensorCaptureData& Other);
	FAGX_CameraSensorCaptureData& operator=(const FAGX_CameraSensorCaptureData& Other);
	~FAGX_CameraSensorCaptureData();

	EAGX_CameraSensorSlotState GetState() const;
	void SetState(EAGX_CameraSensorSlotState NewState);

	FTextureRHIRef StagingTexture;
	FGPUFenceRHIRef CopyFence;
	TAtomic<int32> State {static_cast<int32>(EAGX_CameraSensorSlotState::Free)};
	uint64 OutputNativeAddress {0};
};

using FAGX_CameraSensorCaptureDataPtr =
	TSharedPtr<FAGX_CameraSensorCaptureData, ESPMode::ThreadSafe>;

USTRUCT()
struct AGXUNREAL_API FAGX_CameraSensorCaptureHelper
{
	GENERATED_BODY()

	FAGX_CameraSensorCaptureHelper();
	FAGX_CameraSensorCaptureHelper(const FAGX_CameraSensorCaptureHelper& Other);
	FAGX_CameraSensorCaptureHelper& operator=(const FAGX_CameraSensorCaptureHelper& Other);

	/// Returns nullptr if no free slots exist.
	FAGX_CameraSensorCaptureDataPtr GetFreeSlot();

	/// Returns nullptr if no AwaitingCopyFence slots exist.
	TArray<FAGX_CameraSensorCaptureDataPtr> GetAwaitingCopyFenceSlots();

private:
	std::array<FAGX_CameraSensorCaptureDataPtr, 5> CaptureData;
};
