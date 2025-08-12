// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "AGX_LogCategory.h"
#include "AGX_Real.h"
#include "AGX_RealInterface.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "AGX_RealBlueprintFunctionLibrary.generated.h"

/**
 * A library of Blueprint-callable functions that operate on FAGX_Real instances.
 *
 * It is usually not necessary to use these since most UObject subclasses that contain FAGX_Real
 * properties provide Blueprint-compatible setters and getter that operator on float instead.
 */
UCLASS()
class AGXUNREAL_API UAGX_Real_FL : public UBlueprintFunctionLibrary
{
public:
	GENERATED_BODY()

	// I would like to do something with UE_VERSION_OLDER_THAN(5, 0, 0) here, to support double
	// precision in Unreal Engine 5, but UFUNCTION cannot be inside any preprocessor block except
	// for WITH_EDITORONLY_DATA. So replace all float with double once we no longer support Unreal
	// Engine 4.

	/**
	 * Create a new AGX Real instance initialized to the given value.
	 */
	UFUNCTION(
		BlueprintCallable, BlueprintPure, Category = "AGX Real",
		Meta = (DisplayName = "Make AGX Real"))
	static FAGX_Real MakeAGXReal(float Value)
	{
		return {Value};
	}

	UFUNCTION(BlueprintPure, Category = "AGX Real")
	static void ParseReal(const FString& String, double& Float, FAGX_Real& Real)
	{
		TOptional<double> Result = FAGX_RealInterface::StaticFromString(String);
		if (!Result.IsSet())
		{
			Float = 0.0;
			Real = 0.0;
			return;
		}

		Float = *Result;
		Real = *Result;
	}

	/**
	 * Set a new value to the given AGX Real.
	 *
	 * Takes a single-precision parameter, but the value is converted to and stored in
	 * double-precision.
	 *
	 * @param Real The AGX Real instance to set a new value on. The updated value is returned.
	 * @param Value The new value to write to the give AGX Real instance.
	 * @return The same FAGX_Real, with its stored value updated.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Real")
	static FAGX_Real& SetValue(UPARAM(Ref) FAGX_Real& Real, float Value)
	{
		Real.Value = static_cast<double>(Value);
		return Real;
	}

	/**
	 * Read the value currently stored in the AGX Real.
	 *
	 * While the value is stored in double-precision, it is converted to single-precision before
	 * being returned.
	 *
	 * @param Real The AGX Real instance to read from.
	 * @return The value stored within the given AGX Real instance.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Real")
	static float GetValue(const FAGX_Real& Real)
	{
		return static_cast<float>(Real.Value);
	}

	UFUNCTION(
		BlueprintPure, meta = (BlueprintAutocast, DisplayName = "To AGX Real"),
		Category = "AGX Real")
	static FAGX_Real Conv_DoubleToAGXReal(double In)
	{
		return FAGX_Real(In);
	}

	UFUNCTION(
		BlueprintPure, meta = (BlueprintAutocast, DisplayName = "To Double"), Category = "AGX Real")
	static double Conv_AGXRealToDouble(FAGX_Real In)
	{
		return In.Value;
	}
};
