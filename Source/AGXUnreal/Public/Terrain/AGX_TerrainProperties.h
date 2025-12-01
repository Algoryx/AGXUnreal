// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_Real.h"
#include "Terrain/TerrainPropertiesBarrier.h"

// Unreal Engine includes.
#include "UObject/Object.h"

#include "AGX_TerrainProperties.generated.h"

class UWorld;

/**
 * Contains configuration properties of an AGX Terrain. It is an asset that is created from
 * the Content Browser. Several Terrains can share the same Terrain Properties asset. Default values
 * for all properties has been taken from AGX Dynamics.
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", BlueprintType)
class AGXUNREAL_API UAGX_TerrainProperties : public UObject
{
	GENERATED_BODY()

public:
	/** Whether the native terrain should generate particles or not during shovel interactions. */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Properties")
	bool bCreateParticles {true};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Properties")
	void SetCreateParticles(bool CreateParticles);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Properties")
	bool GetCreateParticles() const;

	/**
	 * Whether the native terrain should auto-delete particles that are out of bounds.
	 * Cannot be combined with Terrain Paging.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Properties")
	bool bDeleteParticlesOutsideBounds = false;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Properties")
	void SetDeleteParticlesOutsideBounds(bool DeleteParticlesOutsideBounds);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Properties")
	bool GetDeleteParticlesOutsideBounds() const;

	/**
	 * Scales the penetration force with the shovel velocity squared in the cutting
	 * direction according to: ( 1.0 + C * v^2 ).
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Properties")
	FAGX_Real PenetrationForceVelocityScaling = 0.0f;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Properties")
	void SetPenetrationForceVelocityScaling(double InPenetrationForceVelocityScaling);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Properties")
	double GetPenetrationForceVelocityScaling() const;

	/**
	 * Sets the maximum volume of active zone wedges that should wake particles [cm^3].
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Properties")
	FAGX_Real MaximumParticleActivationVolume = std::numeric_limits<double>::infinity();

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Properties")
	void SetMaximumParticleActivationVolume(double InMaximumParticleActivationVolume);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Properties")
	double GetMaximumParticleActivationVolume() const;

	/**
	 * The soil particle size scaling factor scales the nominal radius that the algorithm will aim
	 * for during the dynamic resizing of particles that occur during terrain interaction. This is
	 * used to alter the desired number of soil particles in the Terrain.
	 * Default value is 1.0, where the nominal particle size matches the Terrain grid size, which in
	 * turn matches the Landscape quad size.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Properties")
	float SoilParticleSizeScaling {1.f};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Properties")
	void SetSoilParticleSizeScaling(float InScaling);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Properties")
	float GetSoilParticleSizeScaling() const;

	/**
	 * Copy Property values from the native AGX Dynamics instance to the Terrain Properties asset
	 * the instance was created from. That may be this UAGX_TerrainProperties, if IsAsset returns
	 * true, or the UAGX_TerrainProperties that this was created from, if IsInstance returns true.
	 */
	void CommitToAsset();

	void CopyFrom(const UAGX_TerrainProperties* Source);
	void CopyFrom(const FTerrainPropertiesBarrier& Source);

	/**
	 * Create the Play instance for the given Source Terrain Properties, which should be an asset.
	 * The AGX Dynamics Native will be created immediately.
	 */
	static UAGX_TerrainProperties* CreateInstanceFromAsset(
		const UWorld* PlayingWorld, UAGX_TerrainProperties* Source);

	/**
	 * Get the instance, i.e. Play version, of this Terrain Properties.
	 *
	 * For an asset Terrain Properties GetInstance will return nullptr if we are not currently in
	 * Play or if an instance has not been created with GetOrCreateInstance yet.
	 *
	 * For an instance GetInstance will always return itself.
	 */
	UAGX_TerrainProperties* GetInstance();

	/**
	 * If PlayingWorld is an in-game World and this TerrainProperties is a
	 * UAGX_TerrainProperties Asset, returns a UAGX_TerrainProperties Instance representing the
	 * TerrainProperties asset throughout the lifetime of the GameInstance. If this is already a
	 * UAGX_TerrainPropertiesInstance it returns itself. Returns null if not in-game (invalid call).
	 */
	UAGX_TerrainProperties* GetOrCreateInstance(const UWorld* PlayingWorld);

	/**
	 * If this TerrainProperties is a UAGX_TerrainPropertiesInstance, returns the
	 * UAGX_TerrainPropertiesAsset it was created from (if it still exists). Else returns null.
	 */
	UAGX_TerrainProperties* GetAsset();

	bool IsInstance() const;

	bool HasNative() const;
	FTerrainPropertiesBarrier* GetNative();
	const FTerrainPropertiesBarrier* GetNative() const;
	FTerrainPropertiesBarrier* GetOrCreateNative();

	void UpdateNativeProperties();

	// ~Begin UObject interface.
	virtual void PostInitProperties() override;
#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
#endif
	// ~End UObject interface.

private:
#if WITH_EDITOR
	virtual void InitPropertyDispatcher();
#endif

	void CreateNative();

private:
	TWeakObjectPtr<UAGX_TerrainProperties> Asset;
	TWeakObjectPtr<UAGX_TerrainProperties> Instance;
	FTerrainPropertiesBarrier NativeBarrier;
};
