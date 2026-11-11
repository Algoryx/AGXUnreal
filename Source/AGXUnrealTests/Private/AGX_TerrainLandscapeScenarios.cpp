// Copyright 2026, Algoryx Simulation AB.

#include "AGX_TerrainLandscapeTestUtilities.h"

// AGX Dynamics for Unreal includes.
#include "AGX_MotionControl.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_Simulation.h"
#include "Terrain/AGX_HeightFieldBoundsComponent.h"
#include "Terrain/AGX_LandscapeMaterialLibrary.h"
#include "Terrain/AGX_Terrain.h"
#include "Terrain/AGX_TerrainPartitionedLandscapeRendererComponent.h"
#include "Terrain/TerrainPagerBarrier.h"

// Unreal Engine includes.
#include "Editor.h"
#include "DynamicRHI.h"
#include "Engine/TargetPoint.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "HAL/PlatformTime.h"
#include "Landscape.h"
#include "LandscapeHeightfieldCollisionComponent.h"
#include "LandscapeStreamingProxy.h"
#include "Materials/Material.h"
#include "RHI.h"
#include "Tests/AutomationEditorCommon.h"

bool FAGX_PrepareLandscapeScenario::Update()
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	for (TActorIterator<AAGX_Terrain> It(World); It; ++It)
	{
		AAGX_Terrain& Terrain = **It;
		ALandscape* Landscape = Terrain.SourceLandscape;
		if (Landscape == nullptr)
			continue;
		if (bCoarse)
		{
			const FVector Anchor = Terrain.GetActorLocation();
			const FVector Local = Landscape->GetActorTransform().InverseTransformPosition(Anchor);
			Landscape->SetActorScale3D(FVector(200, 200, Landscape->GetActorScale3D().Z));
			Landscape->SetActorLocation(
				Anchor - Landscape->GetActorTransform().TransformVector(Local));
			Terrain.SimulationResolution = 25;
			Terrain.TerrainBounds->HalfExtent = FVector(40000, 40000, 100);
			Terrain.TerrainPagingSettings.TileSize = 2400;
			Terrain.TerrainPagingSettings.TileOverlap = 800;
			if (bLargeRadius)
			{
				for (auto& Shovel : Terrain.Shovels)
				{
					Shovel.RequiredRadius = 6000;
					Shovel.PreloadRadius = 7000;
				}
				for (auto& Body : Terrain.TerrainPagingSettings.TrackedRigidBodies)
				{
					Body.RequiredRadius = 6000;
					Body.PreloadRadius = 7000;
				}
				for (auto& Shovel : Terrain.TerrainPagingSettings.TrackedShovels)
				{
					Shovel.RequiredRadius = 6000;
					Shovel.PreloadRadius = 7000;
				}
			}
		}
		if (bNormals)
		{
			Landscape->LandscapeMaterial = LoadObject<UMaterial>(
				nullptr, TEXT("/Game/Tests/TerrainLandscape/M_TestDeformableNormals"));
			Test.TestNotNull(
				TEXT("GPU normal validation material"), Landscape->LandscapeMaterial.Get());
		}
		Landscape->CollisionMipLevel = CollisionMip;
		Landscape->RecreateCollisionComponents();
		for (ULandscapeHeightfieldCollisionComponent* Collision : Landscape->CollisionComponents)
			Test.TestEqual(
				TEXT("Collision mip is used by the fixture"), Collision->CollisionSizeQuads,
				(((Landscape->SubsectionSizeQuads + 1) >> CollisionMip) - 1) *
					Landscape->NumSubsections);
		if (bCoarse)
			Landscape->InvalidateNaniteRepresentation(false);
		Test.TestTrue(
			TEXT("Scenario Nanite build and deformation preparation"),
			UAGX_LandscapeMaterialLibrary::BuildLandscapeNanite(Landscape));
		Test.AddInfo(
			FString::Printf(
				TEXT("Scenario: source=%.0f cm, physics=%.0f cm, collision mip=%d"),
				Landscape->GetActorScale3D().X, Terrain.GetSimulationResolution(), CollisionMip));
		return true;
	}
	Test.AddError(TEXT("Landscape scenario has no Terrain."));
	return true;
}

namespace
{
	class FPrepareEmptyStartup final : public IAutomationLatentCommand
	{
	public:
		virtual bool Update() override
		{
			UWorld* World = GEditor->GetEditorWorldContext().World();
			for (TActorIterator<APlayerStart> It(World); It; ++It)
				It->SetActorLocation(FVector(1000000, 1000000, 5000));
			return true;
		}
	};

	class FCheckEmptyStartup final : public IAutomationLatentCommand
	{
	public:
		explicit FCheckEmptyStartup(FAutomationTestBase& InTest)
			: Test(InTest)
		{
		}
		virtual bool Update() override
		{
			if (Start == 0)
				Start = FPlatformTime::Seconds();
			if (FPlatformTime::Seconds() - Start > 90)
			{
				Test.AddError(TEXT("Empty World Partition startup did not recover within 90 s."));
				return true;
			}
			UWorld* World = GEditor->PlayWorld;
			if (World == nullptr)
				return false;
			AAGX_Terrain* Terrain = nullptr;
			for (TActorIterator<AAGX_Terrain> It(World); It; ++It)
				Terrain = *It;
			if (Terrain == nullptr)
				return false;
			auto* Simulation = UAGX_Simulation::GetFrom(Terrain);
			auto* Renderer =
				Terrain->FindComponentByClass<UAGX_TerrainPartitionedLandscapeRendererComponent>();
			if (Simulation == nullptr || Renderer == nullptr || Simulation->GetTimeStamp() < 1)
				return false;
			int32 Proxies = 0;
			for (TActorIterator<ALandscapeStreamingProxy> It(World); It; ++It)
				if (!It->LandscapeComponents.IsEmpty() &&
					It->LandscapeComponents[0]->IsRegistered())
					++Proxies;
			if (!bStreamingNear)
			{
				Test.TestEqual(
					TEXT("BeginPlay succeeds with zero loaded Landscape proxies"), Proxies, 0);
				Test.TestTrue(
					TEXT("Empty startup creates the renderer"),
					Renderer->GetDisplacementVirtualTexture() != nullptr);
				Test.TestTrue(
					TEXT("Physics remains responsive with missing source"),
					Terrain->HasNativeTerrainPager());
				Test.TestEqual(
					TEXT("Missing source fabricates no native tiles"),
					Terrain->GetNativeTerrainPager()->GetActiveTileTransforms().Num(), 0);
				Target = World->SpawnActor<ATargetPoint>();
				Target->SetActorLocation(Terrain->GetActorLocation() + FVector(0, 0, 1000));
				for (auto It = World->GetPlayerControllerIterator(); It; ++It)
					if (It->IsValid())
						It->Get()->SetViewTarget(Target);
				bStreamingNear = true;
				return false;
			}
			if (Proxies == 0 || Renderer->GetRetainedTileCount() == 0 ||
				Renderer->GetGeneratedPageCount() == 0)
				return false;
			Test.AddInfo(
				FString::Printf(
					TEXT("Empty startup recovered: %d proxies, %d native/rendered tiles"), Proxies,
					Renderer->GetRetainedTileCount()));
			Target->Destroy();
			return true;
		}

	private:
		FAutomationTestBase& Test;
		double Start = 0;
		bool bStreamingNear = false;
		ATargetPoint* Target = nullptr;
	};

	class FCheckLongDig final : public IAutomationLatentCommand
	{
	public:
		explicit FCheckLongDig(FAutomationTestBase& InTest)
			: Test(InTest)
		{
		}
		virtual bool Update() override
		{
			const double Now = FPlatformTime::Seconds();
			if (Start == 0)
				Start = Now;
			if (Now - Start > 240)
			{
				Test.AddError(TEXT("Long-dig scenario timed out."));
				return true;
			}
			UWorld* World = GEditor->PlayWorld;
			if (World == nullptr)
				return false;
			AAGX_Terrain* Terrain = nullptr;
			for (TActorIterator<AAGX_Terrain> It(World); It; ++It)
				Terrain = *It;
			if (Terrain == nullptr)
				return false;
			auto* Simulation = UAGX_Simulation::GetFrom(Terrain);
			auto* Renderer =
				Terrain->FindComponentByClass<UAGX_TerrainPartitionedLandscapeRendererComponent>();
			if (Simulation == nullptr || Renderer == nullptr || Simulation->GetTimeStamp() < 4)
				return false;
			if (Body == nullptr)
			{
				for (TActorIterator<AActor> It(World); It; ++It)
					if (It->GetActorLabel().Contains(TEXT("Shovel")))
					{
						Body = It->FindComponentByClass<UAGX_RigidBodyComponent>();
						if (Body != nullptr && Body->HasNative())
						{
							It->SetActorTickEnabled(false);
							break;
						}
					}
				if (Body == nullptr || !Body->HasNative())
				{
					Test.AddError(TEXT("Long-dig fixture has no native shovel body."));
					return true;
				}
				Body->SetMotionControl(MC_KINEMATICS);
				Body->SetVelocity(FVector(150, 0, 0));
				Origin = Body->GetPosition();
				Origin.Z -= 60;
				for (TActorIterator<AActor> It(World); It; ++It)
				{
					TInlineComponentArray<UAGX_RigidBodyComponent*> Components(*It);
					for (auto* Other : Components)
						if (Other != Body && Other->HasNative())
						{
							OtherBodies.Add(Other);
							OtherPositions.Add(Other->GetPosition());
						}
				}
				DigStart = Simulation->GetTimeStamp();
			}
			const double Elapsed = Simulation->GetTimeStamp() - DigStart;
			PeakTiles = FMath::Max(PeakTiles, Renderer->GetRetainedTileCount());
			PeakBytes = FMath::Max(PeakBytes, Renderer->GetRetainedHeightBytes());
			const auto Extrema = Renderer->GetActiveDisplacementExtrema();
			MinimumDelta = FMath::Min(MinimumDelta, Extrema.X);
			MaximumDelta = FMath::Max(MaximumDelta, Extrema.Y);
			if (Elapsed < 24)
			{
				Body->SetPosition(Origin + FVector(150 * Elapsed, 0, 0));
				if (LastFrame != 0)
				{
					WallMilliseconds += (Now - LastFrame) * 1000;
					GPUMilliseconds += FPlatformTime::ToMilliseconds(RHIGetGPUFrameCycles());
					ProductionMilliseconds += Renderer->GetLastPageProductionMilliseconds();
					++Frames;
				}
				LastFrame = Now;
				return false;
			}
			if (Elapsed < 26)
			{
				Body->SetVelocity(FVector(0, 0, 500));
				Body->SetPosition(Origin + FVector(3600, 0, 500 * (Elapsed - 24)));
				return false;
			}
			if (Elapsed < 30)
			{
				Body->SetVelocity(FVector::ZeroVector);
				Body->SetPosition(Origin + FVector(12000, 0, 1000));
				for (int32 Index = 0; Index < OtherBodies.Num(); ++Index)
				{
					OtherBodies[Index]->GetOwner()->SetActorTickEnabled(false);
					OtherBodies[Index]->SetMotionControl(MC_KINEMATICS);
					OtherBodies[Index]->SetVelocity(FVector::ZeroVector);
					OtherBodies[Index]->SetPosition(
						OtherPositions[Index] + FVector(12000, 0, 1000));
				}
				const auto Active = Terrain->GetNativeTerrainPager()->GetActiveTileTransforms();
				bUnloadedOriginal |= !Active.ContainsByPredicate(
					[this](const FTransform& Tile)
					{ return FVector::Dist2D(Tile.GetLocation(), Origin) < 2000; });
				return false;
			}
			if (Elapsed < 32)
			{
				Body->SetPosition(Origin + FVector(0, 0, 1000));
				return false;
			}
			Test.TestTrue(TEXT("Long cut excavates terrain"), MinimumDelta < -1);
			Test.TestTrue(TEXT("Long cut also produces positive height changes"), MaximumDelta > 1);
			Test.TestTrue(
				TEXT("Scenario exercises a substantial native working set"), PeakTiles >= 50);
			Test.TestTrue(
				TEXT("Long journey unloads the original native region"), bUnloadedOriginal);
			Test.TestTrue(
				TEXT("Returning restores excavation without another shovel contact"),
				Extrema.X < -1);
			Test.TestTrue(
				TEXT("Long-dig renderer working set stays below 64 MiB"),
				PeakBytes < 64 * 1024 * 1024);
			Test.AddInfo(
				FString::Printf(
					TEXT(
						"36 m dig / 120 m travel / return: peak tiles=%d, bytes=%lld, "
						"delta=[%.2f,%.2f] cm; %d moving frames, wall=%.3f ms, GPU=%.3f ms, "
						"last-batch CPU mean=%.3f ms. "
						"Native stepping and ordinary particle generation included; timings are "
						"diagnostic."),
					PeakTiles, PeakBytes, MinimumDelta, MaximumDelta, Frames,
					WallMilliseconds / FMath::Max(Frames, 1),
					GPUMilliseconds / FMath::Max(Frames, 1),
					ProductionMilliseconds / FMath::Max(Frames, 1)));
			return true;
		}

	private:
		FAutomationTestBase& Test;
		UAGX_RigidBodyComponent* Body = nullptr;
		TArray<UAGX_RigidBodyComponent*> OtherBodies;
		TArray<FVector> OtherPositions;
		FVector Origin;
		double Start = 0, DigStart = 0, LastFrame = 0;
		double WallMilliseconds = 0, GPUMilliseconds = 0, ProductionMilliseconds = 0;
		double MinimumDelta = 0, MaximumDelta = 0;
		int32 Frames = 0, PeakTiles = 0;
		int64 PeakBytes = 0;
		bool bUnloadedOriginal = false;
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAGX_TerrainEmptyStartupTest, "AGXUnreal.TerrainLandscape.EmptyWorldPartitionStartup",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAGX_TerrainEmptyStartupTest::RunTest(const FString&)
{
	ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(TEXT("/Game/Levels/TerrainLandscapePaging_WP")));
	ADD_LATENT_AUTOMATION_COMMAND(FPrepareEmptyStartup());
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(FCheckEmptyStartup(*this));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAGX_TerrainLongDigTest, "AGXUnreal.TerrainLandscape.LongDigCoarseSource",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAGX_TerrainLongDigTest::RunTest(const FString&)
{
	ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(TEXT("/Game/Levels/TerrainLandscapePaging")));
	ADD_LATENT_AUTOMATION_COMMAND(FAGX_PrepareLandscapeScenario(*this, true, false, true));
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(FCheckLongDig(*this));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}
