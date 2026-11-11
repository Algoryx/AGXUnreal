// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Terrain/AGX_TerrainHeightFetcher.h"

/** Immutable collision-grid data. Resampling uses numbers only, including after proxy unload. */
struct FAGX_TerrainLandscapeSource
{
	struct FChunk
	{
		int32 Quads = 0;
		TArray<float> Heights;
	};
	TMap<FIntPoint, TSharedPtr<const FChunk, ESPMode::ThreadSafe>> Chunks;
	FTransform LandscapeTransform;
	FTransform Frame;
	int32 ComponentQuads = 0;
	double Spacing = 0.0;

	bool Sample(const FVector& World, float& Height) const
	{
		if (ComponentQuads <= 0)
			return false;
		const FVector Grid = LandscapeTransform.InverseTransformPosition(World);
		const FIntPoint Key(
			FMath::FloorToInt((Grid.X + 0.00001) / ComponentQuads),
			FMath::FloorToInt((Grid.Y + 0.00001) / ComponentQuads));
		// Either adjacent chunk can supply an exact edge, including the Landscape's outer edge.
		for (int32 DY = 0; DY >= -1; --DY)
			for (int32 DX = 0; DX >= -1; --DX)
			{
				const FIntPoint Candidate = Key + FIntPoint(DX, DY);
				const double X = Grid.X - Candidate.X * ComponentQuads;
				const double Y = Grid.Y - Candidate.Y * ComponentQuads;
				if (X < -0.00001 || Y < -0.00001 || X > ComponentQuads + 0.00001 ||
					Y > ComponentQuads + 0.00001)
					continue;
				const auto* Found = Chunks.Find(Candidate);
				if (Found == nullptr)
					continue;
				const FChunk& Chunk = **Found;
				const double CX = FMath::Clamp(X / ComponentQuads, 0.0, 1.0) * Chunk.Quads;
				const double CY = FMath::Clamp(Y / ComponentQuads, 0.0, 1.0) * Chunk.Quads;
				const int32 IX = FMath::Min(FMath::FloorToInt(CX), Chunk.Quads - 1);
				const int32 IY = FMath::Min(FMath::FloorToInt(CY), Chunk.Quads - 1);
				const int32 Side = Chunk.Quads + 1;
				const float H00 = Chunk.Heights[IX + IY * Side];
				const float H10 = Chunk.Heights[IX + 1 + IY * Side];
				const float H01 = Chunk.Heights[IX + (IY + 1) * Side];
				const float H11 = Chunk.Heights[IX + 1 + (IY + 1) * Side];
				const double FX = CX - IX;
				const double FY = CY - IY;
				// Match Landscape collision's triangle diagonal, also for nonzero collision mips.
				Height = static_cast<float>(
					FX > FY ? H00 + FX * (H10 - H00) + FY * (H11 - H10)
							: H00 + FY * (H01 - H00) + FX * (H11 - H01));
				return true;
			}
		return false;
	}

	TSharedPtr<const FAGX_TerrainSourceTile, ESPMode::ThreadSafe> Resample(
		const FVector& Corner, int32 SizeX, int32 SizeY) const
	{
		auto Tile = MakeShared<FAGX_TerrainSourceTile, ESPMode::ThreadSafe>();
		Tile->Corner = Corner;
		Tile->SizeX = SizeX;
		Tile->SizeY = SizeY;
		Tile->Heights.Reserve(SizeX * SizeY);
		for (int32 Y = SizeY - 1; Y >= 0; --Y)
			for (int32 X = 0; X < SizeX; ++X)
			{
				float Height;
				if (!Sample(
						Corner + Frame.TransformVectorNoScale(FVector(X * Spacing, Y * Spacing, 0)),
						Height))
					return nullptr;
				Tile->Heights.Add(Height);
			}
		return Tile;
	}
};
