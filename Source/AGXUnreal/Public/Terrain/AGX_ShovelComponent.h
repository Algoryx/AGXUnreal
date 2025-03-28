// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal include.s
#include "AGX_Edge.h"
#include "AGX_Frame.h"
#include "AGX_NativeOwner.h"
#include "AGX_RigidBodyReference.h"
#include "Terrain/ShovelBarrier.h"
#include "Terrain/AGX_TerrainEnums.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Components/SceneComponent.h"

#include "AGX_ShovelComponent.generated.h"

struct FAGX_ImportContext;

class UAGX_ShovelProperties;

UCLASS(ClassGroup = "AGX_Terrain", meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_ShovelComponent : public USceneComponent, public IAGX_NativeOwner
{
	GENERATED_BODY()

public:
	UAGX_ShovelComponent();

	UPROPERTY(EditAnywhere, Category = "AGX Shovel")
	bool bEnabled {true};

	UFUNCTION(BlueprintCallable, Category = "AGX Shovel")
	void SetEnabled(bool bInEnable);

	UFUNCTION(BlueprintCallable, Category = "AGX Shovel")
	bool IsEnabled() const;

	/// The Rigid Body associated with this Shovel. There may only be one Shovel per Rigid Body.
	UPROPERTY(EditAnywhere, Category = "AGX Shovel")
	FAGX_RigidBodyReference RigidBody;

	/**
	 * Configuration properties for the Shovel. If not set then the AGX Dynamics defaults are
	 * used for all properties.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Shovel")
	UAGX_ShovelProperties* ShovelProperties;

	UFUNCTION(BlueprintCallable, Category = "AGX Shovel")
	void SetShovelProperties(UAGX_ShovelProperties* Properties);

	/**
	 * The top edge of the active zone.
	 *
	 * If the top edge is edited while the simulation is running then either Set Top Edge or
	 * Finalize Shovel Edit must be called to commit the changes to the native AGX Dynamics
	 * representation of the shovel
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX Shovel")
	FAGX_Edge TopEdge;

	UFUNCTION(BlueprintCallable, Category = "AGX Shovel")
	void SetTopEdge(FAGX_Edge InTopEdge);

	/**
	 * The cutting edge of the active zone.
	 *
	 * If the cutting edge is edited while the simulation is running then either Set Cutting Edge or
	 * Finalize Shovel Edit must be called to commit the changes to the native AGX Dynamics
	 * representation of the shovel
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX Shovel")
	FAGX_Edge CuttingEdge;

	UFUNCTION(BlueprintCallable, Category = "AGX Shovel")
	void SetCuttingEdge(FAGX_Edge InCuttingEdge);

	/**
	 * The cutting direction of the shovel where the penetration resistance will be active, which is
	 * usually parallel to the lowest shovel plate that is used to initially penetrate the soil.
	 *
	 * If the cutting direction is edited while the simulation is running then either Set Cutting
	 * Direction or Finalize Shovel Edit must be called to commit the changes to the native AGX
	 * Dynamics representation of the shovel
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX Shovel")
	FAGX_Frame CuttingDirection;

	UFUNCTION(BlueprintCallable, Category = "AGX Shovel")
	void SetCuttingDirection(FAGX_Frame InCuttingDirection);

	/*
	 * The import Guid of this Component. Only used by the AGX Dynamics for Unreal import system.
	 * Should never be assigned manually.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "AGX Dynamics Import Guid")
	FGuid ImportGuid;

	/**
	 * Apply any changes made to the Top Edge, Cutting Edge, or Cutting Direction properties onto
	 * the native AGX Dynamics representation of the shovel.
	 *
	 * It is not necessary to call this if using the Set functions to edit the shovel configuration.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Shovel")
	void FinalizeShovelEdit();

	/**
	 * Get one of the frames that are used to define the edges and directions of the shovel.
	 */
	FAGX_Frame* GetFrame(EAGX_ShovelFrame Frame);

	void CopyFrom(const FShovelBarrier& Barrier, FAGX_ImportContext* Context);

	bool SwapEdgeDirections();

	// ~Begin UObject interface.
	virtual void PostInitProperties() override;
#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
#endif
	// ~End UObject interface.

	//~ Begin ActorComponent Interface
	virtual void OnRegister() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual TStructOnScope<FActorComponentInstanceData> GetComponentInstanceData() const override;
	// ~End UActorComponent interface.

	// ~Begin AGX NativeOwner interface.
	virtual bool HasNative() const override;
	virtual uint64 GetNativeAddress() const override;
	virtual void SetNativeAddress(uint64 NativeAddress) override;
	// ~/End IAGX_NativeOwner interface.

	/// Get the native AGX Dynamics representation of this shovel. Create it if necessary.
	FShovelBarrier* GetOrCreateNative();

	/// Return the native AGX Dynamics representation of this shovel. May return nullptr.
	FShovelBarrier* GetNative();

	/// Return the native AGX Dynamics representation of this shovel. May return nullptr.
	const FShovelBarrier* GetNative() const;

	/// Write all Component Properties to the Native. This is rarely needed.
	bool WritePropertiesToNative();

private:
	// Alias for SetEnable used by Property Dispatcher.
	void SetbEnabled(bool bInEnable);

#if WITH_EDITOR
	// Fill in a bunch of callbacks in PropertyDispatcher so we don't have to manually check each
	// and every UPROPERTY in PostEditChangeProperty and PostEditChangeChainProperty.
	void InitPropertyDispatcher();
#endif

	// Create the native AGX Dynamics object.
	void AllocateNative();

private:
	FShovelBarrier NativeBarrier;
};
