// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/CameraOutputBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "Sensors/CameraBarrier.h"
#include "Sensors/CameraBackendBarrier.h"
#include "Sensors/SensorRef.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxSensor/CameraOutput.h>
#include "EndAGXIncludes.h"

FCameraOutputBarrier::FCameraOutputBarrier()
	: NativeRef(std::make_shared<FCameraOutputRef>())
{
}

FCameraOutputBarrier::FCameraOutputBarrier(std::shared_ptr<FCameraOutputRef> Native)
	: NativeRef(std::move(Native))
{
	check(NativeRef);
}

bool FCameraOutputBarrier::HasNative() const
{
	AGX_CHECK(NativeRef != nullptr);
	return NativeRef->Native != nullptr;
}

FCameraOutputRef* FCameraOutputBarrier::GetNative()
{
	check(HasNative());
	return NativeRef.get();
}

const FCameraOutputRef* FCameraOutputBarrier::GetNative() const
{
	check(HasNative());
	return NativeRef.get();
}

uint64 FCameraOutputBarrier::GetNativeAddress() const
{
	return HasNative() ? reinterpret_cast<uint64>(NativeRef->Native.get()) : 0;
}

void FCameraOutputBarrier::ReleaseNative()
{
	NativeRef->Native = nullptr;
}

void FCameraOutputBarrier::RegisterWithBackend(FCameraBarrier& Camera)
{
	check(HasNative());
	check(Camera.HasNative());

	if (FCameraBackendBarrier::GetInstance().HasNative())
		FCameraBackendBarrier::GetInstance().RegisterOutput(Camera, *this);
}

void FCameraOutputBarrier::UnregisterFromBackend(FCameraBarrier& Camera)
{
	if (FCameraBackendBarrier::GetInstance().HasNative())
		FCameraBackendBarrier::GetInstance().UnregisterOutput(Camera, *this);
}

void FCameraOutputBarrier::SetResolution(FIntPoint InResolution)
{
	check(HasNative());
	NativeRef->Native->setResolution(agx::Vec2i(InResolution.X, InResolution.Y));
}

FIntPoint FCameraOutputBarrier::GetResolution() const
{
	check(HasNative());
	const agx::Vec2i ResolutionAGX = NativeRef->Native->getResolution();
	return {static_cast<int32>(ResolutionAGX.x()), static_cast<int32>(ResolutionAGX.y())};
}

void FCameraOutputBarrier::SetConstantCapture(double InFrameRate)
{
	check(HasNative());
	NativeRef->Native->setConstantCapture(InFrameRate);
}

void FCameraOutputBarrier::SetManualCapture()
{
	check(HasNative());
	NativeRef->Native->setManualCapture();
}

bool FCameraOutputBarrier::GetConstantCapture() const
{
	check(HasNative());
	return NativeRef->Native->isConstantlyCapturing();
}

double FCameraOutputBarrier::GetFrameRate() const
{
	check(HasNative());
	const agxSensor::OptionalFramerate FrameRateAGX = NativeRef->Native->getFramerate();
	return FrameRateAGX.has_value() ? *FrameRateAGX : 0.0;
}
