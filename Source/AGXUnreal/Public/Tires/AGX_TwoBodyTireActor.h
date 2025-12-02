// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "AGX_TwoBodyTireActor.generated.h"

class UAGX_TwoBodyTireComponent;
class UAGX_RigidBodyComponent;

/**
 * An Actor that has a Two Body Tire Component and two Rigid Body Components; a hub Rigid Body and a
 * Tire Rigid Body. Shape components may be attached to these two Rigid Bodies to build up the final
 * tire.
 */
UCLASS(
	ClassGroup = "AGX", Blueprintable,
	Meta = (BlueprintSpawnableComponent, ToolTip = "Actor with an AGX_TwoBodyTireComponent."))
class AGXUNREAL_API AAGX_TwoBodyTireActor : public AActor
{
	GENERATED_BODY()

public:
	AAGX_TwoBodyTireActor();

	/**
	 * The tire Rigid Body.
	 */
	UPROPERTY(Category = "AGX Dynamics", VisibleAnywhere, BlueprintReadOnly)
	UAGX_RigidBodyComponent* TireRigidBodyComponent;

	/**
	 * The hub Rigid Body.
	 */
	UPROPERTY(Category = "AGX Dynamics", VisibleAnywhere, BlueprintReadOnly)
	UAGX_RigidBodyComponent* HubRigidBodyComponent;

	/**
	 * The Two Body Tire Component. This component allows for specifying the physical behavior of
	 * the tire model.
	 */
	UPROPERTY(Category = "AGX Dynamics", VisibleAnywhere, BlueprintReadOnly)
	UAGX_TwoBodyTireComponent* TwoBodyTireComponent;
};
