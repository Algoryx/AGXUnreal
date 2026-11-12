// Copyright 2026, Algoryx Simulation AB.

#include "Terrain/AGX_TerrainRVTHeightSource.h"

// AGX Dynamics for Unreal includes.
#include "Terrain/TerrainBarrier.h"

FAGX_TerrainRVTHeightSource::FAGX_TerrainRVTHeightSource(
	const FTerrainBarrier& Native, const FTransform& InFrame)
	: Frame(InFrame)
	, GridSize(Native.GetGridSizeX(), Native.GetGridSizeY())
	, HalfSize(Native.GetSize() * 0.5)
	, Spacing(Native.GetElementSize())
{
	check(IsInGameThread());
	Native.GetHeights(InitialHeights, false);
	CurrentHeights = InitialHeights;
}

FIntPoint FAGX_TerrainRVTHeightSource::GetChunkCount() const
{
	return FIntPoint(
		FMath::DivideAndRoundUp(GridSize.X - 1, ChunkQuads),
		FMath::DivideAndRoundUp(GridSize.Y - 1, ChunkQuads));
}

float FAGX_TerrainRVTHeightSource::Sample(const TArray<float>& Heights, int32 X, int32 Y) const
{
	return Heights
		[FMath::Clamp(X, 0, GridSize.X - 1) + FMath::Clamp(Y, 0, GridSize.Y - 1) * GridSize.X];
}

FTerrainTileHeights FAGX_TerrainRVTHeightSource::MakeTile(
	const FIntPoint& Id, bool bFullRefresh) const
{
	FTerrainTileHeights Tile;
	Tile.Id = Id;
	const FIntPoint Start = Id * ChunkQuads - FIntPoint(Halo);
	Tile.Corner = Frame.TransformPositionNoScale(
		FVector(Start.X * Spacing - HalfSize.X, Start.Y * Spacing - HalfSize.Y, 0));
	Tile.SideVertices = ChunkSide;
	Tile.Spacing = Spacing;
	Tile.bFullRefresh = bFullRefresh;
	if (bFullRefresh)
	{
		Tile.Heights.Reserve(ChunkSide * ChunkSide);
		for (int32 Y = ChunkSide - 1; Y >= 0; --Y)
			for (int32 X = 0; X < ChunkSide; ++X)
				Tile.Heights.Add(Sample(CurrentHeights, Start.X + X, Start.Y + Y));
	}
	return Tile;
}

TArray<FTerrainTileHeights> FAGX_TerrainRVTHeightSource::GetTileHeightUpdates(
	const FTerrainBarrier& Native, TMap<FIntPoint, uint64>& KnownTiles)
{
	check(IsInGameThread());
	TArray<FTerrainTileHeights> Updates;
	if (GridSize.X < 2 || GridSize.Y < 2 ||
		InitialHeights.Num() != static_cast<int64>(GridSize.X) * GridSize.Y)
		return Updates;
	// A renderer which was disabled has missed native change lists. Refresh authoritative
	// heights on rebind, while keeping the original baseline captured before simulation.
	Native.GetHeights(CurrentHeights, !KnownTiles.IsEmpty());
	const FIntPoint Count = GetChunkCount();
	TMap<FIntPoint, int32> Changed;
	for (int32 Y = 0; Y < Count.Y; ++Y)
		for (int32 X = 0; X < Count.X; ++X)
		{
			const FIntPoint Id(X, Y);
			if (!KnownTiles.Contains(Id))
			{
				Changed.Add(Id, Updates.Num());
				Updates.Add(MakeTile(Id, true));
			}
		}
	for (const auto& Vertex : Native.GetModifiedVertices())
	{
		const FIntPoint Position(std::get<0>(Vertex), std::get<1>(Vertex));
		const FIntPoint Min(
			FMath::Max(
				0,
				FMath::CeilToInt(static_cast<double>(Position.X - ChunkQuads - Halo) / ChunkQuads)),
			FMath::Max(
				0, FMath::CeilToInt(
					   static_cast<double>(Position.Y - ChunkQuads - Halo) / ChunkQuads)));
		const FIntPoint Max(
			FMath::Min(Count.X - 1, (Position.X + Halo) / ChunkQuads),
			FMath::Min(Count.Y - 1, (Position.Y + Halo) / ChunkQuads));
		for (int32 Y = Min.Y; Y <= Max.Y; ++Y)
			for (int32 X = Min.X; X <= Max.X; ++X)
			{
				const FIntPoint Id(X, Y);
				int32* Index = Changed.Find(Id);
				if (Index == nullptr)
				{
					const int32 Added = Updates.Add(MakeTile(Id, false));
					Index = &Changed.Add(Id, Added);
				}
				FTerrainTileHeights& Tile = Updates[*Index];
				if (Tile.bFullRefresh)
					continue;
				const FIntPoint Local = Position - (Id * ChunkQuads - FIntPoint(Halo));
				// Replicate outer-edge edits into the clamped padding, including the halo.
				const int32 X1 = Position.X == 0 ? 0 : Local.X;
				const int32 X2 = Position.X == GridSize.X - 1 ? ChunkSide - 1 : Local.X;
				const int32 Y1 = Position.Y == 0 ? 0 : Local.Y;
				const int32 Y2 = Position.Y == GridSize.Y - 1 ? ChunkSide - 1 : Local.Y;
				for (int32 Row = Y1; Row <= Y2; ++Row)
					for (int32 Column = X1; Column <= X2; ++Column)
					{
						Tile.ModifiedVertices.Emplace(Column, ChunkSide - 1 - Row);
						Tile.Heights.Add(CurrentHeights[Position.X + Position.Y * GridSize.X]);
					}
			}
	}
	KnownTiles.Reset();
	for (int32 Y = 0; Y < Count.Y; ++Y)
		for (int32 X = 0; X < Count.X; ++X)
			KnownTiles.Add(FIntPoint(X, Y), 1);
	return Updates;
}

TSharedPtr<const FAGX_TerrainSourceTile, ESPMode::ThreadSafe>
FAGX_TerrainRVTHeightSource::GetSourceTileSnapshot(const FVector& Corner, int32 Side) const
{
	if (Side < 2 || static_cast<int64>(Side) * Side > 1024 * 1024 || InitialHeights.IsEmpty())
		return nullptr;
	const FVector Local = Frame.InverseTransformPositionNoScale(Corner);
	const FIntPoint Start(
		FMath::RoundToInt((Local.X + HalfSize.X) / Spacing),
		FMath::RoundToInt((Local.Y + HalfSize.Y) / Spacing));
	auto Snapshot = MakeShared<FAGX_TerrainSourceTile, ESPMode::ThreadSafe>();
	Snapshot->Corner = Corner;
	Snapshot->SizeX = Snapshot->SizeY = Side;
	Snapshot->Heights.Reserve(Side * Side);
	for (int32 Y = Side - 1; Y >= 0; --Y)
		for (int32 X = 0; X < Side; ++X)
			Snapshot->Heights.Add(Sample(InitialHeights, Start.X + X, Start.Y + Y));
	return Snapshot;
}

int64 FAGX_TerrainRVTHeightSource::GetRetainedBytes() const
{
	return InitialHeights.GetAllocatedSize() + CurrentHeights.GetAllocatedSize();
}
