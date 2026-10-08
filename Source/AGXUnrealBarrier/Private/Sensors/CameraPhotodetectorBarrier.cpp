// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/CameraPhotodetectorBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "Sensors/SensorRef.h"

FCameraPhotodetectorBarrier::FCameraPhotodetectorBarrier()
	: NativeRef(std::make_shared<FCameraPhotodetectorRef>())
{
}

FCameraPhotodetectorBarrier::FCameraPhotodetectorBarrier(
	std::shared_ptr<FCameraPhotodetectorRef> Native)
	: NativeRef(std::move(Native))
{
	check(NativeRef);
}

bool FCameraPhotodetectorBarrier::HasNative() const
{
	AGX_CHECK(NativeRef != nullptr);
	return NativeRef->Native != nullptr;
}

FCameraPhotodetectorRef* FCameraPhotodetectorBarrier::GetNative()
{
	check(HasNative());
	return NativeRef.get();
}

const FCameraPhotodetectorRef* FCameraPhotodetectorBarrier::GetNative() const
{
	check(HasNative());
	return NativeRef.get();
}

void FCameraPhotodetectorBarrier::ReleaseNative()
{
	NativeRef->Native = nullptr;
}
