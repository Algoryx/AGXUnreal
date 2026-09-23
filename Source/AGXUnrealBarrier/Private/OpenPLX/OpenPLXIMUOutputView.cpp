// Copyright 2026, Algoryx Simulation AB.

#include "OpenPLX/OpenPLXIMUOutputView.h"

// AGX Dynamics for Unreal includes.
#include "BarrierOnly/AGXTypeConversions.h"
#include "BarrierOnly/OpenPLX/OpenPLXRefs.h"
#include "Utilities/PLXMarshallingUtilities.h"

// Standard library includes.
#include <string>
#include <utility>

namespace OpenPLXIMUOutputView_helpers
{
	using namespace PLXMarshallingUtilities;

	bool HasVectorInternal(openplx::Marshalling& Marshalling, const std::string& Name)
	{
		FWindowLayout Layout;
		if (!GetWindowLayout(Marshalling, Layout, /*bRequireBuffer*/ false))
			return false;

		const openplx::Field* XField = nullptr;
		const openplx::Field* YField = nullptr;
		const openplx::Field* ZField = nullptr;
		return GetNestedVectorFields(*Layout.Marshalling, Name, XField, YField, ZField);
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
	if (!HasNative())
		return false;

	return OpenPLXIMUOutputView_helpers::HasVectorInternal(
		*NativeRef->Marshalling, "accelerometer_logic");
}

bool FOpenPLXIMUOutputView::HasGyroscope() const
{
	if (!HasNative())
		return false;

	return OpenPLXIMUOutputView_helpers::HasVectorInternal(
		*NativeRef->Marshalling, "gyroscope_logic");
}

bool FOpenPLXIMUOutputView::HasMagnetometer() const
{
	if (!HasNative())
		return false;

	return OpenPLXIMUOutputView_helpers::HasVectorInternal(
		*NativeRef->Marshalling, "magnetometer_logic");
}

bool FOpenPLXIMUOutputView::GetAccelerometerData(FVector& OutAccelerometerData)
{
	if (!HasNative())
		return false;

	PLXMarshallingUtilities::FWindowLayout Layout;
	if (!PLXMarshallingUtilities::GetWindowLayout(
			*NativeRef->Marshalling, Layout, /*bRequireBuffer*/ true))
		return false;

	return PLXMarshallingUtilities::ReadVector(
		*Layout.Marshalling, "accelerometer_logic", OutAccelerometerData,
		[](const agx::Vec3& Value) { return ConvertDisplacement(Value); }, TEXT("accelerometer"));
}

bool FOpenPLXIMUOutputView::GetGyroscopeData(FVector& OutGyroscopeData)
{
	if (!HasNative())
		return false;

	PLXMarshallingUtilities::FWindowLayout Layout;
	if (!PLXMarshallingUtilities::GetWindowLayout(
			*NativeRef->Marshalling, Layout, /*bRequireBuffer*/ true))
		return false;

	return PLXMarshallingUtilities::ReadVector(
		*Layout.Marshalling, "gyroscope_logic", OutGyroscopeData,
		[](const agx::Vec3& Value) { return ConvertAngularVelocity(Value); }, TEXT("gyroscope"));
}

bool FOpenPLXIMUOutputView::GetMagnetometerData(FVector& OutMagnetometerData)
{
	if (!HasNative())
		return false;

	PLXMarshallingUtilities::FWindowLayout Layout;
	if (!PLXMarshallingUtilities::GetWindowLayout(
			*NativeRef->Marshalling, Layout, /*bRequireBuffer*/ true))
		return false;

	return PLXMarshallingUtilities::ReadVector(
		*Layout.Marshalling, "magnetometer_logic", OutMagnetometerData,
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
