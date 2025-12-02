// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_Real.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_ShovelExcavationSettings.generated.h"

/**
 * Shovel settings that apply per excavation mode, i.e. primary, back, left, and right.
 *
 * Held by AGX Shovel Properties assets.
 */
USTRUCT(BlueprintType)
struct FAGX_ShovelExcavationSettings
{
	GENERATED_BODY()

	/**
	 * Whether the Shovel excavation mode associated with these settings should be creating dynamic
	 * mass and generating force feedback or not.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Shovel Excavation Settings")
	bool bEnabled {true};

	/**
	 * Whether the Shovel excavation mode associated with these settings should create dynamic mass
	 * or not.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Shovel Excavation Settings")
	bool bEnableCreateDynamicMass {true};

	/**
	 * Whether the Shovel excavation mode associated with these settings should generate force
	 * feedback from created aggregates or not.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Shovel Excavation Settings")
	bool bEnableForceFeedback {true};
};
