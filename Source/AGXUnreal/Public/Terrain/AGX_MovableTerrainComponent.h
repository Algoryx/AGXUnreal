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




UENUM(BlueprintType, Category = "AGX Terrain Mesh")
enum class EAGX_MeshTilingPattern : uint8
{
	None UMETA(DisplayName = "None"),
	StretchedTiles UMETA(DisplayName = "Stretched Tiles")
};
struct MeshTile
{
	int MeshIndex;
	FIntVector2 Resolution;
	FVector2D Center;
	FVector2D Size;
	FBox2D BoundingBox;

	MeshTile(int MeshSectionIndex, FVector2D TileCenter, FVector2D TileSize, FIntVector2 TileRes)
	{
		MeshIndex = MeshSectionIndex;
		Center = TileCenter;
		Size = TileSize;
		Resolution = TileRes;
	}
};


UENUM(BlueprintType, Category = "AGX Terrain Mesh")
enum class EAGX_VertexFunctionType : uint8
{
	None UMETA(DisplayName = "None"),
	DefaultTerrain UMETA(DisplayName = "DefaultTerrain"),
};

/**
 *
 */
UCLASS(ClassGroup = "AGX_Terrain", Category = "AGX", Meta = (BlueprintSpawnableComponent),
	Hidecategories = (Cooking, Collision, Input, LOD, Physics, Replication, Materials, ProceduralMesh))
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
	UPROPERTY(EditAnywhere, Category = "AGX Editor")
	bool bRebuildMesh = false;
	void ForceRebuildMesh();


protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Movable Terrain")
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

	UPROPERTY(
		EditAnywhere, BlueprintReadWrite, Category = "AGX Movable Terrain", Meta = (ExposeOnSpawn))
	float BaseHeight = 0.0f;

	// Bed Shapes
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


	// Initial Noise Height
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

	UPROPERTY(EditAnywhere, Category = "AGX Terrain")
	TArray<FAGX_ShovelReference> ShovelComponents;
	void CreateNativeShovels();
	UFUNCTION(BlueprintCallable, Category = "AGX Terrain")
	void AddShovel(UAGX_ShovelComponent* ShovelComponent);

	bool AddNativeShovel(UAGX_ShovelComponent* ShovelComponent);

	/** Defines physical properties of the surface of the Terrain. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Terrain")
	UAGX_ShapeMaterial* ShapeMaterial;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain")
	bool SetShapeMaterial(UAGX_ShapeMaterial* InShapeMaterial);
	bool UpdateNativeShapeMaterial();

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


	
	// --- Heightfield Mesh
	// --------------------
	UPROPERTY(
		EditAnywhere, Category = "AGX Terrain Rendering",
		Meta = (EditCondition = "!bAutoMeshResolution"))
	FIntVector2 MeshResolution = FIntVector2(20, 20);
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Rendering")
	bool bAutoMeshResolution = true;
	UPROPERTY(
		EditAnywhere, Category = "AGX Terrain Rendering",
		Meta =
			(ClampMin = "0", UIMin = "0", ClampMax = "8", UIMax = "8",
			 EditCondition = "bAutoMeshResolution"))
	int MeshLevelOfDetail = 1;

	UPROPERTY(
		EditAnywhere, Category = "AGX Terrain Rendering",
		Meta = (ClampMin = "-2.5", UIMin = "-2.5", ClampMax = "2.5", UIMax = "2.5"),
		AdvancedDisplay)
	double MeshZOffset = -1.0;

	UPROPERTY(EditAnywhere, Category = "AGX Terrain Rendering", AdvancedDisplay)
	EAGX_MeshTilingPattern MeshTilingPattern = EAGX_MeshTilingPattern::StretchedTiles;
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Rendering", AdvancedDisplay)
	int MeshTileResolution = 10;

	UPROPERTY(EditAnywhere, Category = "AGX Terrain Rendering", AdvancedDisplay)
	bool bMeshTileSkirts = true;
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Rendering", AdvancedDisplay)
	bool bClampMeshEdges = false;

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

	//UPROPERTY()
	TArray<float> CurrentHeights;

	UPROPERTY()
	TArray<float> BedHeights;
	
	void InitializeHeights();

	float GetCurrentHeight(const FVector& LocalPos) const;
	float GetBedHeight(const FVector& LocalPos) const;

	float CalcInitialHeight(const FVector& LocalPos) const;
	float CalcInitialBedHeight(const FVector& LocalPos) const;

	FVector2D ToUvCord(const FVector& LocalPos) const
	{
		return FVector2D(LocalPos.X / Size.X + 0.5, LocalPos.Y / Size.Y + 0.5);
	};

	FVector2D ToUvCentimeters(const FVector& LocalPos) const
	{
		return FVector2D((LocalPos.X + Size.X / 2) / 100.0, (LocalPos.Y + Size.Y / 2) / 100.0);
	};
	float DistFromEdge(const FVector& LocalPos) const
	{
		float DistX = Size.X / 2 - FMath::Abs(LocalPos.X);
		float DistY = Size.Y / 2 - FMath::Abs(LocalPos.Y);

		return FMath::Min(DistX, DistY);
	}

	float GetMeshHeight(const FVector& LocalPos) const;

	TArray<MeshTile> MeshTiles;
	TArray<MeshTile> GenerateMeshTiles(
		const FIntVector2& MeshResolution, 
		const EAGX_MeshTilingPattern& MeshTilingPattern,
		int MeshLod = 0) const;
	void RecreateMesh();


	// Mesh Section (using UProceduralMeshComponent's GPU interface)
	void CreateTileMeshSection(
		const MeshTile& Tile, const EAGX_VertexFunctionType& VertexFunc,
		UMaterialInterface * MeshSectionMaterial, bool bMeshSectionSkirt = false, bool bMeshSectionVisible = true,
		bool bMeshSectionUnrealCollision = false);
	void UpdateTileMeshSection(const MeshTile& Tile, const EAGX_VertexFunctionType& VertexFunc);


	// Mesh Description (Vertex Position, Color, etc)
	TMap<int, TSharedPtr<FAGX_TileMeshDescription>> MeshSectionDescriptions;
	void UpdateTileMeshDescription(const MeshTile& Tile, const EAGX_VertexFunctionType& VertexFunc);
};
