// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "PCGSettings.h"
#include "PCGElement.h"

#include "AGX_LogPoints.generated.h"

struct FPCGPoint;

/**
 * Tooltip for Log Points here.
 */
UCLASS(MinimalAPI, BlueprintType, ClassGroup="AGX")
class UAGX_LogPointsSettings : public UPCGSettings
{
public:
	GENERATED_BODY()

	// ~Begin UPCGSettings interface.
#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override;
	virtual FText GetDefaultNodeTitle() const override;
	virtual FText GetNodeTooltipText() const override;
	virtual EPCGSettingsType GetType() const override;
#endif // WITH_EDITOR

protected:
	virtual TArray<FPCGPinProperties> InputPinProperties() const override;
	virtual TArray<FPCGPinProperties> OutputPinProperties() const override;
	virtual FPCGElementPtr CreateElement() const override;
	//~End UPCGSettings interface

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX", Meta = (PCG_Overridable))
	bool bWithPrefix {false};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX", Meta = (PCG_Overridable))
	FString Prefix;
};

class FAGX_LogPointsElement : public IPCGElement
{
protected:
	virtual bool ExecuteInternal(FPCGContext* Context) const override;

private:
	void ProcessPoint(const FPCGPoint& Point, const UAGX_LogPointsSettings& Settings) const;
};
