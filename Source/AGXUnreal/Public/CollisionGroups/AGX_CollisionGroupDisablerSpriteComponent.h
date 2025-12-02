// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Components/SceneComponent.h"

#include "AGX_CollisionGroupDisablerSpriteComponent.generated.h"

UCLASS(ClassGroup = "AGX", Category = "AGX", HideCategories = (Cooking, LOD, Replication))
class AGXUNREAL_API UAGX_CollisionGroupDisablerSpriteComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UAGX_CollisionGroupDisablerSpriteComponent();

protected:
	// ~Begin UActorComponent interface.
	virtual void OnRegister() override;
	// ~End UActorComponent interface.
};
