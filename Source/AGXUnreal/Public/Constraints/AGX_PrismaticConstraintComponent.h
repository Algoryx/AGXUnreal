#pragma once

// AGXUnreal includes.
#include "Constraints/AGX_Constraint1DofComponent.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_PrismaticConstraintComponent.generated.h"

/**
 * Locks all degrees of freedom except for translation along the Z-axis.
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", Blueprintable, meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_PrismaticConstraintComponent : public UAGX_Constraint1DofComponent
{
	GENERATED_BODY()

public:
	UAGX_PrismaticConstraintComponent();
	virtual ~UAGX_PrismaticConstraintComponent();

protected:
	virtual void AllocateNative() override;
};