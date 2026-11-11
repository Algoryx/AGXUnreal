// Copyright 2026, Algoryx Simulation AB.

#pragma once
#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "AGX_DistributedSpawnBubbleComponent.generated.h"

/** Attach to a moving Actor. Bubble-managed placements acquire/release distributed interests. */
UCLASS(ClassGroup = (AGX), meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_DistributedSpawnBubbleComponent : public USceneComponent
{
	GENERATED_BODY()
public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	/** Activation radius, Unreal centimeters. */
	UPROPERTY(
		EditAnywhere, BlueprintReadWrite, Category = "AGX Distributed",
		meta = (ClampMin = "0", Units = "cm"))
	double SpawnRadius {5000.0};
	/** Deactivation radius, clamped to at least SpawnRadius. */
	UPROPERTY(
		EditAnywhere, BlueprintReadWrite, Category = "AGX Distributed",
		meta = (ClampMin = "0", Units = "cm"))
	double ReleaseRadius {6000.0};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Distributed")
	bool bEnabled {true};
};
