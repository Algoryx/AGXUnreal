// Copyright 2026, Algoryx Simulation AB.

#include "OpenPLX/OpenPLXCameraColorOutputView.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "BarrierOnly/OpenPLX/OpenPLXRefs.h"
#include "Utilities/PLXMarshallingUtilities.h"

// OpenPLX includes.
#include "BeginAGXIncludes.h"
#include "openplx/Sensors/Signals/CameraOutputChannelType.h"
#include "EndAGXIncludes.h"

// Standard library includes.
#include <limits>
#include <string>
#include <utility>
#include <vector>

namespace OpenPLXCameraColorOutputView_helpers
{
	using namespace PLXMarshallingUtilities;

	constexpr size_t MaxChannelCount = 4;

	struct FPixelLayout
	{
		openplx::Marshalling* Marshalling = nullptr;
		size_t Stride = 0;
		size_t BufferSize = 0;
		size_t NumPixels = 0;
	};

	EAGX_CameraOutputChannelType GetChannelType(
		const openplx::Sensors::Signals::CameraColorOutput& Output)
	{
		const openplx::Sensors::Signals::CameraOutputChannelType ChannelTypes;
		if (Output.channel_type() == ChannelTypes.U8())
			return EAGX_CameraOutputChannelType::U8;
		if (Output.channel_type() == ChannelTypes.F32())
			return EAGX_CameraOutputChannelType::F32;

		return EAGX_CameraOutputChannelType::UNSUPPORTED;
	}

	bool GetChannelLayout(
		EAGX_CameraOutputChannelType ChannelType, openplx::FieldType& OutFieldType,
		size_t& OutChannelSize)
	{
		switch (ChannelType)
		{
			case EAGX_CameraOutputChannelType::U8:
				OutFieldType = openplx::FieldType::UInt;
				OutChannelSize = sizeof(uint8);
				return true;
			case EAGX_CameraOutputChannelType::F32:
				OutFieldType = openplx::FieldType::Real;
				OutChannelSize = sizeof(float);
				return true;
			case EAGX_CameraOutputChannelType::UNSUPPORTED:
				return false;
		}

		return false;
	}

	bool GetPixelLayout(openplx::Marshalling& Marshalling, FPixelLayout& OutLayout)
	{
		if (Marshalling.get_buffer_size() > 0 && Marshalling.get_buffer() == nullptr)
			return false;

		std::unique_ptr<openplx::Marshalling>& PixelMarshallingPtr =
			Marshalling.get_or_add_nested_marshalling("pixel");
		Marshalling.calculate_nested_buffer_sizes();

		openplx::Marshalling* PixelMarshalling = PixelMarshallingPtr.get();
		if (PixelMarshalling == nullptr)
			return false;

		const size_t PixelStride = PixelMarshalling->get_stride();
		const size_t PixelBufferSize = PixelMarshalling->get_buffer_size();
		if (PixelStride == 0 || PixelBufferSize % PixelStride != 0)
			return false;

		if (PixelBufferSize > 0 && PixelMarshalling->get_buffer() == nullptr)
			return false;

		OutLayout.Marshalling = PixelMarshalling;
		OutLayout.Stride = PixelStride;
		OutLayout.BufferSize = PixelBufferSize;
		OutLayout.NumPixels = PixelBufferSize / PixelStride;
		return true;
	}

	bool GetConfiguration(
		const FOpenPLXCameraColorOutputView& View,
		const openplx::Sensors::Signals::CameraColorOutput*& OutOutput,
		size_t& OutChannelCount)
	{
		OutOutput = nullptr;
		OutChannelCount = 0;
		if (!View.HasNative())
			return false;

		const FOpenPLXCameraColorOutputViewRef* NativeRef = View.GetNative();
		OutOutput = NativeRef->CameraColorOutput.get();
		const std::shared_ptr<openplx::Sensors::Signals::CameraColorOutputMatrix> Matrix =
			OutOutput->matrix();
		if (Matrix == nullptr || Matrix->outputs() < 1 || Matrix->outputs() > MaxChannelCount)
			return false;

		OutChannelCount = static_cast<size_t>(Matrix->outputs());
		return true;
	}

	bool ValidatePixelLayout(
		const FOpenPLXCameraColorOutputView& View, const FPixelLayout& PixelLayout)
	{
		const openplx::Sensors::Signals::CameraColorOutput* Output = nullptr;
		size_t ChannelCount = 0;
		if (!GetConfiguration(View, Output, ChannelCount))
			return false;

		openplx::FieldType ExpectedFieldType;
		size_t ChannelSize = 0;
		if (!GetChannelLayout(GetChannelType(*Output), ExpectedFieldType, ChannelSize))
			return false;

		if (ChannelCount > std::numeric_limits<size_t>::max() / ChannelSize ||
			PixelLayout.Stride != ChannelCount * ChannelSize)
		{
			return false;
		}

		const auto& Fields = PixelLayout.Marshalling->get_field_map();
		for (size_t ChannelIndex = 0; ChannelIndex < ChannelCount; ++ChannelIndex)
		{
			const std::string FieldName = "c_" + std::to_string(ChannelIndex + 1);
			const openplx::Field* Field = FindField(Fields, FieldName);
			if (Field == nullptr || Field->field_type != ExpectedFieldType ||
				Field->size != ChannelSize || Field->offset != ChannelIndex * ChannelSize ||
				Field->field_array_type != openplx::FieldArrayType::None)
			{
				return false;
			}
		}

		return true;
	}

	template <typename T>
	bool CopyPixelData(const FPixelLayout& PixelLayout, TArray<T>& OutData)
	{
		if (PixelLayout.BufferSize % sizeof(T) != 0)
			return false;

		const size_t NumElements = PixelLayout.BufferSize / sizeof(T);
		if (NumElements > static_cast<size_t>(TNumericLimits<int32>::Max()))
			return false;

		OutData.SetNumUninitialized(static_cast<int32>(NumElements), EAllowShrinking::No);
		if (PixelLayout.BufferSize > 0)
		{
			FMemory::Memcpy(
				OutData.GetData(), PixelLayout.Marshalling->get_buffer(), PixelLayout.BufferSize);
		}
		return true;
	}

	bool GetValidatedPixelLayout(const FOpenPLXCameraColorOutputView& View, FPixelLayout& OutLayout)
	{
		if (!View.HasNative() || !GetPixelLayout(*View.GetNative()->Marshalling, OutLayout))
			return false;

		if (ValidatePixelLayout(View, OutLayout))
			return true;

		UE_LOG(
			LogAGX, Warning,
			TEXT("OpenPLX Camera Color Output View: The pixel marshalling layout does not match "
				 "the Camera Color Output configuration."));
		return false;
	}
}

FOpenPLXCameraColorOutputView::FOpenPLXCameraColorOutputView()
	: NativeRef {new FOpenPLXCameraColorOutputViewRef}
{
}

FOpenPLXCameraColorOutputView::FOpenPLXCameraColorOutputView(
	std::shared_ptr<FOpenPLXCameraColorOutputViewRef> Native)
	: NativeRef(std::move(Native))
{
	check(NativeRef);
}

bool FOpenPLXCameraColorOutputView::HasNative() const
{
	return NativeRef != nullptr && NativeRef->Marshalling != nullptr &&
		   NativeRef->CameraColorOutput != nullptr;
}

FIntPoint FOpenPLXCameraColorOutputView::GetResolution() const
{
	if (!HasNative())
		return FIntPoint::ZeroValue;

	const std::shared_ptr<openplx::Sensors::Signals::CameraColorOutput>& Output =
		NativeRef->CameraColorOutput;
	const int64 Width = Output->horizontal_resolution();
	const int64 Height = Output->vertical_resolution();
	if (Width < 1 || Height < 1 || Width > TNumericLimits<int32>::Max() ||
		Height > TNumericLimits<int32>::Max())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("OpenPLX Camera Color Output View: Invalid camera resolution: %lldx%lld."), Width,
			Height);
		return FIntPoint::ZeroValue;
	}

	return FIntPoint(static_cast<int32>(Width), static_cast<int32>(Height));
}

int32 FOpenPLXCameraColorOutputView::GetNumPixels() const
{
	OpenPLXCameraColorOutputView_helpers::FPixelLayout PixelLayout;
	if (!OpenPLXCameraColorOutputView_helpers::GetValidatedPixelLayout(*this, PixelLayout))
		return 0;

	if (PixelLayout.NumPixels > static_cast<size_t>(TNumericLimits<int32>::Max()))
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("OpenPLX Camera Color Output View: The received image has too many pixels for an "
				 "int32."));
		return 0;
	}

	return static_cast<int32>(PixelLayout.NumPixels);
}

EAGX_CameraOutputChannelType FOpenPLXCameraColorOutputView::GetChannelType() const
{
	if (!HasNative())
		return EAGX_CameraOutputChannelType::UNSUPPORTED;

	return OpenPLXCameraColorOutputView_helpers::GetChannelType(*NativeRef->CameraColorOutput);
}

double FOpenPLXCameraColorOutputView::GetGamma() const
{
	if (!HasNative())
		return 1.0;

	return NativeRef->CameraColorOutput->gamma().value_or(1.0);
}

FAGX_ColorMappingMatrix FOpenPLXCameraColorOutputView::GetColorMappingMatrix() const
{
	FAGX_ColorMappingMatrix Result;
	if (!HasNative())
		return Result;

	const std::shared_ptr<openplx::Sensors::Signals::CameraColorOutputMatrix> Matrix =
		NativeRef->CameraColorOutput->matrix();
	if (Matrix == nullptr)
		return Result;

	const std::vector<double> Values = Matrix->matrix();
	auto GetValue = [&Values](size_t Row, size_t Column)
	{
		const size_t Index = Row * 4 + Column;
		return Index < Values.size() ? static_cast<float>(Values[Index]) : 0.0f;
	};
	Result.Row0 = FLinearColor(GetValue(0, 0), GetValue(0, 1), GetValue(0, 2), GetValue(0, 3));
	Result.Row1 = FLinearColor(GetValue(1, 0), GetValue(1, 1), GetValue(1, 2), GetValue(1, 3));
	Result.Row2 = FLinearColor(GetValue(2, 0), GetValue(2, 1), GetValue(2, 2), GetValue(2, 3));
	Result.Row3 = FLinearColor(GetValue(3, 0), GetValue(3, 1), GetValue(3, 2), GetValue(3, 3));
	return Result;
}

uint8 FOpenPLXCameraColorOutputView::GetChannelCount() const
{
	const openplx::Sensors::Signals::CameraColorOutput* Output = nullptr;
	size_t ChannelCount = 0;
	if (!OpenPLXCameraColorOutputView_helpers::GetConfiguration(*this, Output, ChannelCount))
		return 0;

	return static_cast<uint8>(ChannelCount);
}

bool FOpenPLXCameraColorOutputView::GetDataBytes(TArray<uint8>& OutData) const
{
	OutData.Reset();
	OpenPLXCameraColorOutputView_helpers::FPixelLayout PixelLayout;
	if (!OpenPLXCameraColorOutputView_helpers::GetValidatedPixelLayout(*this, PixelLayout))
		return false;

	return OpenPLXCameraColorOutputView_helpers::CopyPixelData(PixelLayout, OutData);
}

bool FOpenPLXCameraColorOutputView::CopyDataBytesTo(void* OutData, uint64 OutDataSize) const
{
	OpenPLXCameraColorOutputView_helpers::FPixelLayout PixelLayout;
	if (!OpenPLXCameraColorOutputView_helpers::GetValidatedPixelLayout(*this, PixelLayout) ||
		PixelLayout.BufferSize != OutDataSize ||
		(PixelLayout.BufferSize > 0 && OutData == nullptr))
	{
		return false;
	}

	if (PixelLayout.BufferSize > 0)
	{
		FMemory::Memcpy(
			OutData, PixelLayout.Marshalling->get_buffer(), PixelLayout.BufferSize);
	}
	return true;
}

bool FOpenPLXCameraColorOutputView::GetDataU8(TArray<uint8>& OutData) const
{
	OutData.Reset();
	if (GetChannelType() != EAGX_CameraOutputChannelType::U8)
		return false;

	OpenPLXCameraColorOutputView_helpers::FPixelLayout PixelLayout;
	if (!OpenPLXCameraColorOutputView_helpers::GetValidatedPixelLayout(*this, PixelLayout))
		return false;

	return OpenPLXCameraColorOutputView_helpers::CopyPixelData(PixelLayout, OutData);
}

bool FOpenPLXCameraColorOutputView::GetDataF32(TArray<float>& OutData) const
{
	OutData.Reset();
	if (GetChannelType() != EAGX_CameraOutputChannelType::F32)
		return false;

	OpenPLXCameraColorOutputView_helpers::FPixelLayout PixelLayout;
	if (!OpenPLXCameraColorOutputView_helpers::GetValidatedPixelLayout(*this, PixelLayout))
		return false;

	return OpenPLXCameraColorOutputView_helpers::CopyPixelData(PixelLayout, OutData);
}

bool FOpenPLXCameraColorOutputView::MakePersistant()
{
	if (!HasNative())
		return false;

	std::shared_ptr<openplx::Marshalling> Detached = NativeRef->Marshalling->detach_copy();
	if (Detached == nullptr)
		return false;

	NativeRef->Marshalling = std::move(Detached);
	return true;
}

FOpenPLXCameraColorOutputViewRef* FOpenPLXCameraColorOutputView::GetNative()
{
	check(NativeRef);
	return NativeRef.get();
}

const FOpenPLXCameraColorOutputViewRef* FOpenPLXCameraColorOutputView::GetNative() const
{
	check(NativeRef);
	return NativeRef.get();
}
