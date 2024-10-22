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

protected:
	UPROPERTY(EditAnywhere)
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

};
