// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/CameraOutputColorBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "Sensors/CameraBackendBarrier.h"
#include "Sensors/SensorRef.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxSensor/CameraColorOutput.h>
#include "EndAGXIncludes.h"

FCameraOutputColorBarrier::FCameraOutputColorBarrier(std::shared_ptr<FCameraOutputRef> Native)
	: FCameraOutputBarrier(std::move(Native))
{
}

void FCameraOutputColorBarrier::AllocateNative()
{
	check(!HasNative());
	NativeRef->Native = new agxSensor::CameraColorOutput();

	 // TODO: this should come from the Output struct.
	NativeRef->Native->asSafe<agxSensor::CameraColorOutput>()->setConstantCapture(10);
}

void FCameraOutputColorBarrier::GetData(TArray<FColor>& OutData) const
{
	check(HasNative());
	AGX_CHECK(sizeof(FColor) == GetNative()->Native->getElementSize());

	agxSensor::ICameraOutput* NativeOutput = GetNative()->Native.get();
	const uint64 NativeOutputAddress = reinterpret_cast<uint64>(NativeOutput);
	FCameraBackendBarrier::GetInstance().StageUnreadDataIfExists(NativeOutputAddress);

	if (!NativeOutput->hasUnreadData(/*markAsRead*/ false))
	{
		OutData.SetNumUninitialized(0, EAllowShrinking::No);
		return;
	}

	agxSensor::BinaryOutputView<FColor> ViewAGX = NativeOutput->view<FColor>();

	OutData.SetNumUninitialized(ViewAGX.size(), EAllowShrinking::No);
	FMemory::Memcpy(OutData.GetData(), ViewAGX.begin(), ViewAGX.size() * sizeof(FColor));
}

bool FCameraOutputColorBarrier::IsColorOutput(const FCameraOutputBarrier& Output)
{
	if (!Output.HasNative())
		return false;

	return Output.GetNative()->Native->is<agxSensor::CameraColorOutput>();
}
