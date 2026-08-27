// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/CameraOutputColorBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "BarrierOnly/AGXTypeConversions.h"
#include "Sensors/CameraBackendBarrier.h"
#include "Sensors/SensorRef.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxSensor/CameraColorOutput.h>
#include "EndAGXIncludes.h"

namespace CameraOutputColorBarrier_helpers
{
	agxSensor::CameraColorOutput* GetCameraColorOutputNative(FCameraOutputColorBarrier& Output)
	{
		AGX_CHECK(Output.HasNative());
		return Output.GetNative()->Native->asSafe<agxSensor::CameraColorOutput>();
	}

	const agxSensor::CameraColorOutput* GetCameraColorOutputNative(
		const FCameraOutputColorBarrier& Output)
	{
		AGX_CHECK(Output.HasNative());
		return Output.GetNative()->Native->asSafe<agxSensor::CameraColorOutput>();
	}
}

FCameraOutputColorBarrier::FCameraOutputColorBarrier(std::shared_ptr<FCameraOutputRef> Native)
	: FCameraOutputBarrier(std::move(Native))
{
}

void FCameraOutputColorBarrier::AllocateNative()
{
	check(!HasNative());
	NativeRef->Native = new agxSensor::CameraColorOutput();
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

void FCameraOutputColorBarrier::SetChannelType(EAGX_CameraOutputChannelType InChannelType)
{
	check(HasNative());
	CameraOutputColorBarrier_helpers::GetCameraColorOutputNative(*this)->setChannelType(
		Convert(InChannelType));
}

EAGX_CameraOutputChannelType FCameraOutputColorBarrier::GetChannelType() const
{
	check(HasNative());
	return Convert(
		CameraOutputColorBarrier_helpers::GetCameraColorOutputNative(*this)->getChannelType());
}

void FCameraOutputColorBarrier::SetGamma(double InGamma)
{
	check(HasNative());
	CameraOutputColorBarrier_helpers::GetCameraColorOutputNative(*this)->setGamma(InGamma);
}

double FCameraOutputColorBarrier::GetGamma() const
{
	check(HasNative());
	return CameraOutputColorBarrier_helpers::GetCameraColorOutputNative(*this)->getGamma();
}

void FCameraOutputColorBarrier::SetChannelCount(uint8 InChannelCount)
{
	check(HasNative());
	CameraOutputColorBarrier_helpers::GetCameraColorOutputNative(*this)->setChannelCount(
		InChannelCount);
}

uint8 FCameraOutputColorBarrier::GetChannelCount() const
{
	check(HasNative());
	const agxSensor::CameraColorOutput* Output =
		CameraOutputColorBarrier_helpers::GetCameraColorOutputNative(*this);
	return static_cast<uint8>(Output->getChannelCount());
}
