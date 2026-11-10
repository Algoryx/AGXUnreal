// Copyright 2026, Algoryx Simulation AB.

#include "Terrain/LandscapeRendering/AGX_TerrainLandscapeRendererBaseComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Terrain/AGX_Terrain.h"

void UAGX_TerrainLandscapeRendererBaseComponent::BeginPlay()
{
	Super::BeginPlay();
	if (Cast<AAGX_Terrain>(GetOwner()) == nullptr)
		UE_LOG(
			LogAGX, Warning, TEXT("Landscape renderer '%s' must be owned by an AGX Terrain actor."),
			*GetName());
}

void UAGX_TerrainLandscapeRendererBaseComponent::SetEnabled(bool bInEnabled)
{
	bEnabled = bInEnabled;
	if (!bEnabled)
		Shutdown();
}

bool UAGX_TerrainLandscapeRendererBaseComponent::GetEnabled() const
{
	return bEnabled;
}
bool UAGX_TerrainLandscapeRendererBaseComponent::IsRendererInitialized() const
{
	return bInitialized;
}

bool UAGX_TerrainLandscapeRendererBaseComponent::Initialize(AAGX_Terrain& InTerrain)
{
	if (GetOwner() != &InTerrain)
	{
		UE_LOG(LogAGX, Error, TEXT("Landscape renderer must be owned by its AGX Terrain."));
		return false;
	}
	Terrain = &InTerrain;
	bInitialized = InitializeRenderer();
	if (!bInitialized)
		ShutdownRenderer();
	return bInitialized;
}

void UAGX_TerrainLandscapeRendererBaseComponent::Update(
	const TArray<FTerrainPagerHeightSample>& Samples)
{
	if (bInitialized && bEnabled)
		UpdateRenderer(Samples);
}

void UAGX_TerrainLandscapeRendererBaseComponent::Shutdown()
{
	if (bInitialized)
		ShutdownRenderer();
	bInitialized = false;
}

void UAGX_TerrainLandscapeRendererBaseComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	Shutdown();
	Super::EndPlay(Reason);
}

#if WITH_EDITOR
void UAGX_TerrainLandscapeRendererBaseComponent::PostEditChangeChainProperty(
	FPropertyChangedChainEvent& Event)
{
	SetEnabled(bEnabled);
	Super::PostEditChangeChainProperty(Event);
}
#endif
