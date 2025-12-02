// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "PlayRecord/AGX_PlayRecordState.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_PlayRecord.generated.h"

/**
 * EXPERIMENTAL
 *
 * This Asset can hold Recorded data stored by a Play Record Component.
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", Experimental, BlueprintType, Blueprintable)
class AGXUNREAL_API UAGX_PlayRecord : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Play Record")
	TArray<FAGX_PlayRecordState> States;
};
