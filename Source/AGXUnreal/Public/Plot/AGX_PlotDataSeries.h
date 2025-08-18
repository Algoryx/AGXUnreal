// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Plot/PlotDataSeriesBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "AGX_PlotDataSeries.generated.h"

USTRUCT(BlueprintType)
struct AGXUNREAL_API FAGX_PlotDataSeries
{
	GENERATED_BODY()

	FAGX_PlotDataSeries() = default;

	UPROPERTY(EditAnywhere, Category = "AGX Plot")
	FString Label {TEXT("Label")};

	void Write(float Data);

	bool HasNative() const;

	// We must provide operator = because the Unreal framework will attempt to invoke it.
	FAGX_PlotDataSeries& operator=(const FAGX_PlotDataSeries& Other);
	FAGX_PlotDataSeries(const FAGX_PlotDataSeries& Other);

	friend class UAGX_PlotComponent;

private:
	FPlotDataSeriesBarrier NativeBarrier;
};

/**
 * This class acts as an API that exposes functions of FAGX_PlotDataSeries in Blueprints.
 */
UCLASS()
class AGXUNREAL_API UAGX_PlotDataSeries_FL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "AGX Plot")
	static void Write(UPARAM(ref) FAGX_PlotDataSeries& Series, float Data)
	{
		Series.Write(Data);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Plot")
	static void ClearData(UPARAM(ref) FAGX_PlotDataSeries& Series)
	{
		Series.ClearData();
	}
};
