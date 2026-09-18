// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_ElementaryConstraintEnabledState.generated.h"

/** The enable state of an Elementary Constraint, identified by its native AGX Dynamics name. */
USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_ElementaryConstraintEnabledState
{
	GENERATED_BODY()

	FAGX_ElementaryConstraintEnabledState() = default;

	FAGX_ElementaryConstraintEnabledState(FName InName, bool bInEnabled)
		: Name(InName)
		, bEnabled(bInEnabled)
	{
	}

	/** The native AGX Dynamics name of this Elementary Constraint. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Constraint")
	FName Name;

	/** Whether this Elementary Constraint is enabled. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Constraint")
	bool bEnabled {true};
};
