// Copyright 2026, Algoryx Simulation AB.

#include "Distributed/AGX_DistributedSpawnPointComponentVisualizer.h"

// AGX Dynamics for Unreal includes.
#include "Distributed/AGX_DistributedAsset.h"
#include "Distributed/AGX_DistributedSpawnPoint.h"
#include "Distributed/AGX_DistributedSpawnPointVisualizerComponent.h"
#include "Utilities/AGX_SlateUtilities.h"

// Unreal Engine includes.
#include "Engine/World.h"
#include "SceneManagement.h"

void FAGX_DistributedSpawnPointComponentVisualizer::DrawVisualization(
	const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	const auto* Visualizer = Cast<UAGX_DistributedSpawnPointVisualizerComponent>(Component);
	const auto* SpawnPoint =
		Visualizer != nullptr ? Cast<AAGX_DistributedSpawnPoint>(Visualizer->GetOwner()) : nullptr;
	if (SpawnPoint == nullptr || SpawnPoint->Asset == nullptr ||
		(SpawnPoint->GetWorld() != nullptr && SpawnPoint->GetWorld()->IsGameWorld()))
		return;

	const auto& Asset = *SpawnPoint->Asset;
	if (Asset.AssetType == EAGX_DistributedAssetType::Static ||
		!FMath::IsFinite(Asset.SimulationAuthoritySensorRadius) ||
		Asset.SimulationAuthoritySensorRadius <= 0)
		return;

	DrawWireSphere(
		PDI, FTransform(SpawnPoint->GetProximitySensorCenter()),
		FAGX_SlateUtilities::GetAGXColorOrange(), Asset.SimulationAuthoritySensorRadius, 32,
		SDPG_World);
}
