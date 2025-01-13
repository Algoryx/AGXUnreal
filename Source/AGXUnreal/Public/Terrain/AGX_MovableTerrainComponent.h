#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_NativeOwner.h"
#include "AGX_ShovelReference.h"
#include "Terrain/AGX_TerrainMeshUtilities.h"
#include "Terrain/TerrainBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"

#include "AGX_MovableTerrainComponent.generated.h"

class UAGX_ShapeComponent;
class UAGX_TerrainMaterial;
class UAGX_ShovelComponent;
class UNiagaraSystem;
class UNiagaraComponent;

/**
 *
 */
UCLASS(ClassGroup = "AGX_Terrain", Category = "AGX", Meta = (BlueprintSpawnableComponent),
	Hidecategories = (Cooking, Collision, Input, LOD, Physics, Replication, Materials, ProceduralMesh))
class AGXUNREAL_API UAGX_MovableTerrainComponent : public UProceduralMeshComponent,
												   public IAGX_NativeOwner
{
	GENERATED_BODY()
public:
	UAGX_MovableTerrainComponent(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void TickComponent(
		float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction);


#if WITH_EDITOR
	virtual void PostInitProperties() override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
	void InitPropertyDispatcher();
	void RebuildEditorMesh();
#endif

	UPROPERTY(EditAnywhere, Category = "AGX Editor")
	bool bRebuildMesh = false;
	UPROPERTY(EditAnywhere, Category = "AGX Editor")
	bool bShowDebugPlane = false;
	UPROPERTY(EditAnywhere, Category = "AGX Editor")
	bool bShowUnrealCollision = false;
	UPROPERTY(EditAnywhere, Category = "AGX Editor")
	bool bHideTerrain = false;
	void SetSize(FVector2D Size);
	void SetShowDebugPlane(bool bShow);
	void SetShowUnrealCollision(bool bShow);
	void SetHideTerrain(bool bHide);

	void CreateNative();
	void ConnectTerrainMeshToNative();
	bool FetchNativeHeights();

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain")
	bool WriteTransformToNative();

	// IAGX_NativeOwner:
	//----------------
	virtual FTerrainBarrier* GetNative();
	virtual const FTerrainBarrier* GetNative() const;
	virtual void UpdateNativeProperties();
	virtual FTerrainBarrier* GetOrCreateNative();

	// ~Begin IAGX_NativeObject interface.
	virtual bool HasNative() const override;
	virtual uint64 GetNativeAddress() const override;
	virtual void SetNativeAddress(uint64 NativeAddress) override;
	// ~End IAGX_NativeObject interface.

	//~ Begin UActorComponent Interface
	virtual TStructOnScope<FActorComponentInstanceData> GetComponentInstanceData() const override;
	//~ End UActorComponent Interface

	//~ Begin USceneComponent Interface
	virtual void OnUpdateTransform(
		EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport) override;
	virtual void OnAttachmentChanged() override;
	//~ End USceneComponent Interface
 
	// ~Begin UObject interface.
#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif
	// ~End UObject interface.
protected:

//Movable Terrain:
//----------------
	UPROPERTY(
		EditAnywhere, BlueprintReadWrite, Category = "AGX Movable Terrain", Meta = (ExposeOnSpawn))
	UMaterialInterface* Material;

	UPROPERTY(
		EditAnywhere, Category = "AGX Movable Terrain", 
		BlueprintReadWrite, Meta = (ExposeOnSpawn))
	FVector2D Size = FVector2D(200.0f, 200.0f);

	UPROPERTY(
		EditAnywhere, Category = "AGX Movable Terrain",
		BlueprintReadWrite, 
		Meta = (ExposeOnSpawn, ClampMin = "1", UIMin = "1", ClampMax = "100", UIMax = "100"))
	double ElementSize = 10;

	FIntVector2 GetTerrainResolution() const
	{
		return FIntVector2(Size.X / ElementSize + 1, Size.Y / ElementSize + 1);
	};
	FVector2D GetTerrainSize() const
	{
		return FVector2D(
			GetTerrainResolution().X * ElementSize, GetTerrainResolution().Y * ElementSize);
	};

	UPROPERTY(
		EditAnywhere, BlueprintReadWrite, Category = "AGX Movable Terrain", Meta = (ExposeOnSpawn))
	float InitialHeight = 0.0f;

	// BedShapes
	// ______________________
	UPROPERTY(
		EditAnywhere, Category = "AGX Movable Terrain", BlueprintReadWrite,
		Meta = (ExposeOnSpawn))
	bool bUseBedShapes = false;

	UPROPERTY(
		EditAnywhere, Category = "AGX Movable Terrain",
		BlueprintReadWrite, 
		Meta = (GetOptions = "GetBedShapesOptions", EditCondition = "bUseBedShapes"))
	TArray<FName> BedShapes;

	UFUNCTION(CallInEditor)
	TArray<FString> GetBedShapesOptions() const;

	UPROPERTY(BlueprintReadWrite, Category = "AGX Movable Terrain", Meta = (ExposeOnSpawn))
	TArray<UMeshComponent*> BedShapeComponents;

	TArray<UMeshComponent*> GetBedShapes() const;

	UPROPERTY(
		EditAnywhere, Category = "AGX Movable Terrain", BlueprintReadWrite,
		Meta = (ExposeOnSpawn, EditCondition = "bUseBedShapes"))
	double BedZOffset = 0.5;


	// InitialNoise Height
	//______________________
	UPROPERTY(
		EditAnywhere, BlueprintReadWrite, Category = "AGX Movable Terrain",
		Meta = (ExposeOnSpawn))
	bool bUseInitialNoise = false;
	UPROPERTY(
		EditAnywhere, Category = "AGX Movable Terrain", BlueprintReadWrite,
		meta = (EditCondition = "bUseInitialNoise", ExposeOnSpawn))
	FAGX_BrownianNoiseParams InitialNoise;
	
	// AGX_Terrain
	// _________________
	UPROPERTY(EditAnywhere, Category = "AGX Terrain")
	bool bCanCollide {true};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain")
	void SetCanCollide(bool bInCanCollide);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain")
	bool GetCanCollide() const;
	
	/**
	 * List of collision groups that this Terrain is part of.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain")
	TArray<FName> CollisionGroups;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain")
	void AddCollisionGroup(FName GroupName);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain")
	void RemoveCollisionGroupIfExists(FName GroupName);

	/** Defines physical properties of the surface of the Terrain. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Terrain")
	UAGX_ShapeMaterial* ShapeMaterial;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain")
	bool SetShapeMaterial(UAGX_ShapeMaterial* InShapeMaterial);
	bool UpdateNativeShapeMaterial();

	
	/** Shovels that can cut into the Terrain. */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain")
	TArray<FAGX_ShovelReference> ShovelComponents;
	void CreateNativeShovels();
	UFUNCTION(BlueprintCallable, Category = "AGX Terrain")
	void AddShovel(UAGX_ShovelComponent* ShovelComponent);

	bool AddNativeShovel(UAGX_ShovelComponent* ShovelComponent);

	/** The physical bulk, compaction, particle and surface properties of the Terrain. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Terrain")
	UAGX_TerrainMaterial* TerrainMaterial;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain")
	bool SetTerrainMaterial(UAGX_TerrainMaterial* InTerrainMaterial);
	bool UpdateNativeTerrainMaterial();


	/** Whether the native terrain should generate particles or not during shovel interactions. */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain")
	bool bCreateParticles = true;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain")
	void SetCreateParticles(bool CreateParticles);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain")
	bool GetCreateParticles() const;

	UPROPERTY(
		EditAnywhere, Category = "AGX Terrain")
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
	double PenetrationForceVelocityScaling = 0.0f;

	void SetPenetrationForceVelocityScaling(double InPenetrationForceVelocityScaling);

	double GetPenetrationForceVelocityScaling() const;

	/**
	 * Sets the maximum volume of active zone wedges that should wake particles [cm^3].
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain")
	double MaximumParticleActivationVolume = std::numeric_limits<double>::infinity();

	void SetMaximumParticleActivationVolume(double InMaximumParticleActivationVolume);

	double GetMaximumParticleActivationVolume() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain")
	void ConvertToDynamicMassInShape(UAGX_ShapeComponent* Shape);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain")
	void SetIsNoMerge(bool IsNoMerge);
	UFUNCTION(BlueprintCallable, Category = "AGX Terrain")
	bool GetIsNoMerge() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Terrain", AdvancedDisplay)
	TEnumAsByte<enum ECollisionEnabled::Type> AdditionalUnrealCollision {
		ECollisionEnabled::NoCollision};

	//--- AGX_Terrain Rendering
	//--------------------------
	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Rendering")
	UNiagaraComponent* GetSpawnedParticleSystemComponent();

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Rendering")
	int32 GetNumParticles() const;

	/** Whether soil particles should be rendered or not. */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Rendering", AdvancedDisplay)
	bool bEnableParticleRendering = false;

	/**
	 * Rough estimation of number of particles that will exist at once. Should not be too low,
	 * or some particles might not be rendered.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Terrain Rendering", AdvancedDisplay,
		Meta =
			(EditCondition = "bEnableParticleRendering", ClampMin = "1", UIMin = "1",
			 UIMax = "4096"))
	int32 MaxNumRenderParticles = 2048;

	UPROPERTY(
		EditAnywhere, Category = "AGX Terrain Rendering", AdvancedDisplay,
		Meta = (EditCondition = "bEnableParticleRendering"))
	UNiagaraSystem* ParticleSystemAsset;
	UNiagaraComponent* ParticleSystemComponent = nullptr;

	bool InitializeParticles();
	void UpdateParticles();
	
	// --- Terrain Mesh
	// --------------------
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Rendering")
	bool bAutoMeshResolution = true;
	UPROPERTY(
		EditAnywhere, Category = "AGX Terrain Rendering",
		Meta = (EditCondition = "!bAutoMeshResolution"))
	FIntVector2 MeshResolution = FIntVector2(20, 20);
	UPROPERTY(
		EditAnywhere, Category = "AGX Terrain Rendering",
		Meta =
			(ClampMin = "0", UIMin = "0", ClampMax = "2", UIMax = "2"))
	int MeshLevelOfDetail = 1;

	UPROPERTY(
		EditAnywhere, Category = "AGX Terrain Rendering")
	bool bShowMeshSides = true;

	UPROPERTY(EditAnywhere, Category = "AGX Terrain Rendering")
	bool bShowMeshBottom = true;

	UPROPERTY(
		EditAnywhere, Category = "AGX Terrain Rendering",
		Meta = (ClampMin = "-2.5", UIMin = "-2.5", ClampMax = "2.5", UIMax = "2.5"))
	double MeshZOffset = -1.0;

	UPROPERTY(EditAnywhere, Category = "AGX Terrain Rendering", AdvancedDisplay)
	EAGX_MeshTilingPattern MeshTilingPattern = EAGX_MeshTilingPattern::StretchedTiles;
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Rendering", AdvancedDisplay)
	int MeshTileResolution = 10;

	UPROPERTY(EditAnywhere, Category = "AGX Terrain Rendering", AdvancedDisplay)
	bool bFixMeshSeams = true;

private:
	FTerrainBarrier NativeBarrier;
	FDelegateHandle PostStepForwardHandle;

	UPROPERTY()
	TArray<float> CurrentHeights;

	UPROPERTY()
	TArray<float> BedHeights;

	bool bHeightsInitialized = false;

	HeightMesh TerrainMesh;
	HeightMesh BedMesh;
	HeightMesh CollisionMesh;
	HeightMesh DebugMesh;
	
	void InitializeHeights();

	float GetCurrentHeight(const FVector& LocalPos) const;
	float GetBedHeight(const FVector& LocalPos) const;

	float CalcInitialHeight(const FVector& LocalPos) const;
	float CalcInitialBedHeight(const FVector& LocalPos) const;

	void RecreateMeshes();

	HeightMesh CreateHeightMesh(
		int StartMeshIndex, 
		const FVector& MeshCenter, const FVector2D& MeshSize,
		const FIntVector2& MeshRes, const FAGX_UvParams& Uv0Params, const FAGX_UvParams& Uv1Params, 
		const FAGX_MeshVertexFunction MeshHeightFunc, const FAGX_MeshVertexFunction EdgeHeightFunc,
		UMaterialInterface* MeshMaterial = nullptr,
		int MeshLod = 0,
		EAGX_MeshTilingPattern TilingPattern = EAGX_MeshTilingPattern::None,
		int TileResolution = 10,
		bool bCreateEdges = false,
		bool bFixSeams = false,
		bool bMeshReverseWinding = false,
		bool bMeshCollision = false,
		bool bMeshVisible = true);

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
