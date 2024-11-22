// Copyright 2024, Algoryx Simulation AB.

#include "Terrain/AGX_HeightFieldBoundsComponentVisualizer.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"
#include "Terrain/AGX_HeightFieldBoundsComponent.h"
#include "Terrain/AGX_Shovel.h"
#include "Terrain/AGX_Terrain.h"
#include "Terrain/TerrainPagerBarrier.h"

// Unreal Engine includes.
#include "Landscape.h"
#include "SceneManagement.h"

#define LOCTEXT_NAMESPACE "FAGX_HeightFieldBoundsComponentVisualizer"

namespace AGX_HeightFieldBoundsComponentVisualizer_helpers
{
	void DrawRectangle(
		const FTransform& CenterTransform, double X, double Y, const FLinearColor& Color,
		float LineThickness, FPrimitiveDrawInterface* PDI)
	{
		const double Xh = X / 2.0;
		const double Yh = Y / 2.0;
		const FVector Corner0 = CenterTransform.TransformPositionNoScale(FVector(-Xh, -Yh, 0));
		const FVector Corner1 = CenterTransform.TransformPositionNoScale(FVector(-Xh, Yh, 0));
		const FVector Corner2 = CenterTransform.TransformPositionNoScale(FVector(Xh, Yh, 0));
		const FVector Corner3 = CenterTransform.TransformPositionNoScale(FVector(Xh, -Yh, 0));

		// We use a large DepthBias here so that the lines are drawn clearly and not dark
		// (underneath the landscape).
#if PLATFORM_LINUX
		// As of Unreal Engine 5.2 and NVIDIA graphics drivers 525,
		// SDPG_World + DepthOffset=BIG_NUMBER does not make the lines render on top of the
		// Landscape on Linux. Using SDPG_Foreground instead. The lines are weirdly shaded and not
		// clear when under the Landscape, but better than nothing.
		PDI->DrawLine(Corner0, Corner1, Color, SDPG_Foreground, LineThickness);
		PDI->DrawLine(Corner1, Corner2, Color, SDPG_Foreground, LineThickness);
		PDI->DrawLine(Corner2, Corner3, Color, SDPG_Foreground, LineThickness);
		PDI->DrawLine(Corner3, Corner0, Color, SDPG_Foreground, LineThickness);
#else
		PDI->DrawLine(Corner0, Corner1, Color, SDPG_World, LineThickness, BIG_NUMBER, false);
		PDI->DrawLine(Corner1, Corner2, Color, SDPG_World, LineThickness, BIG_NUMBER, false);
		PDI->DrawLine(Corner2, Corner3, Color, SDPG_World, LineThickness, BIG_NUMBER, false);
		PDI->DrawLine(Corner3, Corner0, Color, SDPG_World, LineThickness, BIG_NUMBER, false);
#endif
	}

	void DrawCircle(
		const FVector& CenterPosGlobal, float Radius, const FLinearColor& Color,
		float LineThickness, FPrimitiveDrawInterface* PDI)
	{
		DrawCircle(
			PDI, CenterPosGlobal, FVector(1, 0, 0), FVector(0, 1, 0), Color, Radius, 24, SDPG_World,
			LineThickness, BIG_NUMBER);
	}

	void DrawTerrainPagerLoadRadii(
		const AAGX_Terrain& Terrain, const FTransform& BoundsTransform,
		FPrimitiveDrawInterface* PDI)
	{
		check(Terrain.bEnableTerrainPaging);
		check(Terrain.SourceLandscape != nullptr);

		for (const FAGX_Shovel& Shovel : Terrain.Shovels)
		{
			AActor* Actor = Shovel.RigidBodyActor;
			if (Actor == nullptr)
				continue;

			TArray<UAGX_RigidBodyComponent*> Bodies;
			Actor->GetComponents(Bodies, false);

			UAGX_RigidBodyComponent** It =
				Bodies.FindByPredicate([&Shovel](UAGX_RigidBodyComponent* Body)
									   { return Shovel.BodyName == Body->GetName(); });

			if (It == nullptr || *It == nullptr)
				continue;

			const UAGX_RigidBodyComponent& ShovelBody = **It;
			if (Shovel.RequiredRadius > 0.0)
			{
				DrawCircle(
					ShovelBody.GetPosition(), Shovel.RequiredRadius, FLinearColor::White, 8.f, PDI);
			}

			if (Shovel.PreloadRadius > 0.0)
			{
				DrawCircle(
					ShovelBody.GetPosition(), Shovel.PreloadRadius, FLinearColor::Gray, 3.f, PDI);
			}
		}

		// Tracked (non-shovel) Rigid Bodies.
		for (const FAGX_TerrainPagingBodyReference& TrackedBody :
			 Terrain.TerrainPagingSettings.TrackedRigidBodies)
		{
			UAGX_RigidBodyComponent* Body = TrackedBody.RigidBody.GetRigidBody();
			if (Body == nullptr)
				continue;

			if (TrackedBody.RequiredRadius > 0.0)
			{
				DrawCircle(
					Body->GetPosition(), TrackedBody.RequiredRadius, FLinearColor::White, 8.f, PDI);
			}

			if (TrackedBody.PreloadRadius > 0.0)
			{
				DrawCircle(
					Body->GetPosition(), TrackedBody.PreloadRadius, FLinearColor::Gray, 3.f, PDI);
			}
		}
	}

	void DrawTerrainPagerLoadedTiles(
		const AAGX_Terrain& Terrain, const FTransform& BoundsTransform,
		FPrimitiveDrawInterface* PDI)
	{
		check(Terrain.bEnableTerrainPaging);
		check(Terrain.SourceLandscape != nullptr);
		check(Terrain.HasNativeTerrainPager());

		const FAGX_TerrainPagingSettings& Settings = Terrain.TerrainPagingSettings;

		const FTerrainPagerBarrier* TPBarrier = Terrain.GetNativeTerrainPager();
		if (TPBarrier == nullptr)
			return;

		const auto QuadSize = Terrain.SourceLandscape->GetActorScale().X;
		const int32 TileNumQuadsSide = FMath::RoundToInt(Settings.TileSize / QuadSize);
		const double TileSize = QuadSize * TileNumQuadsSide;

		const FVector BoundsPosGlobal = BoundsTransform.GetLocation();

		for (auto TileTransform : TPBarrier->GetActiveTileTransforms())
		{
			const FVector TilePosGlobal = TileTransform.GetLocation();

			// We want to draw the tiles at the same height as the bounds to align everything. The
			// tiles may have a z-offset that depends on the first loaded tile average height (set
			// by AGX Dynamics automatically).
			TileTransform.SetLocation(FVector(TilePosGlobal.X, TilePosGlobal.Y, BoundsPosGlobal.Z));
			DrawRectangle(
				TileTransform, TileSize, TileSize, Settings.LoadedTileOutlineColor,
				2.f * Settings.TileOutlineThickness, PDI);
		}
	}

	void DrawTerrainPagerGrid(
		const AAGX_Terrain& Terrain, const FTransform& BoundsTransform, const FVector& HalfExtents,
		FPrimitiveDrawInterface* PDI)
	{
		check(Terrain.bEnableTerrainPaging);
		check(Terrain.SourceLandscape != nullptr);

		const FAGX_TerrainPagingSettings& Settings = Terrain.TerrainPagingSettings;

		const auto QuadSize = Terrain.SourceLandscape->GetActorScale().X;
		const int32 TileNumQuadsSide = FMath::RoundToInt(Settings.TileSize / QuadSize);
		const int32 TileOverlapNumQuads = FMath::RoundToInt(Settings.TileOverlap / QuadSize);

		const double TileSize = QuadSize * TileNumQuadsSide;
		const double TileOverlap = QuadSize * TileOverlapNumQuads;
		const double Tolerance = QuadSize / 1000.0;

		// p and n postfixes are for positive direction and negative direction respectively.
		// In the positive x and negative y directions, the length span of the tiles are given by
		// n * TileSize - (n - 1) * TileOverlap, and for the other directions given by
		// n * (TileSize - Overlap). This is due to the placement of the TerrainPager reference
		// position in relation to the Tiles.
		int32 NumTilesXp = static_cast<int32>(HalfExtents.X / (TileSize - TileOverlap));
		if (NumTilesXp * TileSize - (NumTilesXp - 1) * TileOverlap > HalfExtents.X + Tolerance)
		{
			// We overshot by one Tile (see comment above).
			NumTilesXp--;
		}

		int32 NumTilesXn = static_cast<int32>(HalfExtents.X / (TileSize - TileOverlap));

		int32 NumTilesYn = static_cast<int32>(HalfExtents.Y / (TileSize - TileOverlap));
		if (NumTilesYn * TileSize - (NumTilesYn - 1) * TileOverlap > HalfExtents.Y + Tolerance)
		{
			// We overshot by one Tile (see comment above).
			NumTilesYn--;
		}

		int32 NumTilesYp = static_cast<int32>(HalfExtents.Y / (TileSize - TileOverlap));

		for (int32 Y = NumTilesYp; Y > -NumTilesYn; Y--)
		{
			for (int32 X = -NumTilesXn; X < NumTilesXp; X++)
			{
				// The 0,0 Tile is actually extending in the x+ and y- direction in Unreal because
				// of AGX Dynamics coordinate system having flipped y-axis. That's why we negate the
				// y-directions.
				const double StartPosLocalX =
					static_cast<double>(X) * (TileSize - TileOverlap) + TileSize / 2.0;
				const double StartPosLocalY =
					static_cast<double>(Y) * (TileSize - TileOverlap) - TileSize / 2.0;
				const FVector StartPosGlobal = BoundsTransform.TransformPositionNoScale(
					FVector(StartPosLocalX, StartPosLocalY, 0.0));

				const FTransform RectangleTransform(BoundsTransform.GetRotation(), StartPosGlobal);
				DrawRectangle(
					RectangleTransform, TileSize, -TileSize, Settings.TileOutlineColor,
					Settings.TileOutlineThickness, PDI);
			}
		}
	}

	void DrawTerrainPagerDebugRendering(
		const AAGX_Terrain& Terrain, const FTransform& BoundsTransform, const FVector& HalfExtents,
		FPrimitiveDrawInterface* PDI)
	{
		if (!Terrain.bEnableTerrainPaging)
			return;

		if (Terrain.SourceLandscape == nullptr)
			return;

		if (Terrain.TerrainPagingSettings.bDrawDebugGrid)
		{
			DrawTerrainPagerGrid(Terrain, BoundsTransform, HalfExtents, PDI);

			if (Terrain.HasNativeTerrainPager())
				DrawTerrainPagerLoadedTiles(Terrain, BoundsTransform, PDI);
		}

		if (Terrain.TerrainPagingSettings.bDrawDebugLoadRadii)
		{
			DrawTerrainPagerLoadRadii(Terrain, BoundsTransform, PDI);
		}
	}

	void DrawBounds(
		const FTransform& Transform, const FVector& HalfExtent, const FLinearColor& Color,
		float LineThickness, FPrimitiveDrawInterface* PDI)
	{
		const FBox BoundingBox(-HalfExtent, HalfExtent);
		DrawWireBox(
			PDI, Transform.ToMatrixNoScale(), BoundingBox, Color, SDPG_Foreground, LineThickness,
			0.f, true);
	}
}

void FAGX_HeightFieldBoundsComponentVisualizer::DrawVisualization(
	const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	using namespace AGX_HeightFieldBoundsComponentVisualizer_helpers;
	const UAGX_HeightFieldBoundsComponent* Bounds =
		Cast<const UAGX_HeightFieldBoundsComponent>(Component);
	if (Bounds == nullptr)
		return;

	if (auto BoundsInfo = Bounds->GetUserSetBounds())
	{
		DrawBounds(BoundsInfo->Transform, BoundsInfo->HalfExtent, FLinearColor::Gray, 2.f, PDI);
	}

	auto BoundsInfo = Bounds->GetLandscapeAdjustedBounds();
	if (BoundsInfo.IsSet())
	{
		DrawBounds(BoundsInfo->Transform, BoundsInfo->HalfExtent, FLinearColor::Green, 4.f, PDI);
	}

	// If part of a Terrain using Terrain Paging, draw debug rendering for it.
	AAGX_Terrain* Terrain = Cast<AAGX_Terrain>(Bounds->GetOwner());
	if (BoundsInfo.IsSet() && Terrain != nullptr && Terrain->bEnableTerrainPaging)
	{
		DrawTerrainPagerDebugRendering(
			*Terrain, BoundsInfo->Transform, BoundsInfo->HalfExtent, PDI);
	}
}

#undef LOCTEXT_NAMESPACE
