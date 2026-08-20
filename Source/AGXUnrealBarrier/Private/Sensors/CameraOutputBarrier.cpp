// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/CameraOutputBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "Sensors/SensorRef.h"

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

void FCameraOutputBarrier::ReleaseNative()
{
	NativeRef->Native = nullptr;
}
