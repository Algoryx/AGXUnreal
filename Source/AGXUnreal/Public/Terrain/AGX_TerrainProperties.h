// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_Real.h"
#include "Terrain/TerrainPropertiesBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "AGX_TerrainProperties.generated.h"

class AAGX_Terrain;
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
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Properties|Particle")
	bool bCreateParticles {true};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Properties|Particle")
	void SetCreateParticles(bool CreateParticles);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Properties|Particle")
	bool GetCreateParticles() const;

	/**
	 * Whether the native terrain should auto-delete particles that are out of bounds.
	 * Cannot be combined with Terrain Paging.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Properties|Particle")
	bool bDeleteParticlesOutsideBounds {false};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Properties|Particle")
	void SetDeleteParticlesOutsideBounds(bool DeleteParticlesOutsideBounds);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Properties|Particle")
	bool GetDeleteParticlesOutsideBounds() const;

	/**
	 * Sets the maximum volume of active zone wedges that should wake particles [cm^3].
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Properties|Particle")
	FAGX_Real MaximumParticleActivationVolume = std::numeric_limits<double>::infinity();

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Properties|Particle")
	void SetMaximumParticleActivationVolume(double InMaximumParticleActivationVolume);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Properties|Particle")
	double GetMaximumParticleActivationVolume() const;

	/**
	 * The soil particle size scaling factor scales the nominal radius that the algorithm will aim
	 * for during the dynamic resizing of particles that occur during terrain interaction. This is
	 * used to alter the desired number of soil particles in the Terrain.
	 * Default value is 1.0, where the nominal particle size matches the Terrain grid size, which in
	 * turn matches the Landscape quad size.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Properties|Particle")
	float SoilParticleSizeScaling {1.f};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Properties|Particle")
	void SetSoilParticleSizeScaling(float InScaling);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Properties|Particle")
	float GetSoilParticleSizeScaling() const;

	/**
	 * The rate factor in the algorithm that governs particle growth from fluid mass and resizing.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Properties|Particle")
	double SoilParticleGrowthRate {0.05};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Properties|Particle")
	void SetSoilParticleGrowthRate(double InRate);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Properties|Particle")
	double GetSoilParticleGrowthRate() const;

	/**
	 * The merge rate for soil particles in the terrain. The merge rate is defined as the fraction
	 * of the current particle mass that should be merged into the terrain each second.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Properties|Particle")
	double SoilParticleMergeRate {9.0};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Properties|Particle")
	void SetSoilParticleMergeRate(double InRate);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Properties|Particle")
	double GetSoilParticleMergeRate() const;

	/**
	 * The fraction of the particle nominal radius that will trigger instant merge of particles
	 * into the terrain during merging.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Properties|Particle")
	double SoilParticleMergeThreshold {0.3};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Properties|Particle")
	void SetSoilParticleMergeThreshold(double InThreshold);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Properties|Particle")
	double GetSoilParticleMergeThreshold() const;

	/**
	 * The absolute lower speed threshold where soil particles are allowed to merge with the terrain
	 * [cm/s].
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Properties|Particle")
	double SoilMergeSpeedThreshold {400.0};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Properties|Particle")
	void SetSoilMergeSpeedThreshold(double InThreshold);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Properties|Particle")
	double GetSoilMergeSpeedThreshold() const;

	/**
	 * The lifetime of the created soil particles [s].
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Properties|Particle")
	double SoilParticleLifeTime {std::numeric_limits<double>::infinity()};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Properties|Particle")
	void SetSoilParticleLifeTime(double InLifeTime);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Properties|Particle")
	double GetSoilParticleLifeTime() const;

	/**
	 * Whether or not to use avalanching in the terrain.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Properties|Avalanching")
	bool bEnableAvalanching {true};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Properties|Avalanching")
	void SetEnableAvalanching(bool Enable);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Properties|Avalanching")
	bool GetEnableAvalanching() const;

	/**
	 * The maximum allowed height transfer per time step due to avalanching.
	 * [cm].
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Properties|Avalanching")
	double AvalancheMaxHeightGrowth {std::numeric_limits<double>::infinity()};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Properties|Avalanching")
	void SetAvalancheMaxHeightGrowth(double InValue);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Properties|Avalanching")
	double GetAvalancheMaxHeightGrowth() const;

	/**
	 * The fraction of the height difference that violates the angle of repose condition
	 * that will be transferred in each time step during avalanching.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Properties|Avalanching")
	double AvalancheDecayFraction {0.1};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Properties|Avalanching")
	void SetAvalancheDecayFraction(double InValue);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Properties|Avalanching")
	double GetAvalancheDecayFraction() const;

	/**
	 * The fractional error threshold for the height difference that will trigger avalanching in a
	 * grid point. Note: Height difference error threshold is calculated as: ErrorThreshold *
	 * ElementSize.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Properties|Avalanching")
	double AvalancheErrorThreshold {0.05};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Properties|Avalanching")
	void SetAvalancheErrorThreshold(double InValue);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Properties|Avalanching")
	double GetAvalancheErrorThreshold() const;

	/**
	 * Scales the penetration force with the shovel velocity squared in the cutting
	 * direction according to: ( 1.0 + C * v^2 ).
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Properties")
	FAGX_Real PenetrationForceVelocityScaling {0.0};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Properties")
	void SetPenetrationForceVelocityScaling(double InPenetrationForceVelocityScaling);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Properties")
	double GetPenetrationForceVelocityScaling() const;

	/**
	 * The shovel speed threshold in the excavation plane of shovel deformers when static mass
	 * in the active zone is converted to dynamic mass [cm/s].
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Properties")
	double ActivationSpeed {10.0};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Properties")
	void SetActivationSpeed(double InSpeed);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Properties")
	double GetActivationSpeed() const;

	/**
	 * Whether the terrain should be deformable. If false, no avalanching will occur,
	 * no dynamic particles will be created, and the soil will not be compacted.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Properties")
	bool bEnableDeformation {true};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Properties")
	void SetEnableDeformation(bool bEnable);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Properties")
	bool GetEnableDeformation() const;

	/**
	 * Whether or not to use locked borders, i.e the borders of the terrain are not allowed to
	 * change from excavation and avalanching.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Properties")
	bool bEnableLockedBorders {false};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Properties")
	void SetEnableLockedBorders(bool Enable);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Properties")
	bool GetEnableLockedBorders() const;

	/**
	 * Whether or not to use soil compaction calculation in the terrain.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Properties")
	bool bEnableSoilCompaction {true};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Properties")
	void SetEnableSoilCompaction(bool Enable);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Properties")
	bool GetEnableSoilCompaction() const;

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

	/**
	 * Register the given Terrain pager as one that uses this Terrain Properties runtime instance.
	 * When a property is changed for this runtime instance, the OnTemplateTerrainChanged (Barrier)
	 * function is called on all registered Terrain pagers.
	 *
	 * Should not be called on persistent assets.
	 */
	void RegisterTerrainPager(AAGX_Terrain& Terrain);

	/**
	 * Unregister the given Terrain pager from this Terrain Properties runtime instance.
	 * Should not be called on persistent assets.
	 */
	void UnregisterTerrainPager(AAGX_Terrain& Terrain);

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
	void CallOnTemplateTerrainChangedOnAll();

private:
	TWeakObjectPtr<UAGX_TerrainProperties> Asset;
	TWeakObjectPtr<UAGX_TerrainProperties> Instance;
	FTerrainPropertiesBarrier NativeBarrier;

	/// Registered Terrain Pagers that will have their OnTemplateTerrainChanged called whenever a
	/// property is changed. Only populated for runtime instances, not for persistent assets.
	TArray<TWeakObjectPtr<AAGX_Terrain>> TerrainPagers;
};
