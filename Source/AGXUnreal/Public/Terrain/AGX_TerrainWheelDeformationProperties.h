// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Terrain/TerrainWheelDeformationPropertiesBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "AGX_TerrainWheelDeformationProperties.generated.h"

class UWorld;

/**
 * Contains deformation properties for AGX Terrain Wheel. Several Terrain Wheel Components can
 * share the same Terrain Wheel Deformation Properties.
 */
UCLASS(ClassGroup = "AGX_Terrain", Category = "AGX", BlueprintType)
class AGXUNREAL_API UAGX_TerrainWheelDeformationProperties : public UObject
{
	GENERATED_BODY()

public:
	UAGX_TerrainWheelDeformationProperties() = default;

	/**
	 * Determines whether Terrain Wheels using these properties will deform the Terrain they are in
	 * contact with.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Wheel Deformation Properties")
	bool bEnableTerrainDeformation {true};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel Deformation Properties")
	void SetEnableTerrainDeformation(bool InEnable);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel Deformation Properties")
	bool GetEnableTerrainDeformation() const;

	/**
	 * Determines whether Terrain Wheels using these properties will displace Terrain soil to create
	 * ridges.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Wheel Deformation Properties")
	bool bEnableTerrainDisplacement {true};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel Deformation Properties")
	void SetEnableTerrainDisplacement(bool InEnable);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel Deformation Properties")
	bool GetEnableTerrainDisplacement() const;

	static UAGX_TerrainWheelDeformationProperties* CreateInstanceFromAsset(
		const UWorld* PlayingWorld, UAGX_TerrainWheelDeformationProperties* Source);

	UAGX_TerrainWheelDeformationProperties* GetInstance();

	/**
	 * If PlayingWorld is an in-game World and this TerrainWheelDeformationProperties is an asset,
	 * returns a TerrainWheelDeformationProperties instance representing the asset throughout the
	 * lifetime of the GameInstance. If this is already an instance it returns itself.
	 */
	UAGX_TerrainWheelDeformationProperties* GetOrCreateInstance(const UWorld* PlayingWorld);

	/**
	 * If this TerrainWheelDeformationProperties is an instance, returns the asset it was created
	 * from. Else returns itself.
	 */
	UAGX_TerrainWheelDeformationProperties* GetAsset();

	bool IsInstance() const;

	bool HasNative() const;
	FTerrainWheelDeformationPropertiesBarrier* GetNative();
	const FTerrainWheelDeformationPropertiesBarrier* GetNative() const;
	FTerrainWheelDeformationPropertiesBarrier* GetOrCreateNative();

	void UpdateNativeProperties();

	// ~Begin UObject interface.
	virtual void PostInitProperties() override;
#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
#endif
	// ~End UObject interface.

private:
#if WITH_EDITOR
	void InitPropertyDispatcher();
#endif

	void CopyFrom(const UAGX_TerrainWheelDeformationProperties* Source);
	void CreateNative();

private:
	TWeakObjectPtr<UAGX_TerrainWheelDeformationProperties> Asset;
	TWeakObjectPtr<UAGX_TerrainWheelDeformationProperties> Instance;
	FTerrainWheelDeformationPropertiesBarrier NativeBarrier;
};
