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

UCLASS(ClassGroup = "AGX_Terrain", meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_TerrainWheelComponent : public UActorComponent, public IAGX_NativeOwner
{
	GENERATED_BODY()

public:
	UAGX_TerrainWheelComponent();

	/**
	 * The radius of this Terrain Wheel [cm].
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX Terrain Wheel", Meta = (ExposeOnSpawn))
	double Radius {30.0};

	/**
	 * The width of this Terrain Wheel [cm].
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX Terrain Wheel", Meta = (ExposeOnSpawn))
	double Width {20.0};

	/**
	 * Reference to the Rigid Body to be used for this Terrain Wheel Component.
	 * This Rigid Body must contain a Cylinder Shape to act as the contacting Shape of this wheel.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Terrain Wheel", Meta = (ExposeOnSpawn))
	FAGX_RigidBodyReference RigidBody;

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

	void CreateNative();

private:
	FTerrainWheelBarrier NativeBarrier;
};
