// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Shapes/ShapeBarrier.h"

#include "HeightFieldShapeBarrier.generated.h"

USTRUCT(BlueprintType)
struct AGXUNREALBARRIER_API FHeightFieldShapeBarrier : public FShapeBarrier
{
	GENERATED_BODY()

	FHeightFieldShapeBarrier();
	FHeightFieldShapeBarrier(std::shared_ptr<FGeometryAndShapeRef> Native);

	void AllocateNative(int32 NumVerticesX, int32 NumVerticesY, double SizeX, double SizeY);

	void SetHeights(const TArray<float>& Heights);

private:
	virtual void AllocateNativeShape() override;
	virtual void ReleaseNativeShape() override;

	void AllocateNativeHeightField(
		int32 NumVerticesX, int32 NumVerticesY, double SizeX, double SizeY);
};
