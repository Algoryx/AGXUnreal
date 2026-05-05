// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_VehicleTypes.generated.h"

/**
 * Initial Track tension mode.
 */
UENUM(BlueprintType)
enum class EAGX_TrackInitialTensionMode : uint8
{
	/** Initial Track tension [N]. */
	Force,

	/** Track initial tension due to offset node distance [cm]. */
	Distance
};

/**
* Track Initial Tension used when creating a native AGX Track.
*/
USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_TrackInitialTension
{
	GENERATED_BODY()

	/**
	 * Determines how the Value property of this Track Initial Tension is interpreted.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Track")
	EAGX_TrackInitialTensionMode Mode {EAGX_TrackInitialTensionMode::Force};

	/**
	 * Track Initial Tension value. Depending on the selected Mode, this will either be a node
	 * distance offset in cm, or a tension in Newtons.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Track")
	double Value {0.0};
};

