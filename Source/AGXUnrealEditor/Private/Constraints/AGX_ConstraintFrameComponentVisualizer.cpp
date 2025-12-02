// Copyright 2025, Algoryx Simulation AB.

#include "Constraints/AGX_ConstraintFrameComponentVisualizer.h"

// AGX Dynamics for Unreal includes.
#include "Constraints/AGX_ConstraintActor.h"
#include "Constraints/AGX_ConstraintComponent.h"
#include "Constraints/AGX_ConstraintComponentVisualizer.h"
#include "Constraints/AGX_ConstraintFrameActor.h"
#include "Constraints/AGX_ConstraintFrameComponent.h"

// Unreal Engine includes.
#include "Editor/UnrealEdEngine.h"
#include "SceneManagement.h"
#include "UnrealEdGlobals.h"

#define LOCTEXT_NAMESPACE "FAGX_ConstraintFrameComponentVisualizer"

void FAGX_ConstraintFrameComponentVisualizer::DrawVisualization(
	const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	const UAGX_ConstraintFrameComponent* ConstraintFrameComponent =
		Cast<const UAGX_ConstraintFrameComponent>(Component);
	if (ConstraintFrameComponent == nullptr)
	{
		return;
	}

	for (UAGX_ConstraintComponent* Constraint : ConstraintFrameComponent->GetConstraintUsage())
	{
		/// \note Here I would like to also draw the constraint frame degrees of freedom, i.e., the
		/// arrows that are rendered by the ConstraintComponent's render proxy. I don't know how to
		/// do that.
		FAGX_ConstraintComponentVisualizer::DrawConstraint(Constraint, View, PDI);
	}
}

#undef LOCTEXT_NAMESPACE
