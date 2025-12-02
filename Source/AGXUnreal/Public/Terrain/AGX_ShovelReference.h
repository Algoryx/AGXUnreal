// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_ComponentReference.h"

#include "AGX_ShovelReference.generated.h"

class UAGX_ShovelComponent;

USTRUCT()
struct AGXUNREAL_API FAGX_ShovelReference : public FAGX_ComponentReference
{
	GENERATED_BODY()

	FAGX_ShovelReference();

	UAGX_ShovelComponent* GetShovelComponent() const;
};
