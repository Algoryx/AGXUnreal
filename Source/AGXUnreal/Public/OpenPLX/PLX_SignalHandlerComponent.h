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
 * The signal handler Component is used send and receive OpenPLX Signals. It keeps track of all
 * Inputs and Outputs available in the model.
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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "OpenPLX")
	TMap<FString, FPLX_Input> Inputs;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "OpenPLX")
	TMap<FString, FPLX_Output> Outputs;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "OpenPLX")
	TMap<FString, FString> InputAliases;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "OpenPLX")
	TMap<FString, FString> OutputAliases;

	/**
	 * Outputs the first found Input given a full or partial name or alias. In other words, partial
	 * name matching is supported. Returns true if an Input was found, returns false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool GetInput(FString NameOrAlias, FPLX_Input& OutInput);

	/**
	 * Outputs the first found Input given a type and full or partial name or alias. In other words,
	 * partial name matching is supported. Returns true if an Input was found, returns false
	 * otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool GetInputFromType(EPLX_InputType Type, FString NameOrAlias, FPLX_Input& OutInput);

	/**
	 * Outputs the first found Output given a full or partial name or alias. In other words, partial
	 * name matching is supported. Returns true if an Output was found, returns false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool GetOutput(FString NameOrAlias, FPLX_Output& OutOutput);

	/**
	 * Outputs the first found Output given a type and full or partial name or alias. In other
	 * words, partial name matching is supported. Returns true if an Output was found, returns false
	 * otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool GetOutputFromType(EPLX_OutputType Type, FString NameOrAlias, FPLX_Output& OutOutput);

	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool SendReal(const FPLX_Input& Input, double Value);

	/**
	 * Uses the Name Or Alias to get an Input and use that to send a Signal of Real type.
	 * Internally calls the 'GetInput' function to match the given Name or Alias string with the
	 * Input to use.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool SendRealByName(const FString& NameOrAlias, double Value);

	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool ReceiveReal(const FPLX_Output& Output, double& OutValue);

	/**
	 * Uses the Name Or Alias to get an Output and use that to receive a Signal of Real type.
	 * Internally calls the 'GetOutput' function to match the given Name or Alias string with the
	 * Output to use.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool ReceiveRealByName(const FString& NameOrAlias, double& Value);

	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool SendRangeReal(const FPLX_Input& Input, FVector2D Value);

	/**
	 * Uses the Name Or Alias to get an Input and use that to send a Signal of Real Range type.
	 * Internally calls the 'GetInput' function to match the given Name or Alias string with the
	 * Input to use.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool SendRangeRealByName(const FString& NameOrAlias, FVector2D Value);

	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool ReceiveRangeReal(const FPLX_Output& Output, FVector2D& OutValue);

	/**
	 * Uses the Name Or Alias to get an Output and use that to receive a Signal of Real Range type.
	 * Internally calls the 'GetOutput' function to match the given Name or Alias string with the
	 * Output to use.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool ReceiveRangeRealByName(const FString& NameOrAlias, FVector2D& OutValue);

	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool SendVector(const FPLX_Input& Input, FVector Value);

	/**
	 * Uses the Name Or Alias to get an Input and use that to send a Signal of Vector type.
	 * Internally calls the 'GetInput' function to match the given Name or Alias string with the
	 * Input to use.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool SendVectorByName(const FString& NameOrAlias, FVector Value);

	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool ReceiveVector(const FPLX_Output& Output, FVector& OutValue);

	/**
	 * Uses the Name Or Alias to get an Output and use that to receive a Signal of Vector type.
	 * Internally calls the 'GetOutput' function to match the given Name or Alias string with the
	 * Output to use.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool ReceiveVectorByName(const FString& NameOrAlias, FVector& OutValue);

	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool SendInteger(const FPLX_Input& Input, int64 Value);

	/**
	 * Uses the Name Or Alias to get an Input and use that to send a Signal of Integer type.
	 * Internally calls the 'GetInput' function to match the given Name or Alias string with the
	 * Input to use.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool SendIntegerByName(const FString& NameOrAlias, int64 Value);

	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool ReceiveInteger(const FPLX_Output& Output, int64& OutValue);

	/**
	 * Uses the Name Or Alias to get an Output and use that to receive a Signal of Integer type.
	 * Internally calls the 'GetOutput' function to match the given Name or Alias string with the
	 * Output to use.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool ReceiveIntegerByName(const FString& NameOrAlias, int64& OutValue);

	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool SendBoolean(const FPLX_Input& Input, bool Value);

	/**
	 * Uses the Name Or Alias to get an Input and use that to send a Signal of Boolean type.
	 * Internally calls the 'GetInput' function to match the given Name or Alias string with the
	 * Input to use.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool SendBooleanByName(const FString& NameOrAlias, bool Value);

	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool ReceiveBoolean(const FPLX_Output& Output, bool& OutValue);

	/**
	 * Uses the Name Or Alias to get an Output and use that to receive a Signal of Boolean type.
	 * Internally calls the 'GetOutput' function to match the given Name or Alias string with the
	 * Output to use.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool ReceiveBooleanByName(const FString& NameOrAlias, bool& OutValue);

	UPROPERTY(Transient)
	bool bShowDisabledOutputs {false};

	//~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	//~ End UActorComponent Interface

	void CopyFrom(
		const TArray<FPLX_Input>& Inputs, TArray<FPLX_Output> Outputs, FAGX_ImportContext* Context);

private:
	FPLXSignalHandler SignalHandler;
};
