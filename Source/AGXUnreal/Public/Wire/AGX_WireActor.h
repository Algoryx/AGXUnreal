// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "AGX_WireActor.generated.h"

class UAGX_WireComponent;

UCLASS(
	ClassGroup = "MyAGX", Blueprintable,
	Meta = (ToolTip = "Actor with an AGX Wire Component as Root Component."))
class AGXUNREAL_API AAGX_WireActor : public AActor
{
	GENERATED_BODY()

public:
	AAGX_WireActor();

	UPROPERTY(Category = "AGX Dynamics", VisibleAnywhere, BlueprintReadOnly)
	UAGX_WireComponent* WireComponent;
};
