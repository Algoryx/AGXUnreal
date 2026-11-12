// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_Real.h"
#include "Terrain/AGX_DelegateParticleData.h"
#include "Terrain/AGX_Shovel.h"
#include "Terrain/AGX_TerrainHeightFetcher.h"
#include "Terrain/AGX_TerrainPagingSettings.h"
#include "Terrain/TerrainBarrier.h"
#include "Terrain/TerrainPagerBarrier.h"
#include "Terrain/TerrainParticleTypes.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

// Standard library includes.
#include <limits>

#include "AGX_Terrain.generated.h"

class UAGX_HeightFieldBoundsComponent;
class UAGX_LidarSurfaceMaterial;
class UAGX_SoilParticleRendererComponent;
class UAGX_TerrainMaterial;
class UAGX_TerrainProperties;
class UAGX_TerrainSpriteComponent;
class UAGX_TerrainLandscapeRendererComponent;
class UAGX_TerrainRVTLandscapeRendererComponent;
class UAGX_ShapeMaterial;
class ALandscape;
class UNiagaraComponent;
class UNiagaraSystem;
class UTextureRenderTarget2D;
class FAGX_TerrainRVTHeightSource;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FParticleDataMulticastDelegate, FDelegateParticleData&, ParticleData);

UCLASS(ClassGroup = "AGX_Terrain", Blueprintable, Category = "AGX")
class AGXUNREAL_API AAGX_Terrain : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AAGX_Terrain();

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

	UPROPERTY(Category = "AGX Terrain", VisibleAnywhere, BlueprintReadOnly)
	UAGX_TerrainLandscapeRendererComponent* DefaultLandscapeRenderer;

	UPROPERTY(EditAnywhere, Category = "AGX Terrain")
	bool bCanCollide {true};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain")
	void SetCanCollide(bool bInCanCollide);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain")
	bool GetCanCollide() const;

	/**
	 * Landscape supplying runtime collision heights. Partitioned sources require Terrain Paging
	 * and finite Terrain Bounds. Arrange streaming sources around tracked bodies and shovels.
	 * The RVT renderer supports translation and yaw; pitch, roll and negative scale are
	 * unsupported. Materials must be configured for the selected Landscape renderer.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain")
	ALandscape* SourceLandscape;

	/**
	 * Distance between terrain height points [cm]. Zero uses the source Landscape's X quad spacing.
	 * Rendering deformation finer than the Landscape's vertex spacing requires the RVT Landscape
	 * Renderer. Cannot be changed during Play.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX Terrain",
		meta = (ClampMin = "0", Units = "cm"))
	double ElementSize = 0.0;

	UFUNCTION(BlueprintPure, Category = "AGX Terrain")
	double GetElementSize() const;

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

	/** Whether the height field rendering should be updated with deformation data. */
	UPROPERTY(
		meta = (DeprecatedProperty, DeprecationMessage = "Use DefaultLandscapeRenderer.bEnabled."))
	bool bEnableDisplacementRendering = true;

	// TODO: Should try finding this from the material automatically!
	/**
	 * The Displacement Map Render Target which AGX Terrain will write height changes to.
	 *
	 * Requirements:
	 *
	 * 1. Used as Displacement Map for the AGX Landscape Material.
	 * 2. Render Target Format must be "R16f".
	 * 3. SizeX and SizeY must equal number of Landscape Vertices in respective dimension (Quads +
	 * 1).
	 * 4. Texture Address Mode should be Clamp.
	 * 5. No Mip Maps.
	 * 6. Texture Group should preferrably be either "RenderTarget" for smooth results,
	 *    or "2D Pixels (unfiltered)" for more precise results.
	 */
	UPROPERTY(
		meta =
			(DeprecatedProperty,
			 DeprecationMessage = "Use DefaultLandscapeRenderer.LandscapeDisplacementMap."))
	UTextureRenderTarget2D* LandscapeDisplacementMap;

	/** Whether shovel active zone should be rendered or not. */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Debug Rendering")
	bool bEnableActiveZoneRendering = false;

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

	bool UsesRVTLandscapeRendering() const;
	void ValidateLandscapeBounds();
	TSharedPtr<const FAGX_TerrainSourceTile, ESPMode::ThreadSafe> GetSourceTileSnapshot(
		const FVector& Corner, int32 Side) const;
	/** Authoritative heights for either active native pager tiles or fixed rendering chunks. */
	TArray<FTerrainTileHeights> GetLandscapeRenderUpdates(TMap<FIntPoint, uint64>& KnownTiles);
	int64 GetLandscapeSourceHeightBytes() const;

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

	void InitializeRendering();
	void UpdateLandscapeRendering();
	void UpdateParticlesArrays();
#if WITH_EDITOR
	void InitPropertyDispatcher();
#endif
	virtual void PostActorCreated() override;
	virtual void PostLoad() override;
	virtual void Serialize(FArchive& Archive) override;

	friend class FAGX_TerrainHeightFetcher;
	friend class UAGX_TerrainLandscapeRendererComponent;

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
	TSharedPtr<FAGX_TerrainRVTHeightSource> RVTHeightSource;
	TSharedRef<FAGX_TerrainHeightFetcher, ESPMode::ThreadSafe> HeightFetcher =
		MakeShared<FAGX_TerrainHeightFetcher, ESPMode::ThreadSafe>();
	FDelegateHandle PreStepForwardHandle;
	FDelegateHandle PostStepForwardHandle;

	// Height field related variables.
	int32 NumVerticesX = 0;
	int32 NumVerticesY = 0;
	bool bLastDeprecatedRenderingEnabled = true;
	TWeakObjectPtr<UTextureRenderTarget2D> LastDeprecatedDisplacementMap;
	bool bRendererConflictReported = false;

	FTransform GetNativeTransform() const;

	UAGX_TerrainProperties* GetOrCreateTerrainPropertiesForOldTerrain();
};
