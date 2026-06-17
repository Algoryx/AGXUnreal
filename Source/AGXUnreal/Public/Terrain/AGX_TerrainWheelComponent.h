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

struct FAGX_ImportContext;
class UAGX_TerrainWheelSettings;

/**
 * The Terrain Wheel Component is used to model the interaction between a rolling wheel and a
 * Terrain. The Terrain Wheel computes terramechanics quantities such as slip, sinkage, normal
 * pressure, shear stress and terrain displacement. This makes it suitable for off-road wheels where
 * the terrain deformation and soil response are important parts of the simulation.
 */
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
	 * Terrain Wheel Settings used by this Terrain Wheel.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Terrain Wheel")
	UAGX_TerrainWheelSettings* TerrainWheelSettings {nullptr};

	/**
	 * Set Terrain Wheel Settings used by this Terrain Wheel.
	 * If setting nullptr, the default Terrain Wheel Settings are used.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	bool SetTerrainWheelSettings(UAGX_TerrainWheelSettings* InTerrainWheelSettings);

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

	UPROPERTY(EditAnywhere, Category = "Rendering")
	bool Visible {true};

	/*
	 * The import Guid of this Component. Only used by the AGX Dynamics for Unreal import system.
	 * Should never be assigned manually.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Dynamics Import")
	FGuid ImportGuid;

	/*
	 * The import name of this Component. Only used by the AGX Dynamics for Unreal import system.
	 * Should never be assigned manually.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Dynamics Import")
	FString ImportName;

	void CopyFrom(const FTerrainWheelBarrier& Barrier, FAGX_ImportContext* Context);

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

	void CreateNative();
	bool UpdateNativeTerrainWheelSettings();

private:
	FTerrainWheelBarrier NativeBarrier;
};
