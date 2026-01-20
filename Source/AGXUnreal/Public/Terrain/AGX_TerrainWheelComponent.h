// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_NativeOwner.h"
#include "AGX_RigidBodyReference.h"
#include "Terrain/TerrainWheelBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "AGX_TerrainWheelComponent.generated.h"

class UAGX_TerrainWheelMaterial;

UCLASS(ClassGroup = "AGX_Terrain", meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_TerrainWheelComponent : public UActorComponent, public IAGX_NativeOwner
{
	GENERATED_BODY()

public:
	UAGX_TerrainWheelComponent();

	/**
	 * Reference to the Rigid Body to be used for this Terrain Wheel Component.
	 * This Rigid Body must contain a Cylinder Shape to act as the contacting Shape of this wheel.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX Terrain Wheel", Meta = (ExposeOnSpawn))
	FAGX_RigidBodyReference RigidBody;

	/**
	 * The Terrain Wheel Material used, affects the phyiscal behaviour of the Terrain Wheel.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Terrain Wheel")
	UAGX_TerrainWheelMaterial* TerrainWheelMaterial;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	bool SetTerrainWheelMaterial(UAGX_TerrainWheelMaterial* Material);

	/**
	 * Determines whether this Terrain Wheeel will deform the Terrain it is in contact with.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Wheel")
	bool bEnableTerrainDeformation {true};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	void SetTerrainDeformationEnabled(bool InEnable);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	bool IsTerrainDeformationEnabled() const;

	/**
	 * Determines whether this Terrain Wheeel will displace Terrain soil (to create ridges).
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Wheel")
	bool bEnableTerrainDisplacement {true};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	void SetTerrainDisplacementEnabled(bool InEnable);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	bool IsTerrainDisplacementEnabled() const;

	/**
	 * Determines whether detailed debug rendering in AGX for this Terrain Wheel is active. This
	 * will be visible in the AGX Web Debugger.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Wheel", AdvancedDisplay)
	bool bEnableAGXDebugRendering {false};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	void SetAGXDebugRenderingEnabled(bool InEnable);

	UPROPERTY(EditAnywhere, Category = "Rendering")
	bool Visible {true};

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

	/// Get the native AGX Dynamics representation of this TerrainWheel. Create it if necessary.
	FTerrainWheelBarrier* GetOrCreateNative();

	/// Return the native AGX Dynamics representation of this TerrainWheel. May return nullptr.
	FTerrainWheelBarrier* GetNative();

	/// Return the native AGX Dynamics representation of this TerrainWheel. May return nullptr.
	const FTerrainWheelBarrier* GetNative() const;

private:
#if WITH_EDITOR
	void InitPropertyDispatcher();
	virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif

	// To allow usage of Dispatcher macro.
	void SetEnableTerrainDisplacement(bool InEnable);
	void SetEnableTerrainDeformation(bool InEnable);
	void SetEnableAGXDebugRendering(bool InEnable);

	void CreateNative();

	bool UpdateNativeTerrainWheelMaterial();

private:
	FTerrainWheelBarrier NativeBarrier;
};
