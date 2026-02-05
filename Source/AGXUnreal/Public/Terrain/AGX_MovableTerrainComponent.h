// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_NativeOwner.h"
#include "Terrain/AGX_DelegateParticleData.h"
#include "Terrain/AGX_TerrainMeshUtilities.h"
#include "Terrain/TerrainBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"

#include "AGX_MovableTerrainComponent.generated.h"

class UAGX_ShapeComponent;
class UAGX_TerrainMaterial;
class UAGX_TerrainProperties;
class UMaterialInterface;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FParticleDataMovableMulticastDelegate, FDelegateParticleData&, ParticleData);

/**
 * The Movable Terrain Components represents an AGX Terrain similar to the AGX Terrain Actor, but
 * does not use a Landscape to render itself, but instead uses a Mesh that is dynamically updated
 * during Play when digging/displacements occur.
 *
 * The Movable Terrain Component can be attached to a Rigid Body (by making it a child Component to
 * a Rigid Body Component) to allow it to move in the world during Play.
 */
UCLASS(
	ClassGroup = "AGX_Terrain", Category = "AGX", Meta = (BlueprintSpawnableComponent),
	Hidecategories =
		(Cooking, Collision, Input, LOD, Physics, Replication, Materials, ProceduralMesh))
class AGXUNREAL_API UAGX_MovableTerrainComponent : public UProceduralMeshComponent,
												   public IAGX_NativeOwner
{
	GENERATED_BODY()
public:
	UAGX_MovableTerrainComponent(const FObjectInitializer& ObjectInitializer);

	/**
	 * If enabled, a flat debug plane is rendered underneath the terrain mesh.
	 * Useful for visualizing the terrain extents.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Editor")
	bool bShowDebugPlane = false;

	void SetShowDebugPlane(bool bShow);

	void CreateNative();
	void ConnectMeshToNative();
	bool FetchNativeHeights();

	UFUNCTION(BlueprintCallable, Category = "AGX Movable Terrain")
	bool WriteTransformToNative();

	UPROPERTY(BlueprintAssignable, Category = "AGX Movable Terrain")
	FParticleDataMovableMulticastDelegate OnParticleData;

	virtual FTerrainBarrier* GetNative();
	virtual const FTerrainBarrier* GetNative() const;
	virtual FTerrainBarrier* GetOrCreateNative();

	// ~Begin IAGX_NativeObject interface.
	virtual bool HasNative() const override;
	virtual uint64 GetNativeAddress() const override;
	virtual void SetNativeAddress(uint64 NativeAddress) override;
	// ~End IAGX_NativeObject interface.

	//~ Begin UActorComponent Interface
	virtual void OnComponentCreated() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual TStructOnScope<FActorComponentInstanceData> GetComponentInstanceData() const override;
	//~ End UActorComponent Interface

	//~ Begin USceneComponent Interface
	virtual void OnUpdateTransform(
		EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport) override;
	virtual void OnAttachmentChanged() override;
	//~ End USceneComponent Interface

	// ~Begin UObject interface.
	virtual void PostInitProperties() override;
#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* InProperty) const override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
	void InitPropertyDispatcher();
#endif
	// ~End UObject interface.

	void RecreateMeshesEditor();

	/**
	 * Enable or disable Terrrain computations.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Movable Terrain")
	bool bEnabled = true;

	UFUNCTION(BlueprintCallable, Category = "AGX Movable Terrain")
	void SetEnabled(bool bInEnabled);

	UFUNCTION(BlueprintCallable, Category = "AGX Movable Terrain")
	bool IsEnabled() const;

	/**
	 * Render Material applied to the underlying terrain mesh.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadWrite, Category = "AGX Movable Terrain", Meta = (ExposeOnSpawn))
	UMaterialInterface* Material = nullptr;

	/**
	 * Size of the terrain in X and Y dimensions [cm].
	 * The terrain boundaries are derived directly from this value.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Movable Terrain", BlueprintReadWrite, Meta = (ExposeOnSpawn))
	FVector2D Size = FVector2D(200.0f, 200.0f);

	/**
	 * Spacing between heightfield samples (grid resolution) [cm].
	 * Smaller values yield higher-resolution terrain but is more performance costly.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Movable Terrain", BlueprintReadWrite,
		Meta = (ExposeOnSpawn, ClampMin = "1", UIMin = "1", ClampMax = "100", UIMax = "100"))
	double ElementSize = 10;

	/**
	 * Initial height added uniformly to the entire terrain at initialization [cm].
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadWrite, Category = "AGX Movable Terrain", Meta = (ExposeOnSpawn))
	float InitialHeight = 10.f;

	FIntVector2 GetTerrainResolution() const
	{
		return FIntVector2(Size.X / ElementSize + 1, Size.Y / ElementSize + 1);
	};
	FVector2D GetTerrainSize() const
	{
		return FVector2D(
			GetTerrainResolution().X * ElementSize, GetTerrainResolution().Y * ElementSize);
	};

	void SetSize(FVector2D Size);
	void SetElementSize(double ElementSize);

	// BedShapes
	// ______________________

	/**
	 * Enables use of Bed Shapes meshes that influence the initial "bottom" shape
	 * (minimum heights) of the terrain.
	 * Commonly used to match the contour of a dumper bed or other underlying geometry.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Movable Terrain", BlueprintReadWrite, Meta = (ExposeOnSpawn))
	bool bUseBedShapes = false;

	/**
	 * Names of mesh components whose geometry defines the "bottom" shape
	 * (minimum heights) of the Terrain at initialization.
	 * Commonly used to match the contour of a dumper bed or other underlying geometry.
	 * Only applicable when bUseBedShapes is enabled.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Movable Terrain", BlueprintReadWrite,
		Meta = (GetOptions = "GetBedShapesOptions", EditCondition = "bUseBedShapes"))
	TArray<FName> BedShapes;

	UFUNCTION(CallInEditor)
	TArray<FName> GetBedShapesOptions() const;

	/**
	 * Mesh components whose geometry defines the "bottom" (minimum heights) of the Terrain at
	 * initialization. Commonly used to match the contour of a dumper bed or other underlying
	 * geometry. Only applicable when bUseBedShapes is enabled.
	 */
	UPROPERTY(BlueprintReadWrite, Category = "AGX Movable Terrain", Meta = (ExposeOnSpawn))
	TArray<UMeshComponent*> BedShapeComponents;

	TArray<UMeshComponent*> GetBedShapes() const;

	/**
	 * Vertical offset for the "bottom" (minimum heights) of the Terrain at initialization.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Movable Terrain", BlueprintReadWrite,
		Meta = (ExposeOnSpawn, EditCondition = "bUseBedShapes"))
	double BedZOffset = 0.0;

	// InitialNoise
	//______________________

	/**
	 * Enables generation of Brownian noise across the terrain, which applies to the initial heights
	 * of the Terrain.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadWrite, Category = "AGX Movable Terrain", Meta = (ExposeOnSpawn))
	bool bUseInitialNoise = false;

	/**
	 * Controls Brownian noise applied at terrain initialization.
	 * Noise contributes to the initial height but does not change dynamically afterward.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Movable Terrain", BlueprintReadWrite,
		meta = (EditCondition = "bUseInitialNoise", ExposeOnSpawn))
	FAGX_BrownianNoiseParams InitialNoise;

	// AGX Terrain
	// _________________

	/**
	 * If true, the terrains underlying HeightField interacts with AGX Shapes.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Movable Terrain")
	bool bCanCollide {true};

	UFUNCTION(BlueprintCallable, Category = "AGX Movable Terrain")
	void SetCanCollide(bool bInCanCollide);

	UFUNCTION(BlueprintCallable, Category = "AGX Movable Terrain")
	bool GetCanCollide() const;

	/**
	 * List of collision groups that this Terrain is part of.
	 * Controls which objects can collide or interact with the terrain.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Movable Terrain")
	TArray<FName> CollisionGroups;

	UFUNCTION(BlueprintCallable, Category = "AGX Movable Terrain")
	void AddCollisionGroup(FName GroupName);

	UFUNCTION(BlueprintCallable, Category = "AGX Movable Terrain")
	void RemoveCollisionGroupIfExists(FName GroupName);

	/**
	 * Properties that define the behavior of this Terrain.
	 * If left unspecified, default Terrain properties will be used.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Movable Terrain")
	UAGX_TerrainProperties* TerrainProperties {nullptr};

	UFUNCTION(BlueprintCallable, Category = "AGX Movable Terrain")
	bool SetTerrainProperties(UAGX_TerrainProperties* InTerrainProperties);

	/** The physical bulk, compaction, particle and surface properties of the Terrain. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Movable Terrain")
	UAGX_TerrainMaterial* TerrainMaterial = nullptr;

	UFUNCTION(BlueprintCallable, Category = "AGX Movable Terrain")
	bool SetTerrainMaterial(UAGX_TerrainMaterial* InTerrainMaterial);

	/** Defines physical properties of the surface of the Terrain. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Movable Terrain")
	UAGX_ShapeMaterial* ShapeMaterial = nullptr;

	UFUNCTION(BlueprintCallable, Category = "AGX Movable Terrain")
	bool SetShapeMaterial(UAGX_ShapeMaterial* InShapeMaterial);

	UFUNCTION(BlueprintCallable, Category = "AGX Movable Terrain")
	void ConvertToDynamicMassInShape(UAGX_ShapeComponent* Shape);

	/**
	 * If true, prevents dynamic mass (particles)to merge into the terrain.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Movable Terrain")
	bool bNoMerge = false;

	UFUNCTION(BlueprintCallable, Category = "AGX Movable Terrain")
	void SetNoMerge(bool bInNoMerge);

	UFUNCTION(BlueprintCallable, Category = "AGX Movable Terrain")
	bool IsNoMerge() const;

	/**
	 * Shows Unreal Engine's internal collision mesh for debugging.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Movable Terrain", AdvancedDisplay)
	bool bShowUnrealCollision = false;

	/**
	 * Level of detail used for generating Unreal's built-in collision mesh.
	 * Higher values reduce resolution but improve performance.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Movable Terrain", AdvancedDisplay)
	int UnrealCollisionLOD = 4;

	/**
	 * Additional Unreal collision mode applied to the generated collision mesh.
	 * Allows enabling UE collision separately from AGX collision.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadWrite, Category = "AGX Movable Terrain", AdvancedDisplay,
		Meta = (ClampMin = "0", UIMin = "0", ClampMax = "5", UIMax = "5"))
	TEnumAsByte<enum ECollisionEnabled::Type> AdditionalUnrealCollision {
		ECollisionEnabled::NoCollision};

	void SetShowUnrealCollision(bool bShow);
	void SetUnrealCollisionLOD(int Lod);
	void SetUnrealCollisionType(TEnumAsByte<enum ECollisionEnabled::Type> CollisionType);

	// Terrain Mesh
	// --------------------

	/**
	 * If true, mesh resolution is automatically derived from heightfield resolution.
	 * When disabled, MeshResolution determines the rendering resolution.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Rendering")
	bool bAutoMeshResolution = true;

	/**
	 * Resolution of the rendered terrain mesh when not using automatic resolution.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Terrain Rendering",
		Meta = (EditCondition = "!bAutoMeshResolution"))
	FIntVector2 MeshResolution = FIntVector2(20, 20);

	/**
	 * Level of detail applied when generating terrain mesh tiles.
	 * Higher values decrease the resolution.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Terrain Rendering",
		Meta = (ClampMin = "0", UIMin = "0", ClampMax = "3", UIMax = "3"))
	int32 MeshLevelOfDetail = 1;

	/**
	 * If true, a backside mesh is generated to close off the terrain mesh volume.
	 * Useful for visual continuity when the underside may be visible.
	 * Does not affect the underlying AGX Terrain.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Rendering")
	bool bCloseMesh = true;

	/**
	 * Vertical offset applied to the entire terrain mesh.
	 * Does not affect the underlying AGX Terrain.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Rendering")
	double MeshZOffset = 0.0;

	/**
	 * Tiling pattern used when generating terrain mesh tiles.
	 * Affects performance, UV layout, and how tile boundaries are handled.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Rendering", AdvancedDisplay)
	EAGX_MeshTilingPattern MeshTilingPattern = EAGX_MeshTilingPattern::StretchedTiles;

	/**
	 * Tile scale used by this Movable Terrain Component when splitting the terrain into
	 * multiple mesh tiles for rendering. Higher values increase the scale of each tile but also
	 * makes updating the underlying mesh tile more computationally expensive.
	 * Does not effect the underlying AGX Terrain.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Rendering", AdvancedDisplay)
	int MeshTileScale = 10;

	/**
	 * Enables seam-fixing which hide overlapping vertices between adjacent mesh tiles.
	 * May cause causing rendering artifacts if disabled.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Rendering", AdvancedDisplay)
	bool bFixMeshSeams = true;

	void SetMeshMaterial(UMaterialInterface* Material);
	void SetAutoMeshResolution(bool bAuto);
	void SetMeshResolution(FIntVector2 Resolution);
	void SetMeshLOD(int32 Lod);
	void SetMeshZOffset(double zOffset);
	void SetCloseMesh(bool bClose);
	void SetMeshTileScale(int TileScale);
	void SetMeshTilingPattern(EAGX_MeshTilingPattern Pattern);
	void SetFixMeshSeams(bool bFix);

private:
	FTerrainBarrier NativeBarrier;
	FDelegateHandle PostStepForwardHandle;

	UPROPERTY()
	TArray<float> CurrentHeights;

	UPROPERTY()
	TArray<float> BedHeights;

	bool bHeightsInitialized = false;

	HeightMesh TerrainMesh;
	HeightMesh CollisionMesh;

	void InitializeHeights();

	float GetCurrentHeight(const FVector& LocalPos) const;
	float GetBedHeight(const FVector& LocalPos) const;

	float CalcInitialHeight(const FVector& LocalPos) const;
	float CalcInitialBedHeight(const FVector& LocalPos) const;

	virtual void UpdateNativeProperties();
	bool UpdateNativeTerrainProperties();
	bool UpdateNativeTerrainMaterial();
	bool UpdateNativeShapeMaterial();
	void UpdateParticleData();

	void RecreateMeshes();

	HeightMesh CreateHeightMesh(
		int StartMeshIndex, const FVector& MeshCenter, const FVector2D& MeshSize,
		const FIntVector2& MeshRes, const FAGX_UvParams& Uv0Params, const FAGX_UvParams& Uv1Params,
		const FAGX_MeshVertexFunction MeshHeightFunc, const FAGX_MeshVertexFunction EdgeHeightFunc,
		UMaterialInterface* MeshMaterial = nullptr, int MeshLod = 0,
		EAGX_MeshTilingPattern TilingPattern = EAGX_MeshTilingPattern::None,
		int TileResolution = 10, bool bCreateEdges = false, bool bFixSeams = false,
		bool bMeshReverseWinding = false, bool bMeshCollision = false, bool bMeshVisible = true,
		bool bCalcFastTerrainBedNormals = true);

	FVector2D ToUv(const FVector& LocalPos, const FVector2D& PlaneSize) const
	{
		return FVector2D(LocalPos.X / PlaneSize.X + 0.5, LocalPos.Y / PlaneSize.Y + 0.5);
	};

	float DistFromEdge(const FVector& LocalPos) const
	{
		float DistX = Size.X / 2 - FMath::Abs(LocalPos.X);
		float DistY = Size.Y / 2 - FMath::Abs(LocalPos.Y);

		return FMath::Min(DistX, DistY);
	};
};
