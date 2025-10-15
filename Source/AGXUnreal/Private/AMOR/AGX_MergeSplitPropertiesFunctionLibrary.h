// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AMOR/MergeSplitPropertiesBarrier.h"
#include "Utilities/AGX_BarrierUtilities.h"
#include "AGX_LogCategory.h"

// Unreal Engine includes.
#include "Kismet/BlueprintFunctionLibrary.h"

#include "AGX_MergeSplitPropertiesFunctionLibrary.generated.h"

/**
 * Blueprint Function Library that exposes the Merge Split Properties Barrier member functions to
 * Blueprint script.
 */
UCLASS()
class AGXUNREAL_API UMergeSplitProperties_FL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintPure, Category = "AGX MergeSplit Properties")
	static bool HasNative(const FMergeSplitPropertiesBarrier& Properties)
	{
		return Properties.HasNative();
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Merge Split Properties")
	static void SetEnableMerge(
		UPARAM(Ref) FMergeSplitPropertiesBarrier& Properties, bool bEnableMerge)
	{
		AGX_BARRIER_BP_SET_PROPERTY_BOOL(Properties, EnableMerge);
	}

	UFUNCTION(BlueprintPure, Category = "AGX Merge Split Properties")
	static bool GetEnableMerge(const FMergeSplitPropertiesBarrier& Properties)
	{
		AGX_BARRIER_BP_GET_PROPERTY_BOOL(Properties, EnableMerge, false);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Merge Split Properties")
	static void SetEnableSplit(
		UPARAM(Ref) FMergeSplitPropertiesBarrier& Properties, bool bEnableSplit)
	{
		AGX_BARRIER_BP_SET_PROPERTY_BOOL(Properties, EnableSplit);
	}

	UFUNCTION(BlueprintPure, Category = "AGX Merge Split Properties")
	static bool GetEnableSplit(const FMergeSplitPropertiesBarrier& Properties)
	{
		AGX_BARRIER_BP_GET_PROPERTY_BOOL(Properties, EnableSplit, false);
	}

	// TODO Add setters and getters for the thresholds once those has been exposed to Blueprint.
};
