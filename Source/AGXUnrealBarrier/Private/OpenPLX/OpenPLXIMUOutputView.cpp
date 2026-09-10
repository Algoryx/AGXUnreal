// Copyright 2026, Algoryx Simulation AB.

#include "OpenPLX/OpenPLXIMUOutputView.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "BarrierOnly/AGXTypeConversions.h"
#include "BarrierOnly/OpenPLX/OpenPLXRefs.h"
#include "Utilities/PLXMarshallingUtilities.h"

// Standard library includes.
#include <string>
#include <utility>

namespace OpenPLXIMUOutputView_helpers
{
	using namespace PLXMarshallingUtilities;

	template <typename ConvertFuncT>
	bool ReadVectorInternal(
		openplx::Marshalling& Marshalling, const std::string& Name, FVector& OutValue,
		ConvertFuncT ConvertFunc, const TCHAR* DisplayName)
	{
		OutValue = FVector::ZeroVector;
		if (Marshalling.get_buffer_size() == 0)
			return true;

		FWindowLayout Layout;
		if (!GetWindowLayout(Marshalling, Layout, /*bRequireBuffer*/ true))
			return false;

		const openplx::Field* XField = nullptr;
		const openplx::Field* YField = nullptr;
		const openplx::Field* ZField = nullptr;
		if (!GetNestedVectorFields(
				*Layout.Marshalling, Layout.Stride, Name, IsDoubleFieldInsideStride, XField,
				YField, ZField))
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
		const agx::Vec3 ValueAGX {
			static_cast<agx::Real>(ReadValue<double>(WindowBuffer + XField->offset)),
			static_cast<agx::Real>(ReadValue<double>(WindowBuffer + YField->offset)),
			static_cast<agx::Real>(ReadValue<double>(WindowBuffer + ZField->offset))};
		OutValue = ConvertFunc(ValueAGX);

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

bool FOpenPLXIMUOutputView::HasAccelerometer() const
{
	using namespace OpenPLXIMUOutputView_helpers;

	if (!HasNative())
		return false;

	FWindowLayout Layout;
	if (!GetWindowLayout(*NativeRef->Marshalling, Layout, /*bRequireBuffer*/ false))
		return false;

	const openplx::Field* XField = nullptr;
	const openplx::Field* YField = nullptr;
	const openplx::Field* ZField = nullptr;
	return GetNestedVectorFields(
		*Layout.Marshalling, Layout.Stride, "accelerometer_logic", IsDoubleFieldInsideStride,
		XField, YField, ZField);
}

bool FOpenPLXIMUOutputView::HasGyroscope() const
{
	using namespace OpenPLXIMUOutputView_helpers;

	if (!HasNative())
		return false;

	FWindowLayout Layout;
	if (!GetWindowLayout(*NativeRef->Marshalling, Layout, /*bRequireBuffer*/ false))
		return false;

	const openplx::Field* XField = nullptr;
	const openplx::Field* YField = nullptr;
	const openplx::Field* ZField = nullptr;
	return GetNestedVectorFields(
		*Layout.Marshalling, Layout.Stride, "gyroscope_logic", IsDoubleFieldInsideStride, XField,
		YField, ZField);
}

bool FOpenPLXIMUOutputView::HasMagnetometer() const
{
	using namespace OpenPLXIMUOutputView_helpers;

	if (!HasNative())
		return false;

	FWindowLayout Layout;
	if (!GetWindowLayout(*NativeRef->Marshalling, Layout, /*bRequireBuffer*/ false))
		return false;

	const openplx::Field* XField = nullptr;
	const openplx::Field* YField = nullptr;
	const openplx::Field* ZField = nullptr;
	return GetNestedVectorFields(
		*Layout.Marshalling, Layout.Stride, "magnetometer_logic", IsDoubleFieldInsideStride,
		XField, YField, ZField);
}

bool FOpenPLXIMUOutputView::GetAccelerometerData(FVector& OutAccelerometerData)
{
	if (!HasNative())
		return false;

	return OpenPLXIMUOutputView_helpers::ReadVectorInternal(
		*NativeRef->Marshalling, "accelerometer_logic", OutAccelerometerData,
		[](const agx::Vec3& Value) { return ConvertDisplacement(Value); }, TEXT("accelerometer"));
}

bool FOpenPLXIMUOutputView::GetGyroscopeData(FVector& OutGyroscopeData)
{
	if (!HasNative())
		return false;

	return OpenPLXIMUOutputView_helpers::ReadVectorInternal(
		*NativeRef->Marshalling, "gyroscope_logic", OutGyroscopeData,
		[](const agx::Vec3& Value) { return ConvertAngularVelocity(Value); }, TEXT("gyroscope"));
}

bool FOpenPLXIMUOutputView::GetMagnetometerData(FVector& OutMagnetometerData)
{
	if (!HasNative())
		return false;

	return OpenPLXIMUOutputView_helpers::ReadVectorInternal(
		*NativeRef->Marshalling, "magnetometer_logic", OutMagnetometerData,
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
