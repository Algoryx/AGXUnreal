// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Components/SceneComponent.h"

#include "AGX_ObserverFrameComponent.generated.h"

struct FAGX_ImportContext;
struct FObserverFrameData;

UCLASS(ClassGroup = "AGX", Category = "AGX", Meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_ObserverFrameComponent : public USceneComponent
{
	GENERATED_BODY()

public:

	void CopyFrom(const FObserverFrameData& Data, FAGX_ImportContext* Context);

	/*
	 * The import Guid of this Component. Only used by the AGX Dynamics for Unreal import system.
	 * Should never be assigned manually.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "AGX Dynamics Import Guid")
	FGuid ImportGuid;
};
