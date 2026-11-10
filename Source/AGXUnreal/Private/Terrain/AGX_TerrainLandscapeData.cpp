// Copyright 2026, Algoryx Simulation AB.

#include "Terrain/AGX_TerrainLandscapeData.h"

// Unreal Engine includes.
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Landscape.h"
#include "LandscapeComponent.h"
#include "LandscapeHeightfieldCollisionComponent.h"
#include "LandscapeProxy.h"
#include "Misc/ScopeRWLock.h"

FAGX_TerrainLandscapeData::FAGX_TerrainLandscapeData(
	ALandscape& Landscape, const FBox& InBounds, double InSpacing)
	: Bounds(InBounds)
	, Origin(InBounds.GetCenter())
	, Spacing(InSpacing)
{
	LandscapeOrigin = FVector2D(Landscape.LandscapeActorToWorld().GetLocation());
	LandscapeSpacing = FVector2D(Landscape.GetActorScale());
	ComponentQuads = Landscape.ComponentSizeQuads;
	Refresh(Landscape);
}

void FAGX_TerrainLandscapeData::Refresh(ALandscape& Landscape)
{
	check(IsInGameThread());
	LoadedSourceKeys.Reset();
	for (TActorIterator<ALandscapeProxy> It(Landscape.GetWorld()); It; ++It)
	{
		ALandscapeProxy& Proxy = **It;
		if (Proxy.GetLandscapeGuid() != Landscape.GetLandscapeGuid() ||
			Proxy.IsActorBeingDestroyed())
			continue;
		for (ULandscapeComponent* Component : Proxy.LandscapeComponents)
		{
			if (!IsValid(Component) || !Component->IsRegistered())
				continue;
			if (Component->ComponentSizeQuads == ComponentQuads)
			{
				const FIntPoint Key = Component->GetSectionBase() / ComponentQuads;
				const FVector2D Min =
					LandscapeOrigin + FVector2D(Component->GetSectionBase()) * LandscapeSpacing;
				const FVector2D Max = Min + FVector2D(ComponentQuads) * LandscapeSpacing;
				const bool InCoverage = Max.X >= Bounds.Min.X && Max.Y >= Bounds.Min.Y &&
										Min.X <= Bounds.Max.X && Min.Y <= Bounds.Max.Y;
				if (InCoverage)
					LoadedSourceKeys.Add(Key);
				auto* Collision = Component->GetCollisionComponent();
				if (InCoverage && !SourceChunks.Contains(Key) && IsValid(Collision) &&
					Collision->IsRegistered())
				{
					const int32 Quads = Collision->CollisionSizeQuads;
					if (Quads <= 0)
						continue;
					const int32 Side = Quads + 1;
					auto Chunk = MakeShared<FSourceChunk>();
					Chunk->Quads = Quads;
					Chunk->Heights.SetNumUninitialized(Side * Side);
					// Runtime API reads the live complex collision heightfield, including exact
					// edge vertices. It also handles nonzero collision mip levels without querying
					// an adjacent, possibly unloaded proxy at shared edges.
					if (Collision->FillHeightTile(Chunk->Heights, 0, Side) &&
						!Chunk->Heights.ContainsByPredicate([](float Height)
															{ return !FMath::IsFinite(Height); }))
					{
						FWriteScopeLock Lock(SourceLock);
						SourceChunks.Add(Key, MoveTemp(Chunk));
					}
				}
			}
		}
	}
	Prune();
}

void FAGX_TerrainLandscapeData::UpdateActiveTiles(
	const TArray<FIntPoint>& ActiveIds, const TArray<FTerrainPagerHeightSample>& Samples)
{
	TSet<FIntPoint> Active;
	Active.Append(ActiveIds);
	for (auto It = ActiveTileBounds.CreateIterator(); It; ++It)
		if (!Active.Contains(It.Key()))
			It.RemoveCurrent();
	for (const auto& Sample : Samples)
		if (Active.Contains(Sample.TileId))
			ActiveTileBounds.FindOrAdd(Sample.TileId, FBox2D(ForceInit)) +=
				FVector2D(Sample.Position);
	Prune();
}

void FAGX_TerrainLandscapeData::Prune()
{
	TSet<FIntPoint> Needed = LoadedSourceKeys;
	const FVector2D ComponentSize = LandscapeSpacing * ComponentQuads;
	for (const auto& Entry : ActiveTileBounds)
	{
		const FBox2D& TileBounds = Entry.Value;
		if (!TileBounds.bIsValid)
			continue;
		const FVector2D Min = (TileBounds.Min - LandscapeOrigin) / ComponentSize;
		const FVector2D Max = (TileBounds.Max - LandscapeOrigin) / ComponentSize;
		for (int32 Y = FMath::FloorToInt(Min.Y) - 1; Y <= FMath::FloorToInt(Max.Y); ++Y)
			for (int32 X = FMath::FloorToInt(Min.X) - 1; X <= FMath::FloorToInt(Max.X); ++X)
				Needed.Add(FIntPoint(X, Y));
	}
	FWriteScopeLock Lock(SourceLock);
	for (auto It = SourceChunks.CreateIterator(); It; ++It)
		if (!Needed.Contains(It.Key()))
			It.RemoveCurrent();
}

bool FAGX_TerrainLandscapeData::GetBaseHeight(double X, double Y, float& Height) const
{
	FReadScopeLock Lock(SourceLock);
	return TryBaseHeight(X, Y, Height);
}

int32 FAGX_TerrainLandscapeData::GetSnapshotCount() const
{
	FReadScopeLock Lock(SourceLock);
	return SourceChunks.Num();
}

int64 FAGX_TerrainLandscapeData::GetMissingRequestCount() const
{
	return MissingSourceRequests.load();
}

bool FAGX_TerrainLandscapeData::FetchHeights(
	const FVector& Start, int32 VertsX, int32 VertsY, TArray<float>& Heights) const
{
	// Called on the AGX worker. Never wait for proxy loading or game-thread work.
	Heights.Reset();
	if (Start.X < Bounds.Min.X - 0.01 || Start.Y < Bounds.Min.Y - 0.01 ||
		Start.X + (VertsX - 1) * Spacing > Bounds.Max.X + 0.01 ||
		Start.Y + (VertsY - 1) * Spacing > Bounds.Max.Y + 0.01)
		return false;
	FReadScopeLock Lock(SourceLock);
	Heights.Reserve(VertsX * VertsY);
	for (int32 Y = VertsY - 1; Y >= 0; --Y)
		for (int32 X = 0; X < VertsX; ++X)
		{
			float Height;
			if (!TryBaseHeight(Start.X + X * Spacing, Start.Y + Y * Spacing, Height))
			{
				++MissingSourceRequests;
				Heights.Reset();
				return false;
			}
			Heights.Add(Height - Origin.Z);
		}
	return true;
}
