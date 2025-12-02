// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_Real.h"
#include "Terrain/AGX_ShovelExcavationSettings.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <limits>

#include "AGX_Shovel.generated.h"

class FShovelBarrier;

/**
 * @deprecated Use FAGX_ShovelComponent instead.
 */
USTRUCT()
struct AGXUNREAL_API FAGX_Shovel
{
	GENERATED_BODY()

	/**
	 * The rigid body actor that should be used as terrain shovel.
	 *
	 * Every actor MUST have the following components:
	 *
	 * Terrain Shovel Top Edge,
	 * Terrain Shovel Cut Edge,
	 * Terrain Shovel Cut Direction,
	 *
	 * in addition to the usual Rigid Body and Shape components.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Shovel")
	AActor* RigidBodyActor = nullptr;

	UPROPERTY(EditAnywhere, Category = "AGX Shovel")
	FString BodyName;

	/**
	 * Number of teeth of the Shovel.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Shovel")
	int32 NumberOfTeeth {6};

	/**
	 * The length of each Shovel tooth [cm].
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Shovel")
	FAGX_Real ToothLength {15.0};

	/**
	 * The minimum radius of each Shovel tooth [cm].
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Shovel")
	FAGX_Real MinimumToothRadius {1.5};

	/**
	 * The maximum radius of each Shovel tooth [cm].
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Shovel")
	FAGX_Real MaximumToothRadius {7.5};

	/**
	 * Extension outside the shovel bounding box where soil particle merging
	 * is forbidden [cm].
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Shovel")
	FAGX_Real NoMergeExtensionDistance {50.0};

	/**
	 * The minimum submerged cutting edge length fraction that generates submerged cutting.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Shovel",
		Meta = (ClampMin = "0.0", UIMin = "0.0", ClampMax = "1.0", UIMax = "1.0"))
	FAGX_Real MinimumSubmergedContactLengthFraction {0.5};

	/**
	 * Vertical distance under the blade cutting edge that the soil is allowed
	 * to instantly merge up to [cm].
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Shovel")
	FAGX_Real VerticalBladeSoilMergeDistance {0.0};

	/**
	 * Sets the dead-load limit where secondary separation will start to activate where the forward
	 * direction starts to change according to the virtual separation plate created by the material
	 * inside the shovel.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Shovel",
		Meta = (ClampMin = "0.0", UIMin = "0.0", ClampMax = "1.0", UIMax = "1.0"))
	FAGX_Real SecondarySeparationDeadloadLimit {0.8};

	/**
	 * Set the vertical penetration depth threshold for when the shovel tooth for penetration
	 * resistance should reach full effectiveness. The penetration depth is defined as the vertical
	 * distance between the tip of a shovel tooth and the surface position of the height field. The
	 * penetration resistance will increase from a baseline of 10% until maximum effectiveness is
	 * reached when the vertical penetration depth of the shovel reaches the specified value [cm].
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Shovel")
	FAGX_Real PenetrationDepthThreshold {50.0};

	/**
	 * Linear scaling coefficient for the penetration force that the terrain will
	 * generated on this shovel.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Shovel")
	FAGX_Real PenetrationForceScaling {1.0};

	/**
	 * The maximum limit on penetration force that the terrain will generate on this shovel [N].
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Shovel")
	FAGX_Real MaximumPenetrationForce {std::numeric_limits<double>::infinity()};

	/**
	 * The max distance from the Shovel at which new Terrain Tiles is guaranteed to be loaded [cm].
	 * Only relevant when using Terrain Paging.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Shovel")
	FAGX_Real RequiredRadius {600.f};

	/**
	 * The max distance from the Shovel at which new Terrain Tiles will be preloaded [cm].
	 * Only relevant when using Terrain Paging.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Shovel")
	FAGX_Real PreloadRadius {1000.f};

	/**
	 * Determines if shovel <-> terrain contact should always be removed.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Shovel")
	bool AlwaysRemoveShovelContacts = false;

	/**
	 * Whether or not to override the default value (CuttingEdgeLength/10.0) for
	 * ContactRegionThreshold.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Shovel")
	bool bOverrideContactRegionThreshold = false;

	/**
	 * The starting distance threshold from the shovel planes where regular
	 * geometry contacts between the shovel underside and the terrain are
	 * allowed to be created [cm].
	 *
	 * Contacts that are not past the distance threshold will be filtered away.
	 * The value of this property will not be used if
	 * OverrideContactRegionThreshold is false. In that case AGX Dynamics
	 * automatically uses the value: CuttingEdgeLength/10.0.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Shovel",
		Meta = (EditCondition = "bOverrideContactRegionThreshold"))
	FAGX_Real ContactRegionThreshold = 10;

	UPROPERTY(EditAnywhere, Category = "AGX Shovel")
	FAGX_ShovelExcavationSettings PrimaryExcavationSettings;

	UPROPERTY(EditAnywhere, Category = "AGX Shovel")
	FAGX_ShovelExcavationSettings DeformBackExcavationSettings;

	UPROPERTY(EditAnywhere, Category = "AGX Shovel")
	FAGX_ShovelExcavationSettings DeformRightExcavationSettings;

	UPROPERTY(EditAnywhere, Category = "AGX Shovel")
	FAGX_ShovelExcavationSettings DeformLeftExcavationSettings;

	static void UpdateNativeShovelProperties(
		FShovelBarrier& ShovelBarrier, const FAGX_Shovel& Shovel);
};
