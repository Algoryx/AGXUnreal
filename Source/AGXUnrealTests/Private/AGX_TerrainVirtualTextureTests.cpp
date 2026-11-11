// Copyright 2026, Algoryx Simulation AB.

#if WITH_DEV_AUTOMATION_TESTS

// AGX Dynamics for Unreal includes.
#include "Terrain/AGX_TerrainVirtualTexture.h"

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

#endif
