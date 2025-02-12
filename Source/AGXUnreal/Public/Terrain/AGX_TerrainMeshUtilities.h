// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
#include "Engine/EngineTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"

// Standard Library includes.
#include <functional>

#include "AGX_TerrainMeshUtilities.generated.h"

/**
 * A collection of parameters for Bownian noise generation.
 */
USTRUCT(BlueprintType, Category = "AGX Procedural")
struct AGXUNREAL_API FAGX_BrownianNoiseParams
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Procedural")
	float NoiseHeight = 10.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Procedural")
	float BaseHeight = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Procedural")
	float Scale = 100;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Procedural")
	int Octaves = 3;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Procedural")
	float Persistance = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Procedural")
	float Lacunarity = 2.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Procedural")
	float Exp = 2.0f;
};

/**
 * Data used to create a Procedural Mesh section.
 */
struct FAGX_MeshDescription
{
	FAGX_MeshDescription(FIntVector2 VertexRes)
	{
		VertexResolution = VertexRes;

		int NrOfVerts = VertexRes.X * VertexRes.Y;
		Vertices.SetNum(NrOfVerts);
		Triangles.SetNum((VertexRes.X - 1) * (VertexRes.Y - 1) * 6);
		UV0.SetNum(NrOfVerts);
		UV1.SetNum(NrOfVerts);
		Tangents.SetNum(NrOfVerts);
		Normals.SetNum(NrOfVerts);
		Colors.SetNum(NrOfVerts);
	}
	FIntVector2 VertexResolution;

	TArray<FVector> Vertices;
	TArray<int> Triangles;
	TArray<FVector2D> UV0;
	TArray<FVector2D> UV1;
	TArray<FVector> Normals;
	TArray<struct FProcMeshTangent> Tangents;
	TArray<FColor> Colors;
};

struct FAGX_UvParams
{
	FVector2D Offset;
	FVector2D Scale;
	FAGX_UvParams()
	{
		Offset = FVector2D(0, 0);
		Scale = FVector2D(1, 1);
	}

	FAGX_UvParams(FVector2D UvOffset, FVector2D UvScale)
	{
		Offset = UvOffset;
		Scale = UvScale;
	}
};

using FAGX_MeshVertexFunction = std::function<double(const FVector& LocalPos)>;

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
	FVector Center;
	FVector2D Size;

	MeshTile(int MeshSectionIndex, FVector TileCenter, FVector2D TileSize, FIntVector2 TileRes)
	{
		MeshIndex = MeshSectionIndex;
		Center = TileCenter;
		Size = TileSize;
		Resolution = TileRes;
	}
};

struct HeightMesh
{
	FVector Center;
	FVector2D Size;

	FAGX_UvParams Uv0;
	FAGX_UvParams Uv1;

	bool bCreateEdges;
	bool bFixSeams;
	bool bReverseWinding;

	FAGX_MeshVertexFunction HeightFunc;
	FAGX_MeshVertexFunction EdgeHeightFunc;

	TArray<MeshTile> Tiles;
	HeightMesh()
	{
	}

	HeightMesh(
		const FVector& MeshCenter, const FVector2D& MeshSize, const FAGX_UvParams& Uv0Params,
		const FAGX_UvParams& Uv1Params, FAGX_MeshVertexFunction MeshHeightFunction,
		FAGX_MeshVertexFunction EdgeHeightFunction, bool CreateEdges, bool FixSeams,
		bool ReverseWinding, TArray<MeshTile> MeshTiles)
	{
		Center = MeshCenter;
		Size = MeshSize;

		Uv0 = Uv0Params;
		Uv1 = Uv1Params;

		HeightFunc = MeshHeightFunction;
		EdgeHeightFunc = EdgeHeightFunction;

		bCreateEdges = CreateEdges;
		bFixSeams = FixSeams;
		bReverseWinding = ReverseWinding;

		Tiles = MeshTiles;
	}
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
	static TSharedPtr<FAGX_MeshDescription> CreateHeightMeshTileDescription(
		const FVector& TileCenter, const FVector2D& TileSize, FIntVector2 TileResolution,
		const FVector& MeshCenter, const FVector2D& MeshSize, const FAGX_UvParams& Uv0,
		const FAGX_UvParams& Uv1, const FAGX_MeshVertexFunction MeshHeightFunc,
		const FAGX_MeshVertexFunction EdgeHeightFunc, bool bCreateEdges = false,
		bool bFixSeams = false, bool bReverseWinding = false);

	static HeightMesh CreateHeightMesh(
		const int StartMeshIndex, const FVector& MeshCenter, const FVector2D& MeshSize,
		const FIntVector2& MeshRes, const FAGX_UvParams& Uv0, const FAGX_UvParams& Uv1,
		const int MeshLod, const EAGX_MeshTilingPattern& TilingPattern, int TileResolution,
		const FAGX_MeshVertexFunction MeshHeightFunc, const FAGX_MeshVertexFunction EdgeHeightFunc,
		bool bCreateEdges, bool bFixSeams, bool bReverseWinding);

	static float SampleHeightArray(
		FVector2D UV, const TArray<float>& HeightArray, int Width, int Height);

	static float GetBrownianNoise(
		const FVector& Pos, int Octaves, float Scale, float Persistance, float Lacunarity,
		float Exp);

	static float GetBedHeight(
		const FVector& LocalPos, const FTransform Transform,
		const TArray<UMeshComponent*>& BedMeshes, const float MaxHeight = 1000.0f);

	static float GetNoiseHeight(
		const FVector& LocalPos, const FTransform Transform,
		const FAGX_BrownianNoiseParams& NoiseParams);

private:
};
