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

bool FOpenPLXLidarOutputView::ReadPositions(TArray<FVector>& OutPositions)
{
	using namespace OpenPLXLidarOutputView_helpers;

	OutPositions.Reset();
	if (!HasNative())
		return false;

	openplx::Marshalling* Marshalling = NativeRef->Marshalling.get();
	if (Marshalling->get_buffer_size() == 0)
		return true;

	if (Marshalling->get_buffer() == nullptr)
		return false;

	std::unique_ptr<openplx::Marshalling>& WindowMarshallingPtr =
		Marshalling->get_or_add_nested_marshalling("window");
	Marshalling->calculate_nested_buffer_sizes();

	openplx::Marshalling* WindowMarshalling = WindowMarshallingPtr.get();
	if (WindowMarshalling == nullptr || WindowMarshalling->get_buffer() == nullptr)
		return false;

	const size_t WindowStride = WindowMarshalling->get_stride();
	const size_t WindowBufferSize = WindowMarshalling->get_buffer_size();
	if (WindowStride == 0 || WindowBufferSize % WindowStride != 0)
		return false;

	const size_t NumWindows = WindowBufferSize / WindowStride;
	if (!CanConvert(NumWindows))
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT(
				"OpenPLX Lidar Output View: Refusing to read positions because the number of "
				"positions is too large for a TArray."));
		return false;
	}

	std::unique_ptr<openplx::Marshalling>& PositionMarshallingPtr =
		WindowMarshalling->get_or_add_nested_marshalling("position3d");
	openplx::Marshalling* PositionMarshalling = PositionMarshallingPtr.get();
	if (PositionMarshalling == nullptr)
		return false;

	const std::unordered_map<std::string, openplx::Field>& PositionFields =
		PositionMarshalling->get_field_map();
	const openplx::Field* XField = FindField(PositionFields, "x");
	const openplx::Field* YField = FindField(PositionFields, "y");
	const openplx::Field* ZField = FindField(PositionFields, "z");
	if (!IsFloatFieldInsideStride(XField, WindowStride) ||
		!IsFloatFieldInsideStride(YField, WindowStride) ||
		!IsFloatFieldInsideStride(ZField, WindowStride))
	{
		return false;
	}

	const size_t LastWindowOffset = NumWindows > 0 ? (NumWindows - 1) * WindowStride : 0;
	const size_t MaxFieldEnd = FMath::Max3(
		XField->offset + XField->size, YField->offset + YField->size,
		ZField->offset + ZField->size);
	if (NumWindows > 0 && LastWindowOffset + MaxFieldEnd > WindowBufferSize)
		return false;

	const uint8_t* WindowBuffer = WindowMarshalling->get_buffer();
	OutPositions.SetNumUninitialized(static_cast<int32>(NumWindows));
	for (int32 I = 0; I < OutPositions.Num(); ++I)
	{
		const uint8_t* Window = WindowBuffer + static_cast<size_t>(I) * WindowStride;
		const float X = ReadFloat(Window + XField->offset);
		const float Y = ReadFloat(Window + YField->offset);
		const float Z = ReadFloat(Window + ZField->offset);
		OutPositions[I] =
			ConvertDisplacement(static_cast<agx::Real>(X), static_cast<agx::Real>(Y), static_cast<agx::Real>(Z));
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
