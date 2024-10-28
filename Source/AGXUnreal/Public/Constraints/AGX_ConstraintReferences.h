// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_ComponentReference.h"

// Unreal Engine includes.
#include "Kismet/BlueprintFunctionLibrary.h"

#include "AGX_ConstraintReferences.generated.h"

class UAGX_HingeConstraintComponent;

/**
 * A reference to an UAGX_HingeConstraintComponent.
 *
 * See comment in FAGX_ComponentReference for usage instructions and limitations.
 */
USTRUCT()
struct AGXUNREAL_API FAGX_HingeReference : public FAGX_ComponentReference
{
	GENERATED_BODY()

	FAGX_HingeReference();

	UAGX_HingeConstraintComponent* GetHinge() const;
};

UCLASS()
class AGXUNREAL_API UAGX_HingeReference_FL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "AGX Rigid Body Reference")
	static UAGX_HingeConstraintComponent* GetHinge(UPARAM(Ref) FAGX_HingeReference& Reference)
	{
		return Reference.GetHinge();
	}

	UFUNCTION(
		BlueprintPure, Category = "AGX Rigid Body Reference",
		Meta = (DisplayName = "Get Rigid Body", BlueprintAutocast))
	static UPARAM(DisplayName = "Rigid Body")
		UAGX_HingeConstraintComponent* CastHingeReferenceToHinge(
			UPARAM(Ref) const FAGX_HingeReference& Reference)
	{
		return Reference.GetHinge();
	}
};
