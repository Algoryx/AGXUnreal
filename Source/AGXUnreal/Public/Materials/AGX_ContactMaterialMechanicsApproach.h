﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Materials/AGX_ContactMaterialEnums.h"
#include "AGX_ContactMaterialMechanicsApproach.generated.h"

/**
 * Contact mechanics approach properties of the AGX Contact Material.
 */
USTRUCT()
struct AGXUNREAL_API FAGX_ContactMaterialMechanicsApproach
{
	GENERATED_USTRUCT_BODY()

public:
		
	/**
	 * Whether contacts using this contact material use the new area-based approach for processing contacts, instead
	 * of the default point-based approach.
	 *
	 * If set to enabled, an approximation to the contact area will be geometrically computed for each contact
	 * involving this contact material. For each contact, its area will then be evenly distributed between its
	 * contact points. The contact compliance will be scaled with the inverse of the area for each contact point.
	 *
	 * Note that this is an experimental feature, which has been tested on contacts involving meshes and boxes, but
	 * with a cruder approximation for contacts involving spheres or capsules.
	 *
	 * Recommended for contact mechanics with a higher level of fidelity, such as grasping involving meshes or boxes.
	 */
	UPROPERTY(EditAnywhere)
	bool bUseContactAreaApproach;

	/**
	 * Minimum elastic rest length of the contact material, in meters.
	 * 
     * This is only used if the contact area approach is used if the property 'Use Contact Area Approach' is enabled.
	 */
	UPROPERTY(EditAnywhere, Meta = (ClampMin = "0", UIMin = "0", EditCondition = "bUseContactAreaApproach"))
	double MinElasticRestLength;

	/**
	 * Maximum elastic rest length of the contact material, in meters.
	 * 
     * This is only used if the contact area approach is used if the property 'Use Contact Area Approach' is enabled.
	 */
	UPROPERTY(EditAnywhere, Meta = (ClampMin = "0", UIMin = "0", EditCondition = "bUseContactAreaApproach"))
	double MaxElasticRestLength;

public:

	FAGX_ContactMaterialMechanicsApproach();

};