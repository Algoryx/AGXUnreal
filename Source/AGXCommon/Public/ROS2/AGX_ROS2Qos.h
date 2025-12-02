// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "ROS2/AGX_ROS2Enums.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/Class.h"

#include "AGX_ROS2Qos.generated.h"

USTRUCT()
struct AGXCOMMON_API FAGX_ROS2Qos
{
	GENERATED_BODY()

	/**
	 * By default the Reliability QOS settings are the same as in ROS2.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX ROS2")
	EAGX_ROS2QosReliability Reliability {EAGX_ROS2QosReliability::ReliabilityDefault};

	/**
	 * By default the Durability QOS settings are the same as in ROS2.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX ROS2")
	EAGX_ROS2QosDurability Durability {EAGX_ROS2QosDurability::DurabilityDefault};

	/**
	 * By default the History QOS settings are the same as in ROS2.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX ROS2")
	EAGX_ROS2QosHistory History {EAGX_ROS2QosHistory::HistoryDefault};

	/**
	 * HistoryDepth determines how many messages can be queued before reading, without loosing
	 * messages. By default the History Depth is the same as in ROS2.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX ROS2")
	int32 HistoryDepth {10};
};
