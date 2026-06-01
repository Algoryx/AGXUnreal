// Copyright 2026, Algoryx Simulation AB.

#include "Shapes/BoxShapeBarrier.h"

// AGX Dynamics for Unreal includes.
#include "BarrierOnly/AGXRefs.h"
#include "BarrierOnly/AGXTypeConversions.h"

// Unreal Engine includes.
#include "Misc/AssertionMacros.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxCollide/Box.h>
#include "EndAGXIncludes.h"

namespace
{
	agxCollide::Box* NativeBox(FBoxShapeBarrier* Barrier)
	{
		return Barrier->GetNative()->NativeShape->as<agxCollide::Box>();
	}

	const agxCollide::Box* NativeBox(const FBoxShapeBarrier* Barrier)
	{
		return Barrier->GetNative()->NativeShape->as<agxCollide::Box>();
	}
}

FBoxShapeBarrier::FBoxShapeBarrier()
	: FShapeBarrier()
{
}

FBoxShapeBarrier::FBoxShapeBarrier(std::shared_ptr<FGeometryAndShapeRef> Native)
	: FShapeBarrier(std::move(Native))
{
	check(NativeRef->NativeShape->is<agxCollide::Box>());
}

void FBoxShapeBarrier::SetHalfExtents(const FVector& HalfExtentsUnreal)
{
	check(HasNative());
	agx::Vec3 HalfExtentsAGX = ConvertDistance(HalfExtentsUnreal);
	NativeBox(this)->setHalfExtents(HalfExtentsAGX);
}

FVector FBoxShapeBarrier::GetHalfExtents() const
{
	check(HasNative());
	agx::Vec3 HalfExtentsAGX = NativeBox(this)->getHalfExtents();
	FVector HalfExtentsUnreal = ConvertDistance(HalfExtentsAGX);
	return HalfExtentsUnreal;
}

void FBoxShapeBarrier::AllocateNativeShape()
{
	check(!HasNative());
	NativeRef->NativeShape = new agxCollide::Box(agx::Vec3());
}

void FBoxShapeBarrier::ReleaseNativeShape()
{
	check(HasNative());
	NativeRef->NativeShape = nullptr;
}
