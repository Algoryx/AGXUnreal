// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/CameraBackendBarrier.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/SensorRef.h"

// Standard library includes.
#include <utility>

FCameraBackendBarrier::FCameraBackendBarrier()
{
}

FCameraBackendBarrier::FCameraBackendBarrier(std::shared_ptr<FCameraBackendRef> Native)
	: NativeRef(std::move(Native))
{
}

FCameraBackendBarrier::~FCameraBackendBarrier()
{
	ReleaseNative();
}

bool FCameraBackendBarrier::HasNative() const
{
	return NativeRef != nullptr;
}

void FCameraBackendBarrier::AllocateNative()
{
	check(!HasNative());
	NativeRef = std::make_shared<FCameraBackendRef>();
}

FCameraBackendRef* FCameraBackendBarrier::GetNative()
{
	check(HasNative());
	return NativeRef.get();
}

const FCameraBackendRef* FCameraBackendBarrier::GetNative() const
{
	check(HasNative());
	return NativeRef.get();
}

void FCameraBackendBarrier::ReleaseNative()
{
	if (HasNative())
		NativeRef.reset();
}
