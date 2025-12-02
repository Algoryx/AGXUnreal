// Copyright 2025, Algoryx Simulation AB.

#include "Constraints/AGX_ConstraintFrameActor.h"

// AGX Dynamics for Unreal includes.
#include "Constraints/AGX_ConstraintFrameComponent.h"

// Unreal Engine includes.
#include "EngineUtils.h"

AAGX_ConstraintFrameActor::AAGX_ConstraintFrameActor()
{
	// Create a root SceneComponent so that this Actor has a transform
	// which can be modified in the Editor.
	{
		ConstraintFrameComponent = CreateDefaultSubobject<UAGX_ConstraintFrameComponent>(
			USceneComponent::GetDefaultSceneRootVariableName());

		ConstraintFrameComponent->Mobility = EComponentMobility::Movable;
		ConstraintFrameComponent->SetFlags(ConstraintFrameComponent->GetFlags() | RF_Transactional);

#if WITH_EDITORONLY_DATA
		ConstraintFrameComponent->bVisualizeComponent = true;
#endif

		SetRootComponent(ConstraintFrameComponent);
	}
}

void AAGX_ConstraintFrameActor::AddConstraintUsage(UAGX_ConstraintComponent* Constraint)
{
	ConstraintFrameComponent->AddConstraintUsage(Constraint);
}

void AAGX_ConstraintFrameActor::RemoveConstraintUsage(UAGX_ConstraintComponent* Constraint)
{
	ConstraintFrameComponent->RemoveConstraintUsage(Constraint);
}

const TArray<UAGX_ConstraintComponent*>& AAGX_ConstraintFrameActor::GetConstraintUsage() const
{
	return ConstraintFrameComponent->GetConstraintUsage();
}
