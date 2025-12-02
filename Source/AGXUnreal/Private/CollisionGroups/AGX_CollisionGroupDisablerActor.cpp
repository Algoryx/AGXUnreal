// Copyright 2025, Algoryx Simulation AB.

#include "CollisionGroups/AGX_CollisionGroupDisablerActor.h"

// AGX Dynamics for Unreal includes.
#include "AGX_CustomVersion.h"
#include "CollisionGroups/AGX_CollisionGroupDisablerComponent.h"
#include "CollisionGroups/AGX_CollisionGroupDisablerSpriteComponent.h"

#define LOCTEXT_NAMESPACE "AAGX_CollisionGroupDisablerActor"

AAGX_CollisionGroupDisablerActor::AAGX_CollisionGroupDisablerActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SpriteComponent = CreateDefaultSubobject<UAGX_CollisionGroupDisablerSpriteComponent>(
		USceneComponent::GetDefaultSceneRootVariableName());
	RootComponent = SpriteComponent;

	CollisionGroupDisablerComponent = CreateDefaultSubobject<UAGX_CollisionGroupDisablerComponent>(
		TEXT("AGX_CollisionGroupDisabler"));
}

void AAGX_CollisionGroupDisablerActor::Serialize(FArchive& Archive)
{
	Super::Serialize(Archive);
	Archive.UsingCustomVersion(FAGX_CustomVersion::GUID);

	if (SpriteComponent == nullptr && RootComponent == nullptr &&
		ShouldUpgradeTo(Archive, FAGX_CustomVersion::TerrainCGDisablerCMRegistrarViewporIcons))
	{
		SpriteComponent = CreateDefaultSubobject<UAGX_CollisionGroupDisablerSpriteComponent>(
			USceneComponent::GetDefaultSceneRootVariableName());
		RootComponent = SpriteComponent;
	}
}

#undef LOCTEXT_NAMESPACE
