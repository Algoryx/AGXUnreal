// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "OpenPLX/OpenPLX_Enums.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/Class.h"

#include "OpenPLX_Outputs.generated.h"

/**
 * EXPERIMENTAL
 */
USTRUCT(BlueprintType)
struct AGXCOMMON_API FOpenPLX_Output
{
	GENERATED_BODY()

	FOpenPLX_Output() = default;
	FOpenPLX_Output(const FName& InName, const FName& InAlias, EOpenPLX_OutputType InType, bool InEnabled)
		: Name(InName)
		, Alias(InAlias)
		, Type(InType)
		, bEnabled(InEnabled)
	{
	}

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "OpenPLX")
	FName Name;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "OpenPLX")
	FName Alias;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "OpenPLX")
	EOpenPLX_OutputType Type {EOpenPLX_OutputType::Unsupported};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "OpenPLX")
	bool bEnabled {false};
};
