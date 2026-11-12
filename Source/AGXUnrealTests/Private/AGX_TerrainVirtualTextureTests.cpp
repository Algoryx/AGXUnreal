// Copyright 2026, Algoryx Simulation AB.

#if WITH_DEV_AUTOMATION_TESTS

// AGX Dynamics for Unreal includes.
#include "Shapes/HeightFieldShapeBarrier.h"
#include "Terrain/AGX_TerrainLandscapeSource.h"
#include "Terrain/AGX_TerrainRVTHeightSource.h"
#include "Terrain/AGX_TerrainVirtualTexture.h"
#include "Terrain/TerrainBarrier.h"

// Unreal Engine includes.
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAGX_TerrainVirtualTextureSamplingTest, "AGXUnreal.TerrainLandscape.RVTSampling",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAGX_TerrainVirtualTextureSamplingTest::RunTest(const FString&)
{
	FAGX_TerrainVirtualTextureData Data;
	FAGX_TerrainVirtualTextureData::FTile Tile;
	Tile.Id = FIntPoint::ZeroValue;
	Tile.Corner = FVector2D(-100.0, -100.0);
	Tile.Side = 3;
	Tile.Spacing = 100.0;
	for (int32 Y = 0; Y < 3; ++Y)
		for (int32 X = 0; X < 3; ++X)
			Tile.Heights.Add(10.f * X + 20.f * (2 - Y) - 30.f);
	TArray<const FAGX_TerrainVirtualTextureData::FTile*> Candidates {&Tile};
	TestEqual(
		TEXT("Digging is signed negative and interpolates in Unreal Y order"),
		Data.Sample(FVector2D(-50.0, -50.0), Candidates).X, -15.f);
	TestEqual(
		TEXT("Piling is signed positive and interpolates in Unreal Y order"),
		Data.Sample(FVector2D(50.0, 50.0), Candidates).X, 15.f);
	TestEqual(
		TEXT("The complete tile includes neutral values"),
		Data.Sample(FVector2D::ZeroVector, Candidates).X, 0.f);
	TestEqual(
		TEXT("Border sampling does not wrap to another part of the world"),
		Data.Sample(FVector2D(-101.0, -100.0), Candidates).X, 0.f);
	TestEqual(
		TEXT("Last vertex interpolates without reading beyond the tile"),
		Data.Sample(FVector2D(100.0, 100.0), Candidates).X, 30.f);
	FAGX_TerrainVirtualTextureData::FTile Overlap = Tile;
	Overlap.Id = FIntPoint(1, 0);
	for (float& Value : Overlap.Heights)
		Value += 100.f;
	Candidates.Add(&Overlap);
	const float Before = Data.Sample(FVector2D(50.0, 50.0), Candidates).X;
	Candidates.Swap(0, 1);
	TestEqual(
		TEXT("Overlap ownership is independent of active tile iteration order"),
		Data.Sample(FVector2D(50.0, 50.0), Candidates).X, Before);
	TestEqual(TEXT("X gradient"), Data.Sample(FVector2D::ZeroVector, Candidates).Y, 0.1f);
	TestEqual(TEXT("Y gradient"), Data.Sample(FVector2D::ZeroVector, Candidates).Z, 0.2f);
	Candidates.Reset();
	TestEqual(
		TEXT("Missing working set is explicitly invalid"),
		Data.Sample(FVector2D::ZeroVector, Candidates).W, 0.f);
	TestEqual(
		TEXT("An evicted page with no active tile regenerates neutrally"),
		Data.Sample(FVector2D::ZeroVector, Candidates).X, 0.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAGX_TerrainVirtualTextureMipTest, "AGXUnreal.TerrainLandscape.FilteredRVT",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAGX_TerrainVirtualTextureMipTest::RunTest(const FString&)
{
	FAGX_TerrainVirtualTextureData Data;
	FAGX_TerrainVirtualTextureData::FTile Tile;
	Tile.Id = FIntPoint::ZeroValue;
	Tile.Corner = FVector2D::ZeroVector;
	Tile.Side = 8;
	Tile.Spacing = 20;
	for (int32 Y = 0; Y < Tile.Side; ++Y)
		for (int32 X = 0; X < Tile.Side; ++X)
			Tile.Heights.Add((X + Y) % 2 == 0 ? -100.f : 100.f);
	Tile.UpdateMips();
	TArray<const FAGX_TerrainVirtualTextureData::FTile*> Candidates {&Tile};
	TestEqual(
		TEXT("Fine native vertices retain exact heights"),
		Data.Sample(FVector2D::ZeroVector, Candidates).X, 100.f);
	TestEqual(
		TEXT("Coarse texels average alternating digging and piling instead of aliasing"),
		Data.Sample(FVector2D(60, 60), Candidates, 40).X, 0.f);
	Tile.Heights[0] += 40;
	TArray<FIntPoint> Modified {FIntPoint::ZeroValue};
	Tile.UpdateMips(Modified);
	TestEqual(
		TEXT("Incremental mip updates propagate through every parent"), Tile.Mips.Last().Heights[0],
		40.f / 64.f);
	for (int32 Y = 0; Y < Tile.Side; ++Y)
		for (int32 X = 0; X < Tile.Side; ++X)
			Tile.Heights[X + Y * Tile.Side] = 2.f * X + 4.f * (Tile.Side - 1 - Y);
	Tile.UpdateMips();
	const FVector4f Plane = Data.Sample(FVector2D(60, 60), Candidates, 40);
	TestEqual(TEXT("Box filtering retains plane height"), Plane.X, 18.f);
	TestEqual(TEXT("Box filtering retains X gradient"), Plane.Y, 0.1f);
	TestEqual(TEXT("Box filtering retains Y gradient"), Plane.Z, 0.2f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAGX_TerrainLandscapeSourceTest, "AGXUnreal.TerrainLandscape.CollisionGridResampling",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAGX_TerrainLandscapeSourceTest::RunTest(const FString&)
{
	FAGX_TerrainLandscapeSource Source;
	Source.ComponentQuads = 4;
	Source.Spacing = 25;
	Source.LandscapeTransform = FTransform(
		FRotator(0, 30, 0), FVector(20000000, -30000000, 5000000), FVector(200, 300, 100));
	Source.Frame = FTransform(
		Source.LandscapeTransform.GetRotation(), Source.LandscapeTransform.GetTranslation());
	auto Chunk = MakeShared<FAGX_TerrainLandscapeSource::FChunk, ESPMode::ThreadSafe>();
	Chunk->Quads = 2; // Collision mip 1: fewer collision quads than Landscape quads.
	Chunk->Heights = {0, 0, 0, 0, 100, 100, 0, 100, 100};
	Source.Chunks.Add(FIntPoint::ZeroValue, Chunk);
	float Height = -1;
	const FVector Probe = Source.LandscapeTransform.TransformPosition(FVector(1, 0.5, 0));
	TestTrue(
		TEXT("Coarse collision data samples in a translated, rotated, nonuniform frame"),
		Source.Sample(Probe, Height));
	TestEqual(
		TEXT("Triangle interpolation agrees with collision, rather than bilinear height"), Height,
		25.f);
	const FVector Edge = Source.LandscapeTransform.TransformPosition(FVector(4, 4, 0));
	TestTrue(
		TEXT("Outer component edge needs no adjacent loaded proxy"), Source.Sample(Edge, Height));
	TestEqual(TEXT("Exact outer edge height"), Height, 100.f);
	const auto Tile = Source.Resample(Source.Frame.GetTranslation(), 17, 17);
	TestTrue(TEXT("Fine physics grid resamples immutable coarse source data"), Tile.IsValid());
	if (Tile.IsValid())
	{
		TestEqual(TEXT("Complete native tile"), Tile->Heights.Num(), 17 * 17);
		TestEqual(TEXT("Output uses AGX reversed Y row order"), Tile->Heights[0], 0.f);
	}
	Source.Chunks.Empty();
	TestFalse(
		TEXT("Missing source is recoverable, with no fabricated heights"),
		Source.Resample(Source.Frame.GetTranslation(), 17, 17).IsValid());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAGX_TerrainFixedRVTSourceTest, "AGXUnreal.TerrainLandscape.FixedRVTSource",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAGX_TerrainFixedRVTSourceTest::RunTest(const FString&)
{
	constexpr int32 SizeX = 130;
	constexpr int32 SizeY = 78;
	TArray<float> Heights;
	for (int32 Y = 0; Y < SizeY; ++Y)
		for (int32 X = 0; X < SizeX; ++X)
			Heights.Add(static_cast<float>(X + 10 * Y));
	TArray<float> Minimum;
	Minimum.Init(-10000, Heights.Num());
	FTerrainBarrier Native;
	Native.AllocateNative(SizeX, SizeY, 20, Heights, Minimum);
	const FTransform Frame(FRotator(0, 30, 0), FVector(20000000, -30000000, 5000000));
	FAGX_TerrainRVTHeightSource Source(Native, Frame);
	TMap<FIntPoint, uint64> Known;
	const auto Initial = Source.GetTileHeightUpdates(Native, Known);
	TestEqual(TEXT("Rectangular heightfield uses six bounded chunks"), Initial.Num(), 6);
	const auto* Edge =
		Initial.FindByPredicate([](const auto& Tile) { return Tile.Id == FIntPoint(2, 1); });
	TestNotNull(TEXT("Partial outer chunk"), Edge);
	if (Edge == nullptr)
		return false;
	TestEqual(TEXT("Chunk includes gradient halos"), Edge->SideVertices, 67);
	TestEqual(
		TEXT("Outer padding clamps to the final native vertex"), Edge->Heights[66], Heights.Last());
	TestEqual(
		TEXT("Chunk rows use native reversed Y order"), Edge->Heights[66 * 67],
		Heights[127 + 63 * SizeX]);
	const auto Baseline = Source.GetSourceTileSnapshot(Edge->Corner, Edge->SideVertices);
	TestTrue(TEXT("Baseline supports rotated large-world coordinates"), Baseline.IsValid());
	if (Baseline.IsValid())
		TestEqual(TEXT("Initial chunk exactly matches baseline"), Baseline->Heights, Edge->Heights);
	for (float& Height : Heights)
		Height += 123;
	// The raw HeightField setter takes AGX rows; Terrain's getter returns Unreal rows.
	TArray<float> NativeRows;
	NativeRows.Reserve(Heights.Num());
	for (int32 Y = SizeY - 1; Y >= 0; --Y)
		for (int32 X = 0; X < SizeX; ++X)
			NativeRows.Add(Heights[X + Y * SizeX]);
	FHeightFieldShapeBarrier Shape = Native.GetHeightField();
	Shape.SetHeights(NativeRows);
	Known.Empty(); // Renderer re-enabled after missing physics updates.
	const auto Restored = Source.GetTileHeightUpdates(Native, Known);
	const auto* RestoredEdge =
		Restored.FindByPredicate([](const auto& Tile) { return Tile.Id == FIntPoint(2, 1); });
	TestNotNull(TEXT("Rebind refreshes the fixed working set"), RestoredEdge);
	if (RestoredEdge != nullptr && Baseline.IsValid())
	{
		TestEqual(
			TEXT("Rebind reads current authoritative heights"), RestoredEdge->Heights[66],
			Heights.Last());
		const auto Original =
			Source.GetSourceTileSnapshot(RestoredEdge->Corner, RestoredEdge->SideVertices);
		TestEqual(
			TEXT("Rebind preserves the original baseline"), Original->Heights, Baseline->Heights);
	}
	return true;
}

#endif
