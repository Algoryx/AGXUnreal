#pragma once

// AGXUnreal includes.
#include "Constraints/AGX_ConstraintActor.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_LockConstraintActor.generated.h"

class UAGX_LockConstraintComponent;

UCLASS(ClassGroup = "AGX", Blueprintable)
class AGXUNREAL_API AAGX_LockConstraintActor : public AAGX_ConstraintActor
{
	GENERATED_BODY()

public:
	AAGX_LockConstraintActor();
	virtual ~AAGX_LockConstraintActor() override;
};