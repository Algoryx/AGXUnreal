// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.

// Unreal Engine includes.

#include "AGX_ExampleDriveTrainComponent.generated.h"

/**
 *
 */

UCLASS(Category = "AGX", ClassGroup = "AGX_Vehicle", Meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_ExampleDriveTrainComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAGX_ExampleDriveTrainComponent();
	virtual ~UAGX_ExampleDriveTrainComponent() = default;
};
