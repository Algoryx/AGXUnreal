// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Constraints/AGX_ConstraintActor.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_Constraint1DofActor.generated.h"

class UAGX_Constraint1DofComponent;

UCLASS(ClassGroup = "AGX_Constraint", Abstract, NotBlueprintable)
class AGXUNREAL_API AAGX_Constraint1DofActor : public AAGX_ConstraintActor
{
	GENERATED_BODY()

public:
	/// \todo Can this constructor be removed?
	/// Error message is
	///
	/// error: static_assert failed
	/// "You have to define AAGX_Constraint1DofActor::AAGX_Constraint1DofActor() or
	/// AAGX_Constraint1DofActor::AAGX_Constraint1DofActor(const FObjectInitializer&).
	/// This is required by UObject system to work correctly."
	AAGX_Constraint1DofActor();
	virtual ~AAGX_Constraint1DofActor();

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint 1DOF Actor")
	UAGX_Constraint1DofComponent* Get1DofComponent();
};
