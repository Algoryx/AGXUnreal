// Copyright 2026, Algoryx Simulation AB.

#include "Terrain/AGX_TerrainHeightFetcher.h"

// AGX Dynamics for Unreal includes.
#include "Terrain/AGX_TerrainLandscapeSource.h"

// Unreal Engine includes.
#include "Async/Async.h"
#include "EngineGlobals.h"
#include "HAL/PlatformTime.h"
#include "Landscape.h"
#include "LandscapeHeightfieldCollisionComponent.h"
#include "LandscapeInfo.h"

struct FAGX_TerrainHeightFetcher::FSourceCache
{
	struct FEntry
	{
		TSharedPtr<const FAGX_TerrainLandscapeSource::FChunk, ESPMode::ThreadSafe> Chunk;
		uint64 LastUsedFrame = 0;
	};
	TMap<FIntPoint, FEntry> Chunks;
	TFuture<TSharedPtr<const FAGX_TerrainSourceTile, ESPMode::ThreadSafe>> Job;
	FIntPoint JobKey;
};

FAGX_TerrainHeightFetcher::FAGX_TerrainHeightFetcher()
	: SourceCache(MakeUnique<FSourceCache>())
{
}

FAGX_TerrainHeightFetcher::~FAGX_TerrainHeightFetcher() = default;

bool FAGX_TerrainHeightFetcher::FetchHeights(
	const FVector& WorldPosStart, int32 VertsX, int32 VertsY, TArray<float>& OutHeights) const
{
	OutHeights.Reset();
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
		auto Oldest = Requests.CreateConstIterator();
		FIntPoint OldestKey = Oldest.Key();
		uint64 OldestFrame = Oldest.Value().LastUsedFrame;
		for (const auto& Entry : Requests)
			if (Entry.Value.LastUsedFrame < OldestFrame)
			{
				OldestKey = Entry.Key;
				OldestFrame = Entry.Value.LastUsedFrame;
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
	int64 Bytes = 0;
	{
		std::lock_guard<std::mutex> Lock(SnapshotMutex);
		ServiceFrame = GFrameCounter;
		if (SourceCache->Job.IsValid() && SourceCache->Job.IsReady())
		{
			auto Tile = SourceCache->Job.Get();
			SourceCache->Job = {};
			if (Tile.IsValid())
				if (FRequest* Request = Requests.Find(SourceCache->JobKey))
					if (Request->Corner == Tile->Corner && Request->SizeX == Tile->SizeX &&
						Request->SizeY == Tile->SizeY)
						Request->Snapshot = Tile;
		}
		for (auto It = Requests.CreateIterator(); It; ++It)
		{
			if (GFrameCounter > It.Value().LastUsedFrame + 120)
				It.RemoveCurrent();
			else if (!It.Value().Snapshot.IsValid())
				Pending.Emplace(It.Key(), It.Value());
		}
		for (const auto& Entry : Requests)
			if (Entry.Value.Snapshot.IsValid())
				Bytes += Entry.Value.Snapshot->Heights.GetAllocatedSize();
	}
	for (auto It = SourceCache->Chunks.CreateIterator(); It; ++It)
	{
		if (GFrameCounter > It.Value().LastUsedFrame + 120)
			It.RemoveCurrent();
		else
			Bytes += It.Value().Chunk->Heights.GetAllocatedSize();
	}
	// Completed tiles and collision chunks share the existing 32 MiB cache budget.
	// A running job can additionally pin one cache generation and one output tile (<=4 MiB).
	// Leave admission room for the largest supported component, even at a full cache.
	while (Bytes > MaxSourceSnapshotBytes - MaxVerticesPerTile * sizeof(float))
	{
		uint64 OldestFrame = MAX_uint64;
		FIntPoint OldestKey = FIntPoint::ZeroValue;
		bool bChunk = false;
		bool bFound = false;
		for (const auto& Entry : SourceCache->Chunks)
			if (Entry.Value.LastUsedFrame <= OldestFrame)
			{
				OldestFrame = Entry.Value.LastUsedFrame;
				OldestKey = Entry.Key;
				bChunk = bFound = true;
			}
		std::lock_guard<std::mutex> Lock(SnapshotMutex);
		for (const auto& Entry : Requests)
			if (Entry.Value.Snapshot.IsValid() && Entry.Value.LastUsedFrame <= OldestFrame)
			{
				OldestFrame = Entry.Value.LastUsedFrame;
				OldestKey = Entry.Key;
				bChunk = false;
				bFound = true;
			}
		if (!bFound)
			break;
		if (bChunk)
		{
			Bytes -= SourceCache->Chunks[OldestKey].Chunk->Heights.GetAllocatedSize();
			SourceCache->Chunks.Remove(OldestKey);
		}
		else
		{
			Bytes -= Requests[OldestKey].Snapshot->Heights.GetAllocatedSize();
			Requests[OldestKey].Snapshot.Reset();
		}
	}
	if (SourceCache->Job.IsValid() || Pending.IsEmpty())
		return;
	ULandscapeInfo* Info = Landscape.GetLandscapeInfo();
	const int32 ComponentQuads = Landscape.ComponentSizeQuads;
	if (Info == nullptr || ComponentQuads <= 0)
		return;
	// UE keys collision components by SectionBase / CollisionSizeQuads. Probe the small
	// set of supported collision mips, verifying the section base before accepting a hit.
	// No per-frame traversal of all loaded Landscape components is necessary.
	auto FindCollision = [&](const FIntPoint& Key) -> ULandscapeHeightfieldCollisionComponent*
	{
		const FIntPoint Section = Key * ComponentQuads;
		for (int32 Mip = 0; ((Landscape.SubsectionSizeQuads + 1) >> Mip) > 1; ++Mip)
		{
			const int32 Quads =
				(((Landscape.SubsectionSizeQuads + 1) >> Mip) - 1) * Landscape.NumSubsections;
			auto* Component = Info->XYtoCollisionComponentMap.FindRef(Section / Quads);
			if (IsValid(Component) && Component->IsRegistered() &&
				Component->GetSectionBase() == Section)
				return Component;
		}
		return nullptr;
	};
	const double Start = FPlatformTime::Seconds();
	int32 Samples = 0;
	for (const auto& Entry : Pending)
	{
		if (Samples >= 65536 || FPlatformTime::Seconds() - Start >= 0.004)
			break;
		FAGX_TerrainLandscapeSource Source;
		Source.LandscapeTransform = Landscape.LandscapeActorToWorld();
		Source.Frame = SnapshotFrame;
		Source.Spacing = SnapshotSpacing;
		Source.ComponentQuads = ComponentQuads;
		const FRequest& Request = Entry.Value;
		FBox2D GridBounds(ForceInit);
		for (int32 Y : {0, Request.SizeY - 1})
			for (int32 X : {0, Request.SizeX - 1})
			{
				const FVector World =
					Request.Corner + SnapshotFrame.TransformVectorNoScale(
										 FVector(X * SnapshotSpacing, Y * SnapshotSpacing, 0));
				const FVector Grid = Source.LandscapeTransform.InverseTransformPosition(World);
				GridBounds += FVector2D(Grid.X, Grid.Y);
			}
		const FIntPoint Min(
			FMath::FloorToInt(GridBounds.Min.X / ComponentQuads) - 1,
			FMath::FloorToInt(GridBounds.Min.Y / ComponentQuads) - 1);
		const FIntPoint Max(
			FMath::FloorToInt(GridBounds.Max.X / ComponentQuads),
			FMath::FloorToInt(GridBounds.Max.Y / ComponentQuads));
		if (static_cast<int64>(Max.X - Min.X + 1) * (Max.Y - Min.Y + 1) > MaxRequests)
			continue;
		for (int32 Y = Min.Y; Y <= Max.Y; ++Y)
			for (int32 X = Min.X; X <= Max.X; ++X)
			{
				const FIntPoint Key(X, Y);
				auto* Cached = SourceCache->Chunks.Find(Key);
				if (Cached == nullptr && Samples < 65536 &&
					FPlatformTime::Seconds() - Start < 0.004)
				{
					auto* Collision = FindCollision(Key);
					if (!IsValid(Collision) || !Collision->IsRegistered() ||
						Collision->CollisionSizeQuads <= 0)
						continue;
					const int32 Side = Collision->CollisionSizeQuads + 1;
					const int64 Count = static_cast<int64>(Side) * Side;
					if (Count > MaxVerticesPerTile ||
						Bytes + Count * sizeof(float) > MaxSourceSnapshotBytes)
						continue;
					auto Chunk =
						MakeShared<FAGX_TerrainLandscapeSource::FChunk, ESPMode::ThreadSafe>();
					Chunk->Quads = Side - 1;
					Chunk->Heights.SetNumUninitialized(static_cast<int32>(Count));
					Samples += static_cast<int32>(Count);
					// Runtime collision API includes exact edges and respects CollisionMipLevel.
					// Each read is indivisible; check the time budget between component reads.
					if (!Collision->FillHeightTile(Chunk->Heights, 0, Side) ||
						Chunk->Heights.ContainsByPredicate([](float Height)
														   { return !FMath::IsFinite(Height); }))
						continue;
					for (float& Height : Chunk->Heights)
						Height = static_cast<float>(Height - SnapshotFrame.GetTranslation().Z);
					Bytes += Chunk->Heights.GetAllocatedSize();
					SourceCache->Chunks.Add(Key, {Chunk, GFrameCounter});
					Cached = SourceCache->Chunks.Find(Key);
				}
				if (Cached != nullptr)
				{
					Cached->LastUsedFrame = GFrameCounter;
					Source.Chunks.Add(Key, Cached->Chunk);
				}
			}
		bool bCornersAvailable = true;
		for (int32 Y : {0, Request.SizeY - 1})
			for (int32 X : {0, Request.SizeX - 1})
			{
				float Height;
				bCornersAvailable &= Source.Sample(
					Request.Corner + SnapshotFrame.TransformVectorNoScale(
										 FVector(X * SnapshotSpacing, Y * SnapshotSpacing, 0)),
					Height);
			}
		if (!bCornersAvailable)
			continue;
		SourceCache->JobKey = Entry.Key;
		SourceCache->Job = Async(
			EAsyncExecution::ThreadPool,
			[Source = MoveTemp(Source), Corner = Request.Corner, SizeX = Request.SizeX,
			 SizeY = Request.SizeY]()
			{
				TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("AGXUnreal:LandscapeSourceResampling"));
				return Source.Resample(Corner, SizeX, SizeY);
			});
		break;
	}
}

void FAGX_TerrainHeightFetcher::ResetSnapshots()
{
	check(IsInGameThread());
	// Outstanding numeric jobs do not refer to the fetcher, Landscape, or world.
	SourceCache = MakeUnique<FSourceCache>();
	std::lock_guard<std::mutex> Lock(SnapshotMutex);
	Requests.Empty();
}

int32 FAGX_TerrainHeightFetcher::GetSnapshotCount() const
{
	std::lock_guard<std::mutex> Lock(SnapshotMutex);
	return Requests.Num();
}
