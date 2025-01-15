
#include "Terrain/AGX_TerrainMeshUtilities.h"
#include "Shapes/AGX_SimpleMeshComponent.h"
#include <KismetProceduralMeshLibrary.h>

TSharedPtr<FAGX_MeshDescription> UAGX_TerrainMeshUtilities::CreateHeightMeshTileDescription(
	const FVector& TileCenter, const FVector2D& TileSize, FIntVector2 TileResolution,
	const FVector& MeshCenter, const FVector2D& MeshSize, 
	const FAGX_UvParams& Uv0, const FAGX_UvParams& Uv1, 
	const FAGX_MeshVertexFunction MeshHeightFunc,
	const FAGX_MeshVertexFunction EdgeHeightFunc, 
	bool bCreateEdges, bool bFixSeams, bool bReverseWinding)
{

	//Tile corners
	FVector2D T1(TileCenter.X - TileSize.X / 2, TileCenter.Y - TileSize.Y / 2);
	FVector2D T2(TileCenter.X + TileSize.X / 2, TileCenter.Y + TileSize.Y / 2);

	//Mesh corners
	FVector2D M1(MeshCenter.X - MeshSize.X / 2, MeshCenter.Y - MeshSize.Y / 2);
	FVector2D M2(MeshCenter.X + MeshSize.X / 2, MeshCenter.Y + MeshSize.Y / 2);


	// Vertex resolution: Number of faces + 1
	FIntVector2 VertexRes = FIntVector2(TileResolution.X + 1, TileResolution.Y + 1);
	// With skirts add +2 extra vertices per row/column
	if (bFixSeams)
		VertexRes = FIntVector2(VertexRes.X + 2, VertexRes.Y + 2);
	else if (bCreateEdges)
	{
		// Add extra vertices if the Tile (corners T1, T2) is on the edge of the full Mesh (M1, M2)
		VertexRes.X += (FMath::Abs(T1.X - M1.X) < SMALL_NUMBER) ? 1 : 0;
		VertexRes.X += (FMath::Abs(T2.X - M2.X) < SMALL_NUMBER) ? 1 : 0;
		VertexRes.Y += (FMath::Abs(T1.Y - M1.Y) < SMALL_NUMBER) ? 1 : 0;
		VertexRes.Y += (FMath::Abs(T2.Y - M2.Y) < SMALL_NUMBER) ? 1 : 0;
	}

	// Edge Row/Column index
	int EdgeL = -1;
	int EdgeR = -1;
	int EdgeT = -1;
	int EdgeB = -1;
	if (bCreateEdges)
	{
		if (FMath::Abs(T1.X - M1.X) < SMALL_NUMBER)
			EdgeL = 0;
		if (FMath::Abs(T2.X - M2.X) < SMALL_NUMBER)
			EdgeR = VertexRes.X - 1;
		if (FMath::Abs(T1.Y - M1.Y) < SMALL_NUMBER)
			EdgeT = 0;
		if (FMath::Abs(T2.Y - M2.Y) < SMALL_NUMBER)
			EdgeB = VertexRes.Y - 1;
	}


	auto MeshDescPtr = MakeShared<FAGX_MeshDescription>(VertexRes);
	auto& MeshDesc = MeshDescPtr.Get();
	
	// Size of individual triangle
	FVector2D TriangleSize =
		FVector2D(TileSize.X / TileResolution.X, TileSize.Y / TileResolution.Y);


	FIntVector2 StartIndex = FIntVector2(((EdgeL != -1 || bFixSeams) ? 1 : 0),
										((EdgeT != -1 || bFixSeams) ? 1 : 0));

	// Populate vertices, uvs, colors
	int32 VertexIndex = 0;
	int32 TriangleIndex = 0;
	for (int32 y = 0; y < VertexRes.Y; ++y)
	{
		for (int32 x = 0; x < VertexRes.X; ++x)
		{
			//Local Plane Position
			FVector P = TileCenter + FVector(
				(x - StartIndex.X) * TriangleSize.X - TileSize.X / 2,
				(y - StartIndex.Y) * TriangleSize.Y - TileSize.Y / 2, 0.0);


			bool IsEdgeVertex =
				bCreateEdges && (x == EdgeL || x == EdgeR || y == EdgeT || y == EdgeB);
			if (!IsEdgeVertex)
			{
				double Height = MeshHeightFunc(P);
				// Vertex Position
				MeshDesc.Vertices[VertexIndex] = P + FVector::UpVector * Height;

				// Vertex UV
				MeshDesc.UV0[VertexIndex] = FVector2D(
					(P.X + Uv0.Offset.X) * Uv0.Scale.X, (P.Y + Uv0.Offset.Y) * Uv0.Scale.Y);
				MeshDesc.UV1[VertexIndex] = FVector2D(
					(P.X + Uv1.Offset.X) * Uv1.Scale.X, (P.Y + Uv1.Offset.Y) * Uv1.Scale.Y);
			}
			else if (IsEdgeVertex)
			{
				FVector V = P;

				// Clamped position inside MeshSize
				P = FVector(FMath::Clamp(P.X, M1.X, M2.X), FMath::Clamp(P.Y, M1.Y, M2.Y), P.Z);
				bool IsXAxis = FMath::Abs(P.Y - V.Y) < FMath::Abs(P.X - V.X);

				double Height = MeshHeightFunc(P);
				double EdgeHeight = EdgeHeightFunc(P);
				double HeightDist = FMath::Abs(Height - EdgeHeight);

				// Move VertexPosition down to EdgeHeight
				MeshDesc.Vertices[VertexIndex] = P + EdgeHeight * FVector::UpVector;

				// Fix UVs
				MeshDesc.UV0[VertexIndex] = FVector2D(
					(P.X + Uv0.Offset.X + (IsXAxis ? HeightDist : 0)) * Uv0.Scale.X,
					(P.Y + Uv0.Offset.Y + (IsXAxis ? 0 : HeightDist)) * Uv0.Scale.Y);
				MeshDesc.UV1[VertexIndex] = FVector2D(
					(P.X + Uv1.Offset.X + (IsXAxis ? HeightDist : 0)) * Uv1.Scale.X,
					(P.Y + Uv1.Offset.Y + (IsXAxis ? 0 : HeightDist)) * Uv1.Scale.Y);
			}

			bool IsEdgeCorner = (x == EdgeL || x == EdgeR - 1) && (y == EdgeT || y == EdgeB - 1);
			// Triangle winding: Skip last row/column, aswell as edge corners
			if ((x < VertexRes.X - 1 && y < VertexRes.Y - 1) && !IsEdgeCorner)
			{
				int32 BottomLeft = VertexIndex;
				int32 BottomRight = BottomLeft + 1;
				int32 TopLeft = BottomLeft + VertexRes.X;
				int32 TopRight = TopLeft + 1;

                if (bReverseWinding)
                {
                    // First triangle with flipped winding order
                    MeshDesc.Triangles[TriangleIndex++] = BottomLeft;
                    MeshDesc.Triangles[TriangleIndex++] = TopRight;
                    MeshDesc.Triangles[TriangleIndex++] = TopLeft;

                    // Second triangle with flipped winding order
                    MeshDesc.Triangles[TriangleIndex++] = BottomLeft;
                    MeshDesc.Triangles[TriangleIndex++] = BottomRight;
                    MeshDesc.Triangles[TriangleIndex++] = TopRight;
                }
                else
                {
                    // First triangle
                    MeshDesc.Triangles[TriangleIndex++] = BottomLeft;
                    MeshDesc.Triangles[TriangleIndex++] = TopLeft;
                    MeshDesc.Triangles[TriangleIndex++] = TopRight;

                    // Second triangle
                    MeshDesc.Triangles[TriangleIndex++] = BottomLeft;
                    MeshDesc.Triangles[TriangleIndex++] = TopRight;
                    MeshDesc.Triangles[TriangleIndex++] = BottomRight;
                }
			}

			++VertexIndex;
		}
	}

	//Before 5.5: MeshDesc.Triangles.SetNum(TriangleIndex, true);
	MeshDesc.Triangles.SetNum(TriangleIndex);

	UKismetProceduralMeshLibrary::CalculateTangentsForMesh(
		MeshDesc.Vertices, MeshDesc.Triangles, MeshDesc.UV0, MeshDesc.Normals, MeshDesc.Tangents);

	// Move skirt vertices after lightning calculation
	if (bFixSeams)
	{
		float SkirtLength = 1.0f;
		VertexIndex = 0;
		for (int32 y = 0; y < VertexRes.Y; ++y)
		{
			for (int32 x = 0; x < VertexRes.X; ++x)
			{
				if (x == 0 || x == VertexRes.X - 1 || y == 0 || y == VertexRes.Y - 1)
				{
					FVector P = MeshDesc.Vertices[VertexIndex];

					// Clamp to Tile
					P = FVector(FMath::Clamp(P.X, T1.X, T2.X), FMath::Clamp(P.Y, T1.Y, T2.Y), MeshCenter.Z);

					// Is on edge of Mesh
					bool IsOnEdge = (P.X - M1.X) < SMALL_NUMBER || (M2.X - P.X) < SMALL_NUMBER ||
									(P.Y - M1.Y) < SMALL_NUMBER || (M2.Y - P.Y) < SMALL_NUMBER;

					double Height = bCreateEdges && IsOnEdge ? EdgeHeightFunc(P) : MeshHeightFunc(P);

					MeshDesc.Vertices[VertexIndex] = P + FVector::UpVector * (Height - SkirtLength);
				}
				VertexIndex++;
			}
		}
	}

	return MeshDescPtr;
}

HeightMesh UAGX_TerrainMeshUtilities::CreateHeightMesh(
	const int StartMeshIndex, const FVector& MeshCenter, const FVector2D& MeshSize,
	const FIntVector2& MeshRes, const FAGX_UvParams& Uv0, const FAGX_UvParams& Uv1,
	const int MeshLod, 
	const EAGX_MeshTilingPattern& TilingPattern, int TileResolution,
	const FAGX_MeshVertexFunction MeshHeightFunc, const FAGX_MeshVertexFunction EdgeHeightFunc,
	bool bCreateEdges, bool bFixSeams, bool bReverseWinding)
{
	int MeshIndex = StartMeshIndex;
	float LodScaling = 1.0f / (FMath::Pow(2.0f, MeshLod));
	FVector2D ScaledResolution = FVector2D(MeshRes.X * LodScaling, MeshRes.Y * LodScaling);

	TArray<MeshTile> Tiles;

	// Generate Tiles
	if (TilingPattern == EAGX_MeshTilingPattern::None)
	{
		// One single tile
		FVector TileCenter = MeshCenter;
		FVector2D TileSize = MeshSize;
		FIntVector2 TileRes = FIntVector2(
			FMath::Max(1, FMath::RoundToInt(ScaledResolution.X)),
			FMath::Max(1, FMath::RoundToInt(ScaledResolution.Y)));

		Tiles.Add(MeshTile(MeshIndex++, TileCenter, TileSize, TileRes));
	}
	else if (TilingPattern == EAGX_MeshTilingPattern::StretchedTiles)
	{
		// Multiple tiles in a grid
		FIntVector2 NrOfTiles = FIntVector2(
			FMath::Max(1, FMath::RoundToInt(ScaledResolution.X / TileResolution)),
			FMath::Max(1, FMath::RoundToInt(ScaledResolution.Y / TileResolution)));
		FIntVector2 TileRes = FIntVector2(TileResolution, TileResolution);
		FVector2D TileSize = FVector2D(MeshSize.X / NrOfTiles.X, MeshSize.Y / NrOfTiles.Y);

		for (int Tx = 0; Tx < NrOfTiles.X; Tx++)
		{
			for (int Ty = 0; Ty < NrOfTiles.Y; Ty++)
			{
				FVector2D PlanePos =
					TileSize / 2 - MeshSize / 2 + FVector2D(Tx * TileSize.X, Ty * TileSize.Y);
				FVector TileCenter = MeshCenter + FVector(PlanePos.X, PlanePos.Y, 0.0);
				Tiles.Add(MeshTile(MeshIndex++, TileCenter, TileSize, TileRes));
			}
		}
	}
	
	return HeightMesh(
		MeshCenter, MeshSize, Uv0, Uv1, MeshHeightFunc,
		EdgeHeightFunc, bCreateEdges, bFixSeams, bReverseWinding, Tiles);
}


float UAGX_TerrainMeshUtilities::SampleHeightArray(
	FVector2D UV, const TArray<float>& HeightArray, int Width, int Height)
{
	// Normalize UV coordinates to the range [0, 1]
	UV.X = FMath::Clamp(UV.X, 0.0f, 1.0f);
	UV.Y = FMath::Clamp(UV.Y, 0.0f, 1.0f);

	// Calculate the positions of the four surrounding texels
	double X = UV.X * (Width - 1);
	double Y = UV.Y * (Height - 1);

	int32 X1 = FMath::FloorToInt(X);
	int32 Y1 = FMath::FloorToInt(Y);
	int32 X2 = FMath::Clamp(X1 + 1, 0, Width - 1);
	int32 Y2 = FMath::Clamp(Y1 + 1, 0, Height - 1);

	double FracX = X - X1;
	double FracY = Y - Y1;

	auto GetValue = [&](int32 X, int32 Y) -> float { return HeightArray[Y * Width + X]; };

	float C00 = GetValue(X1, Y1);
	float C10 = GetValue(X2, Y1);
	float C01 = GetValue(X1, Y2);
	float C11 = GetValue(X2, Y2);

	// Bilinear interpolation
	return FMath::BiLerp(C00, C10, C01, C11, FracX, FracY);
}

float UAGX_TerrainMeshUtilities::GetBrownianNoise(
	const FVector& Pos, int Octaves, float Scale, float Persistance, float Lacunarity, float Exp)
{
	float Amplitude = 1.0;
	float Frequency = 1.0;
	float Normalization = 0;
	float Total = 0;
	for (int o = 0; o < Octaves; o++)
	{
		float noiseValue = FMath::PerlinNoise3D(Pos * Frequency / Scale) * 0.5f + 0.5f;
		Total += noiseValue * Amplitude;
		Normalization += Amplitude;
		Amplitude *= Persistance;
		Frequency *= Lacunarity;
	}
	Total /= Normalization;
	return FMath::Pow(Total, Exp);
}

float UAGX_TerrainMeshUtilities::GetNoiseHeight(
	const FVector& LocalPos, const FTransform Transform,
	const FAGX_BrownianNoiseParams& NoiseParams)
{
	// To make it easier to align blocks of noise together
	// we project the sampleposition to a plane
	FVector Up = Transform.GetRotation().GetUpVector();
	FVector Pos = Transform.TransformPosition(LocalPos);
	Pos = Pos - Up * FVector::DotProduct(Pos, Up);

	float Noise = GetBrownianNoise(
		Pos, NoiseParams.Octaves, NoiseParams.Scale, NoiseParams.Persistance,
		NoiseParams.Lacunarity, NoiseParams.Exp);

	return Noise * NoiseParams.NoiseHeight + NoiseParams.BaseHeight;
}

float UAGX_TerrainMeshUtilities::GetBedHeight(
	const FVector& LocalPos, const FTransform Transform, const TArray<UMeshComponent*>& BedMeshes,
	const float MaxHeight)
{
	float h = 0.0f;
	FVector Up = Transform.GetRotation().GetUpVector();
	FVector Stop = Transform.TransformPosition(FVector(LocalPos.X, LocalPos.Y, 0.0));
	FVector Start = Stop + Up * MaxHeight;
	FHitResult OutHit;

	for (auto& MeshComponent : BedMeshes)
	{
		if (UAGX_SimpleMeshComponent* ShapeComponent =
				Cast<UAGX_SimpleMeshComponent>(MeshComponent))
		{
			// UAGX_SimpleMeshComponent:
			if (ShapeComponent->LineTraceMesh(OutHit, Start, Stop))
				h = FMath::Max(MaxHeight - OutHit.Distance, h);
		}
		else
		{
			// UMeshComponents:
			FCollisionQueryParams Params;
			if (MeshComponent->LineTraceComponent(OutHit, Start, Stop, Params))
				h = FMath::Max(MaxHeight - OutHit.Distance, h);
		}
	}

	return h;
}
