// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "OpenPLX/OpenPLX_Enums.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/Class.h"

#include "OpenPLX_Outputs.generated.h"

/**
 * Describes an OpenPLX output by its model name, Signal Interface alias and effective signal type.
 */
USTRUCT(BlueprintType)
struct AGXCOMMON_API FOpenPLX_Output
{
	GENERATED_BODY()

	FOpenPLX_Output() = default;
	FOpenPLX_Output(
		const FName& InName, const FName& InAlias, EOpenPLX_OutputType InType, bool InEnabled)
		: Name(InName)
		, Alias(InAlias)
		, Type(InType)
		, bEnabled(InEnabled)
	{
	}

	/** Full name of the output in the OpenPLX model. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "OpenPLX")
	FName Name;

	/** Signal Interface alias, if present. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "OpenPLX")
	FName Alias;

	/** Effective signal type. Resolved from the source for virtual outputs; Unknown if ambiguous. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "OpenPLX")
	EOpenPLX_OutputType Type {EOpenPLX_OutputType::Unsupported};

	/** Virtual control operation, or None for an ordinary output. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "OpenPLX")
	EOpenPLX_VirtualType VirtualType {EOpenPLX_VirtualType::None};

	/** Field layout, such as a scalar, range or vector. Unspecified derives the layout from Type. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "OpenPLX")
	EOpenPLX_ValueLayout ValueLayout {EOpenPLX_ValueLayout::Unspecified};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "OpenPLX")
	bool bEnabled {false};
};
