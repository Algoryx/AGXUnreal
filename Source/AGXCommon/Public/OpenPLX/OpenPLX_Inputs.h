// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Error: 'USTRUCT' must not be inside preprocessor blocks, except for WITH_EDITORONLY_DATA
// #if AGXUNREAL_USE_OPENPLX
#if 0


// AGX Dynamics for Unreal includes.
#include "OpenPLX/OpenPLX_Enums.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/Class.h"

#include "OpenPLX_Inputs.generated.h"

/**
 * EXPERIMENTAL
 */
USTRUCT(BlueprintType)
struct AGXCOMMON_API FOpenPLX_Input
{
	GENERATED_BODY()

	FOpenPLX_Input() = default;

	FOpenPLX_Input(const FName& InName, const FName& InAlias, EOpenPLX_InputType InType)
		: Name(InName)
		, Alias(InAlias)
		, Type(InType)
	{
	}

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "OpenPLX")
	FName Name;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "OpenPLX")
	FName Alias;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "OpenPLX")
	EOpenPLX_InputType Type {EOpenPLX_InputType::Unsupported};
};

#endif
