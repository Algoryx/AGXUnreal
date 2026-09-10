// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/LensDistortionBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "Sensors/SensorRef.h"

FLensDistortionBarrier::FLensDistortionBarrier()
	: NativeRef(std::make_shared<FLensDistortionRef>())
{
}

FLensDistortionBarrier::FLensDistortionBarrier(std::shared_ptr<FLensDistortionRef> Native)
	: NativeRef(std::move(Native))
{
	check(NativeRef);
}

bool FLensDistortionBarrier::HasNative() const
{
	AGX_CHECK(NativeRef != nullptr);
	return NativeRef->Native != nullptr;
}

FLensDistortionRef* FLensDistortionBarrier::GetNative()
{
	check(HasNative());
	return NativeRef.get();
}

const FLensDistortionRef* FLensDistortionBarrier::GetNative() const
{
	check(HasNative());
	return NativeRef.get();
}

void FLensDistortionBarrier::ReleaseNative()
{
	NativeRef->Native = nullptr;
}
