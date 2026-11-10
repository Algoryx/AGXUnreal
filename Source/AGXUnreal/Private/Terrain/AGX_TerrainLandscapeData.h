// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Terrain/TerrainPagerBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "HAL/CriticalSection.h"

// Standard library includes.
#include <atomic>

class ALandscape;

/** Runtime collision snapshots for the Terrain Pager. Numeric data is safe for its cache worker. */
struct FAGX_TerrainLandscapeData
{
	FAGX_TerrainLandscapeData(ALandscape& Landscape, const FBox& Bounds, double Spacing);
	void Refresh(ALandscape& Landscape);
	void UpdateActiveTiles(
		const TArray<FIntPoint>& Active, const TArray<FTerrainPagerHeightSample>& Samples);
	bool FetchHeights(
		const FVector& Start, int32 VertsX, int32 VertsY, TArray<float>& Heights) const;
	bool GetBaseHeight(double X, double Y, float& Height) const;
	int32 GetSnapshotCount() const;
	int64 GetMissingRequestCount() const;
	FBox Bounds;
	FVector Origin;
	double Spacing;

private:
	void Prune();
	TSet<FIntPoint> LoadedSourceKeys;
	TMap<FIntPoint, FBox2D> ActiveTileBounds;
	struct FSourceChunk
	{
		int32 Quads = 0;
		TArray<float> Heights;
	};
	// Published on the game thread. The native cache worker holds a read lock per fetch.
	mutable FRWLock SourceLock;
	TMap<FIntPoint, TSharedPtr<const FSourceChunk>> SourceChunks;
	FVector2D LandscapeOrigin;
	int32 ComponentQuads = 0;
	mutable std::atomic<int64> MissingSourceRequests {0};
	FVector2D LandscapeSpacing;
	bool TryBaseHeight(double X, double Y, float& Height) const
	{
		const double GX = (X - LandscapeOrigin.X) / LandscapeSpacing.X;
		const double GY = (Y - LandscapeOrigin.Y) / LandscapeSpacing.Y;
		const FIntPoint Key(
			FMath::FloorToInt(GX / ComponentQuads), FMath::FloorToInt(GY / ComponentQuads));
		// Shared edges may be supplied by either adjacent component, including outer edges.
		for (int32 DY = 0; DY >= -1; --DY)
			for (int32 DX = 0; DX >= -1; --DX)
			{
				const FIntPoint Candidate = Key + FIntPoint(DX, DY);
				const double LX = GX - Candidate.X * ComponentQuads;
				const double LY = GY - Candidate.Y * ComponentQuads;
				if (LX < -0.00001 || LY < -0.00001 || LX > ComponentQuads + 0.00001 ||
					LY > ComponentQuads + 0.00001)
					continue;
				const auto* Chunk = SourceChunks.Find(Candidate);
				if (Chunk == nullptr)
					continue;
				const int32 Quads = (*Chunk)->Quads;
				const double CX = FMath::Clamp(LX / ComponentQuads, 0.0, 1.0) * Quads;
				const double CY = FMath::Clamp(LY / ComponentQuads, 0.0, 1.0) * Quads;
				const int32 IX = FMath::Min(FMath::FloorToInt(CX), Quads - 1);
				const int32 IY = FMath::Min(FMath::FloorToInt(CY), Quads - 1);
				const int32 Side = Quads + 1;
				const auto& H = (*Chunk)->Heights;
				const float H00 = H[IX + IY * Side];
				const float H10 = H[IX + 1 + IY * Side];
				const float H01 = H[IX + (IY + 1) * Side];
				const float H11 = H[IX + 1 + (IY + 1) * Side];
				const double FX = CX - IX;
				const double FY = CY - IY;
				Height = FX > FY ? H00 + FX * (H10 - H00) + FY * (H11 - H10)
								 : H00 + FY * (H01 - H00) + FX * (H11 - H01);
				return true;
			}
		return false;
	}
};
