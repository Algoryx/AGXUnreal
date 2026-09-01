// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/CameraOutputColorBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "BarrierOnly/AGXTypeConversions.h"
#include "Sensors/CameraBackendBarrier.h"
#include "Sensors/SensorRef.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agx/Matrix4x4.h>
#include <agxSensor/CameraColorOutput.h>
#include "EndAGXIncludes.h"

namespace CameraOutputColorBarrier_helpers
{
	agx::Matrix4x4 ConvertToAGX(const FAGX_ColorMappingMatrix& Matrix)
	{
		return agx::Matrix4x4(
			Matrix.Row0.R, Matrix.Row0.G, Matrix.Row0.B, Matrix.Row0.A, Matrix.Row1.R,
			Matrix.Row1.G, Matrix.Row1.B, Matrix.Row1.A, Matrix.Row2.R, Matrix.Row2.G,
			Matrix.Row2.B, Matrix.Row2.A, Matrix.Row3.R, Matrix.Row3.G, Matrix.Row3.B,
			Matrix.Row3.A);
	}

	FLinearColor GetRow(const agx::Matrix4x4& Matrix, int32 RowIndex)
	{
		return FLinearColor(
			static_cast<float>(Matrix(RowIndex, 0)), static_cast<float>(Matrix(RowIndex, 1)),
			static_cast<float>(Matrix(RowIndex, 2)), static_cast<float>(Matrix(RowIndex, 3)));
	}

	FAGX_ColorMappingMatrix ConvertToUnreal(const agx::Matrix4x4& Matrix)
	{
		FAGX_ColorMappingMatrix Result;
		Result.Row0 = GetRow(Matrix, 0);
		Result.Row1 = GetRow(Matrix, 1);
		Result.Row2 = GetRow(Matrix, 2);
		Result.Row3 = GetRow(Matrix, 3);
		return Result;
	}

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

FCameraOutputColorBarrier FCameraOutputColorBarrier::CreateFrom(FCameraOutputBarrier& Output)
{
	check(IsColorOutput(Output));
	std::shared_ptr<FCameraOutputRef> NativeRef(Output.GetNative(), [](FCameraOutputRef*) {});
	return FCameraOutputColorBarrier(NativeRef);
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

void FCameraOutputColorBarrier::SetColorMappingMatrix(
	const FAGX_ColorMappingMatrix& InColorMappingMatrix)
{
	using namespace CameraOutputColorBarrier_helpers;
	check(HasNative());
	GetCameraColorOutputNative(*this)->setColorMappingMatrix(ConvertToAGX(InColorMappingMatrix));
}

FAGX_ColorMappingMatrix FCameraOutputColorBarrier::GetColorMappingMatrix() const
{
	using namespace CameraOutputColorBarrier_helpers;
	check(HasNative());
	return ConvertToUnreal(GetCameraColorOutputNative(*this)->getColorMappingMatrix());
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
