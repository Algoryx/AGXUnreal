#include "Shapes/BoxShapeBarrier.h"

#include "AGXRefs.h"
#include "TypeConversions.h"

#include "Misc/AssertionMacros.h"

#include <agxCollide/Box.h>

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

FBoxShapeBarrier::FBoxShapeBarrier(std::unique_ptr<FGeometryAndShapeRef> Native)
	: FShapeBarrier(std::move(Native))
{
	check(NativeRef->NativeShape->is<agxCollide::Box>());
}

FBoxShapeBarrier::FBoxShapeBarrier(FBoxShapeBarrier&& Other)
	: FShapeBarrier(std::move(Other))
{
}


FBoxShapeBarrier::~FBoxShapeBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the
	// std::unique_ptr NativeRef's destructor must be able to see the definition,
	// not just the forward declaration, of FBoxShapeRef.
}

void FBoxShapeBarrier::SetHalfExtents(FVector HalfExtentsUnreal)
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