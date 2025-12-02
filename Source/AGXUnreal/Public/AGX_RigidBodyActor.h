// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "AGX_RigidBodyActor.generated.h"

class UAGX_RigidBodyComponent;

/**
 * An Actor that has an AGX_RigidBodyComponent as its RootComponent.
 * AGX_ShapeComponents can be added beneath the RigidBody to define the shape
 * of the body.
 */
UCLASS(
	ClassGroup = "AGX", Blueprintable,
	Meta =
		(BlueprintSpawnableComponent,
		 ToolTip = "Actor with an AGX_RigidBodyComponent as RootComponent."))
class AGXUNREAL_API AAGX_RigidBodyActor : public AActor
{
	GENERATED_BODY()

public:
	AAGX_RigidBodyActor();

	UPROPERTY(Category = "AGX Dynamics", VisibleAnywhere, BlueprintReadOnly)
	UAGX_RigidBodyComponent* RigidBodyComponent;

protected:
	virtual void BeginPlay() override;
};
