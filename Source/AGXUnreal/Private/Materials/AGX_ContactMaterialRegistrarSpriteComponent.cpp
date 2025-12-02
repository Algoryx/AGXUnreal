// Copyright 2025, Algoryx Simulation AB.

#include "Materials/AGX_ContactMaterialRegistrarSpriteComponent.h"

// Unreal Engine includes.
#include "Components/BillboardComponent.h"
#include "Engine/Texture2D.h"

UAGX_ContactMaterialRegistrarSpriteComponent::UAGX_ContactMaterialRegistrarSpriteComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
#if WITH_EDITORONLY_DATA
	bVisualizeComponent = true;
#endif
}

void UAGX_ContactMaterialRegistrarSpriteComponent::OnRegister()
{
	Super::OnRegister();
#if WITH_EDITORONLY_DATA
	if (SpriteComponent)
	{
		FName NewName = MakeUniqueObjectName(
			SpriteComponent->GetOuter(), SpriteComponent->GetClass(),
			TEXT("ContactMaterialRegistrarIcon"));
		SpriteComponent->Rename(*NewName.ToString(), nullptr, REN_DontCreateRedirectors);
		SpriteComponent->SetSprite(LoadObject<UTexture2D>(
			nullptr, TEXT("/AGXUnreal/Editor/Icons/contact_material_register_64x64")));
		SpriteComponent->SetRelativeScale3D(FVector(2.0, 2.0, 2.0));
	}
#endif
}
