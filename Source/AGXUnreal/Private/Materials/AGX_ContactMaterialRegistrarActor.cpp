// Copyright 2025, Algoryx Simulation AB.

#include "Materials/AGX_ContactMaterialRegistrarActor.h"

// AGX Dynamics for Unreal includes.
#include "AGX_CustomVersion.h"
#include "Materials/AGX_ContactMaterialRegistrarComponent.h"
#include "Materials/AGX_ContactMaterialRegistrarSpriteComponent.h"

#define LOCTEXT_NAMESPACE "AAGX_ContactMaterialRegistrarActor"

AAGX_ContactMaterialRegistrarActor::AAGX_ContactMaterialRegistrarActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SpriteComponent = CreateDefaultSubobject<UAGX_ContactMaterialRegistrarSpriteComponent>(
		USceneComponent::GetDefaultSceneRootVariableName());
	RootComponent = SpriteComponent;

	ContactMaterialRegistrarComponent =
		CreateDefaultSubobject<UAGX_ContactMaterialRegistrarComponent>(
			TEXT("AGX_ContactMaterialRegistrar"));
}

void AAGX_ContactMaterialRegistrarActor::Serialize(FArchive& Archive)
{
	Super::Serialize(Archive);
	Archive.UsingCustomVersion(FAGX_CustomVersion::GUID);

	if (SpriteComponent == nullptr && RootComponent == nullptr &&
		ShouldUpgradeTo(Archive, FAGX_CustomVersion::TerrainCGDisablerCMRegistrarViewporIcons))
	{
		SpriteComponent = CreateDefaultSubobject<UAGX_ContactMaterialRegistrarSpriteComponent>(
			USceneComponent::GetDefaultSceneRootVariableName());
		RootComponent = SpriteComponent;
	}
}

#undef LOCTEXT_NAMESPACE
