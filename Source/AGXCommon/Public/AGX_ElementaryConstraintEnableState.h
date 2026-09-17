// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_ElementaryConstraintEnableState.generated.h"

/** The enable state of an Elementary Constraint, identified by its native AGX Dynamics name. */
USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_ElementaryConstraintEnableState
{
	GENERATED_BODY()

	FAGX_ElementaryConstraintEnableState() = default;

	FAGX_ElementaryConstraintEnableState(FName InName, bool bInEnabled)
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
