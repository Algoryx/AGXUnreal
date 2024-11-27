// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/LidarOutputBarrier.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/SensorRef.h"
#include "TypeConversions.h"


FLidarOutputBarrier::FLidarOutputBarrier()
	: NativeRef {new FLidarOutputRef}
{
}

FLidarOutputBarrier::FLidarOutputBarrier(std::unique_ptr<FLidarOutputRef> Native)
	: NativeRef(std::move(Native))
{
}

FLidarOutputBarrier::FLidarOutputBarrier(FLidarOutputBarrier&& Other)
	: NativeRef {std::move(Other.NativeRef)}
{
	Other.NativeRef.reset(new FLidarOutputRef);
}

FLidarOutputBarrier::~FLidarOutputBarrier()
{
}

bool FLidarOutputBarrier::HasNative() const
{
	return NativeRef->Native != nullptr;
}

FLidarOutputRef* FLidarOutputBarrier::GetNative()
{
	check(HasNative());
	return NativeRef.get();
}

const FLidarOutputRef* FLidarOutputBarrier::GetNative() const
{
	check(HasNative());
	return NativeRef.get();
}

void FLidarOutputBarrier::ReleaseNative()
{
	if (HasNative())
		NativeRef->Native = nullptr;
}

