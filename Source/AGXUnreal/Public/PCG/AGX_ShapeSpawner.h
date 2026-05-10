// Copyright 2026, Algoryx Simulation AB.

#pragma once

#include "PCGElement.h"
#include "PCGSettings.h"

#include "AGX_ShapeSpawner.generated.h"

/*
 * AGX_ShapeSpawner is a non-cacheable, main-thread PCG side-effect node that consumes
 * PCG point data and spawns one AGX Shape Component per point into a PCG-managed container actor.
 * It is the point-driven complement to AGX_SpawnShapeComponents (which is component-reference
 * driven). Use it in parallel with a Static Mesh Spawner to create matching visual meshes and AGX
 * physics shapes from the same point set.
 */

UENUM(BlueprintType)
enum class EAGX_ShapeSpawnerType : uint8
{
	Sphere UMETA(DisplayName = "Sphere"),
	Box UMETA(DisplayName = "Box"),
	Capsule UMETA(DisplayName = "Capsule"),
};

/**
 * Settings for an AGX PCG node that creates AGX Shape Components at each input PCG point.
 *
 * Expected input: any standard PCG point data (Surface Sampler, Grid, Scatter Points, etc.).
 * For mixed shape types, place multiple AGX Shape Spawner nodes in parallel, one per type.
 */
UCLASS(MinimalAPI, BlueprintType, ClassGroup = "AGX")
class UAGX_ShapeSpawnerSettings : public UPCGSettings
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
	/** Which primitive shape to create at each input point. One type per node. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX", Meta = (PCG_Overridable))
	EAGX_ShapeSpawnerType ShapeType = EAGX_ShapeSpawnerType::Sphere;

	/**
	 * Base radius for Sphere and Capsule shapes, in cm.
	 * Final radius = BaseRadius * point scale X (when bScaleByPointTransform is true),
	 * or the value of RadiusAttribute if that is set.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadWrite, Category = "AGX",
		Meta = (PCG_Overridable, ClampMin = "0.0"))
	float BaseRadius = 50.f;

	/**
	 * Base half extents for Box shapes, in cm.
	 * Final half extents = BaseHalfExtent * point scale (component-wise, when
	 * bScaleByPointTransform is true), or the value of HalfExtentAttribute if that is set.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX", Meta = (PCG_Overridable))
	FVector BaseHalfExtent {50.f, 50.f, 50.f};

	/**
	 * Base height for Capsule shapes (distance between hemisphere centers), in cm.
	 * Final height = BaseHeight * point scale Z (when bScaleByPointTransform is true),
	 * or the value of HeightAttribute if that is set.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadWrite, Category = "AGX",
		Meta = (PCG_Overridable, ClampMin = "0.0"))
	float BaseHeight = 100.f;

	/**
	 * When true, base dimensions are multiplied by the point's scale before use.
	 * This allows upstream Transform Points or Set Attributes nodes to control shape size
	 * through the standard PCG scale workflow.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX", Meta = (PCG_Overridable))
	bool bScaleByPointTransform = true;

	/**
	 * Name of a float attribute in the input point data that overrides the radius.
	 * Leave as NAME_None to use BaseRadius (and scale, if enabled). Applies to Sphere and Capsule.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX|Attributes")
	FName RadiusAttribute = NAME_None;

	/**
	 * Name of a FVector attribute in the input point data that overrides the box half extents.
	 * Leave as NAME_None to use BaseHalfExtent (and scale, if enabled). Applies to Box only.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX|Attributes")
	FName HalfExtentAttribute = NAME_None;

	/**
	 * Name of a float attribute in the input point data that overrides the capsule height.
	 * Leave as NAME_None to use BaseHeight (and scale, if enabled). Applies to Capsule only.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX|Attributes")
	FName HeightAttribute = NAME_None;

	/**
	 * Base name used when the node creates its dedicated container actor.
	 * The final object name is still unique in the level.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX", Meta = (PCG_Overridable))
	FString SpawnedActorName = TEXT("AGX_PCGShapeSpawner");
};

/**
 * PCG element for UAGX_ShapeSpawnerSettings.
 * Executes on the main thread (required for UObject creation) and is not cacheable
 * (because it produces world-side-effects rather than pure data transformations).
 */
class FAGX_ShapeSpawnerElement : public IPCGElement
{
public:
	// UObject creation and component registration require the game thread.
	virtual bool CanExecuteOnlyOnMainThread(FPCGContext* Context) const override { return true; }
	// Spawners produce world objects; caching would desync data graph from world state.
	virtual bool IsCacheable(const UPCGSettings* InSettings) const override { return false; }

protected:
	virtual bool ExecuteInternal(FPCGContext* Context) const override;
	virtual EPCGElementExecutionLoopMode ExecutionLoopMode(
		const UPCGSettings* Settings) const override
	{
		return EPCGElementExecutionLoopMode::SinglePrimaryPin;
	}
};
