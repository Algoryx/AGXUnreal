// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Shapes/ShapeBarrier.h"

#include "AnyShapeBarrier.generated.h"

/**
 * A Shape Barrier that may wrap any AGX Dynamics shape type.
 *
 * Cannot be used to allocate new AGX Dynamics shapes, since it doesn't know what type of shape to
 * allocate.
 */
USTRUCT(BlueprintType)
struct AGXUNREALBARRIER_API FAnyShapeBarrier : public FShapeBarrier
{
	GENERATED_BODY()

	FAnyShapeBarrier();
	FAnyShapeBarrier(std::shared_ptr<FGeometryAndShapeRef> Native);

private:
	/**
	 * Will always fail with an error message.
	 */
	virtual void AllocateNativeShape() override;

	/**
	 * Will always fail with an error message.
	 */
	virtual void ReleaseNativeShape() override;
};
