// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

class AGXUNREALBARRIER_API FAGX_ShapeUtilities
{
public:
	static bool ComputeOrientedBox(
		const TArray<FVector>& Vertices, FVector& OutHalfExtents, FTransform& OutTransform);

	static bool ComputeOrientedCylinder(
		const TArray<FVector>& Vertices, float& OutRadius, float& OutHeight,
		FTransform& OutTransform);

	static bool ComputeOrientedCapsule(
		const TArray<FVector>& Vertices, float& OutRadius, float& OutHeight,
		FTransform& OutTransform);
};
