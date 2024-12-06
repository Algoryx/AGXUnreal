// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/Class.h"

#include "PLX_Inputs.generated.h"

USTRUCT(BlueprintType)
struct AGXCOMMON_API FPLX_Input
{
	GENERATED_BODY()

	FPLX_Input() = default;
	explicit FPLX_Input(const FString& InName)
		: Name(InName)
	{
	}

	virtual ~FPLX_Input() = default;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "OpenPXL")
	FString Name;

	virtual UScriptStruct* GetType() const
	{
		return FPLX_Input::StaticStruct();
	}
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FPLX_LinearVelocity1DInput : public FPLX_Input
{
	GENERATED_BODY()

	FPLX_LinearVelocity1DInput() = default;
	explicit FPLX_LinearVelocity1DInput(const FString& InName)
		: FPLX_Input(InName)
	{
	}

	virtual UScriptStruct* GetType() const override
	{
		return FPLX_LinearVelocity1DInput::StaticStruct();
	}
};
