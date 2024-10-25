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

/**
 *
 */
UCLASS(ClassGroup = "AGX_Terrain", meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_MovableTerrainComponent : public UProceduralMeshComponent
{
	GENERATED_BODY()
public:
	bool HasNative() const;
	void CreateNative();
	FTerrainBarrier* GetNative();
	const FTerrainBarrier* GetNative() const;

protected:
	UPROPERTY(EditAnywhere, Meta = (ClampMin = "1", UIMin = "1", ClampMax = "256", UIMax = "256"))
	int Resolution = 20;

	UPROPERTY(EditAnywhere)
	UMaterialInterface* Material;

	UPROPERTY(EditAnywhere)
	FVector2D Size = FVector2D(200.0f, 200.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (GetOptions = "GetBedGeometryOptions"))
	TArray<FName> BedGeometries;
	UFUNCTION(CallInEditor)
	TArray<FString> GetBedGeometryOptions() const;
	TArray<UAGX_ShapeComponent*> GetBedGeometries() const;

	UPROPERTY(EditAnywhere)
	double BedZOffset = -1.0;

	UPROPERTY(EditAnywhere)
	float NoiseHeight = 50.0f;

	UPROPERTY(EditAnywhere)
	float StartHeight = 0.0f;

	virtual void PostInitProperties() override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& event) override;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void UpdateInEditorMesh();

	void AutoFitToBed();

	void RebuildHeightMesh(
		const FVector2D& MeshSize, const int ResX, const int ResY,
		const TArray<float>& HeightArray);

	void SetupHeights(
		TArray<float>& InitialHeights, TArray<float>& MinimumHeights, int ResX, int ResY,
		double ElementSize, bool FlipYAxis) const;

	void AddBedHeights(
		TArray<float>& Heights, int ResX, int ResY, double ElementSize, bool FlipYAxis) const;

	void AddNoiseHeights(
		TArray<float>& Heights, int ResX, int ResY, double ElementSize, bool FlipYAxis) const;

private:
	FTerrainBarrier NativeBarrier;
	TArray<float> CurrentHeights;
	FDelegateHandle PostStepForwardHandle;

	
	/*
	--- AGX_Terrain Implementation
	------------------------------
	*/
protected:

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
	 * List of collision groups that this Terrain is part of.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain")
	TArray<FName> CollisionGroups;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain")
	void AddCollisionGroup(FName GroupName);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain")
	void RemoveCollisionGroupIfExists(FName GroupName);

	
	UPROPERTY(EditAnywhere, Category = "AGX Terrain")
	TArray<FAGX_ShovelReference> ShovelReferences;
};
