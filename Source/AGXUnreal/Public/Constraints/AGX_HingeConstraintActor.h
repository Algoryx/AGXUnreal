// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Constraints/AGX_Constraint1DofActor.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_HingeConstraintActor.generated.h"

UCLASS(ClassGroup = "AGX_Constraint", Blueprintable)
class AGXUNREAL_API AAGX_HingeConstraintActor : public AAGX_Constraint1DofActor
{
	GENERATED_BODY()

public:
	AAGX_HingeConstraintActor();
	virtual ~AAGX_HingeConstraintActor() override;
};
