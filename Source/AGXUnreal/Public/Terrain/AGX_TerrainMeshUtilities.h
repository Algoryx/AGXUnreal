#pragma once

#include "CoreMinimal.h"
#include <functional>
#include "ProceduralMeshComponent.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/EngineTypes.h"
#include "AGX_TerrainMeshUtilities.generated.h"

/*
 *
 */
USTRUCT(BlueprintType, Category = "AGX Procedural")
struct AGXUNREAL_API FAGX_BrownianNoiseParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "AGX Procedural")
	float Height = 50.0f;
	UPROPERTY(EditAnywhere, Category = "AGX Procedural")
	float Scale = 100;
	UPROPERTY(EditAnywhere, Category = "AGX Procedural")
	int Octaves = 3;
	UPROPERTY(EditAnywhere, Category = "AGX Procedural")
	float Persistance = 0.5f;
	UPROPERTY(EditAnywhere, Category = "AGX Procedural")
	float Lacunarity = 2.0f;
	UPROPERTY(EditAnywhere, Category = "AGX Procedural")
	float Exp = 2.0f;
};

struct HfMeshDescription
{
	HfMeshDescription(FIntVector2 VertexRes)
	{
		int NrOfVerts = VertexRes.X * VertexRes.Y;
		Vertices.SetNum(NrOfVerts);
		Triangles.SetNum((VertexRes.X - 1) * (VertexRes.Y - 1) * 6);
		UV0.SetNum(NrOfVerts);
		UV1.SetNum(NrOfVerts);
		Tangents.SetNum(NrOfVerts);
		Normals.SetNum(NrOfVerts);
		Colors.SetNum(NrOfVerts);
	}

	TArray<FVector> Vertices;
	TArray<int> Triangles;
	TArray<FVector2D> UV0;
	TArray<FVector2D> UV1;
	TArray<FVector> Normals;
	TArray<struct FProcMeshTangent> Tangents;
	TArray<FColor> Colors;
};


class UAGX_SimpleMeshComponent;

/**
 *
 */
UCLASS()
class AGXUNREAL_API UAGX_TerrainMeshUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	static TSharedPtr<HfMeshDescription> CreateMeshDescription(
		const FVector& Center, const FVector2D& Size, const FIntVector2& Resolution, double UvScale,
		std::function<float(const FVector&)> HeightFunction, bool UseSkirt = false);

	static float SampleHeightArray(
		FVector2D UV, const TArray<float>& HeightArray, int Width, int Height);

	static float GetBrownianNoise(
		const FVector& Pos, int Octaves, float Scale, float Persistance, float Lacunarity,
		float Exp);

	static float GetLineTracedHeight(
		const FVector& Pos, const TArray<UAGX_SimpleMeshComponent*>& SimpleMeshComponents,
		const FVector& Up = FVector::UpVector, const float MaxHeight = 1000.0f);

	static void AddNoiseHeights(
		TArray<float>& Heights, const FIntVector2 Res, double ElementSize,
		const FTransform Transform, const FAGX_BrownianNoiseParams& NoiseParams);

	static void AddBedHeights(
		TArray<float>& Heights, const FIntVector2 Res, double ElementSize,
		const FTransform Transform, const TArray<UAGX_SimpleMeshComponent*>& BedMeshes,
		const float MaxHeight = 1000.0f);


private:
};
