// Copyright 2025, Algoryx Simulation AB.

#include "Sensors/SensorBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "Sensors/SensorRef.h"
#include "Sensors/SensorEnvironmentBarrier.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxSensor/Sensor.h>
#include "EndAGXIncludes.h"

FSensorBarrier::FSensorBarrier()
	: NativeRef {new FSensorRef}
	, StepStrideRef {new FSensorGroupStepStrideRef}
{
}

FSensorBarrier::FSensorBarrier(
	std::shared_ptr<FSensorRef> Native, std::shared_ptr<FSensorGroupStepStrideRef> StepStride)
	: NativeRef(std::move(Native))
	, StepStrideRef(std::move(StepStride))
{
}

FSensorBarrier::~FSensorBarrier()
{
	ReleaseNative();
}

void FSensorBarrier::AllocateStepStride()
{
	check(!HasStepStrideNative());
	StepStrideRef->Native = new agxSensor::SensorGroupStepStride();	
}

bool FSensorBarrier::HasStepStrideNative() const
{
	return StepStrideRef->Native != nullptr;
}

bool FSensorBarrier::HasNative() const
{
	return NativeRef->Native != nullptr;
}

FSensorRef* FSensorBarrier::GetNative()
{
	check(HasNative());
	return NativeRef.get();
}

const FSensorRef* FSensorBarrier::GetNative() const
{
	check(HasNative());
	return NativeRef.get();
}

void FSensorBarrier::ReleaseNative()
{
	if (HasNative())
		NativeRef->Native = nullptr;

	if (HasStepStrideNative())
		StepStrideRef->Native = nullptr;
}

void FSensorBarrier::SetEnabled(bool Enabled)
{
	check(HasNative());
	NativeRef->Native->setEnable(Enabled);
}

bool FSensorBarrier::GetEnabled() const
{
	check(HasNative());
	return NativeRef->Native->getEnable();
}

void FSensorBarrier::SetStepStride(uint32 Stride)
{
	check(HasStepStrideNative());
	StepStrideRef->Native->setStride(Stride);
}

uint32 FSensorBarrier::GetStepStride() const
{
	check(HasStepStrideNative());
	return StepStrideRef->Native->getStride();
}

void FSensorBarrier::IncrementRefCount() const
{
	check(HasNative());
	NativeRef->Native->reference();
}

void FSensorBarrier::DecrementRefCount() const
{
	check(HasNative());
	NativeRef->Native->unreference();
}

uint64 FSensorBarrier::GetNativeAddress() const
{
	return HasNative() ? reinterpret_cast<uint64>(NativeRef->Native.get()) : 0;
}

void FSensorBarrier::SetNativeAddress(uint64 Address)
{
	NativeRef->Native = reinterpret_cast<agxSensor::Sensor*>(Address);

	// At this point, we should be able to find any StepStride object (if it exists), since it will
	// have been kept alive by the agxSensor::Environment.
	StepStrideRef->Native = NativeRef->Native->findParent<agxSensor::SensorGroupStepStride>();
}

bool FSensorBarrier::AddToEnvironment(FSensorEnvironmentBarrier& Environment)
{
	check(HasNative());
	check(HasStepStrideNative());
	check(Environment.HasNative());

	// We add the StepStride instead of the Sensor Native in order to ensure correct stepping.
	// This is a quirk of AGX.
	AGX_CHECK(NativeRef->Native->getEnvironment() == nullptr);
	AGX_CHECK(NStepStrideRef->Native->getEnvironment() == nullptr);

	StepStrideRef->Native->add(NativeRef->Native);
	return Environment.GetNative()->Native->add(StepStrideRef->Native);
}

bool FSensorBarrier::RemoveFromEnvironment(FSensorEnvironmentBarrier& Environment)
{
	check(Environment.HasNative());

	// See also AddToEnvironment.
	return Environment.GetNative()->Native->remove(StepStrideRef->Native);
}
