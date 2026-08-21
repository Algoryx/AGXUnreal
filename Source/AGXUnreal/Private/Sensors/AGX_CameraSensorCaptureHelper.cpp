// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/AGX_CameraSensorCaptureHelper.h"

FAGX_CameraSensorCaptureData::FAGX_CameraSensorCaptureData(
	const FAGX_CameraSensorCaptureData& Other)
{
	(void) Other;
	// Unreal requires this to exist, but runtime RHI resources and slot states are not copied.
}

FAGX_CameraSensorCaptureData& FAGX_CameraSensorCaptureData::operator=(
	const FAGX_CameraSensorCaptureData& Other)
{
	(void) Other;
	// Unreal requires this to exist, but runtime RHI resources and slot states are not copied.
	StagingTexture.SafeRelease();
	CopyFence.SafeRelease();
	SetState(EAGX_CameraSensorSlotState::Free);
	return *this;
}

FAGX_CameraSensorCaptureHelper::FAGX_CameraSensorCaptureHelper(
	const FAGX_CameraSensorCaptureHelper& Other)
{
	(void) Other;
	// Unreal requires this to exist, but runtime RHI resources and slot states are not copied.
}

FAGX_CameraSensorCaptureHelper& FAGX_CameraSensorCaptureHelper::operator=(
	const FAGX_CameraSensorCaptureHelper& Other)
{
	(void) Other;
	// Unreal requires this to exist, but runtime RHI resources and slot states are not copied.
	for (FAGX_CameraSensorCaptureData& Data : CaptureData)
	{
		Data = FAGX_CameraSensorCaptureData();
	}

	return *this;
}

EAGX_CameraSensorSlotState FAGX_CameraSensorCaptureData::GetState() const
{
	return static_cast<EAGX_CameraSensorSlotState>(static_cast<int32>(State));
}

void FAGX_CameraSensorCaptureData::SetState(EAGX_CameraSensorSlotState NewState)
{
	State = static_cast<int32>(NewState);
}

FAGX_CameraSensorCaptureData* FAGX_CameraSensorCaptureHelper::GetFreeSlot()
{
	for (auto& Data : CaptureData)
	{
		if (Data.GetState() == EAGX_CameraSensorSlotState::Free)
			return &Data;
	}

	return nullptr;
}

TArray<FAGX_CameraSensorCaptureData*> FAGX_CameraSensorCaptureHelper::GetAwaitingCopyFenceSlots()
{
	TArray<FAGX_CameraSensorCaptureData*> Slots;
	for (auto& Data : CaptureData)
	{
		if (Data.GetState() == EAGX_CameraSensorSlotState::AwaitingCopyFence)
			Slots.Add(&Data);
	}

	return Slots;
}
