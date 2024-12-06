// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/Class.h"

#include "PLX_Outputs.generated.h"

USTRUCT(BlueprintType)
struct AGXCOMMON_API FPLX_Output
{
	GENERATED_BODY()

	FPLX_Output() = default;
	explicit FPLX_Output(const FString& InName)
		: Name(InName)
	{
	}

	virtual ~FPLX_Output() = default;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "OpenPXL")
	FString Name;

	virtual UScriptStruct* GetType() const
	{
		return FPLX_Output::StaticStruct();
	}
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FPLX_AngleOutput : public FPLX_Output
{
	GENERATED_BODY()

	FPLX_AngleOutput() = default;
	explicit FPLX_AngleOutput(const FString& InName)
		: FPLX_Output(InName)
	{
	}

	virtual UScriptStruct* GetType() const override
	{
		return FPLX_AngleOutput::StaticStruct();
	}
};
