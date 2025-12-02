// Copyright 2025, Algoryx Simulation AB.

#include "CollisionGroups/AGX_CollisionGroupDisablerSpriteComponent.h"

// Unreal Engine includes.
#include "Components/BillboardComponent.h"
#include "Engine/Texture2D.h"

UAGX_CollisionGroupDisablerSpriteComponent::UAGX_CollisionGroupDisablerSpriteComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
#if WITH_EDITORONLY_DATA
	bVisualizeComponent = true;
#endif
}

void UAGX_CollisionGroupDisablerSpriteComponent::OnRegister()
{
	Super::OnRegister();
#if WITH_EDITORONLY_DATA
	if (SpriteComponent)
	{
		FName NewName = MakeUniqueObjectName(
			SpriteComponent->GetOuter(), SpriteComponent->GetClass(),
			TEXT("CollisionGroupDisablerIcon"));
		SpriteComponent->Rename(*NewName.ToString(), nullptr, REN_DontCreateRedirectors);
		SpriteComponent->SetSprite(LoadObject<UTexture2D>(
			nullptr, TEXT("/AGXUnreal/Editor/Icons/collision_group_disable_64x64")));
		SpriteComponent->SetRelativeScale3D(FVector(2.0, 2.0, 2.0));
	}
#endif
}
