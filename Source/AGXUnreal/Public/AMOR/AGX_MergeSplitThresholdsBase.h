// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_MergeSplitThresholdsBase.generated.h"

class FMergeSplitThresholdsBarrier;

UCLASS(ClassGroup = "AGX", Category = "AGX", Abstract)
class AGXUNREAL_API UAGX_MergeSplitThresholdsBase : public UObject
{
	GENERATED_BODY()

public:
	/*
	 * The import Guid of this Component. Only used by the AGX Dynamics for Unreal import system.
	 * Should never be assigned manually.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Dynamics Import")
	FGuid ImportGuid;

	virtual void CopyFrom(const FMergeSplitThresholdsBarrier& Barrier)
		PURE_VIRTUAL(UAGX_MergeSplitThresholdsBase::CopyFrom, );
};
