// Copyright 2025, Algoryx Simulation AB.

#include "Shapes/HeightFieldShapeBarrier.h"

// AGX Dynamics for Unreal includes.
#include "BarrierOnly/AGXRefs.h"
#include "BarrierOnly/TypeConversions.h"
#include "ShapeBarrierImpl.h"

namespace
{
	agxCollide::HeightField* NativeHeightField(FHeightFieldShapeBarrier* Barrier)
	{
		return Barrier->GetNative()->NativeShape->as<agxCollide::HeightField>();
	}

	const agxCollide::HeightField* NativeHeightField(const FHeightFieldShapeBarrier* Barrier)
	{
		return Barrier->GetNative()->NativeShape->as<agxCollide::HeightField>();
	}
}

FHeightFieldShapeBarrier::FHeightFieldShapeBarrier()
	: FShapeBarrier()
{
}

FHeightFieldShapeBarrier::FHeightFieldShapeBarrier(std::shared_ptr<FGeometryAndShapeRef> Native)
	: FShapeBarrier(std::move(Native))
{
	check(NativeRef->NativeShape->is<agxCollide::HeightField>());
}

void FHeightFieldShapeBarrier::AllocateNative(
	int32 NumVerticesX, int32 NumVerticesY, double SizeX, double SizeY)
{
	FShapeBarrier::AllocateNative(
		[this, NumVerticesX, NumVerticesY, SizeX, SizeY]()
		{ this->AllocateNativeHeightField(NumVerticesX, NumVerticesY, SizeX, SizeY); });
}

void FHeightFieldShapeBarrier::SetHeights(const TArray<float>& Heights)
{
	check(HasNative());

	agx::VectorPOD<agx::Real> HeightsAGX;
	HeightsAGX.reserve(static_cast<size_t>(Heights.Num()));

	for (auto& Height : Heights)
	{
		HeightsAGX.push_back(ConvertDistanceToAGX(Height));
	}

	NativeHeightField(this)->setHeights(HeightsAGX);
}

void FHeightFieldShapeBarrier::AllocateNativeHeightField(
	int32 NumVerticesX, int32 NumVerticesY, double SizeX, double SizeY)
{
	check(!HasNative());

	agx::Real SizeXAGX = ConvertDistanceToAGX(SizeX);
	agx::Real SizeYAGX = ConvertDistanceToAGX(SizeY);

	NativeRef->NativeShape = new agxCollide::HeightField(
		static_cast<size_t>(NumVerticesX), static_cast<size_t>(NumVerticesY), SizeXAGX, SizeYAGX);
}

void FHeightFieldShapeBarrier::AllocateNativeShape()
{
	checkNoEntry();
}

void FHeightFieldShapeBarrier::ReleaseNativeShape()
{
	NativeRef->NativeShape = nullptr;
}
