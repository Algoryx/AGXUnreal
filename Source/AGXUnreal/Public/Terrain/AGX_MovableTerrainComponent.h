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
	FVector2D Size = FVector2D(200.0f, 200.0f);

	UPROPERTY(EditAnywhere)
	int Resolution = 20;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (GetOptions = "GetBedGeometryOptions"))
	TArray<FName> BedGeometries;

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
	TArray<float> GenerateEditorHeights(int resX, int resY, float cellSize, bool flipYAxis) const;
	void UpdateInEditorMesh();
};
