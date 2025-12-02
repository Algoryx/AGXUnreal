// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Constraints/AGX_Constraint2DofActor.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_CylindricalConstraintActor.generated.h"

UCLASS(ClassGroup = "AGX_Constraint", Blueprintable)
class AGXUNREAL_API AAGX_CylindricalConstraintActor : public AAGX_Constraint2DofActor
{
	GENERATED_BODY()
public:
	AAGX_CylindricalConstraintActor();
	virtual ~AAGX_CylindricalConstraintActor();
};
