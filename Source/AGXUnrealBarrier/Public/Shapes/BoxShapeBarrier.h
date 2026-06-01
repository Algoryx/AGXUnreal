// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Shapes/ShapeBarrier.h"

// Unreal Engine includes.
#include <Math/Vector.h>

#include "BoxShapeBarrier.generated.h"


USTRUCT(BlueprintType)
struct AGXUNREALBARRIER_API FBoxShapeBarrier : public FShapeBarrier
{
	GENERATED_BODY()

	FBoxShapeBarrier();
	FBoxShapeBarrier(std::shared_ptr<FGeometryAndShapeRef> Native);

	void SetHalfExtents(const FVector& NewHalfExtents);
	FVector GetHalfExtents() const;

private:
	virtual void AllocateNativeShape() override;
	virtual void ReleaseNativeShape() override;
};
