// Copyright 2025, Algoryx Simulation AB.

#include "Shapes/SphereShapeBarrier.h"

// AGX Dynamics for Unreal includes.
#include "BarrierOnly/AGXRefs.h"
#include "TypeConversions.h"

// Unreal Engine includes.
#include "Misc/AssertionMacros.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxCollide/Sphere.h>
#include "EndAGXIncludes.h"

namespace
{
	agxCollide::Sphere* NativeSphere(FSphereShapeBarrier* Barrier)
	{
		return Barrier->GetNative()->NativeShape->as<agxCollide::Sphere>();
	}

	const agxCollide::Sphere* NativeSphere(const FSphereShapeBarrier* Barrier)
	{
		return Barrier->GetNative()->NativeShape->as<agxCollide::Sphere>();
	}
}

FSphereShapeBarrier::FSphereShapeBarrier()
	: FShapeBarrier()
{
}

FSphereShapeBarrier::FSphereShapeBarrier(std::shared_ptr<FGeometryAndShapeRef> Native)
	: FShapeBarrier(std::move(Native))
{
	check(GetNative()->NativeShape->is<agxCollide::Sphere>());
}

void FSphereShapeBarrier::SetRadius(float RadiusUnreal)
{
	check(HasNative());
	agx::Real RadiusAGX = ConvertDistanceToAGX(RadiusUnreal);
	NativeSphere(this)->setRadius(RadiusAGX);
}

float FSphereShapeBarrier::GetRadius() const
{
	check(HasNative());
	agx::Real RadiusAGX = NativeSphere(this)->getRadius();
	float RadiusUnreal = ConvertDistanceToUnreal<float>(RadiusAGX);
	return RadiusUnreal;
}

void FSphereShapeBarrier::AllocateNativeShape()
{
	check(!HasNative());
	NativeRef->NativeShape = new agxCollide::Sphere(agx::Real());
}

void FSphereShapeBarrier::ReleaseNativeShape()
{
	check(HasNative());
	NativeRef->NativeShape = nullptr;
}
