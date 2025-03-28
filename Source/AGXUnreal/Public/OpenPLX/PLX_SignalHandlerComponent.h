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

struct FAGX_ImportContext;

/**
 * EXPERIMENTAL
 * 
 * Todo: Add description.
 */
UCLASS(
	ClassGroup = "OpenPLX", Category = "OpenPLX", Experimental,
	Meta = (BlueprintSpawnableComponent),
	Hidecategories = (Cooking, Collision, LOD, Physics, Rendering, Replication))
class AGXUNREAL_API UPLX_SignalHandlerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPLX_SignalHandlerComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OpenPLX")
	TMap<FString, FPLX_Input> Inputs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OpenPLX")
	TMap<FString, FPLX_Output> Outputs;

	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool SendScalar(const FPLX_Input& Input, double Value);

	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool ReceiveScalar(const FPLX_Output& Output, double& OutValue);

	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool SendVector(const FPLX_Input& Input, FVector Value);

	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool ReceiveVector(const FPLX_Output& Output, FVector& OutValue);

	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool SendInteger(const FPLX_Input& Input, int64 Value);

	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool ReceiveInteger(const FPLX_Output& Output, int64& OutValue);

	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool SendBoolean(const FPLX_Input& Input, bool Value);

	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool ReceiveBoolean(const FPLX_Output& Output, bool& OutValue);

	//~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	//~ End UActorComponent Interface

	void CopyFrom(
		const TArray<FPLX_Input>& Inputs, TArray<FPLX_Output> Outputs, FAGX_ImportContext* Context);

private:
	FPLXSignalHandler SignalHandler;
};
