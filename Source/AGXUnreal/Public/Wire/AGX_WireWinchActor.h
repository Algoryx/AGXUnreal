// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "AGX_WireWinchActor.generated.h"

class UAGX_WireWinchComponent;

UCLASS(
	ClassGroup = "AGX", Blueprintable,
	Meta = (ToolTip = "Actor with an AGX WireWinch Component as Root Component."))
class AGXUNREAL_API AAGX_WireWinchActor : public AActor
{
	GENERATED_BODY()

public:
	AAGX_WireWinchActor();

	UPROPERTY(Category = "AGX Dynamics", VisibleAnywhere, BlueprintReadOnly)
	UAGX_WireWinchComponent* WireWinchComponent;
};
