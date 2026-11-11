// Copyright 2026, Algoryx Simulation AB.

#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/Actor.h"
#include "Distributed/AGX_DistributedSpawnRequest.h"
#include "AGX_DistributedGarageWidget.generated.h"
class UComboBoxString;
class UTextBlock;
class UAGX_DistributedAsset;
class AAGX_DistributedSpawnPoint;

/** Example runtime UI: choose an asset and a designated garage location. */
UCLASS(Blueprintable)
class AGXUNREAL_API UAGX_DistributedGarageWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Distributed")
	TArray<TObjectPtr<UAGX_DistributedAsset>> Assets;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Distributed")
	bool bRequestSimulationAuthority {true};
	UPROPERTY(BlueprintAssignable, Category = "AGX Distributed")
	FAGX_DistributedSpawnCompleted OnSpawnCompleted;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;

private:
	UPROPERTY(Transient) TObjectPtr<UComboBoxString> AssetPicker;
	UPROPERTY(Transient) TObjectPtr<UComboBoxString> LocationPicker;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> Status;
	UPROPERTY(Transient) TArray<TObjectPtr<AAGX_DistributedSpawnPoint>> Locations;
	UFUNCTION() void SpawnSelected();
	void Completed(const FAGX_DistributedSpawnResult& Result);
};

/** Place in a level to display the garage example without a Level Blueprint. */
UCLASS(Blueprintable)
class AGXUNREAL_API AAGX_DistributedGarageHUD : public AActor
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "AGX Distributed")
	TArray<TObjectPtr<UAGX_DistributedAsset>> Assets;
	UPROPERTY(EditAnywhere, Category = "AGX Distributed") bool bRequestSimulationAuthority { true };
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;

private:
	UPROPERTY(Transient) TObjectPtr<UAGX_DistributedGarageWidget> Widget;
	bool bPreviousCursor {false};
};
