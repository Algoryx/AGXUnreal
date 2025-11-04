// Copyright 2025, Algoryx Simulation AB.

#include "Sensors/SensorBarrier.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/SensorRef.h"

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
	check(HasNative());

	if (StepStrideRef->Native == nullptr)
	{
		// This is the first time StepStride is used for this Sensor.
		// A quirk of AGX is that if using StepStride for a Sensor, the Sensor itself
		// should not be part of the agxSensor::Environment, but the StepStride should.
		// Instead, the Sensor should be added to the StepStride object (agxSensor::SystemNode).
		StepStrideRef->Native = new agxSensor::SensorGroupStepStride();
		auto SensorAGX = NativeRef->Native;
		if (auto Env = SensorAGX->getEnvironment())
		{
			Env->remove(SensorAGX);
			Env->add(StepStrideRef->Native);
		}

		StepStrideRef->Native->add(SensorAGX);
	}

	StepStrideRef->Native->setStride(Stride);
}

uint32 FSensorBarrier::GetStepStride() const
{
	check(HasNative());

	if (StepStrideRef->Native == nullptr)
		return 1; // This is the effective "default" when not using StepStride.

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
