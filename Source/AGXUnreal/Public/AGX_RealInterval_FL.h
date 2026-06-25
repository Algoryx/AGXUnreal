// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_RealInterval.h"

// Unreal Engine includes.
#include "Kismet/BlueprintFunctionLibrary.h"

#include "AGX_RealInterval_FL.generated.h"

/**
 * This class acts as an API that exposes functions of FAGX_RealInterval in Blueprints.
 *
 * It cannot be in AGX_RealInterval.h because that is a Barrier module and in a Barrier module we
 * cannot inherit from U-types such UBlueprintFunctionLibrary. Should we move AGX_RealInterval.h
 * and AGX_Real.h from AGXUnrealBarrier to AGXUnrealCommon?
 */
UCLASS()
class AGXUNREAL_API UAGX_RealInterval_FL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Real Interval")
	static FVector2D ToVector2(FAGX_RealInterval Interval)
	{
		return {Interval.Min, Interval.Max};
	}
};
