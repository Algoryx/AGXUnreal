#pragma once

// AGX Dynamics for Unreal include.s
#include "AGX_NativeOwner.h"
#include "Terrain/TerrainBarrier.h"
#include "Terrain/AGX_TerrainMeshUtilities.h"
#include "AGX_ShovelReference.h"

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
#include "AGX_MovableTerrainComponent.generated.h"


class UAGX_ShapeComponent;
class UAGX_TerrainMaterial;
class UAGX_ShovelComponent;
class UNiagaraSystem;
class UNiagaraComponent;

struct MeshTile
{
	FVector2D Center;
	FVector2D Size;
	FIntVector2 Resolution;
	MeshTile(FVector2D TileCenter, FVector2D TileSize, FIntVector2 TileRes)
	{
		Center = TileCenter;
		Size = TileSize;
		Resolution = TileRes;
	}
};

/**
 *
 */
UCLASS(
	ClassGroup = "AGX_Terrain", meta = (BlueprintSpawnableComponent),
	Hidecategories = (Cooking, Collision, Input, LOD, Physics, Replication))
class AGXUNREAL_API UAGX_MovableTerrainComponent : public UProceduralMeshComponent
{
	GENERATED_BODY()
public:
	UAGX_MovableTerrainComponent(const FObjectInitializer& ObjectInitializer);

	bool HasNative() const;
	void CreateNative();
	FTerrainBarrier* GetNative();
	const FTerrainBarrier* GetNative() const;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void TickComponent(
		float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction);

#if WITH_EDITOR
	virtual void PostInitProperties() override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
#endif

protected:
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Editor")
	bool bRebuildMesh = false;

	UPROPERTY(
		EditAnywhere, Category = "AGX Terrain Shape", 
		BlueprintReadWrite, Meta = (ExposeOnSpawn))
	FVector2D Size = FVector2D(200.0f, 200.0f);

	UPROPERTY(
		EditAnywhere, Category = "AGX Terrain Shape",
		BlueprintReadWrite, 
		Meta = (ExposeOnSpawn, ClampMin = "1", UIMin = "1", ClampMax = "100", UIMax = "100"))
	double ElementSize = 10;

	UPROPERTY(
		EditAnywhere, Category = "AGX Terrain Shape", BlueprintReadWrite, Meta = (ExposeOnSpawn))
	float InitialHeight = 0.0f;

	UPROPERTY(EditAnywhere, Category = "AGX Terrain Shape")
	bool bInitialNoise = false;

	UPROPERTY(
		EditAnywhere, Category = "AGX Terrain Shape", meta = (EditCondition = "bInitialNoise"))
	FAGX_BrownianNoiseParams InitialNoiseParams;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, 
		Category = "AGX Terrain Shape", 
		Meta = (GetOptions = "GetBedShapesOptions"))
	TArray<FName> BedShapes;

	UFUNCTION(CallInEditor)
	TArray<FString> GetBedShapesOptions() const;

	UPROPERTY(BlueprintReadWrite, Category = "AGX Terrain Shape", Meta = (ExposeOnSpawn))
	TArray<UAGX_ShapeComponent*> BedShapeComponents;

	TArray<UAGX_ShapeComponent*> GetBedShapes() const;

	void UpdateMeshOnPropertyChanged();

	FIntVector2 GetTerrainResolution() const
	{
		return FIntVector2(Size.X / ElementSize + 1, Size.Y / ElementSize + 1);
	};

/*
--- AGX_Terrain Implementation
------------------------------
*/
protected:
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Shape")
	bool bCanCollide {true};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Shape")
	void SetCanCollide(bool bInCanCollide);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Shape")
	bool GetCanCollide() const;

	/** Defines physical properties of the surface of the Terrain. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Terrain Shape")
	UAGX_ShapeMaterial* ShapeMaterial;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Shape")
	bool SetShapeMaterial(UAGX_ShapeMaterial* InShapeMaterial);
	bool UpdateNativeShapeMaterial();

	/**
	 * List of collision groups that this Terrain is part of.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Shape")
	TArray<FName> CollisionGroups;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Shape")
	void AddCollisionGroup(FName GroupName);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Shape")
	void RemoveCollisionGroupIfExists(FName GroupName);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Terrain Shape", AdvancedDisplay)
	TEnumAsByte<enum ECollisionEnabled::Type> AdditionalUnrealCollision {
		ECollisionEnabled::NoCollision};

protected:
	/** The physical bulk, compaction, particle and surface properties of the Terrain. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Terrain")
	UAGX_TerrainMaterial* TerrainMaterial;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain")
	bool SetTerrainMaterial(UAGX_TerrainMaterial* InTerrainMaterial);
	bool UpdateNativeTerrainMaterial();

	UPROPERTY(EditAnywhere, Category = "AGX Terrain")
	TArray<FAGX_ShovelReference> ShovelComponents;
	void CreateNativeShovels();
	UFUNCTION(BlueprintCallable, Category = "AGX Terrain")
	void AddShovel(UAGX_ShovelComponent* ShovelComponent);

	bool AddNativeShovel(UAGX_ShovelComponent* ShovelComponent);

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

	
	UFUNCTION(BlueprintCallable, Category = "AGX Terrain")
	UNiagaraComponent* GetSpawnedParticleSystemComponent();

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain")
	int32 GetNumParticles() const;

protected:

	UPROPERTY(EditAnywhere, Category = "AGX Terrain Rendering")
	UMaterialInterface* Material;

	UPROPERTY(EditAnywhere, Category = "AGX Terrain Rendering")
	FVector2D UvOffset = FVector2D(0.0, 0.0);
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Rendering")
	FVector2D UvScaling = FVector2D(1.0, 1.0);

	UPROPERTY(EditAnywhere, Category = "AGX Terrain Rendering")
	bool bWorldUv = true;

	UPROPERTY(
		EditAnywhere, Category = "AGX Terrain Rendering",
		Meta = (ClampMin = "0.1", UIMin = "0.1", ClampMax = "1.5", UIMax = "1.5"))
	float ResolutionScaling = 0.5f;


	UPROPERTY(EditAnywhere, Category = "AGX Terrain Rendering")
	bool bEnableTiles = true;
	UPROPERTY(
		EditAnywhere, Category = "AGX Terrain Rendering",
		Meta = (ClampMin = "2", UIMin = "2", ClampMax = "32", UIMax = "32",
			EditCondition = "bEnableTiles"))
	int TileResolution = 10;

	UPROPERTY(
		EditAnywhere, Category = "AGX Terrain Rendering",
		Meta = (EditCondition = "bEnableTiles"))
	bool bTileSkirts = true;
	
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Rendering")
	bool ClampToBorders = true;
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Rendering")
	double ZOffset = -0.5;
	/** Whether soil particles should be rendered or not. */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Rendering")
	bool bEnableParticleRendering = false;

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
	UNiagaraComponent* ParticleSystemComponent = nullptr;

	
	bool InitializeParticles();
	void UpdateParticles();

	
private:
	FTerrainBarrier NativeBarrier;
	FDelegateHandle PostStepForwardHandle;

	UPROPERTY()
	TArray<float> CurrentHeights;

	UPROPERTY()
	TArray<float> BedHeights;

	TMap<int, MeshTile> MeshTiles;

	void InitializeHeights();

	float GetHeight(FVector LocalPos) const;
	FVector2D GetUV0(FVector LocalPos) const;

	void InitializeMesh();
	void UpdateMesh(const TArray<std::tuple<int32, int32>>& DirtyHeights);
};
