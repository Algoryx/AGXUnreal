#pragma once

// AGXUnreal includes.
#include "Constraints/AGX_Constraint1DofActor.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_PrismaticConstraintActor.generated.h"

UCLASS(ClassGroup = "AGX", Blueprintable)
class AGXUNREAL_API AAGX_PrismaticConstraintActor : public AAGX_Constraint1DofActor
{
	GENERATED_BODY()

public:
	AAGX_PrismaticConstraintActor();
	virtual ~AAGX_PrismaticConstraintActor();
};