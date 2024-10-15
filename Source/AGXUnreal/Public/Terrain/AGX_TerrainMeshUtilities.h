#pragma once

#include "CoreMinimal.h"
#include <functional>
#include "ProceduralMeshComponent.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AGX_TerrainMeshUtilities.generated.h"

struct HfMeshDescription
{
	HfMeshDescription(FIntVector2 vertexRes)
	{
		int nrOfVerts = vertexRes.X * vertexRes.Y;
		Vertices.SetNum(nrOfVerts);
		Triangles.SetNum((vertexRes.X - 1) * (vertexRes.Y - 1) * 6);
		UV0.SetNum(nrOfVerts);
		UV1.SetNum(nrOfVerts);
		Tangents.SetNum(nrOfVerts);
		Normals.SetNum(nrOfVerts);
		Colors.SetNum(nrOfVerts);
	}

	TArray<FVector> Vertices;
	TArray<int> Triangles;
	TArray<FVector2D> UV0;
	TArray<FVector2D> UV1;
	TArray<FVector> Normals;
	TArray<struct FProcMeshTangent> Tangents;
	TArray<FColor> Colors;
};
/**
 *
 */
UCLASS()
class AGXUNREAL_API UAGX_TerrainMeshUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	static TSharedPtr<HfMeshDescription> CreateMeshDescription(
		const FVector& center, const FVector2D& size, const FIntVector2& resolution, double uvScale,
		std::function<float(const FVector&)> heightFunction, bool isUseSkirt = false);

	static float GetBrownianNoise(
		const FVector& pos, int octaves, float scale, float persistance, float lacunarity,
		float exp);

	static float GetRaycastedHeight(
		const FVector& pos, const TArray<UMeshComponent*>& meshComponents,
		const FVector& up = FVector::UpVector, const float rayLength = 1000.0f);

	static float SampleHeightArray(
		FVector2D UV, const TArray<float>& HeightArray, int Width, int Height);

private:
	static void GenerateTriangles(
		HfMeshDescription& meshDesc, const FVector& center, const FVector2D& size,
		const FIntVector2& resolution, double uvScale,
		std::function<float(const FVector&)> heightFunction, bool isUseSkirt);
};
