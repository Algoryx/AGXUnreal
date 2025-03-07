// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "OpenPLX/PLXSignalHandler.h"

// Unreal Engine includes.
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "OpenPLX/PLX_Inputs.h"
#include "OpenPLX/PLX_Outputs.h"

#include "PLX_SignalHandlerComponent.generated.h"

/**
 * Todo: Add description.
 */
UCLASS(
	ClassGroup = "OpenPLX", Category = "OpenPLX", Meta = (BlueprintSpawnableComponent),
	Hidecategories = (Cooking, Collision, LOD, Physics, Rendering, Replication))
class AGXUNREAL_API UPLX_SignalHandlerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPLX_SignalHandlerComponent();

	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool Send(const FPLX_LinearVelocity1DInput& Input, double Value);

	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool Receive(const FPLX_AngleOutput& Output, double& OutValue);

	//~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	//~ End UActorComponent Interface

private:
	FPLXSignalHandler SignalHandler;
};
