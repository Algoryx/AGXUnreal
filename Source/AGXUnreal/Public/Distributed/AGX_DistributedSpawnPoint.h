// Copyright 2026, Algoryx Simulation AB.

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Distributed/AGX_DistributedSpawnRequest.h"
#include "AGX_DistributedSpawnPoint.generated.h"

class UAGX_DistributedAsset;
class UStaticMeshComponent;

/** Editor placement preview. The subsystem creates the actual entity Actor after joining. */
UCLASS(Blueprintable, ClassGroup = (AGX))
class AGXUNREAL_API AAGX_DistributedSpawnPoint : public AActor
{
	GENERATED_BODY()
public:
	AAGX_DistributedSpawnPoint();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Distributed")
	TObjectPtr<UAGX_DistributedAsset> Asset;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Distributed")
	bool bRequestSimulationAuthority {false};
	/** Turn off to use this Actor as a garage spawn location without startup spawning. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Distributed")
	bool bSpawnOnBeginPlay {true};
	/** Persistent placement identity. Regenerated on editor duplication. */
	UPROPERTY(VisibleInstanceOnly, Category = "AGX Distributed")
	FGuid PlacementId;
	UPROPERTY(BlueprintAssignable, Category = "AGX Distributed")
	FAGX_DistributedSpawnCompleted OnSpawnCompleted;
	UFUNCTION(CallInEditor, Category = "AGX Distributed") void RefreshPreview();
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void PostLoad() override;
	virtual void PostActorCreated() override;
	virtual void PostDuplicate(EDuplicateMode::Type DuplicateMode) override;

private:
	UPROPERTY(Transient) TArray<TObjectPtr<UStaticMeshComponent>> PreviewComponents;
	void Completed(const FAGX_DistributedSpawnResult& Result);
};
