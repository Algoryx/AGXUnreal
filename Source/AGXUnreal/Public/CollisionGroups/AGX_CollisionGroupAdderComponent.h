// Copyright 2025, Algoryx Simulation AB.

#pragma once

#include <tuple>

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "AGX_CollisionGroupAdderComponent.generated.h"

class UAGX_ShapeComponent;

// @todo Figure out how to make it impossible to create this component inside a Blueprint
// since it will not work, due to the object instances weirdness inside Blueprints.

/**
 * The Collision Group Adder Component is a helper Component that makes it easy to add
 * collision groups to all Shape Components that are attached to the same Actor as itself.
 * Note: This Component might not work inside Blueprints.
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", Meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_CollisionGroupAdderComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UAGX_CollisionGroupAdderComponent();

	// Adds all collision groups to all child shape components. Useful
	// when e.g. adding a new shape to a rigid body.
	void ForceRefreshChildShapes();

#if WITH_EDITOR

	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

#endif

	/**
	 * List of collision groups.
	 * Note: Changing this property will affect all AGX_ShapeComponent belonging
	 * to the actor that is parent to this object.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Collision Groups")
	TArray<FName> CollisionGroups;

	UPROPERTY(VisibleAnywhere, Category = "AGX Collision Groups")
	TArray<FName> CollisionGroupsLastChange;

private:
	void ApplyCollisionGroupChanges(FPropertyChangedEvent& PropertyChangedEvent);

	void ApplyChangesToChildShapes(
		UAGX_ShapeComponent* ShapeComponent, EPropertyChangeType::Type ChangeType,
		int32 ChangeIndex);
};
