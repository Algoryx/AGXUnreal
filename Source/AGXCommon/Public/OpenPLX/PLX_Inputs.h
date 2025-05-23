// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "OpenPLX/PLX_Enums.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/Class.h"

#include "PLX_Inputs.generated.h"

/**
 * EXPERIMENTAL
 */
USTRUCT(BlueprintType)
struct AGXCOMMON_API FPLX_Input
{
	GENERATED_BODY()

	FPLX_Input() = default;

	FPLX_Input(const FString& InName, const FString& InAlias, EPLX_InputType InType)
		: Name(InName)
		, Alias(InAlias)
		, Type(InType)
	{
	}

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "OpenPXL")
	FString Name;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "OpenPXL")
	FString Alias;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "OpenPXL")
	EPLX_InputType Type {EPLX_InputType::Unsupported};
};
