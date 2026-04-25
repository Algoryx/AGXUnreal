// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "PCG/AGX_FilterByAggGeom.h"

// Unreal Engine includes.
#include "PCGElement.h"
#include "PCGSettings.h"

#include "AGX_SpawnShapeComponents.generated.h"

/*
 * AGX_SpawnShapeComponents is a non-cacheable, main-thread PCG side-effect node that consumes
 * component-reference Attribute Sets, reuses one PCG-managed container actor, clears its tagged AGX
 * shapes on reevaluation, and recreates AGX primitive shape components from each referenced static
 * mesh component’s simple collision.
 */

/**
 * Settings for an AGX PCG node that turns Unreal primitive collision in a Static Mesh's AggGeom
 * into live AGX Shape Components.
 *
 * This node intentionally consumes the exact Attribute Set output contract already produced by
 * Get Actor Data in "Get Component Reference" mode and filtered by AGX_FilterByAggGeom. Each row
 * is expected to contain one Component Reference attribute that resolves to a UStaticMeshComponent.
 */
UCLASS(MinimalAPI, BlueprintType, ClassGroup = "AGX")
class UAGX_SpawnShapeComponentsSettings : public UPCGSettings
{
	GENERATED_BODY()

public:
	// ~Begin UPCGSettings interface.
#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override;
	virtual FText GetDefaultNodeTitle() const override;
	virtual FText GetNodeTooltipText() const override;
	virtual EPCGSettingsType GetType() const override;

protected:
	virtual EPCGChangeType GetChangeTypeForProperty(const FName& InPropertyName) const override;
#endif
	virtual TArray<FPCGPinProperties> InputPinProperties() const override;
	virtual TArray<FPCGPinProperties> OutputPinProperties() const override;
	virtual FPCGElementPtr CreateElement() const override;
	// ~End UPCGSettings interface.

public:
	/**
	 * Which AggGeom primitive types the node should convert into AGX shapes.
	 *
	 * This is intentionally separate from AGX_FilterByAggGeom so graph authors can reuse a broader
	 * filter result but still choose a narrower spawn set in this node.
	 *
	 * Not sure if this is actually an advantage or not. Why do we need a filter node if the spawn
	 * node can do the filtering itself?
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadWrite, Category = "AGX",
		Meta =
			(PCG_Overridable, Bitmask, BitmaskEnum = "/Script/AGXUnreal.EAGX_FilterAggGeomTypes"))
	int32 ToSpawn = static_cast<int32>(EAGX_FilterAggGeomTypes::All);

	/**
	 * Base name used when the node creates its dedicated preview/debug actor.
	 *
	 * The final object name still has to be unique in the level, but using a readable base name
	 * makes it easier to inspect generated results in the outliner.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX", Meta = (PCG_Overridable))
	FString SpawnedActorName = TEXT("AGX_PCGShapeComponents");

	UFUNCTION(BlueprintCallable, Category = "AGX")
	EAGX_FilterAggGeomTypes GetToSpawn() const;

	UFUNCTION(BlueprintCallable, Category = "AGX")
	bool ShouldSpawn(EAGX_FilterAggGeomTypes Type) const;
};

class FAGX_SpawnShapeComponentsElement : public IPCGElement
{
public:
	// This element mutates the world by spawning actors/components, so it must execute where Unreal
	// allows those UObject operations and it must not be cached as pure data.
	virtual bool CanExecuteOnlyOnMainThread(FPCGContext* Context) const override
	{
		return true;
	}
	virtual bool IsCacheable(const UPCGSettings* InSettings) const override
	{
		return false;
	}

protected:
	virtual bool ExecuteInternal(FPCGContext* Context) const override;

	virtual EPCGElementExecutionLoopMode ExecutionLoopMode(
		const UPCGSettings* Settings) const override
	{
		return EPCGElementExecutionLoopMode::SinglePrimaryPin;
	}
};
