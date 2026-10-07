// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Sensors/CameraOutputBarrier.h"
#include "Sensors/CameraOutputColorBarrier.h"
#include "Utilities/AGX_BarrierUtilities.h"

// Unreal Engine includes.
#include "Kismet/BlueprintFunctionLibrary.h"

#include "AGX_CameraOutputBarrierFunctionLibrary.generated.h"

UCLASS()
class AGXUNREAL_API UCameraOutputBarrier_FL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "AGX Camera Output Barrier")
	static bool HasNative(const FCameraOutputBarrier& Output)
	{
		return Output.HasNative();
	}

	UFUNCTION(BlueprintPure, Category = "AGX Camera Output Barrier")
	static bool IsColorOutput(const FCameraOutputBarrier& Output)
	{
		return FCameraOutputColorBarrier::IsColorOutput(Output);
	}

	/**
	 * Returns true if the Camera Color output has data that has not been marked as read.
	 *
	 * If Mark As Read is true, the current unread data is marked as read. Returns false for an
	 * invalid barrier, another Camera output type, or when no unread data is available.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Camera Output Barrier")
	static bool HasUnreadData(
		UPARAM(ref) FCameraOutputBarrier& Output, bool bMarkAsRead = false)
	{
		if (Output.HasNative() == false)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Cannot check unread data on a Camera output barrier without a native "
					 "Camera output."));
			return false;
		}

		if (FCameraOutputColorBarrier::IsColorOutput(Output) == false)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Cannot check unread Camera Color output data on another Camera output "
					 "type."));
			return false;
		}

		FCameraOutputColorBarrier ColorOutput =
			FCameraOutputColorBarrier::CreateFrom(Output);
		return ColorOutput.HasUnreadData(bMarkAsRead);
	}

	UFUNCTION(BlueprintPure, Category = "AGX Camera Output Barrier")
	static FIntPoint GetResolution(const FCameraOutputBarrier& Output)
	{
		AGX_BARRIER_BP_GET_PROPERTY(Output, Resolution, FIntPoint::ZeroValue);
	}

	UFUNCTION(BlueprintPure, Category = "AGX Camera Output Barrier")
	static bool GetConstantCapture(const FCameraOutputBarrier& Output)
	{
		AGX_BARRIER_BP_GET_PROPERTY_BOOL(Output, ConstantCapture, false);
	}

	UFUNCTION(BlueprintPure, Category = "AGX Camera Output Barrier")
	static double GetFrameRate(const FCameraOutputBarrier& Output)
	{
		AGX_BARRIER_BP_GET_PROPERTY(Output, FrameRate, 0.0);
	}
};
