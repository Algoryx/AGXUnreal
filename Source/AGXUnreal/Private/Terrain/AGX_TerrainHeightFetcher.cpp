// Copyright 2026, Algoryx Simulation AB.

#include "Terrain/AGX_TerrainHeightFetcher.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Terrain/AGX_Terrain.h"

// Unreal Engine includes.
#include "EngineGlobals.h"
#include "HAL/PlatformTime.h"
#include "Landscape.h"

bool FAGX_TerrainHeightFetcher::FetchHeights(
	const FVector& WorldPosStart, int32 VertsX, int32 VertsY, TArray<float>& OutHeights) const
{
	auto Snapshot = GetOrRequestSnapshot(WorldPosStart, VertsX, VertsY);
	if (!Snapshot.IsValid())
		return false;
	OutHeights = Snapshot->Heights;
	return true;
}

void FAGX_TerrainHeightFetcher::SetTerrain(AAGX_Terrain* InTerrain)
{
	Terrain = InTerrain;
}

AAGX_Terrain* FAGX_TerrainHeightFetcher::GetTerrain() const
{
	return Terrain;
}

void FAGX_TerrainHeightFetcher::ConfigureSnapshots(
	const FTransform& Frame, const FVector2D& HalfExtent, double Spacing)
{
	check(IsInGameThread());
	SnapshotFrame = Frame;
	SnapshotHalfExtent = HalfExtent;
	SnapshotSpacing = Spacing;
	bUseSnapshots = true;
}

TSharedPtr<const FAGX_TerrainSourceTile, ESPMode::ThreadSafe>
FAGX_TerrainHeightFetcher::GetOrRequestSnapshot(
	const FVector& Corner, int32 SizeX, int32 SizeY) const
{
	if (!bUseSnapshots || SnapshotSpacing <= 0.0 || SizeX < 2 || SizeY < 2 ||
		static_cast<int64>(SizeX) * SizeY > MaxVerticesPerTile)
		return nullptr;
	const FVector Local = SnapshotFrame.InverseTransformPositionNoScale(Corner);
	if (Local.X < -SnapshotHalfExtent.X - 0.01 || Local.Y < -SnapshotHalfExtent.Y - 0.01 ||
		Local.X + (SizeX - 1) * SnapshotSpacing > SnapshotHalfExtent.X + 0.01 ||
		Local.Y + (SizeY - 1) * SnapshotSpacing > SnapshotHalfExtent.Y + 0.01)
		return nullptr;
	const FIntPoint Key(
		FMath::RoundToInt(Local.X / SnapshotSpacing), FMath::RoundToInt(Local.Y / SnapshotSpacing));
	std::lock_guard<std::mutex> Lock(SnapshotMutex);
	if (FRequest* Request = Requests.Find(Key))
	{
		Request->LastUsedFrame = ServiceFrame;
		return Request->SizeX == SizeX && Request->SizeY == SizeY ? Request->Snapshot : nullptr;
	}
	if (Requests.Num() >= MaxRequests)
	{
		// Evict the least recently used entry; native failures are retryable on the next step.
		auto Oldest = Requests.CreateConstIterator();
		FIntPoint OldestKey = Oldest.Key();
		uint64 OldestFrame = Oldest.Value().LastUsedFrame;
		for (auto It = Requests.CreateIterator(); It; ++It)
			if (It.Value().LastUsedFrame < OldestFrame)
			{
				OldestKey = It.Key();
				OldestFrame = It.Value().LastUsedFrame;
			}
		Requests.Remove(OldestKey);
	}
	Requests.Add(Key, {Corner, SizeX, SizeY, ServiceFrame, nullptr});
	return nullptr;
}

void FAGX_TerrainHeightFetcher::ServiceRequests(ALandscape& Landscape)
{
	check(IsInGameThread());
	TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("AGXUnreal:LandscapeSourceSnapshots"));
	if (LastServiceFrame == GFrameCounter)
		return;
	LastServiceFrame = GFrameCounter;
	TArray<TPair<FIntPoint, FRequest>> Pending;
	{
		std::lock_guard<std::mutex> Lock(SnapshotMutex);
		ServiceFrame = GFrameCounter;
		for (auto It = Requests.CreateIterator(); It; ++It)
		{
			if (GFrameCounter > It.Value().LastUsedFrame + 120)
				It.RemoveCurrent();
			else if (!It.Value().Snapshot.IsValid())
				Pending.Emplace(It.Key(), It.Value());
		}
	}
	if (BuildingTile.IsValid() && !Pending.ContainsByPredicate(
									  [this](const auto& Entry)
									  {
										  return Entry.Key == BuildingKey &&
												 Entry.Value.Corner == BuildingTile->Corner &&
												 Entry.Value.SizeX == BuildingTile->SizeX &&
												 Entry.Value.SizeY == BuildingTile->SizeY;
									  }))
		BuildingTile.Reset();
	Pending.StableSort(
		[this](const auto& A, const auto& B)
		{ return BuildingTile.IsValid() && A.Key == BuildingKey && B.Key != BuildingKey; });
	const double Start = FPlatformTime::Seconds();
	int32 Samples = 0;
	for (const auto& Entry : Pending)
	{
		const FRequest& Request = Entry.Value;
		if (Samples >= 65536)
			break;
		if (!BuildingTile.IsValid())
		{
			BuildingKey = Entry.Key;
			BuildingTile = MakeShared<FAGX_TerrainSourceTile, ESPMode::ThreadSafe>();
			BuildingTile->Corner = Request.Corner;
			BuildingTile->SizeX = Request.SizeX;
			BuildingTile->SizeY = Request.SizeY;
			BuildingTile->Heights.Reserve(Request.SizeX * Request.SizeY);
		}
		auto Tile = BuildingTile;
		bool bAvailable = true;
		while (Tile->Heights.Num() < Request.SizeX * Request.SizeY)
		{
			// Bound work inside a large tile too. Resume only on the game thread next frame.
			if (Samples >= 65536 ||
				((Samples & 63) == 0 && FPlatformTime::Seconds() - Start >= 0.004))
				return;
			const int32 Index = Tile->Heights.Num();
			const int32 X = Index % Request.SizeX;
			const int32 Y = Request.SizeY - 1 - Index / Request.SizeX;
			const FVector Position =
				Request.Corner + SnapshotFrame.TransformVectorNoScale(
									 FVector(X * SnapshotSpacing, Y * SnapshotSpacing, 0.0));
			++Samples;
			const TOptional<float> Height = Landscape.GetHeightAtLocation(Position);
			if (!Height.IsSet())
			{
				bAvailable = false;
				break;
			}
			Tile->Heights.Add(
				static_cast<float>(
					SnapshotFrame
						.InverseTransformPositionNoScale(FVector(Position.X, Position.Y, *Height))
						.Z));
		}
		BuildingTile.Reset();
		if (bAvailable)
		{
			std::lock_guard<std::mutex> Lock(SnapshotMutex);
			if (FRequest* Current = Requests.Find(Entry.Key))
				if (Current->Corner == Tile->Corner && Current->SizeX == Tile->SizeX &&
					Current->SizeY == Tile->SizeY)
					Current->Snapshot = Tile;
			int64 Bytes = 0;
			for (const auto& Cached : Requests)
				if (Cached.Value.Snapshot.IsValid())
					Bytes += Cached.Value.Snapshot->Heights.GetAllocatedSize();
			while (Bytes > MaxSourceSnapshotBytes)
			{
				FIntPoint OldestKey = FIntPoint::ZeroValue;
				uint64 OldestFrame = MAX_uint64;
				bool bFound = false;
				for (const auto& Cached : Requests)
				{
					if (Cached.Key != Entry.Key && Cached.Value.Snapshot.IsValid() &&
						Cached.Value.LastUsedFrame <= OldestFrame)
					{
						bFound = true;
						OldestKey = Cached.Key;
						OldestFrame = Cached.Value.LastUsedFrame;
					}
				}
				if (!bFound)
					break;
				Bytes -= Requests[OldestKey].Snapshot->Heights.GetAllocatedSize();
				Requests.Remove(OldestKey);
			}
		}
	}
}

void FAGX_TerrainHeightFetcher::ResetSnapshots()
{
	check(IsInGameThread());
	BuildingTile.Reset();
	std::lock_guard<std::mutex> Lock(SnapshotMutex);
	Requests.Empty();
}

int32 FAGX_TerrainHeightFetcher::GetSnapshotCount() const
{
	std::lock_guard<std::mutex> Lock(SnapshotMutex);
	return Requests.Num();
}
