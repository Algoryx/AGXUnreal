// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Shapes/ShapeBarrier.h"

#include "CapsuleShapeBarrier.generated.h"

USTRUCT(BlueprintType)
struct AGXUNREALBARRIER_API FCapsuleShapeBarrier : public FShapeBarrier
{
	GENERATED_BODY()

	FCapsuleShapeBarrier();
	FCapsuleShapeBarrier(std::shared_ptr<FGeometryAndShapeRef> Native);

	void SetHeight(double Height);
	double GetHeight() const;

	void SetRadius(double Height);
	double GetRadius() const;

private:
	virtual void AllocateNativeShape() override;
	virtual void ReleaseNativeShape() override;
};
