// Copyright 2025, Algoryx Simulation AB.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "AGX_ConstraintFrameComponent.generated.h"

class UAGX_ConstraintComponent;

/**
 * Component owned by every Constraint Frame Actor so that component features can be used.
 * For example, enables the usage of a Component Visualizer, so that helpful graphics
 * can be shown in the Level Editor Viewport when editing the constraint frame.
 *
 * @see FAGX_ConstraintFrameComponentVisualizer
 *
 */
UCLASS(Category = "AGX", ClassGroup = "AGX", NotPlaceable, Meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_ConstraintFrameComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UAGX_ConstraintFrameComponent();

	/**
	 * The constraint(s) that are referencing this frame.
	 *
	 * Used for convenience only, to be able to quickly access the constraint(s).
	 */
	UPROPERTY(VisibleAnywhere, Category = "AGX Constraint Frame Actor", Transient)
	TArray<UAGX_ConstraintComponent*> UsedByConstraints;

	void AddConstraintUsage(UAGX_ConstraintComponent* Constraint);

	void RemoveConstraintUsage(UAGX_ConstraintComponent* Constraint);

	const TArray<UAGX_ConstraintComponent*>& GetConstraintUsage() const;
};
