// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal include.
#include "AGX_Frame.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_Edge.generated.h"

class USceneComponent;

USTRUCT(BlueprintType)
struct AGXUNREAL_API FAGX_Edge
{
	GENERATED_BODY()

	UPROPERTY(
		EditAnywhere, BlueprintReadWrite, Category = "AGX Edge")
	FAGX_Frame Start;

	UPROPERTY(
		EditAnywhere, BlueprintReadWrite, Category = "AGX Edge")
	FAGX_Frame End;

	FTwoVectors GetLocationsRelativeTo(const USceneComponent& Component) const;

	/**
	 * Get the start and end locations expressed as offsets in the given Component's local
	 * coordinate system.
	 *
	 * If a frame, Start or End, does not have a valid parent Component, then FallbackParent is
	 * used instead for that frame.
	 */
	FTwoVectors GetLocationsRelativeTo(
		const USceneComponent& Component, const USceneComponent& FallbackParent) const;
};
