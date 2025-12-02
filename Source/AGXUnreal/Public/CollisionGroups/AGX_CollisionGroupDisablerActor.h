// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "AGX_CollisionGroupDisablerActor.generated.h"

class UAGX_CollisionGroupDisablerComponent;
class UAGX_CollisionGroupDisablerSpriteComponent;

/**
 * Collision Group Disabler Actor is an Actor with a Collision Group Disabler Component.
 * From the Collision Group Disabler Component's detail panel, it is possible to disable
 * collision between AGX Shape Components in the world with a specific collision group
 * associated with them.
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", HideCategories = (Cooking, LOD, Replication))
class AGXUNREAL_API AAGX_CollisionGroupDisablerActor : public AActor
{
	GENERATED_BODY()

public:
	AAGX_CollisionGroupDisablerActor();

	UPROPERTY(Category = "AGX Dynamics", VisibleAnywhere, BlueprintReadOnly)
	UAGX_CollisionGroupDisablerSpriteComponent* SpriteComponent;

	UPROPERTY(Category = "AGX Dynamics", VisibleAnywhere, BlueprintReadOnly)
	UAGX_CollisionGroupDisablerComponent* CollisionGroupDisablerComponent;

private:
	void Serialize(FArchive& Archive) override;
};
