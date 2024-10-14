#pragma once

// AGX Dynamics for Unreal include.s
#include "AGX_NativeOwner.h"
#include "Terrain/TerrainBarrier.h"

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
#include "AGX_MovableTerrainComponent.generated.h"


class UAGX_RigidBodyComponent;

/**
 *
 */
UCLASS(ClassGroup = "AGX_Terrain", meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_MovableTerrainComponent : public UProceduralMeshComponent
{
	GENERATED_BODY()
public:
	bool HasNative() const;
	void CreateNative(const FIntVector2& resolutionXY, double cellSize);
	FTerrainBarrier* GetNative();
	const FTerrainBarrier* GetNative() const;
	TArray<UAGX_RigidBodyComponent*> GetBedGeometries() const;

protected:
	UPROPERTY(EditAnywhere)
	FVector2D Size = FVector2D(200.0f, 200.0f);

	UPROPERTY(EditAnywhere)
	int Resolution = 20;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (GetOptions = "GetBedGeometryOptions"))
	TArray<FName> BedGeometries;

	UFUNCTION(CallInEditor)
	TArray<FString> GetBedGeometryOptions() const;

	virtual void PostInitProperties() override;

	virtual void PostEditChangeProperty(FPropertyChangedEvent& event) override;


	virtual void BeginPlay() override;

	virtual void RebuildHeightMesh(
		const FVector2D& size, const FIntVector2& resolutionXY, const TArray<float>& heightArray);

private:
	FTerrainBarrier NativeBarrier;
	TArray<float> CurrentHeights;

	TArray<UMeshComponent*> GetBedGeometriesUMeshComponents() const;
	TArray<float> GenerateHeights(int resX, int resY, float cellSize, bool flipYAxis) const;
	void UpdateInEditorMesh();
};
