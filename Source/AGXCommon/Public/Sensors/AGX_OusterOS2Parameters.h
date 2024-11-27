// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_LidarEnums.h"
#include "Sensors/AGX_LidarModelParameters.h"

#include "AGX_OusterOS2Parameters.generated.h"

/**
 * Parameters used to create a Lidar Sensor Component using Lidar Model OusterOS2.
 */
UCLASS(ClassGroup = "AGX_Sensor", Category = "AGX")
class AGXCOMMON_API UAGX_OusterOS2Parameters : public UAGX_LidarModelParameters
{
	GENERATED_BODY()

public:
	/** Number of channels setting for OusterOS Lidars. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Lidar")
	EAGX_OusterOSChannelCount ChannelCount {EAGX_OusterOSChannelCount::CH_32};

	/** Channel distribution setting for OusterOS Lidars. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Lidar")
	EAGX_OusterOSBeamSpacing BeamSpacing {
		EAGX_OusterOSBeamSpacing::Uniform};

	/**
	 * Mode setting for OusterOS Lidars.
	 * The first number represents Horizontal Resolution.
	 * The second number represents frequency.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Lidar")
	EAGX_OusterOSMode Mode {EAGX_OusterOSMode::Mode_512x10};
};
