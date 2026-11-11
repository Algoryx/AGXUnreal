// Copyright 2026, Algoryx Simulation AB.

#pragma once
#include "CoreMinimal.h"
#include "AGX_ComponentReference.h"
#include "Components/SceneComponent.h"
#include "Distributed/AGX_DistributedSpawnRequest.h"
#include "AGX_DistributedPlacementComponent.generated.h"

class UInstancedStaticMeshComponent;

USTRUCT()
struct AGXUNREAL_API FAGX_DistributedInstanceReference : public FAGX_ComponentReference
{
	GENERATED_BODY()
	FAGX_DistributedInstanceReference();
};

USTRUCT(BlueprintType)
struct AGXUNREAL_API FAGX_DistributedPlacement
{
	GENERATED_BODY()
	/** Saved per-placement identity, independent of instance array ordering. */
	UPROPERTY(VisibleAnywhere, Category = "Placement") FGuid Id;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement", meta = (MakeEditWidget))
	FTransform Transform;
	/** Source index is only a validated rendering association, never distributed identity. */
	UPROPERTY(VisibleAnywhere, Category = "Placement") int32 SourceIndex { INDEX_NONE };
	UPROPERTY() FTransform SourceTransform;
};

/** One asset, many placements. Capture an ISM/HISM component, including PCG/painted foliage output.
 */
UCLASS(ClassGroup = (AGX), meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_DistributedPlacementComponent : public USceneComponent
{
	GENERATED_BODY()
public:
	UAGX_DistributedPlacementComponent();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Distributed")
	TObjectPtr<UAGX_DistributedAsset> Asset;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Distributed")
	bool bRequestSimulationAuthority {false};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Distributed")
	bool bManagedBySpawnBubbles {false};
	/** Shared component identity; the saved owning Actor identity distinguishes Blueprint
	 * instances. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Distributed") FGuid CollectionId;
	UPROPERTY(VisibleInstanceOnly, Category = "AGX Distributed") FGuid OwnerPlacementId;
	UPROPERTY(EditAnywhere, Category = "AGX Distributed")
	TArray<FAGX_DistributedPlacement> Placements;
	UPROPERTY(EditAnywhere, Category = "AGX Distributed") FAGX_DistributedInstanceReference Source;
	/** Capture after PCG generation/foliage painting; save the level before launching multiple
	clients. Preserves IDs for unchanged transforms. Landscape Grass is deliberately unsupported. */
	UFUNCTION(CallInEditor, Category = "AGX Distributed") void CaptureInstances();
	/** Application/PCG integration with a caller-supplied stable placement ID. Updates before
	 * activation only. */
	UFUNCTION(BlueprintCallable, Category = "AGX Distributed")
	bool AddPlacement(FGuid PlacementId, FTransform RelativeTransform);
	UPROPERTY(BlueprintAssignable, Category = "AGX Distributed")
	FAGX_DistributedSpawnCompleted OnSpawnCompleted;
	UFUNCTION(CallInEditor, Category = "AGX Distributed") void RefreshPreview();
	virtual void OnRegister() override;
	virtual void OnUnregister() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	virtual void TickComponent(
		float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* TickFunction) override;
	virtual void PostDuplicate(EDuplicateMode::Type Mode) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& Event) override;
#endif
private:
	struct FActivePlacement
	{
		FGuid RequestId;
		TWeakObjectPtr<AActor> Entity;
		bool bWanted {false};
		bool bSourceHidden {false};
		bool bFailed {false};
	};
	UPROPERTY(Transient) TArray<TObjectPtr<UInstancedStaticMeshComponent>> PreviewComponents;
	TMap<FGuid, FActivePlacement> Active;
	void EnsureIds();
	void SetSourceHidden(
		const FAGX_DistributedPlacement& Placement, FActivePlacement& State, bool bHidden);
};
