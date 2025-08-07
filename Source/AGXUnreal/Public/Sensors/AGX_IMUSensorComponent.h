// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/IMUBarrier.h"

// Unreal Engine includes.
#include "Components/SceneComponent.h"
#include "CoreMinimal.h"

#include "AGX_IMUSensorComponent.generated.h"

/**
 * IMU 3D Sensor Component, supporting Accelerometer, Gyroscope and Magnetometer which can be
 * individually enabled or disabled.
 */
UCLASS(
	ClassGroup = "AGX_Sensor", Category = "AGX", Meta = (BlueprintSpawnableComponent),
	Hidecategories = (Cooking, Collision, LOD, Physics, Rendering, Replication))
class AGXUNREAL_API UAGX_IMUSensorComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UAGX_IMUSensorComponent();

	FIMUBarrier NativeBarrier;
};
