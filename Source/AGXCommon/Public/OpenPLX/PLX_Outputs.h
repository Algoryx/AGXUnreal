// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "OpenPLX/PLX_Enums.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/Class.h"

#include "PLX_Outputs.generated.h"

/**
 * EXPERIMENTAL
 */
USTRUCT(BlueprintType)
struct AGXCOMMON_API FPLX_Output
{
	GENERATED_BODY()

	FPLX_Output() = default;
	FPLX_Output(const FString& InName, EPLX_OutputType InType, bool InEnabled)
		: Name(InName)
		, Type(InType)
		, bEnabled(InEnabled)
	{
	}

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "OpenPXL")
	FString Name;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "OpenPXL")
	EPLX_OutputType Type {EPLX_OutputType::Unsupported};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "OpenPXL")
	bool bEnabled {false};
};

