#pragma once

// AGX Dynamics for Unreal include.s
#include "AGX_NativeOwner.h"
#include "Terrain/TerrainBarrier.h"

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
#include "AGX_MovableTerrainComponent.generated.h"


class UAGX_ShapeComponent;

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
	TArray<UAGX_ShapeComponent*> GetBedGeometries() const;

protected:
	UPROPERTY(EditAnywhere)
	int Resolution = 20;

	UPROPERTY(EditAnywhere)
	UMaterialInterface* Material;

	UPROPERTY(EditAnywhere)
	FVector2D Size = FVector2D(200.0f, 200.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (GetOptions = "GetBedGeometryOptions"))
	TArray<FName> BedGeometries;

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

	virtual void RebuildHeightMesh(
		const FVector2D& size, const int resX, const int resY, const TArray<float>& heightArray);

private:
	FTerrainBarrier NativeBarrier;
	TArray<float> CurrentHeights;
	FDelegateHandle PostStepForwardHandle;

	UFUNCTION(CallInEditor)
	TArray<FString> GetBedGeometryOptions() const;

	TArray<UMeshComponent*> GetBedGeometriesUMeshComponents() const;
	void AutoFitToBed();
	void SetupHeights(
		TArray<float>& initialHeights, TArray<float>& minimumHeights, int resX, int resY,
		double elementSize, bool flipYAxis) const;
	void AddRaycastedHeights(
		TArray<float>& heights,
		const TArray<UMeshComponent*>& meshes, const FTransform& origoTransform, int resX, int resY, float cellSize, bool flipYAxis) const;
	
	void AddNoiseHeights(
		TArray<float>& heights, int resX, int resY, float cellSize, bool flipYAxis) const;
	FBox CreateEncapsulatingBoundingBox(
		const TArray<UMeshComponent*>& Meshes, const FTransform& origoTransform);
	void UpdateInEditorMesh();
};
