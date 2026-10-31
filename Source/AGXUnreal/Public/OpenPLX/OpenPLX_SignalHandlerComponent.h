// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "OpenPLX/OpenPLXIMUOutputView.h"
#include "OpenPLX/OpenPLXLidarOutputView.h"
#include "OpenPLX/OpenPLXSignalHandler.h"

// Unreal Engine includes.
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "OpenPLX/OpenPLX_Inputs.h"
#include "OpenPLX/OpenPLX_Outputs.h"

#include "OpenPLX_SignalHandlerComponent.generated.h"

struct FAGX_ImportContext;
struct FOpenPLX_SignalHandlerNativeAddresses;

/**
 * The signal handler Component is used send and receive OpenPLX Signals. It keeps track of all
 * Inputs and Outputs available in the model.
 *
 * Send/receive functions convert to/from Unreal units and coordinates by default. Disable
 * Convert Units to pass model values unchanged. Unknown
 * effective signal types require conversion to be disabled; the value layout must still match.
 * Sensor output views retain this choice for subsequent reads.
 */
UCLASS(
	ClassGroup = "OpenPLX", Category = "OpenPLX", Meta = (BlueprintSpawnableComponent),
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
	 * @param bConvertUnits Convert units and coordinates; false passes model values unchanged.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool SendReal(const FOpenPLX_Input& Input, double Value, bool bConvertUnits = true);

	/**
	 * Uses the Name Or Alias to get an Input and use that to send a Signal of Real type.
	 * Internally calls the 'GetInput' function to match the given Name or Alias string with the
	 * Input to use.
	 * @param bConvertUnits Convert units and coordinates; false passes model values unchanged.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool SendRealByName(FName NameOrAlias, double Value, bool bConvertUnits = true);

	/**
	 * Uses the given Output to receive a Signal of Real type.
	 * @param bConvertUnits Convert units and coordinates; false passes model values unchanged.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool ReceiveReal(const FOpenPLX_Output& Output, double& OutValue, bool bConvertUnits = true);

	/**
	 * Uses the Name Or Alias to get an Output and use that to receive a Signal of Real type.
	 * Internally calls the 'GetOutput' function to match the given Name or Alias string with the
	 * Output to use.
	 * @param bConvertUnits Convert units and coordinates; false passes model values unchanged.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool ReceiveRealByName(FName NameOrAlias, double& Value, bool bConvertUnits = true);

	/**
	 * Uses the given Input to send a Signal of Real Range type with the given Value.
	 * @param bConvertUnits Convert units and coordinates; false passes model values unchanged.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool SendRangeReal(const FOpenPLX_Input& Input, FVector2D Value, bool bConvertUnits = true);

	/**
	 * Uses the Name Or Alias to get an Input and use that to send a Signal of Real Range type.
	 * Internally calls the 'GetInput' function to match the given Name or Alias string with the
	 * Input to use.
	 * @param bConvertUnits Convert units and coordinates; false passes model values unchanged.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool SendRangeRealByName(FName NameOrAlias, FVector2D Value, bool bConvertUnits = true);

	/**
	 * Uses the given Output to receive a Signal of Real Range type.
	 * @param bConvertUnits Convert units and coordinates; false passes model values unchanged.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool ReceiveRangeReal(
		const FOpenPLX_Output& Output, FVector2D& OutValue, bool bConvertUnits = true);

	/**
	 * Uses the Name Or Alias to get an Output and use that to receive a Signal of Real Range type.
	 * Internally calls the 'GetOutput' function to match the given Name or Alias string with the
	 * Output to use.
	 * @param bConvertUnits Convert units and coordinates; false passes model values unchanged.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool ReceiveRangeRealByName(FName NameOrAlias, FVector2D& OutValue, bool bConvertUnits = true);

	/**
	 * Uses the given Input to send a Signal of Vector2 type with the given Value.
	 * @param bConvertUnits Convert units and coordinates; false passes model values unchanged.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool SendVector2(const FOpenPLX_Input& Input, FVector2D Value, bool bConvertUnits = true);

	/**
	 * Uses the Name Or Alias to get an Input and use that to send a Signal of Vector2 type.
	 * Internally calls the 'GetInput' function to match the given Name or Alias string with the
	 * Input to use.
	 * @param bConvertUnits Convert units and coordinates; false passes model values unchanged.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool SendVector2ByName(FName NameOrAlias, FVector2D Value, bool bConvertUnits = true);

	/**
	 * Uses the given Output to receive a Signal of Vector2 type.
	 * @param bConvertUnits Convert units and coordinates; false passes model values unchanged.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool ReceiveVector2(
		const FOpenPLX_Output& Output, FVector2D& OutValue, bool bConvertUnits = true);

	/**
	 * Uses the Name Or Alias to get an Output and use that to receive a Signal of Vector2 type.
	 * Internally calls the 'GetOutput' function to match the given Name or Alias string with the
	 * Output to use.
	 * @param bConvertUnits Convert units and coordinates; false passes model values unchanged.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool ReceiveVector2ByName(FName NameOrAlias, FVector2D& OutValue, bool bConvertUnits = true);

	/**
	 * Uses the given Input to send a Signal of Vector type with the given Value.
	 * @param bConvertUnits Convert units and coordinates; false passes model values unchanged.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool SendVector(const FOpenPLX_Input& Input, FVector Value, bool bConvertUnits = true);

	/**
	 * Uses the Name Or Alias to get an Input and use that to send a Signal of Vector type.
	 * Internally calls the 'GetInput' function to match the given Name or Alias string with the
	 * Input to use.
	 * @param bConvertUnits Convert units and coordinates; false passes model values unchanged.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool SendVectorByName(FName NameOrAlias, FVector Value, bool bConvertUnits = true);

	/**
	 * Uses the given Output to receive a Signal of Vector type.
	 * @param bConvertUnits Convert units and coordinates; false passes model values unchanged.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool ReceiveVector(const FOpenPLX_Output& Output, FVector& OutValue, bool bConvertUnits = true);

	/**
	 * Uses the Name Or Alias to get an Output and use that to receive a Signal of Vector type.
	 * Internally calls the 'GetOutput' function to match the given Name or Alias string with the
	 * Output to use.
	 * @param bConvertUnits Convert units and coordinates; false passes model values unchanged.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool ReceiveVectorByName(FName NameOrAlias, FVector& OutValue, bool bConvertUnits = true);

	/**
	 * Uses the given Input to send a Signal of Integer type with the given Value.
	 * @param bConvertUnits Convert units and coordinates; false passes model values unchanged.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool SendInteger(const FOpenPLX_Input& Input, int64 Value, bool bConvertUnits = true);

	/**
	 * Uses the Name Or Alias to get an Input and use that to send a Signal of Integer type.
	 * Internally calls the 'GetInput' function to match the given Name or Alias string with the
	 * Input to use.
	 * @param bConvertUnits Convert units and coordinates; false passes model values unchanged.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool SendIntegerByName(FName NameOrAlias, int64 Value, bool bConvertUnits = true);

	/**
	 * Uses the given Output to receive a Signal of Integer type.
	 * @param bConvertUnits Convert units and coordinates; false passes model values unchanged.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool ReceiveInteger(const FOpenPLX_Output& Output, int64& OutValue, bool bConvertUnits = true);

	/**
	 * Uses the Name Or Alias to get an Output and use that to receive a Signal of Integer type.
	 * Internally calls the 'GetOutput' function to match the given Name or Alias string with the
	 * Output to use.
	 * @param bConvertUnits Convert units and coordinates; false passes model values unchanged.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool ReceiveIntegerByName(FName NameOrAlias, int64& OutValue, bool bConvertUnits = true);

	/**
	 * Uses the given Input to send a Signal of Boolean type with the given Value.
	 * @param bConvertUnits Convert units and coordinates; false passes model values unchanged.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool SendBoolean(const FOpenPLX_Input& Input, bool Value, bool bConvertUnits = true);

	/**
	 * Uses the Name Or Alias to get an Input and use that to send a Signal of Boolean type.
	 * Internally calls the 'GetInput' function to match the given Name or Alias string with the
	 * Input to use.
	 * @param bConvertUnits Convert units and coordinates; false passes model values unchanged.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool SendBooleanByName(FName NameOrAlias, bool Value, bool bConvertUnits = true);

	/**
	 * Uses the given Output to receive a Signal of Boolean type.
	 * @param bConvertUnits Convert units and coordinates; false passes model values unchanged.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool ReceiveBoolean(const FOpenPLX_Output& Output, bool& OutValue, bool bConvertUnits = true);

	/**
	 * Uses the Name Or Alias to get an Output and use that to receive a Signal of Boolean type.
	 * Internally calls the 'GetOutput' function to match the given Name or Alias string with the
	 * Output to use.
	 * @param bConvertUnits Convert units and coordinates; false passes model values unchanged.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool ReceiveBooleanByName(FName NameOrAlias, bool& OutValue, bool bConvertUnits = true);

	/**
	 * Uses the given Output to receive a Signal of Lidar Output type.
	 * @param bConvertUnits Convert units and coordinates; false passes model values unchanged.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool ReceiveLidarOutput(
		const FOpenPLX_Output& Output, FOpenPLXLidarOutputView& OutView, bool bConvertUnits = true);

	/**
	 * Uses the Name Or Alias to get an Output and use that to receive a Signal of Lidar Output
	 * type. Internally calls the 'GetOutput' function to match the given Name or Alias string with
	 * the Output to use.
	 * @param bConvertUnits Convert units and coordinates; false passes model values unchanged.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool ReceiveLidarOutputByName(
		FName NameOrAlias, FOpenPLXLidarOutputView& OutView, bool bConvertUnits = true);

	/**
	 * Uses the given Output to receive a Signal of IMU Output type.
	 * @param bConvertUnits Convert units and coordinates; false passes model values unchanged.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool ReceiveIMUOutput(
		const FOpenPLX_Output& Output, FOpenPLXIMUOutputView& OutView, bool bConvertUnits = true);

	/**
	 * Uses the Name Or Alias to get an Output and use that to receive a Signal of IMU Output type.
	 * Internally calls the 'GetOutput' function to match the given Name or Alias string with the
	 * Output to use.
	 * @param bConvertUnits Convert units and coordinates; false passes model values unchanged.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool ReceiveIMUOutputByName(
		FName NameOrAlias, FOpenPLXIMUOutputView& OutView, bool bConvertUnits = true);

	UPROPERTY(Transient)
	bool bShowDisabledOutputs {false};

	//~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	virtual TStructOnScope<FActorComponentInstanceData> GetComponentInstanceData() const override;
	//~ End UActorComponent Interface

	void CopyFrom(
		const TArray<FOpenPLX_Input>& Inputs, TArray<FOpenPLX_Output> Outputs,
		FAGX_ImportContext* Context);

	void SetNativeAddresses(const FOpenPLX_SignalHandlerNativeAddresses& Addresses);
	FOpenPLX_SignalHandlerNativeAddresses GetNativeAddresses() const;

	UFUNCTION(
		BlueprintCallable, Category = "OpenPLX",
		Meta = (DeprecatedFunction, DeprecationMessage = "Use SendReal instead."))
	bool SendRealInterface(const FOpenPLX_Input& Input, double Value, bool bConvertUnits = true);

	UFUNCTION(
		BlueprintCallable, Category = "OpenPLX",
		Meta = (DeprecatedFunction, DeprecationMessage = "Use ReceiveReal instead."))
	bool ReceiveRealInterface(
		const FOpenPLX_Output& Output, double& OutValue, bool bConvertUnits = true);

	UFUNCTION(
		BlueprintCallable, Category = "OpenPLX",
		Meta = (DeprecatedFunction, DeprecationMessage = "Use SendRangeReal instead."))
	bool SendRangeRealInterface(
		const FOpenPLX_Input& Input, FVector2D Value, bool bConvertUnits = true);

	UFUNCTION(
		BlueprintCallable, Category = "OpenPLX",
		Meta = (DeprecatedFunction, DeprecationMessage = "Use ReceiveRangeReal instead."))
	bool ReceiveRangeRealInterface(
		const FOpenPLX_Output& Output, FVector2D& OutValue, bool bConvertUnits = true);

	UFUNCTION(
		BlueprintCallable, Category = "OpenPLX",
		Meta = (DeprecatedFunction, DeprecationMessage = "Use ReceiveVector2 instead."))
	bool ReceiveVector2Interface(
		const FOpenPLX_Output& Output, FVector2D& OutValue, bool bConvertUnits = true);

	UFUNCTION(
		BlueprintCallable, Category = "OpenPLX",
		Meta = (DeprecatedFunction, DeprecationMessage = "Use SendVector2 instead."))
	bool SendVector2Interface(
		const FOpenPLX_Input& Input, FVector2D Value, bool bConvertUnits = true);

	UFUNCTION(
		BlueprintCallable, Category = "OpenPLX",
		Meta = (DeprecatedFunction, DeprecationMessage = "Use SendVector instead."))
	bool SendVectorInterface(const FOpenPLX_Input& Input, FVector Value, bool bConvertUnits = true);

	UFUNCTION(
		BlueprintCallable, Category = "OpenPLX",
		Meta = (DeprecatedFunction, DeprecationMessage = "Use ReceiveVector instead."))
	bool ReceiveVectorInterface(
		const FOpenPLX_Output& Output, FVector& OutValue, bool bConvertUnits = true);

	UFUNCTION(
		BlueprintCallable, Category = "OpenPLX",
		Meta = (DeprecatedFunction, DeprecationMessage = "Use SendInteger instead."))
	bool SendIntegerInterface(const FOpenPLX_Input& Input, int64 Value, bool bConvertUnits = true);

	UFUNCTION(
		BlueprintCallable, Category = "OpenPLX",
		Meta = (DeprecatedFunction, DeprecationMessage = "Use ReceiveInteger instead."))
	bool ReceiveIntegerInterface(
		const FOpenPLX_Output& Output, int64& OutValue, bool bConvertUnits = true);

	UFUNCTION(
		BlueprintCallable, Category = "OpenPLX",
		Meta = (DeprecatedFunction, DeprecationMessage = "Use SendBoolean instead."))
	bool SendBooleanInterface(const FOpenPLX_Input& Input, bool Value, bool bConvertUnits = true);

	UFUNCTION(
		BlueprintCallable, Category = "OpenPLX",
		Meta = (DeprecatedFunction, DeprecationMessage = "Use ReceiveBoolean instead."))
	bool ReceiveBooleanInterface(
		const FOpenPLX_Output& Output, bool& OutValue, bool bConvertUnits = true);

private:
	FOpenPLXSignalHandler SignalHandler;
};
