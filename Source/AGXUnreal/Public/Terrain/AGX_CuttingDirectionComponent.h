// Copyright 2025, Algoryx Simulation AB.

#pragma once

#include "CoreMinimal.h"
#include "AGX_VectorComponent.h"
#include "AGX_CuttingDirectionComponent.generated.h"

// This class is deprecated but can still be used. Once we are ready to really deprecated add
// 'DeprecatedClass' to the list of specifiers in UCLASS. At that point Unreal Engine will require
// a bunch of changes and, as per https://benui.ca/unreal/uclass/#deprecated:
//
// > This class is deprecated and Objects of this class will not be saved when serializing.
/**
 * @deprecated Use UAGX_ShovelComponent instead.
 *
 * Specifies the cutting direction of the AGX Terrain shovel.
 *
 * The direction where the penetration resistance will be active, which is
 * usually parallel to th elowest shovel plate that is used to initially
 * penetrate the soil.
 *
 * Remember to also add the Actor to the list of shovels in the AGX Terrain's
 * details panel.
 */
UCLASS(
	ClassGroup = "Deprecated", Category = "AGX",
	Meta =
		(BlueprintSpawnableComponent, DeprecationMessage = "Use the AGX Shovel Component instead."))
class AGXUNREAL_API UAGX_CuttingDirectionComponent : public UAGX_VectorComponent
{
	GENERATED_BODY()
public:
	UAGX_CuttingDirectionComponent();
};
