// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Plot/PlotBarrier.h"
#include "Plot/AGX_PlotDataSeries.h"

// Unreal Engine includes.
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "AGX_PlotComponent.generated.h"

/**
 * Component used for plotting data.
 * Also features data exporting in csv format to file.
 */
UCLASS(
	ClassGroup = "AGX", Category = "AGX", Meta = (BlueprintSpawnableComponent),
	Hidecategories = (Cooking, Collision, LOD, Physics, Rendering, Replication))
class AGXUNREAL_API UAGX_PlotComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAGX_PlotComponent();

	/**
	 * Crate a new plot. If the Name is common with any previously created plot, the curves from
	 * both plots are placed within the same plot graph.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Plot")
	void CreatePlot(
		UPARAM(ref) FAGX_PlotDataSeries& SeriesX, UPARAM(ref) FAGX_PlotDataSeries& SeriesY,
		const FString& Name = TEXT("MyPlot"));

	/**
	 * Opens a plot window in the default web browser for this Plot Component.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Plot")
	void OpenPlotWindow();

	/**
	 * If set to true, the plot window is opened automatically on BeginPlay.
	 * The plot window is opened in the default web browser.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Plot")
	bool bAutoOpenPlotWindow {true};

	/**
	 * If set to true, the plot data will be written to a csv file on disk.
	 * The file name is determined by 'FileOutputName' and the location is the same as the project
	 * root.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Plot")
	bool bWriteToFile {true};

	/**
	 * Write data to file in csv format. The file location is the same as the project root.
	 * The FileOutputName can also be a relative (to the project root) or absolute file path.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Plot", Meta = (EditCondition = "bWriteToFile"))
	FString FileOutputName {TEXT("AGXUnreal")};

	/// Get the native AGX Dynamics representation of this Plot Component. Create it if necessary.
	FPlotBarrier* GetOrCreateNative();

	/// Return the native AGX Dynamics representation of this Plot Component. May return nullptr.
	FPlotBarrier* GetNative();
	const FPlotBarrier* GetNative() const;

	virtual bool HasNative() const;

	//~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	//~ End UActorComponent Interface

private:
	void CreateNative();

	// The AGX Dynamics object only exists while simulating. Initialized in
	// BeginPlay and released in EndPlay.
	FPlotBarrier NativeBarrier;
};
