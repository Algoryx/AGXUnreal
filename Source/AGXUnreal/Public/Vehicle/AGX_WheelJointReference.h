// Copyright 2025, Algoryx Simulation AB.

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
