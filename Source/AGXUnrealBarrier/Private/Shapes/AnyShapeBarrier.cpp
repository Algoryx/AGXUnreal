// Copyright 2026, Algoryx Simulation AB.

#include "Shapes/AnyShapeBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "BarrierOnly/AGXRefs.h"

FAnyShapeBarrier::FAnyShapeBarrier()
	: FShapeBarrier()
{
}

FAnyShapeBarrier::FAnyShapeBarrier(std::shared_ptr<FGeometryAndShapeRef> Native)
	: FShapeBarrier(std::move(Native))
{
}

void FAnyShapeBarrier::AllocateNativeShape()
{
	UE_LOG(
		LogAGX, Error,
		TEXT("AllocateNativeShape called on an FAnyShapeBarrier. This is not supported because "
			 "this type doesn't know what type of AGX Dynamics shape to allocate."));
}

void FAnyShapeBarrier::ReleaseNativeShape()
{
	// While an AnyShapeBarrier cannot allocate a shape it can be given one, so this release
	// function must be properly implemented.
	check(HasNative());
	NativeRef->NativeShape = nullptr;
}
