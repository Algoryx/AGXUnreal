// Copyright 2025, Algoryx Simulation AB.

#include "Cable/AGX_CableComponentVisualizer.h"

// AGX Dynamics for Unreal includes.
#include "Cable/AGX_CableComponent.h"
#include "Utilities/AGX_SlateUtilities.h"

// Unreal Engine includes.
#include "Engine/World.h"
#include "SceneView.h"
#include "SceneManagement.h"

#define LOCTEXT_NAMESPACE "FAGX_CableComponentVisualizer"

namespace AGX_CableComponentVisualizer_helpers
{
	void DrawBodyFixedNodesWithLockedRotation(
		const UAGX_CableComponent& Cable, const FSceneView* View, FPrimitiveDrawInterface* PDI)
	{
		if (Cable.ResolutionPerUnitLength <= 0.0)
			return;

		UWorld* World = Cable.GetWorld();
		if (World != nullptr && World->IsGameWorld())
			return;

		const static FColor Color = FAGX_SlateUtilities::GetAGXColorOrange();
		const double SegLen = 1.0 / Cable.ResolutionPerUnitLength;
		for (auto& Node : Cable.RouteNodes)
		{
			if (Node.NodeType == EAGX_CableNodeType::BodyFixed && Node.LockRotationToBody)
			{
				const FTransform WorldTransform(
					Node.Frame.GetWorldRotation(), Node.Frame.GetWorldLocation(Cable));

				const FVector Start = WorldTransform.GetLocation();
				const FVector Direction = WorldTransform.GetUnitAxis(EAxis::Z);
				const FVector End = Start + Direction * SegLen;
				PDI->DrawLine(Start, End, Color, SDPG_Foreground, 1.5f);
			}
		}
	}
}

void FAGX_CableComponentVisualizer::DrawVisualization(
	const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	using namespace AGX_CableComponentVisualizer_helpers;
	const UAGX_CableComponent* Cable = Cast<const UAGX_CableComponent>(Component);
	if (Cable == nullptr || !Cable->ShouldRender())
		return;

	DrawBodyFixedNodesWithLockedRotation(*Cable, View, PDI);
}

#undef LOCTEXT_NAMESPACE
