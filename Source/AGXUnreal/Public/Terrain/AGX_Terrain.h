// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_Real.h"
#include "Terrain/AGX_DelegateParticleData.h"
#include "Terrain/AGX_TerrainHeightFetcher.h"
#include "Terrain/AGX_TerrainPagingSettings.h"
#include "Terrain/AGX_Shovel.h"
#include "Terrain/TerrainBarrier.h"
#include "Terrain/TerrainPagerBarrier.h"
#include "Terrain/TerrainParticleTypes.h"

// Unreal Engine includes.
#include "Containers/Array.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

// Standard library includes.
#include <memory>
#include <mutex>

#include "AGX_Terrain.generated.h"

class UAGX_HeightFieldBoundsComponent;
class UAGX_LidarSurfaceMaterial;
class UAGX_SoilParticleRendererComponent;
class UAGX_TerrainMaterial;
class UAGX_TerrainProperties;
class UAGX_TerrainSpriteComponent;
class UAGX_TerrainLandscapeRendererBaseComponent;
class UAGX_TerrainLandscapeRendererComponent;
class UAGX_TerrainPartitionedLandscapeRendererComponent;
struct FAGX_TerrainLandscapeData;
class UAGX_ShapeMaterial;
class ALandscape;
class UNiagaraComponent;
class UNiagaraSystem;
class UTextureRenderTarget2D;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FParticleDataMulticastDelegate, FDelegateParticleData&, ParticleData);

UCLASS(ClassGroup = "AGX_Terrain", Blueprintable, Category = "AGX")
class AGXUNREAL_API AAGX_Terrain : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AAGX_Terrain();
	virtual ~AAGX_Terrain();

	/**
	 * Enable or disable Terrrain computations.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain")
	bool bIsEnabled = true;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain")
	void SetEnabled(bool InEnabled);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain")
	bool IsEnabled() const;

	UPROPERTY(Category = "AGX Terrain", VisibleAnywhere, BlueprintReadOnly)
	UAGX_TerrainSpriteComponent* SpriteComponent;

	UPROPERTY(Category = "AGX Terrain", VisibleAnywhere, BlueprintReadOnly)
	UAGX_HeightFieldBoundsComponent* TerrainBounds;

	UPROPERTY(Category = "AGX Terrain", VisibleAnywhere, BlueprintReadOnly)
	UAGX_SoilParticleRendererComponent* DefaultParticleRenderer;

	/** Default render-target renderer. Disable it when using a different Landscape renderer. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Terrain")
	UAGX_TerrainLandscapeRendererComponent* DefaultLandscapeRenderer;

	/** Override the native Terrain grid spacing when paging. Otherwise use the Landscape spacing.
	 * Fixed before Play. */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX Terrain Paging",
		Meta = (EditCondition = "bEnableTerrainPaging"))
	bool bOverrideTerrainResolution = false;

	/** Native Terrain sample spacing [cm], independent of Landscape. Requires Terrain Paging. Fixed
	 * before Play. */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX Terrain Paging",
		Meta =
			(ClampMin = "1", EditCondition = "bEnableTerrainPaging && bOverrideTerrainResolution"))
	double TerrainVertexSpacing = 25.0;

	UFUNCTION(BlueprintPure, Category = "AGX Terrain Paging")
	double GetTerrainVertexSpacing() const;

	/** Tile loads, tile unloads and active tiles; available even with all renderers disabled. */
	UFUNCTION(BlueprintPure, Category = "AGX Terrain Paging")
	FIntVector GetTerrainPagingStatistics() const;

	/** Collision snapshots retained for loaded components or active AGX tiles. Independent of
	 * rendering. */
	UPROPERTY(
		VisibleAnywhere, BlueprintReadOnly, Transient, Category = "AGX Terrain Paging|Statistics")
	int32 SourceSnapshotCount = 0;

	UPROPERTY(
		VisibleAnywhere, BlueprintReadOnly, Transient, Category = "AGX Terrain Paging|Statistics")
	int64 UnavailableHeightRequests = 0;

	/** The enabled Landscape renderer, or nullptr if none or more than one is enabled. */
	UFUNCTION(BlueprintPure, Category = "AGX Terrain Rendering")
	UAGX_TerrainLandscapeRendererBaseComponent* GetLandscapeRenderer() const;

	UPROPERTY(EditAnywhere, Category = "AGX Terrain")
	bool bCanCollide {true};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain")
	void SetCanCollide(bool bInCanCollide);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain")
	bool GetCanCollide() const;

	/**
	 * The Landscape that AGX Terrain will use as initialization data, and will also modify
	 * in-game using a Displacement Map.
	 *
	 * Requirements:
	 *
	 * 1. Must use AGX Landscape Material or a derived material.
	 * 2. Uniform resolution of LandscapeComponents.
	 * 3. Only one Section per LandscapeComponent.
	 * 4. Uniform resolution of Quads per LandscapeComponent.
	 * 5. The Landscape Actor and AGX Terrain Actor must be centered at World Origin and have no
	 * rotation.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain")
	ALandscape* SourceLandscape;

	/**
	 * The maximum depth of the terrain, from local origin [cm].
	 * Should at least be deeper than the lowest height of the initial Landscape. Note that depth is
	 * defined in the direction of the inverted Z-axis, which means that usually positive values are
	 * used.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Terrain",
		Meta = (ClampMin = "0", UIMin = "0", ClampMax = "1000", UIMax = "1000"))
	FAGX_Real MaxDepth = 200.0f;

	/**
	* Properties that define the behavior of this Terrain.
	* If left unspecified, default Terrain properties will be used.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Terrain")
	UAGX_TerrainProperties* TerrainProperties;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain")
	bool SetTerrainProperties(UAGX_TerrainProperties* InTerrainProperties);

	/** The physical bulk, compaction, particle and surface properties of the Terrain. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Terrain")
	UAGX_TerrainMaterial* TerrainMaterial;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain")
	bool SetTerrainMaterial(UAGX_TerrainMaterial* InTerrainMaterial);

	/** Defines physical properties of the surface of the Terrain. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Terrain")
	UAGX_ShapeMaterial* ShapeMaterial;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain")
	bool SetShapeMaterial(UAGX_ShapeMaterial* InShapeMaterial);

	/**
	 * The Lidar Surface Material assigned to the Terrain.
	 * Determines the interaction of lidar laser rays with the Terrain surface.
	 * This property is only relevant when using the Lidar Sensor.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Lidar")
	UAGX_LidarSurfaceMaterial* LidarSurfaceMaterial {nullptr};

	/**
	 * List of collision groups that this Terrain is part of.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain")
	TArray<FName> CollisionGroups;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain")
	void AddCollisionGroup(FName GroupName);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain")
	void RemoveCollisionGroupIfExists(FName GroupName);

	/**
	 * If a Particle System Component has been spawned by the Terrain, this function will return it.
	 * Returns nullptr otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Terrain")
	UNiagaraComponent* GetSpawnedParticleSystemComponent();

	/**
	 * Returns the number of currently spawned particles known to the Terrain. If this Terrain uses
	 * Terrain Paging, the number of particles known by any active Terrain Tile is returned.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Terrain")
	int32 GetNumParticles() const;

	/**
	 * Deprecated. Use Shovel Components instead.
	 *
	 * A list of the rigid body actors that should be used as terrain shovels.
	 *
	 * Every actor used as shovel MUST have the following components:
	 *
	 * Terrain Shovel Top Edge,
	 * Terrain Shovel Cut Edge,
	 * Terrain Shovel Cut Direction,
	 *
	 * in addition to the usual Rigid Body and Shape components.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Terrain",
		Meta =
			(DeprecatedProperty, DeprecationMessage = "Use Shovel Components instead.",
			 DisplayName = "Shovels [Deprecated]"))
	TArray<FAGX_Shovel> Shovels;

	UFUNCTION(BlueprintCallable, Category = "Shovel Properties")
	bool SetPreloadRadius(UAGX_ShovelComponent* Shovel, double InPreloadRadius);

	UFUNCTION(BlueprintCallable, Category = "Shovel Properties")
	bool SetRequiredRadius(UAGX_ShovelComponent* Shovel, double InRequiredRadius);

	UFUNCTION(BlueprintCallable, Category = "Shovel Properties")
	bool SetTerrainPagerRadii(
		UAGX_ShovelComponent* Shovel, double InPreloadRadius, double InRequiredRadius);

	/**
	 * If set to true, Terrain Paging will be used.
	 * The Terrain Paging Settings should be configured accordingly.
	 * Enabling or disabling Terrain Paging during Play is not supported.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain")
	bool bEnableTerrainPaging;

	/**
	 * If true is passed, Terrain Paging will be used.
	 * Enabling or disabling Terrain Paging during Play is not supported.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Terrain")
	void SetEnableTerrainPaging(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain")
	bool GetEnableTerrainPaging() const;

	UPROPERTY(
		EditAnywhere, Category = "AGX Terrain", Meta = (EditCondition = "bEnableTerrainPaging"))
	FAGX_TerrainPagingSettings TerrainPagingSettings;

	bool HasNativeTerrainPager() const;

	/**
	 * Returns true if this Terrain has a Native Terrain and a Native Terrain Pager if Terrain
	 * Paging is enabled. Returns false otherwise.
	 */
	bool HasNative() const;

	FTerrainBarrier* GetOrCreateNative();
	FTerrainBarrier* GetNative();
	const FTerrainBarrier* GetNative() const;

	FTerrainPagerBarrier* GetOrCreateNativeTerrainPager();
	FTerrainPagerBarrier* GetNativeTerrainPager();
	const FTerrainPagerBarrier* GetNativeTerrainPager() const;

	UPROPERTY(BlueprintAssignable, Category = "AGX Terrain Particles")
	FParticleDataMulticastDelegate OnParticleData;

#if WITH_EDITOR
	virtual void PostInitProperties() override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
	virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void InitializeNative();
	bool CreateNative();
	bool CreateNativeTerrainPager();
	void CreateNativeShovels();
	void AddTerrainPagerBodies();
	bool UpdateNativeTerrainMaterial();
	bool UpdateNativeTerrainProperties();
	bool UpdateNativeShapeMaterial();

	void UpdateParticlesArrays();
#if WITH_EDITOR
	void InitPropertyDispatcher();
#endif
	virtual void PostLoad() override;
	virtual void Serialize(FArchive& Archive) override;

	void EnforceFiniteLandscapeBounds();
	void RefreshLandscapeData();
	void UpdateLandscapeRendering();
	bool UsesLandscapeDataCache() const;
	bool EnsureLandscapeData();
	// Fixed before the cache worker starts. The streaming worker reads only numeric snapshots.
	bool bUseCachedHeightSource = false;
	bool GetLandscapeBaseHeight(double X, double Y, float& Height) const;
	bool CopyLandscapeSourceHeights(TArray<float>& Heights) const;
	bool bRendererConflictReported = false;
	UPROPERTY(Transient)
	TObjectPtr<UAGX_TerrainLandscapeRendererBaseComponent> ActiveLandscapeRenderer;
	UPROPERTY(Transient)
	TObjectPtr<UAGX_TerrainLandscapeRendererBaseComponent> FailedLandscapeRenderer;
	std::shared_ptr<FAGX_TerrainLandscapeData> LandscapeData;

	// Released properties deserialize into the default Landscape renderer in Serialize.
	UPROPERTY()
	bool bEnableDisplacementRendering_DEPRECATED = true;
	UPROPERTY()
	UTextureRenderTarget2D* LandscapeDisplacementMap_DEPRECATED;
	UPROPERTY()
	bool bEnableActiveZoneRendering_DEPRECATED = false;

	friend class UAGX_TerrainLandscapeRendererComponent;
	friend class UAGX_TerrainPartitionedLandscapeRendererComponent;
	friend class FAGX_TerrainHeightFetcher;

private:
	UPROPERTY(Transient)
	bool bNeedsShapeMaterialWarning {false};

	UPROPERTY()
	bool bCreateParticles_DEPRECATED {true};

	UPROPERTY()
	bool bDeleteParticlesOutsideBounds_DEPRECATED = false;

	UPROPERTY()
	FAGX_Real PenetrationForceVelocityScaling_DEPRECATED = 0.0f;

	UPROPERTY()
	FAGX_Real MaximumParticleActivationVolume_DEPRECATED = std::numeric_limits<double>::infinity();

	UPROPERTY()
	float SoilParticleSizeScaling_DEPRECATED {1.f};

	UPROPERTY()
	TArray<FShovelReferenceWithSettings> ShovelComponents_DEPRECATED;

	/// Deprecated, see UAGX_SoilParticleRendererComponent.
	UPROPERTY()
	bool bEnableParticleRendering_DEPRECATED {true};

	/// Deprecated, see UAGX_SoilParticleRendererComponent.
	UPROPERTY()
	UNiagaraSystem* ParticleSystemAsset_DEPRECATED;


#if WITH_EDITOR
	void ShowShapeMaterialWarning() const;
#endif

	/**
	 * Even if Terrain paging is enabled, and this Terrain has a NativeTerrainPagerBarrier, it will
	 * also have a regular NativeBarrier agx::Terrain that will in that case be used as a template
	 * Terrain for the terrain Pager. Setting properties on this template Terrain and then calling
	 * OnTemplateTerrainChanged on the Terrain pager barrier will update current and future tiles
	 * in it.
	 */
	FTerrainBarrier NativeBarrier;
	FTerrainPagerBarrier NativeTerrainPagerBarrier;
	FAGX_TerrainHeightFetcher HeightFetcher;
	FDelegateHandle PostStepForwardHandle;

	// Height field related variables.
	mutable std::mutex OriginalHeightsMutex;
	TArray<float> OriginalHeights;
	int32 NumVerticesX = 0;
	int32 NumVerticesY = 0;

	/**
	 * Thread safe convenience function for reading heights from the source Landscape.
	 * The WorldPosStart is projected onto the Landscape and acts as the starting point (corner) of
	 * the area that will be sampled (it does not snap to the nearest vertex). The steps between
	 * height values are determined by the source Landscape quad size, and the number of steps by
	 * VertsX and VertsY in the Landscape local positive X and Y direction respectively. The heights
	 * are written to OutHeights in the ordering of AGX Dynamics.
	 *
	 * Returns true if the heights could be read, false otherwise.
	 */
	bool FetchHeights(
		const FVector& WorldPosStart, int32 VertsX, int32 VertsY, TArray<float>& OutHeights);

	FTransform GetNativeTransform() const;

	UAGX_TerrainProperties* GetOrCreateTerrainPropertiesForOldTerrain();
};
