// Copyright 2025, Algoryx Simulation AB.

#include "Shapes/CapsuleShapeBarrier.h"

// AGX Dynamics for Unreal includes.
#include "BarrierOnly/AGXRefs.h"
#include "BarrierOnly/AGXTypeConversions.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxCollide/Capsule.h>
#include "EndAGXIncludes.h"

// Unreal Engine includes.
#include "Misc/AssertionMacros.h"

namespace
{
	agxCollide::Capsule* NativeCapsule(FCapsuleShapeBarrier* Barrier)
	{
		return Barrier->GetNative()->NativeShape->as<agxCollide::Capsule>();
	}

	const agxCollide::Capsule* NativeCapsule(const FCapsuleShapeBarrier* Barrier)
	{
		return Barrier->GetNative()->NativeShape->as<agxCollide::Capsule>();
	}
}

FCapsuleShapeBarrier::FCapsuleShapeBarrier()
	: FShapeBarrier()
{
}

FCapsuleShapeBarrier::FCapsuleShapeBarrier(std::shared_ptr<FGeometryAndShapeRef> Native)
	: FShapeBarrier(std::move(Native))
{
	check(NativeRef->NativeShape->is<agxCollide::Capsule>());
}

void FCapsuleShapeBarrier::SetHeight(double Height)
{
	check(HasNative());
	NativeCapsule(this)->setHeight(ConvertDistanceToAGX(Height));
}

double FCapsuleShapeBarrier::GetHeight() const
{
	check(HasNative());
	return ConvertDistanceToUnreal<double>(NativeCapsule(this)->getHeight());
}

void FCapsuleShapeBarrier::SetRadius(double Radius)
{
	check(HasNative());
	NativeCapsule(this)->setRadius(ConvertDistanceToAGX(Radius));
}

double FCapsuleShapeBarrier::GetRadius() const
{
	check(HasNative());
	return ConvertDistanceToUnreal<double>(NativeCapsule(this)->getRadius());
}

void FCapsuleShapeBarrier::AllocateNativeShape()
{
	check(!HasNative());
	NativeRef->NativeShape = new agxCollide::Capsule(agx::Real(0.5), agx::Real(1.0));
}

void FCapsuleShapeBarrier::ReleaseNativeShape()
{
	check(HasNative());
	NativeRef->NativeShape = nullptr;
}
