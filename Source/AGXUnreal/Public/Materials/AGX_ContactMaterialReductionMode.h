// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Materials/AGX_ContactMaterialEnums.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "AGX_ContactMaterialReductionMode.generated.h"

/**
 * Contact reduction mode properties of the AGX Contact Material.
 */
USTRUCT(BlueprintType)
struct AGXUNREAL_API FAGX_ContactMaterialReductionMode
{
	GENERATED_BODY()

public:

	bool operator==(const FAGX_ContactMaterialReductionMode& Other) const = default;

	/**
	 * Whether contact reduction should be enabled and to what extent.
	 *
	 * By using contact reduction, the number of contact points later submitted to the solver as
	 * contact constraint can be heavily reduced, hence improving performance.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Contact Material Reduction Mode")
	EAGX_ContactReductionMode Mode {EAGX_ContactReductionMode::Geometry};

	/**
	 * The Contact Reduction Level determines how aggressively the contact reduction is.
	 * Default means the default AGX Dynamics Contact Reduction Level is used.
	 * Aggressive means that many contacts will be removed, while Minimal means that few contacts
	 * will be removed. Moderate sits in between Aggressive and Minimal.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Contact Material Reduction Mode")
	EAGX_ContactReductionLevel ContactReductionLevel {
		EAGX_ContactReductionLevel::Default}; /// \todo Disable if Mode is set to 'None'.

	void Serialize(FArchive& Archive);

private:
	// Deprecated and replaced by the enum equivalent "ContactReductionLevel".
	UPROPERTY()
	uint8 BinResolution_DEPRECATED {0};
};
