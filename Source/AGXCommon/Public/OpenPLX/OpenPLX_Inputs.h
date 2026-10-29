// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "OpenPLX/OpenPLX_Enums.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/Class.h"

#include "OpenPLX_Inputs.generated.h"

/**
 * Describes an OpenPLX input by its model name, Signal Interface alias and effective signal type.
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

	/** Full name of the input in the OpenPLX model. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "OpenPLX")
	FName Name;

	/** Signal Interface alias, if present. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "OpenPLX")
	FName Alias;

	/** Effective signal type. Resolved from targets for virtual inputs; Unknown if ambiguous. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "OpenPLX")
	EOpenPLX_InputType Type {EOpenPLX_InputType::Unsupported};

	/** Virtual control operation, or None for an ordinary input. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "OpenPLX")
	EOpenPLX_VirtualType VirtualType {EOpenPLX_VirtualType::None};

	/** Field layout, such as a scalar, range or vector. Unspecified derives the layout from Type. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "OpenPLX")
	EOpenPLX_ValueLayout ValueLayout {EOpenPLX_ValueLayout::Unspecified};
};
