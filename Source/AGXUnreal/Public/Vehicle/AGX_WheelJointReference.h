// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_ComponentReference.h"

#include "AGX_WheelJointReference.generated.h"

class UAGX_WheelJointComponent;

USTRUCT()
struct AGXUNREAL_API FAGX_WheelJointReference : public FAGX_ComponentReference
{
	GENERATED_BODY()

	FAGX_WheelJointReference();

	UAGX_WheelJointComponent* GetWheelJointComponent() const;
};

UCLASS()
class AGXUNREAL_API UAGX_WheelJointReference_FL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "AGX Wheel Joint")
	static void SetWheelJointComponent(
		UPARAM(Ref) FAGX_WheelJointReference& Reference, UAGX_WheelJointComponent* Component);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Wheel Joint")
	static UAGX_WheelJointComponent* GetWheelJointComponent(
		UPARAM(Ref) FAGX_WheelJointReference& Reference);
};

