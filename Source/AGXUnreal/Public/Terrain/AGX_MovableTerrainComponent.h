#pragma once

// AGX Dynamics for Unreal include.s
#include "AGX_NativeOwner.h"
#include "Terrain/TerrainBarrier.h"
#include "AGX_ShovelReference.h"

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
#include "AGX_MovableTerrainComponent.generated.h"


class UAGX_ShapeComponent;
class UAGX_TerrainMaterial;
class UAGX_ShovelComponent;
class UNiagaraSystem;
class UNiagaraComponent;

/*
 *
 */
USTRUCT()
struct AGXUNREAL_API FAGX_BrownianNoiseParams
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	float Height = 50.0f;
	UPROPERTY(EditAnywhere)
	float Scale = 100;
	UPROPERTY(EditAnywhere)
	int Octaves = 3;
	UPROPERTY(EditAnywhere)
	float Persistance = 0.5f;
	UPROPERTY(EditAnywhere)
	float Lacunarity = 2.0f;
	UPROPERTY(EditAnywhere)
	float Exp = 2.0f;
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
	virtual void TickComponent(
		float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction);


protected:
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Shape")
	FVector2D Size = FVector2D(200.0f, 200.0f);

	UPROPERTY(
		EditAnywhere, Meta = (ClampMin = "1", UIMin = "1", ClampMax = "100", UIMax = "100"),
		Category = "AGX Terrain Shape")
	double ElementSize = 10;

	UPROPERTY(
		EditAnywhere, BlueprintReadWrite, Category = "AGX Terrain Shape",
		Meta = (GetOptions = "GetBedShapesOptions"))
	TArray<FName> BedShapes;
	UFUNCTION(CallInEditor)
	TArray<FString> GetBedShapesOptions() const;
	TArray<UMeshComponent*> GetBedShapes() const;
	
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Shape")
	float StartHeight = 0.0f;

	UPROPERTY(EditAnywhere, Category = "AGX Terrain Shape")
	bool bEnableNoise = false;

	UPROPERTY(EditAnywhere, Category = "AGX Terrain Shape", meta = (EditCondition = "bEnableNoise"))
	FAGX_BrownianNoiseParams BrownianNoise;

	virtual void PostInitProperties() override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& event) override;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void UpdateInEditorMesh();

	void AutoFitToBed();

	void RebuildHeightMesh(
		const FVector2D& MeshSize, const FIntVector2& HightFieldRes,
		const TArray<float>& HeightArray, const TArray<float>& MinimumHeightsArray,
		const TArray<std::tuple<int32, int32>>& DirtyHeights = TArray<std::tuple<int32, int32>>());

	void SetupHeights(
		TArray<float>& InitialHeights, TArray<float>& MinimumHeights, const FIntVector2& Res,
		bool FlipYAxis) const;

	void AddBedHeights(TArray<float>& Heights, const FIntVector2& Res, bool FlipYAxis) const;

	void AddNoiseHeights(TArray<float>& Heights, const FIntVector2& Res, bool FlipYAxis) const;

private:
	FTerrainBarrier NativeBarrier;
	TArray<float> CurrentHeights;
	FDelegateHandle PostStepForwardHandle;

	
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

	UPROPERTY(
		EditAnywhere, Category = "AGX Terrain Rendering",
		Meta = (ClampMin = "0.1", UIMin = "01", ClampMax = "1.5", UIMax = "1.5"))
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
};
