// Copyright 2026, Algoryx Simulation AB.

#include "OpenPLX/OpenPLXIMUOutputView.h"

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

namespace OpenPLXIMUOutputView_helpers
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

	bool IsDoubleFieldInsideStride(const openplx::Field* Field, size_t Stride)
	{
		return Field != nullptr && Field->field_type == openplx::FieldType::Real &&
			   Field->size == sizeof(double) && Field->offset + Field->size <= Stride;
	}

	double ReadDouble(const uint8_t* Data)
	{
		double Value;
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

	bool GetVectorFields(
		openplx::Marshalling& WindowMarshalling, size_t WindowStride, const std::string& Name,
		const openplx::Field*& OutXField, const openplx::Field*& OutYField,
		const openplx::Field*& OutZField)
	{
		std::unique_ptr<openplx::Marshalling>& VectorMarshallingPtr =
			WindowMarshalling.get_or_add_nested_marshalling(Name);
		openplx::Marshalling* VectorMarshalling = VectorMarshallingPtr.get();
		if (VectorMarshalling == nullptr)
			return false;

		const std::unordered_map<std::string, openplx::Field>& VectorFields =
			VectorMarshalling->get_field_map();
		OutXField = FindField(VectorFields, "x");
		OutYField = FindField(VectorFields, "y");
		OutZField = FindField(VectorFields, "z");
		return IsDoubleFieldInsideStride(OutXField, WindowStride) &&
			   IsDoubleFieldInsideStride(OutYField, WindowStride) &&
			   IsDoubleFieldInsideStride(OutZField, WindowStride);
	}

	template <typename ConvertFuncT>
	bool ReadVectorInternal(
		openplx::Marshalling& Marshalling, const std::string& Name, TArray<FVector>& OutValues,
		ConvertFuncT ConvertFunc, const TCHAR* DisplayName)
	{
		OutValues.Reset();
		if (Marshalling.get_buffer_size() == 0)
			return true;

		WindowLayout Layout;
		if (!GetWindowLayout(Marshalling, Layout, /*bRequireBuffer*/ true))
			return false;

		if (!CanConvert(Layout.NumWindows))
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("OpenPLX IMU Output View: Refusing to read %s because the number of "
					 "samples is too large for a TArray."),
				DisplayName);
			return false;
		}

		const openplx::Field* XField = nullptr;
		const openplx::Field* YField = nullptr;
		const openplx::Field* ZField = nullptr;
		if (!GetVectorFields(*Layout.Marshalling, Layout.Stride, Name, XField, YField, ZField))
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("OpenPLX IMU Output View: Tried to read %s, but this IMU output does not "
					 "contain %s."),
				DisplayName, DisplayName);
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
		OutValues.SetNumUninitialized(static_cast<int32>(Layout.NumWindows));
		for (int32 I = 0; I < OutValues.Num(); ++I)
		{
			const uint8_t* Window = WindowBuffer + static_cast<size_t>(I) * Layout.Stride;
			const agx::Vec3 ValueAGX {
				static_cast<agx::Real>(ReadDouble(Window + XField->offset)),
				static_cast<agx::Real>(ReadDouble(Window + YField->offset)),
				static_cast<agx::Real>(ReadDouble(Window + ZField->offset))};
			OutValues[I] = ConvertFunc(ValueAGX);
		}

		return true;
	}
}

FOpenPLXIMUOutputView::FOpenPLXIMUOutputView()
	: NativeRef {new FOpenPLXIMUOutputViewRef}
{
}

FOpenPLXIMUOutputView::FOpenPLXIMUOutputView(std::shared_ptr<FOpenPLXIMUOutputViewRef> Native)
	: NativeRef(std::move(Native))
{
	check(NativeRef);
}

bool FOpenPLXIMUOutputView::HasNative() const
{
	return NativeRef != nullptr && NativeRef->Marshalling != nullptr;
}

int32 FOpenPLXIMUOutputView::GetNumSamples() const
{
	using namespace OpenPLXIMUOutputView_helpers;

	if (!HasNative())
		return 0;

	WindowLayout Layout;
	if (!GetWindowLayout(*NativeRef->Marshalling, Layout, /*bRequireBuffer*/ false))
		return 0;

	return CanConvert(Layout.NumWindows) ? static_cast<int32>(Layout.NumWindows) : 0;
}

bool FOpenPLXIMUOutputView::HasAccelerometer() const
{
	using namespace OpenPLXIMUOutputView_helpers;

	if (!HasNative())
		return false;

	WindowLayout Layout;
	if (!GetWindowLayout(*NativeRef->Marshalling, Layout, /*bRequireBuffer*/ false))
		return false;

	const openplx::Field* XField = nullptr;
	const openplx::Field* YField = nullptr;
	const openplx::Field* ZField = nullptr;
	return GetVectorFields(
		*Layout.Marshalling, Layout.Stride, "accelerometer_logic", XField, YField, ZField);
}

bool FOpenPLXIMUOutputView::HasGyroscope() const
{
	using namespace OpenPLXIMUOutputView_helpers;

	if (!HasNative())
		return false;

	WindowLayout Layout;
	if (!GetWindowLayout(*NativeRef->Marshalling, Layout, /*bRequireBuffer*/ false))
		return false;

	const openplx::Field* XField = nullptr;
	const openplx::Field* YField = nullptr;
	const openplx::Field* ZField = nullptr;
	return GetVectorFields(
		*Layout.Marshalling, Layout.Stride, "gyroscope_logic", XField, YField, ZField);
}

bool FOpenPLXIMUOutputView::HasMagnetometer() const
{
	using namespace OpenPLXIMUOutputView_helpers;

	if (!HasNative())
		return false;

	WindowLayout Layout;
	if (!GetWindowLayout(*NativeRef->Marshalling, Layout, /*bRequireBuffer*/ false))
		return false;

	const openplx::Field* XField = nullptr;
	const openplx::Field* YField = nullptr;
	const openplx::Field* ZField = nullptr;
	return GetVectorFields(
		*Layout.Marshalling, Layout.Stride, "magnetometer_logic", XField, YField, ZField);
}

bool FOpenPLXIMUOutputView::ReadAccelerometer(TArray<FVector>& OutAccelerometer)
{
	if (!HasNative())
		return false;

	return OpenPLXIMUOutputView_helpers::ReadVectorInternal(
		*NativeRef->Marshalling, "accelerometer_logic", OutAccelerometer,
		[](const agx::Vec3& Value) { return ConvertDisplacement(Value); }, TEXT("accelerometer"));
}

bool FOpenPLXIMUOutputView::ReadGyroscope(TArray<FVector>& OutGyroscope)
{
	if (!HasNative())
		return false;

	return OpenPLXIMUOutputView_helpers::ReadVectorInternal(
		*NativeRef->Marshalling, "gyroscope_logic", OutGyroscope,
		[](const agx::Vec3& Value) { return ConvertAngularVelocity(Value); }, TEXT("gyroscope"));
}

bool FOpenPLXIMUOutputView::ReadMagnetometer(TArray<FVector>& OutMagnetometer)
{
	if (!HasNative())
		return false;

	return OpenPLXIMUOutputView_helpers::ReadVectorInternal(
		*NativeRef->Marshalling, "magnetometer_logic", OutMagnetometer,
		[](const agx::Vec3& Value) { return ConvertVector(Value); }, TEXT("magnetometer"));
}

bool FOpenPLXIMUOutputView::MakePersistant()
{
	if (!HasNative())
		return false;

	std::shared_ptr<openplx::Marshalling> Detached = NativeRef->Marshalling->detach_copy();
	if (Detached == nullptr)
		return false;

	NativeRef->Marshalling = std::move(Detached);
	return true;
}

FOpenPLXIMUOutputViewRef* FOpenPLXIMUOutputView::GetNative()
{
	check(NativeRef);
	return NativeRef.get();
}

const FOpenPLXIMUOutputViewRef* FOpenPLXIMUOutputView::GetNative() const
{
	check(NativeRef);
	return NativeRef.get();
}
