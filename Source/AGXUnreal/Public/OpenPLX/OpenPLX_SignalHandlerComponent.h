// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "OpenPLX/OpenPLXSignalHandler.h"

// Unreal Engine includes.
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "OpenPLX/OpenPLX_Inputs.h"
#include "OpenPLX/OpenPLX_Outputs.h"

#include "OpenPLX_SignalHandlerComponent.generated.h"

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
class AGXUNREAL_API UOpenPLX_SignalHandlerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UOpenPLX_SignalHandlerComponent();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "OpenPLX")
	TMap<FName, FOpenPLX_Input> Inputs;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "OpenPLX")
	TMap<FName, FOpenPLX_Output> Outputs;

	/**
	 * Key is the Signal Interface alias and the value is the full name.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "OpenPLX")
	TMap<FName, FName> InputAliases;

	/**
	 * Key is the Signal Interface alias and the value is the full name.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "OpenPLX")
	TMap<FName, FName> OutputAliases;

	/**
	 * Get the Input matching the given full name or alias. Returns true if an Input was found,
	 * returns false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool GetInput(FName NameOrAlias, FOpenPLX_Input& OutInput);

	/**
	 * Get the Input matching the given type as well as a full name or alias. Returns true if an
	 * Input was found, returns false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool GetInputFromType(EOpenPLX_InputType Type, FName NameOrAlias, FOpenPLX_Input& OutInput);

	/**
	 * Get the Output matching the given full name or alias. Returns true if an Output was
	 * found, returns false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool GetOutput(FName NameOrAlias, FOpenPLX_Output& OutOutput);

	/**
	 * Get the Output matching the given type and a full name or alias. Returns true if an
	 * Output was found, returns false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool GetOutputFromType(EOpenPLX_OutputType Type, FName NameOrAlias, FOpenPLX_Output& OutOutput);

	/**
	* Uses the given Input to send a Signal of Real type with the given Value.
	*/
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool SendReal(const FOpenPLX_Input& Input, double Value);

	/**
	 * Uses the Name Or Alias to get an Input and use that to send a Signal of Real type.
	 * Internally calls the 'GetInput' function to match the given Name or Alias string with the
	 * Input to use.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool SendRealByName(FName NameOrAlias, double Value);

	/**
	 * Uses the given Output to receive a Signal of Real type.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool ReceiveReal(const FOpenPLX_Output& Output, double& OutValue);

	/**
	 * Uses the Name Or Alias to get an Output and use that to receive a Signal of Real type.
	 * Internally calls the 'GetOutput' function to match the given Name or Alias string with the
	 * Output to use.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool ReceiveRealByName(FName NameOrAlias, double& Value);

	/**
	 * Uses the given Input to send a Signal of Real Range type with the given Value.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool SendRangeReal(const FOpenPLX_Input& Input, FVector2D Value);

	/**
	 * Uses the Name Or Alias to get an Input and use that to send a Signal of Real Range type.
	 * Internally calls the 'GetInput' function to match the given Name or Alias string with the
	 * Input to use.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool SendRangeRealByName(FName NameOrAlias, FVector2D Value);

	/**
	 * Uses the given Output to receive a Signal of Real Range type.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool ReceiveRangeReal(const FOpenPLX_Output& Output, FVector2D& OutValue);

	/**
	 * Uses the Name Or Alias to get an Output and use that to receive a Signal of Real Range type.
	 * Internally calls the 'GetOutput' function to match the given Name or Alias string with the
	 * Output to use.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool ReceiveRangeRealByName(FName NameOrAlias, FVector2D& OutValue);

	/**
	 * Uses the given Input to send a Signal of Vector type with the given Value.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool SendVector(const FOpenPLX_Input& Input, FVector Value);

	/**
	 * Uses the Name Or Alias to get an Input and use that to send a Signal of Vector type.
	 * Internally calls the 'GetInput' function to match the given Name or Alias string with the
	 * Input to use.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool SendVectorByName(FName NameOrAlias, FVector Value);

	/**
	 * Uses the given Output to receive a Signal of Vector type.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool ReceiveVector(const FOpenPLX_Output& Output, FVector& OutValue);

	/**
	 * Uses the Name Or Alias to get an Output and use that to receive a Signal of Vector type.
	 * Internally calls the 'GetOutput' function to match the given Name or Alias string with the
	 * Output to use.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool ReceiveVectorByName(FName NameOrAlias, FVector& OutValue);

	/**
	 * Uses the given Input to send a Signal of Integer type with the given Value.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool SendInteger(const FOpenPLX_Input& Input, int64 Value);

	/**
	 * Uses the Name Or Alias to get an Input and use that to send a Signal of Integer type.
	 * Internally calls the 'GetInput' function to match the given Name or Alias string with the
	 * Input to use.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool SendIntegerByName(FName NameOrAlias, int64 Value);

	/**
	 * Uses the given Output to receive a Signal of Integer type.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool ReceiveInteger(const FOpenPLX_Output& Output, int64& OutValue);

	/**
	 * Uses the Name Or Alias to get an Output and use that to receive a Signal of Integer type.
	 * Internally calls the 'GetOutput' function to match the given Name or Alias string with the
	 * Output to use.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool ReceiveIntegerByName(FName NameOrAlias, int64& OutValue);

	/**
	 * Uses the given Input to send a Signal of Boolean type with the given Value.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool SendBoolean(const FOpenPLX_Input& Input, bool Value);

	/**
	 * Uses the Name Or Alias to get an Input and use that to send a Signal of Boolean type.
	 * Internally calls the 'GetInput' function to match the given Name or Alias string with the
	 * Input to use.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool SendBooleanByName(FName NameOrAlias, bool Value);

	/**
	 * Uses the given Output to receive a Signal of Boolean type.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool ReceiveBoolean(const FOpenPLX_Output& Output, bool& OutValue);

	/**
	 * Uses the Name Or Alias to get an Output and use that to receive a Signal of Boolean type.
	 * Internally calls the 'GetOutput' function to match the given Name or Alias string with the
	 * Output to use.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool ReceiveBooleanByName(FName NameOrAlias, bool& OutValue);

	UPROPERTY(Transient)
	bool bShowDisabledOutputs {false};

	//~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	//~ End UActorComponent Interface

	void CopyFrom(
		const TArray<FOpenPLX_Input>& Inputs, TArray<FOpenPLX_Output> Outputs, FAGX_ImportContext* Context);

private:
	FOpenPLXSignalHandler SignalHandler;
};
