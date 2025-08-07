// Copyright 2025, Algoryx Simulation AB.

#include "Sensors/IMUBarrier.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/SensorRef.h"
#include "TypeConversions.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
// TWB
#include "EndAGXIncludes.h"


FIMUBarrier::FIMUBarrier()
	: NativeRef {new FIMURef}
{
}

FIMUBarrier::FIMUBarrier(std::unique_ptr<FIMURef> Native)
	: NativeRef(std::move(Native))
{
}

FIMUBarrier::FIMUBarrier(FIMUBarrier&& Other)
	: NativeRef {std::move(Other.NativeRef)}
{
	Other.NativeRef.reset(new FIMURef);
}

FIMUBarrier::~FIMUBarrier()
{
}

bool FIMUBarrier::HasNative() const
{
	return NativeRef->Native != nullptr;
}

void FIMUBarrier::AllocateNative()
{
}

FIMURef* FIMUBarrier::GetNative()
{
	check(HasNative());
	return NativeRef.get();
}

const FIMURef* FIMUBarrier::GetNative() const
{
	check(HasNative());
	return NativeRef.get();
}

uint64 FIMUBarrier::GetNativeAddress() const
{
	return HasNative() ? reinterpret_cast<uint64>(NativeRef->Native.get()) : 0;
}

void FIMUBarrier::SetNativeAddress(uint64 Address)
{
	NativeRef->Native = reinterpret_cast<agxSensor::IMU*>(Address);
}

void FIMUBarrier::ReleaseNative()
{
	if (HasNative())
		NativeRef->Native = nullptr;
}

void FIMUBarrier::SetEnabled(bool Enabled)
{
	check(HasNative());
	NativeRef->Native->setEnable(Enabled);
}

bool FIMUBarrier::GetEnabled() const
{
	check(HasNative());
	return NativeRef->Native->getEnable();
}

void FIMUBarrier::SetTransform(const FTransform& Transform)
{
	check(HasNative());
	*NativeRef->Native->getFrame() =
		*ConvertFrame(Transform.GetLocation(), Transform.GetRotation());
}

FTransform FIMUBarrier::GetTransform() const
{
	check(HasNative());
	return Convert(*NativeRef->Native->getFrame());
}

void FIMUBarrier::IncrementRefCount() const
{
	check(HasNative());
	NativeRef->Native->reference();
}

void FIMUBarrier::DecrementRefCount() const
{
	check(HasNative());
	NativeRef->Native->unreference();
}
