// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Constraints/AGX_ConstraintActor.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_BallConstraintActor.generated.h"

class UAGX_BallConstraintComponent;

UCLASS(ClassGroup = "AGX_Constraint", Blueprintable)
class AGXUNREAL_API AAGX_BallConstraintActor : public AAGX_ConstraintActor
{
	GENERATED_BODY()

public:
	AAGX_BallConstraintActor();
	virtual ~AAGX_BallConstraintActor() override;
};
