// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "AGX_SensorEnvironment.generated.h"

/**
 * This Actor is deprecated.
 *
 * Sensors are now added automatically to the SensorEnvironmentSubsystem that is created
 * automatically on Play. To access settings, go to "Edit > Project Settings... > Plugins > AGX
 * Sensor Environment" or access the AGX_SensorEnvironmentSubsystem directly from a Blueprint
 * Graph.
 */
UCLASS(ClassGroup = "Deprecated", Blueprintable, Category = "AGX")
class AGXUNREAL_API AAGX_SensorEnvironment : public AActor
{
	GENERATED_BODY()

public:
	AAGX_SensorEnvironment();

	UPROPERTY(VisibleAnywhere, Category = "Deprecated", Meta = (MultiLine = "true"))
	FString DeprecationMessage;

protected:
	virtual void BeginPlay() override;
};
