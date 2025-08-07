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
