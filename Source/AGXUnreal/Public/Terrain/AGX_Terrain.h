// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_Real.h"
#include "Sensors/AGX_LidarSurfaceMaterial.h"
#include "Terrain/TerrainBarrier.h"
#include "Terrain/TerrainPagerBarrier.h"
#include "Terrain/AGX_TerrainHeightFetcher.h"
#include "Terrain/AGX_TerrainPagingSettings.h"
#include "Terrain/AGX_Shovel.h"
#include "AGX_ShovelReference.h"

// Unreal Engine includes.
#include "Misc/EngineVersionComparison.h"
#include "Containers/Array.h"
#include "CoreMinimal.h"
#include "Misc/EngineVersionComparison.h"
#if !UE_VERSION_OLDER_THAN(5, 2, 0)
// Possible include loop in Unreal Engine.
// - Engine/TextureRenderTarget2D.h
// - RenderUtils.h
// - RHIShaderPlatform.h
//     Defines FStaticShaderPlatform, but includes RHIDefinitions.h first.
// - RHIDefinitions.h
// - DataDrivenShaderPlatformInfo.h
//   Needs FStaticShaderPlatform so includes RHIShaderPlatform.h. But that file is already being
//   included so ignored. So FStaticShaderPlatform will be defined soon, but it isn't yet. So
//   the compile fails.
//
// We work around this by including DataDrivenShaderPlatformInfo.h ourselves before all of the
// above. Now DataDrivenShaderPlatformInfo.h can include RHIShaderPlatform.h succesfully and
// FStaticShaderPlatform is defined when DataDrivenShaderPlatformInfo.h needs it. When we include
// DynamicMeshBuild.h shortly most of the include files are skipped because they have already been
// included as part of DataDrivenShaderPlatformInfo.h here.
#include "DataDrivenShaderPlatformInfo.h"
#endif
#include "Engine/TextureRenderTarget2D.h"
#include "GameFramework/Actor.h"
#if UE_VERSION_OLDER_THAN(5, 2, 0)
#include "RHI.h"
#else
#include "RHITypes.h"
#endif

// Standard library includes.
#include <mutex>

#include "AGX_Terrain.generated.h"

class UAGX_HeightFieldBoundsComponent;
class UAGX_TerrainMaterial;
class UAGX_TerrainSpriteComponent;
class UAGX_ShapeMaterial;
class ALandscape;
class UNiagaraComponent;
class UNiagaraSystem;

USTRUCT(BlueprintType)
struct AGXUNREAL_API FShovelReferenceWithSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Terrain")
	FAGX_ShovelReference Shovel;

	/**
	 * The max distance from the Shovel at which new Terrain Tiles will be preloaded [cm].
	 * Only relevant when using Terrain Paging.
	 */
	UPROPERTY(EditAnywhere, Category = "Paging Terrain")
	FAGX_Real PreloadRadius {1000.f};

	/**
	 * The max distance from the Shovel at which new Terrain Tiles is guaranteed to be loaded [cm].
	 * Only relevant when using Terrain Paging.
	 */
	UPROPERTY(EditAnywhere, Category = "Paging Terrain")
	FAGX_Real RequiredRadius {600.f};
};

UCLASS(ClassGroup = "AGX_Terrain", Blueprintable, Category = "AGX")
class AGXUNREAL_API AAGX_Terrain : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AAGX_Terrain();

	UPROPERTY(Category = "AGX Terrain", VisibleAnywhere, BlueprintReadOnly)
	UAGX_TerrainSpriteComponent* SpriteComponent;

	UPROPERTY(Category = "AGX Terrain", VisibleAnywhere, BlueprintReadOnly)
	UAGX_HeightFieldBoundsComponent* TerrainBounds;

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

	/** Whether the native terrain should generate particles or not during shovel interactions. */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain")
	bool bCreateParticles = true;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain")
	void SetCreateParticles(bool CreateParticles);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain")
	bool GetCreateParticles() const;

	/**
	 * Whether the native terrain simulation should auto-delete particles that are out of bounds.
	 *
	 * Cannot be combined with Terrain Paging.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Terrain", Meta = (EditCondition = "!bEnableTerrainPaging"))
	bool bDeleteParticlesOutsideBounds = false;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain")
	void SetDeleteParticlesOutsideBounds(bool DeleteParticlesOutsideBounds);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain")
	bool GetDeleteParticlesOutsideBounds() const;

	/**
	 * Scales the penetration force with the shovel velocity squared in the cutting
	 * direction according to: ( 1.0 + C * v^2 ).
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain")
	FAGX_Real PenetrationForceVelocityScaling = 0.0f;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain")
	void SetPenetrationForceVelocityScaling(double InPenetrationForceVelocityScaling);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain")
	double GetPenetrationForceVelocityScaling() const;

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
	 * Sets the maximum volume of active zone wedges that should wake particles [cm^3].
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain")
	FAGX_Real MaximumParticleActivationVolume = std::numeric_limits<double>::infinity();

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain")
	void SetMaximumParticleActivationVolume(double InMaximumParticleActivationVolume);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain")
	double GetMaximumParticleActivationVolume() const;

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
		Meta = (DeprecatedProperty, DeprecationMessage = "Use Shovel Components instead."))
	TArray<FAGX_Shovel> Shovels;

	UPROPERTY(EditAnywhere, Category = "AGX Terrain")
	TArray<FShovelReferenceWithSettings> ShovelComponents;

	UFUNCTION(BlueprintCallable, Category = "Shovel Properties")
	bool SetPreloadRadius(UAGX_ShovelComponent* Shovel, double InPreloadRadius);

	UFUNCTION(BlueprintCallable, Category = "Shovel Properties")
	bool SetRequiredRadius(UAGX_ShovelComponent* Shovel, double InRequiredRadius);

	UFUNCTION(BlueprintCallable, Category = "Shovel Properties")
	bool SetTerrainPagerRadii(
		UAGX_ShovelComponent* Shovel, double InPreloadRadius, double InRequiredRadius);

	/** Whether the height field rendering should be updated with deformation data. */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Rendering")
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
		EditAnywhere, Category = "AGX Terrain Rendering",
		Meta = (EditCondition = "bEnableDisplacementRendering"))
	UTextureRenderTarget2D* LandscapeDisplacementMap;

	/** Whether soil particles should be rendered or not. */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Rendering")
	bool bEnableParticleRendering = true;

	/**
	 * Rough estimation of number of particles that will exist at once. Should not be too low,
	 * or some particles might not be rendered.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Terrain Rendering",
		Meta =
			(EditCondition = "bEnableParticleRendering", ClampMin = "1", UIMin = "1",
			 UIMax = "4096"))
	int32 MaxNumRenderParticles = 2048;

	UPROPERTY(
		EditAnywhere, Category = "AGX Terrain Rendering",
		Meta = (EditCondition = "bEnableParticleRendering"))
	UNiagaraSystem* ParticleSystemAsset;

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
	bool UpdateNativeShapeMaterial();

	void InitializeRendering();
	void InitializeDisplacementMap();
	void UpdateLandscapeMaterialParameters();
	void UpdateDisplacementMap();
	void ClearDisplacementMap();
	bool InitializeParticleSystem();
	bool InitializeParticleSystemComponent();
	void UpdateParticlesArrays();
#if WITH_EDITOR
	void InitPropertyDispatcher();
	virtual void PostLoad() override;
#endif
	virtual void Serialize(FArchive& Archive) override;

	friend class FAGX_TerrainHeightFetcher;

private: // Deprecated functions.

	// clang-format off

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage = "Use SetPenetrationForceVelocityScaling instead of SetPenetrationForceVelocityScaling_BP"))
	void SetPenetrationForceVelocityScaling_BP(float InPenetrationForceVelocityScaling);


	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage =
				"Use GetPenetrationForceVelocityScaling instead of GetPenetrationForceVelocityScaling_BP"))
	float GetPenetrationForceVelocityScaling_BP() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain",
		Meta = (
			DeprecatedFunction,
			DeprecationMessage = "Use SetMaximumParticleActivationVolume instead of SetMaximumParticleActivationVolume_BP"))
	void SetMaximumParticleActivationVolume_BP(float InMaximumParticleActivationVolume);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain",
		Meta = (
			DeprecatedFunction,
			DeprecationMessage = "Use GetMaximumParticleActivationVolume instead of GetMaximumParticleActivationVolume_BP"))
	float GetMaximumParticleActivationVolume_BP() const;

	// clang-format on

private:

	UPROPERTY(Transient)
	bool bNeedsShapeMaterialWarning {false};

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
	std::mutex OriginalHeightsMutex;
	TArray<float> OriginalHeights;
	TArray<float> CurrentHeights;
	TArray<FFloat16> DisplacementData;
	TArray<FUpdateTextureRegion2D> DisplacementMapRegions; // TODO: Remove!
	int32 NumVerticesX = 0;
	int32 NumVerticesY = 0;
	bool DisplacementMapInitialized = false;

	// Particle related variables.
	UNiagaraComponent* ParticleSystemComponent = nullptr;

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
};
