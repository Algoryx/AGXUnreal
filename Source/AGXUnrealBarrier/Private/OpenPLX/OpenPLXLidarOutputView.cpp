// Copyright 2026, Algoryx Simulation AB.

#include "OpenPLX/OpenPLXLidarOutputView.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "BarrierOnly/AGXTypeConversions.h"
#include "BarrierOnly/OpenPLX/OpenPLXRefs.h"

// OpenPLX includes.
#include "BeginAGXIncludes.h"
#include "openplx/Marshalling.h"
#include "EndAGXIncludes.h"

// Standard library includes.
#include <cstring>
#include <string>
#include <unordered_map>
#include <utility>

namespace OpenPLXLidarOutputView_helpers
{
	struct WindowLayout
	{
		openplx::Marshalling* Marshalling = nullptr;
		size_t Stride = 0;
		size_t BufferSize = 0;
		size_t NumWindows = 0;
	};

	const openplx::Field* FindField(
		const std::unordered_map<std::string, openplx::Field>& Fields, const std::string& Name)
	{
		const auto It = Fields.find(Name);
		return It != Fields.end() ? &It->second : nullptr;
	}

	bool IsFloatFieldInsideStride(const openplx::Field* Field, size_t Stride)
	{
		return Field != nullptr && Field->field_type == openplx::FieldType::Real &&
			   Field->size == sizeof(float) && Field->offset + Field->size <= Stride;
	}

	float ReadFloat(const uint8_t* Data)
	{
		float Value;
		std::memcpy(&Value, Data, sizeof(Value));
		return Value;
	}

	bool GetWindowLayout(
		openplx::Marshalling& Marshalling, WindowLayout& OutLayout, bool bRequireBuffer)
	{
		if (bRequireBuffer && Marshalling.get_buffer_size() > 0 && Marshalling.get_buffer() == nullptr)
			return false;

		std::unique_ptr<openplx::Marshalling>& WindowMarshallingPtr =
			Marshalling.get_or_add_nested_marshalling("window");
		Marshalling.calculate_nested_buffer_sizes();

		openplx::Marshalling* WindowMarshalling = WindowMarshallingPtr.get();
		if (WindowMarshalling == nullptr)
			return false;

		if (bRequireBuffer && WindowMarshalling->get_buffer_size() > 0 &&
			WindowMarshalling->get_buffer() == nullptr)
		{
			return false;
		}

		const size_t WindowStride = WindowMarshalling->get_stride();
		const size_t WindowBufferSize = WindowMarshalling->get_buffer_size();
		if (WindowStride == 0 || WindowBufferSize % WindowStride != 0)
			return false;

		OutLayout.Marshalling = WindowMarshalling;
		OutLayout.Stride = WindowStride;
		OutLayout.BufferSize = WindowBufferSize;
		OutLayout.NumWindows = WindowBufferSize / WindowStride;
		return true;
	}

	bool GetPositionFields(
		openplx::Marshalling& WindowMarshalling, size_t WindowStride,
		const openplx::Field*& OutXField, const openplx::Field*& OutYField,
		const openplx::Field*& OutZField)
	{
		std::unique_ptr<openplx::Marshalling>& PositionMarshallingPtr =
			WindowMarshalling.get_or_add_nested_marshalling("position3d");
		openplx::Marshalling* PositionMarshalling = PositionMarshallingPtr.get();
		if (PositionMarshalling == nullptr)
			return false;

		const std::unordered_map<std::string, openplx::Field>& PositionFields =
			PositionMarshalling->get_field_map();
		OutXField = FindField(PositionFields, "x");
		OutYField = FindField(PositionFields, "y");
		OutZField = FindField(PositionFields, "z");
		return IsFloatFieldInsideStride(OutXField, WindowStride) &&
			   IsFloatFieldInsideStride(OutYField, WindowStride) &&
			   IsFloatFieldInsideStride(OutZField, WindowStride);
	}

	const openplx::Field* GetIntensityField(
		openplx::Marshalling& WindowMarshalling, size_t WindowStride)
	{
		const openplx::Field* Field = FindField(WindowMarshalling.get_field_map(), "intensity");
		return IsFloatFieldInsideStride(Field, WindowStride) ? Field : nullptr;
	}

	bool ReadPositionsInternal(
		openplx::Marshalling& Marshalling, TArray<FVector>& OutPositions,
		const FTransform* RelativeTo)
	{
		OutPositions.Reset();
		if (Marshalling.get_buffer_size() == 0)
			return true;

		WindowLayout Layout;
		if (!GetWindowLayout(Marshalling, Layout, /*bRequireBuffer*/ true))
			return false;

		if (!CanConvert(Layout.NumWindows))
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT(
					"OpenPLX Lidar Output View: Refusing to read positions because the number of "
					"positions is too large for a TArray."));
			return false;
		}

		const openplx::Field* XField = nullptr;
		const openplx::Field* YField = nullptr;
		const openplx::Field* ZField = nullptr;
		if (!GetPositionFields(*Layout.Marshalling, Layout.Stride, XField, YField, ZField))
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("OpenPLX Lidar Output View: Tried to read positions, but this Lidar output "
					 "does not contain positions."));
			return false;
		}

		const size_t LastWindowOffset =
			Layout.NumWindows > 0 ? (Layout.NumWindows - 1) * Layout.Stride : 0;
		const size_t MaxFieldEnd = FMath::Max3(
			XField->offset + XField->size, YField->offset + YField->size,
			ZField->offset + ZField->size);
		if (Layout.NumWindows > 0 && LastWindowOffset + MaxFieldEnd > Layout.BufferSize)
			return false;

		const uint8_t* WindowBuffer = Layout.Marshalling->get_buffer();
		OutPositions.SetNumUninitialized(static_cast<int32>(Layout.NumWindows));
		for (int32 I = 0; I < OutPositions.Num(); ++I)
		{
			const uint8_t* Window = WindowBuffer + static_cast<size_t>(I) * Layout.Stride;
			const float X = ReadFloat(Window + XField->offset);
			const float Y = ReadFloat(Window + YField->offset);
			const float Z = ReadFloat(Window + ZField->offset);
			FVector Position = ConvertDisplacement(
				static_cast<agx::Real>(X), static_cast<agx::Real>(Y),
				static_cast<agx::Real>(Z));
			if (RelativeTo != nullptr)
				Position = RelativeTo->TransformPositionNoScale(Position);

			OutPositions[I] = Position;
		}

		return true;
	}
}

FOpenPLXLidarOutputView::FOpenPLXLidarOutputView()
	: NativeRef {new FOpenPLXLidarOutputViewRef}
{
}

FOpenPLXLidarOutputView::FOpenPLXLidarOutputView(
	std::shared_ptr<FOpenPLXLidarOutputViewRef> Native)
	: NativeRef(std::move(Native))
{
	check(NativeRef);
}

bool FOpenPLXLidarOutputView::HasNative() const
{
	return NativeRef != nullptr && NativeRef->Marshalling != nullptr;
}

bool FOpenPLXLidarOutputView::HasPositions() const
{
	using namespace OpenPLXLidarOutputView_helpers;

	if (!HasNative())
		return false;

	WindowLayout Layout;
	if (!GetWindowLayout(*NativeRef->Marshalling, Layout, /*bRequireBuffer*/ false))
		return false;

	const openplx::Field* XField = nullptr;
	const openplx::Field* YField = nullptr;
	const openplx::Field* ZField = nullptr;
	return GetPositionFields(*Layout.Marshalling, Layout.Stride, XField, YField, ZField);
}

bool FOpenPLXLidarOutputView::HasIntensities() const
{
	using namespace OpenPLXLidarOutputView_helpers;

	if (!HasNative())
		return false;

	WindowLayout Layout;
	return GetWindowLayout(*NativeRef->Marshalling, Layout, /*bRequireBuffer*/ false) &&
		   GetIntensityField(*Layout.Marshalling, Layout.Stride) != nullptr;
}

bool FOpenPLXLidarOutputView::ReadPositions(TArray<FVector>& OutPositions)
{
	if (!HasNative())
		return false;

	return OpenPLXLidarOutputView_helpers::ReadPositionsInternal(
		*NativeRef->Marshalling, OutPositions, nullptr);
}

bool FOpenPLXLidarOutputView::ReadPositionsTransformed(
	const FTransform& RelativeTo, TArray<FVector>& OutPositions)
{
	if (!HasNative())
		return false;

	return OpenPLXLidarOutputView_helpers::ReadPositionsInternal(
		*NativeRef->Marshalling, OutPositions, &RelativeTo);
}

bool FOpenPLXLidarOutputView::ReadIntensities(TArray<float>& OutIntensities)
{
	using namespace OpenPLXLidarOutputView_helpers;

	OutIntensities.Reset();
	if (!HasNative())
		return false;

	openplx::Marshalling* Marshalling = NativeRef->Marshalling.get();
	if (Marshalling->get_buffer_size() == 0)
		return true;

	WindowLayout Layout;
	if (!GetWindowLayout(*Marshalling, Layout, /*bRequireBuffer*/ true))
		return false;

	if (!CanConvert(Layout.NumWindows))
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT(
				"OpenPLX Lidar Output View: Refusing to read intensities because the number of "
				"intensities is too large for a TArray."));
		return false;
	}

	const openplx::Field* IntensityField =
		GetIntensityField(*Layout.Marshalling, Layout.Stride);
	if (IntensityField == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("OpenPLX Lidar Output View: Tried to read intensities, but this Lidar output "
				 "does not contain intensities."));
		return false;
	}

	const size_t LastWindowOffset =
		Layout.NumWindows > 0 ? (Layout.NumWindows - 1) * Layout.Stride : 0;
	const size_t FieldEnd = IntensityField->offset + IntensityField->size;
	if (Layout.NumWindows > 0 && LastWindowOffset + FieldEnd > Layout.BufferSize)
		return false;

	const uint8_t* WindowBuffer = Layout.Marshalling->get_buffer();
	OutIntensities.SetNumUninitialized(static_cast<int32>(Layout.NumWindows));
	for (int32 I = 0; I < OutIntensities.Num(); ++I)
	{
		const uint8_t* Window = WindowBuffer + static_cast<size_t>(I) * Layout.Stride;
		OutIntensities[I] = ReadFloat(Window + IntensityField->offset);
	}

	return true;
}

bool FOpenPLXLidarOutputView::MakePersistant()
{
	if (!HasNative())
		return false;

	std::shared_ptr<openplx::Marshalling> Detached = NativeRef->Marshalling->detach_copy();
	if (Detached == nullptr)
		return false;

	NativeRef->Marshalling = std::move(Detached);
	return true;
}

FOpenPLXLidarOutputViewRef* FOpenPLXLidarOutputView::GetNative()
{
	check(NativeRef);
	return NativeRef.get();
}

const FOpenPLXLidarOutputViewRef* FOpenPLXLidarOutputView::GetNative() const
{
	check(NativeRef);
	return NativeRef.get();
}
