// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Shapes/ShapeBarrier.h"

#include "SphereShapeBarrier.generated.h"

USTRUCT(BlueprintType)
struct AGXUNREALBARRIER_API FSphereShapeBarrier : public FShapeBarrier
{
	GENERATED_BODY()

	FSphereShapeBarrier();
	FSphereShapeBarrier(std::shared_ptr<FGeometryAndShapeRef> Native);

	void SetRadius(float Radius);
	float GetRadius() const;

private:
	virtual void AllocateNativeShape() override;
	virtual void ReleaseNativeShape() override;
};
