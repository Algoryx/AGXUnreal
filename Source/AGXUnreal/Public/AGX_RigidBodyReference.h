// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_ComponentReference.h"

// Unreal Engine includes.
#include "Kismet/BlueprintFunctionLibrary.h"

#include "AGX_RigidBodyReference.generated.h"

class UAGX_RigidBodyComponent;

/**
 * A reference to an UAGX_RigidBodyComponent.
 *
 * See comment in FAGX_ComponentReference for usage instructions and limitations.
 */
USTRUCT()
struct AGXUNREAL_API FAGX_RigidBodyReference : public FAGX_ComponentReference
{
	GENERATED_BODY()

	FAGX_RigidBodyReference();

	UAGX_RigidBodyComponent* GetRigidBody() const;
};

UCLASS()
class AGXUNREAL_API UAGX_RigidBodyReference_FL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "AGX Rigid Body Reference")
	static UAGX_RigidBodyComponent* GetRigidBody(UPARAM(Ref) FAGX_RigidBodyReference& Reference)
	{
		return Reference.GetRigidBody();
	}

	UFUNCTION(
		BlueprintPure, Category = "AGX Rigid Body Reference",
		Meta = (DisplayName = "Get Rigid Body", BlueprintAutocast))
	static UPARAM(DisplayName = "Rigid Body")
		UAGX_RigidBodyComponent* CastRigidBodyReferenceToRigidBody(
			UPARAM(Ref) const FAGX_RigidBodyReference& Reference)
	{
		return Reference.GetRigidBody();
	}
};
