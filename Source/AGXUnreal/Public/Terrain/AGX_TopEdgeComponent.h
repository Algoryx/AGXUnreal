// Copyright 2025, Algoryx Simulation AB.

#pragma once

#include "CoreMinimal.h"
#include "AGX_VectorComponent.h"
#include "AGX_TopEdgeComponent.generated.h"

// This class is deprecated but can still be used. Once we are ready to really deprecated add
// 'DeprecatedClass' to the list of specifiers in UCLASS. At that point Unreal Engine will require
// a bunch of changes and, as per https://benui.ca/unreal/uclass/#deprecated:
//
// > This class is deprecated and Objects of this class will not be saved when serializing.
/**
 * @deprecated Use UAGX_ShovelComponent instead.
 */
UCLASS(
	ClassGroup = "Deprecated", Category = "AGX",
	Meta =
		(BlueprintSpawnableComponent, DeprecationMessage = "Use the AGX Shovel Component instead."))
class AGXUNREAL_API UAGX_TopEdgeComponent : public UAGX_VectorComponent
{
	GENERATED_BODY()
public:
	UAGX_TopEdgeComponent();
};
