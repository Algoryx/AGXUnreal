// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/RtShapeBarrier.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/SensorRef.h"
#include "TypeConversions.h"

FRtShapeBarrier::FRtShapeBarrier()
	: NativeRef {new FRtShapeRef}
{
}

FRtShapeBarrier::FRtShapeBarrier(std::unique_ptr<FRtShapeRef> Native)
	: NativeRef(std::move(Native))
{
}

FRtShapeBarrier::FRtShapeBarrier(FRtShapeBarrier&& Other)
	: NativeRef {std::move(Other.NativeRef)}
{
	Other.NativeRef.reset(new FRtShapeRef);
}

FRtShapeBarrier::~FRtShapeBarrier()
{
}

bool FRtShapeBarrier::HasNative() const
{
	return NativeRef->Native != nullptr;
}

bool FRtShapeBarrier::AllocateNative(
	const TArray<FVector>& Vertices, const TArray<FTriIndices>& Indices)
{
	check(!HasNative());
	const agx::Vec3Vector VerticesAGX = ConvertVertices(Vertices);
	const agx::UInt32Vector IndicesAGX = ConvertIndices(Indices);
	NativeRef->Native = agxSensor::RtShape::create(VerticesAGX, IndicesAGX);
	return NativeRef->Native != nullptr;
}

FRtShapeRef* FRtShapeBarrier::GetNative()
{
	check(HasNative());
	return NativeRef.get();
}

const FRtShapeRef* FRtShapeBarrier::GetNative() const
{
	check(HasNative());
	return NativeRef.get();
}

void FRtShapeBarrier::ReleaseNative()
{
	NativeRef->Native = nullptr;
}
