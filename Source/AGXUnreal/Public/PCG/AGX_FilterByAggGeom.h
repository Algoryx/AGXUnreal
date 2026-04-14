// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "Elements/PCGFilterDataBase.h"

#include "AGX_FilterByAggGeom.generated.h"

UENUM(BlueprintType, Meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EAGX_FilterAggGeomTypes : uint8
{
	None = 0 UMETA(Hidden),

	Sphere = 1 << 0,
	Box = 1 << 1,
	Sphyl = 1 << 2,

	All = 0xFF
};

ENUM_CLASS_FLAGS(EAGX_FilterAggGeomTypes)

UCLASS(MinimalAPI, BlueprintType, ClassGroup = "AGX")
class UAGX_FilterByAggGeomSettings : public UPCGFilterDataBaseSettings
{
	GENERATED_BODY()

public:
	// ~Begin UPCGSettings interface.
#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override;
	virtual FText GetDefaultNodeTitle() const override;
	virtual FText GetNodeTooltipText() const override;

protected:
	virtual EPCGChangeType GetChangeTypeForProperty(const FName& InPropertyName) const override;
#endif
	virtual FPCGElementPtr CreateElement() const override;
	// ~End UPCGSettings interface.

public:
	UPROPERTY(
		EditAnywhere, BlueprintReadWrite, Category = "AGX",
		Meta =
			(PCG_Overridable, Bitmask, BitmaskEnum = "/Script/AGXUnreal.EAGX_FilterAggGeomTypes"))
	int32 ToInclude = static_cast<int32>(EAGX_FilterAggGeomTypes::All);

	UFUNCTION(BlueprintCallable)
	EAGX_FilterAggGeomTypes GetToInclude() const;

	UFUNCTION(BlueprintCallable)
	bool ShouldInclude(EAGX_FilterAggGeomTypes Type) const;
};

class FAGX_FilterByAggGeomElement : public IPCGElement
{
protected:
	virtual bool ExecuteInternal(FPCGContext* Context) const override;

	virtual EPCGElementExecutionLoopMode ExecutionLoopMode(
		const UPCGSettings* Settings) const override
	{
		// TODO Figure out what this does.
		return EPCGElementExecutionLoopMode::SinglePrimaryPin;
	}
};
