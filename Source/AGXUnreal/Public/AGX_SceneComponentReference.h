// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_ComponentReference.h"

#include "AGX_SceneComponentReference.generated.h"

class AActor;

/**
 * A reference to a USceneComponent.
 *
 * See comment on FAGX_ComponentReference for usage instructions and limitations.
 */
USTRUCT(BlueprintType)
struct AGXUNREAL_API FAGX_SceneComponentReference : public FAGX_ComponentReference
{
	GENERATED_BODY()

	FAGX_SceneComponentReference();

	USceneComponent* GetSceneComponent() const;
};

UCLASS()
class AGXUNREAL_API UAGX_SceneComponentReference_FL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Scene Component Reference")
	static USceneComponent* GetSceneComponent(UPARAM(Ref) FAGX_SceneComponentReference& Reference)
	{
		return Reference.GetSceneComponent();
	}
};
