// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Constraints/AGX_ConstraintActor.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_Constraint2DofActor.generated.h"

class UAGX_Constraint2DofComponent;

UCLASS(ClassGroup = "AGX_Constraint", Abstract, NotBlueprintable)
class AGXUNREAL_API AAGX_Constraint2DofActor : public AAGX_ConstraintActor
{
	GENERATED_BODY()

public:
	AAGX_Constraint2DofActor();
	virtual ~AAGX_Constraint2DofActor();

	UAGX_Constraint2DofComponent* Get2DofComponent();
};
