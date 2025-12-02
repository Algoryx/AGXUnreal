// Copyright 2025, Algoryx Simulation AB.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "AGX_ContactMaterialRegistrarActor.generated.h"

class UAGX_ContactMaterialRegistrarComponent;
class UAGX_ContactMaterialRegistrarSpriteComponent;

/**
 * Defines which AGX Contact Materials should be used by the owning level.
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", HideCategories = (Cooking, LOD, Replication))
class AGXUNREAL_API AAGX_ContactMaterialRegistrarActor : public AActor
{
	GENERATED_BODY()

public:
	AAGX_ContactMaterialRegistrarActor();

	UPROPERTY(Category = "AGX Dynamics", VisibleAnywhere, BlueprintReadOnly)
	UAGX_ContactMaterialRegistrarSpriteComponent* SpriteComponent;

	UPROPERTY(Category = "AGX Dynamics", VisibleAnywhere, BlueprintReadOnly)
	UAGX_ContactMaterialRegistrarComponent* ContactMaterialRegistrarComponent;

private:
	void Serialize(FArchive& Archive) override;
};
