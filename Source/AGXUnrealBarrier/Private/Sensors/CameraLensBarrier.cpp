// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/CameraLensBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "Sensors/SensorRef.h"

FCameraLensBarrier::FCameraLensBarrier()
	: NativeRef(std::make_shared<FCameraLensRef>())
{
}

FCameraLensBarrier::FCameraLensBarrier(std::shared_ptr<FCameraLensRef> Native)
	: NativeRef(std::move(Native))
{
	check(NativeRef);
}

bool FCameraLensBarrier::HasNative() const
{
	AGX_CHECK(NativeRef != nullptr);
	return NativeRef->Native != nullptr;
}

FCameraLensRef* FCameraLensBarrier::GetNative()
{
	check(HasNative());
	return NativeRef.get();
}

const FCameraLensRef* FCameraLensBarrier::GetNative() const
{
	check(HasNative());
	return NativeRef.get();
}

void FCameraLensBarrier::ReleaseNative()
{
	NativeRef->Native = nullptr;
}
