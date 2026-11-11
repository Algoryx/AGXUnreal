// Copyright 2026, Algoryx Simulation AB.

// AGX Dynamics for Unreal includes.
#include "AgxAutomationCommon.h"
#include "RigidBodyBarrier.h"
#include "Shapes/HeightFieldShapeBarrier.h"
#include "SimulationBarrier.h"
#include "Terrain/AGX_Terrain.h"
#include "Terrain/AGX_TerrainHeightFetcher.h"
#include "Terrain/AGX_TerrainLandscapeRendererComponent.h"
#include "Terrain/TerrainBarrier.h"
#include "Terrain/TerrainHeightFetcherBase.h"
#include "Terrain/TerrainPagerBarrier.h"

// Unreal Engine includes.
#include "Engine/TextureRenderTarget2D.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionFunctionOutput.h"
#include "Materials/MaterialFunction.h"
#include "MaterialShared.h"
#include "Misc/AutomationTest.h"

// Standard library includes.
#include <atomic>

BEGIN_DEFINE_SPEC(
	FAGX_TerrainLandscapeSpec, "AGXUnreal.Spec.TerrainLandscape",
	AgxAutomationCommon::DefaultTestFlags)
END_DEFINE_SPEC(FAGX_TerrainLandscapeSpec)

void FAGX_TerrainLandscapeSpec::Define()
{
	It("Migrates released Blueprint renderer defaults and inheritance",
	   [this]()
	   {
		   for (const TCHAR* Path :
				{TEXT(
					 "/Game/Tests/BackwardsCompatibility/"
					 "BP_LegacyTerrainRendererDisabled.BP_LegacyTerrainRendererDisabled_C"),
				 TEXT(
					 "/Game/Tests/BackwardsCompatibility/"
					 "BP_LegacyTerrainRendererChild.BP_LegacyTerrainRendererChild_C")})
		   {
			   UClass* Class = LoadClass<AAGX_Terrain>(nullptr, Path);
			   TestNotNull(TEXT("Released Blueprint class"), Class);
			   if (Class == nullptr)
				   continue;
			   auto* Defaults = Class->GetDefaultObject<AAGX_Terrain>();
			   TestNotNull(TEXT("Default Landscape renderer"), Defaults->DefaultLandscapeRenderer);
			   if (Defaults->DefaultLandscapeRenderer == nullptr)
				   continue;
			   TestFalse(
				   TEXT("Explicit disabled state survives class regeneration"),
				   Defaults->DefaultLandscapeRenderer->IsEnabled());
			   TestEqual(
				   TEXT("Customized target is inherited"),
				   GetNameSafe(Defaults->DefaultLandscapeRenderer->LandscapeDisplacementMap),
				   FString(TEXT("RT_LegacyTerrainCustom")));
		   }
	   });

	It("Ships connected deformation functions and compilable Nanite materials",
	   [this]()
	   {
		   UMaterialFunction* Function = LoadObject<UMaterialFunction>(
			   nullptr,
			   TEXT("/AGXUnreal/Terrain/Rendering/HeightField/MF_PartitionedLandscapeDeformation"));
		   TestNotNull(TEXT("Deformation function"), Function);
		   if (Function == nullptr)
			   return;
		   for (UMaterialExpression* Expression : Function->GetExpressions())
		   {
			   if (auto* Output = Cast<UMaterialExpressionFunctionOutput>(Expression))
				   TestNotNull(*Output->OutputName.ToString(), Output->A.Expression);
		   }
		   for (const TCHAR* Path :
				{TEXT("/AGXUnreal/Terrain/Rendering/HeightField/M_DeformablePartitionedLandscape"),
				 TEXT(
					 "/AGXUnreal/Terrain/Rendering/HeightField/"
					 "M_DeformablePartitionedLandscape_Minimal")})
		   {
			   UMaterial* Material = LoadObject<UMaterial>(nullptr, Path);
			   TestNotNull(TEXT("Partitioned material"), Material);
			   if (Material != nullptr)
			   {
				   TestTrue(TEXT("Nanite tessellation enabled"), Material->IsTessellationEnabled());
				   TestNull(
					   TEXT("No legacy WPO"),
					   Material->GetEditorOnlyData()->WorldPositionOffset.Expression);
				   TestNull(
					   TEXT("No legacy pixel-depth offset"),
					   Material->GetEditorOnlyData()->PixelDepthOffset.Expression);
				   if (const FMaterialResource* Resource =
						   Material->GetMaterialResource(GMaxRHIFeatureLevel))
					   for (const FString& Error : Resource->GetCompileErrors())
						   AddError(Error);
			   }
		   }
	   });

	It("Retains source snapshots until native paging releases its references",
	   [this]()
	   {
		   TWeakPtr<FTerrainHeightFetcherBase, ESPMode::ThreadSafe> WeakSource;
		   {
			   TSharedPtr<FTerrainHeightFetcherBase, ESPMode::ThreadSafe> Source =
				   MakeShared<FAGX_TerrainHeightFetcher, ESPMode::ThreadSafe>();
			   WeakSource = Source;
			   FTerrainBarrier Template;
			   TArray<float> Heights;
			   Heights.Init(0.f, 25);
			   TArray<float> MinimumHeights;
			   MinimumHeights.Init(-100.f, 25);
			   Template.AllocateNative(5, 5, 100.0, Heights, MinimumHeights);
			   FTerrainPagerBarrier Pager;
			   Pager.AllocateNative(Template, 5, 1, 100.0, 100.0);
			   Pager.CreateTerrainDataSource(Source.ToSharedRef(), nullptr);
			   Source.Reset();
			   TestTrue(
				   TEXT("Native data source owns the fetcher after actor-side release"),
				   WeakSource.IsValid());
			   Pager.ReleaseNative();
		   }
		   TestFalse(
			   TEXT("Releasing the final native data source frees snapshots"),
			   WeakSource.IsValid());
	   });

	It("Bounds missing source requests without touching Landscape UObjects",
	   [this]()
	   {
		   FAGX_TerrainHeightFetcher Fetcher;
		   Fetcher.ConfigureSnapshots(FTransform::Identity, FVector2D(100000.0), 100.0);
		   TArray<float> Heights;
		   for (int32 Index = 0; Index < 1000; ++Index)
			   TestFalse(
				   TEXT("Unavailable source is recoverable"),
				   Fetcher.FetchHeights(FVector(Index * 100.0 - 50000.0, 0.0, 0.0), 3, 3, Heights));
		   TestTrue(
			   TEXT("Pending source requests have a fixed upper bound"),
			   Fetcher.GetSnapshotCount() <= 256);
		   TestEqual(TEXT("Missing source never fabricates flat terrain"), Heights.Num(), 0);
		   Fetcher.ResetSnapshots();
		   TestEqual(TEXT("Teardown frees pending source work"), Fetcher.GetSnapshotCount(), 0);
		   TestFalse(
			   TEXT("Outside finite bounds"),
			   Fetcher.FetchHeights(FVector(200000.0, 0.0, 0.0), 3, 3, Heights));
		   TestEqual(TEXT("Outside requests are not retained"), Fetcher.GetSnapshotCount(), 0);
	   });

	It("Retries missing native data and refreshes restored tiles without another modification",
	   [this]()
	   {
		   class FSource final : public FTerrainHeightFetcherBase
		   {
		   public:
			   std::atomic<bool> bAvailable {false};
			   std::atomic<float> Height {123.f};
			   virtual bool FetchHeights(
				   const FVector& Corner, int32 X, int32 Y, TArray<float>& Out) const override
			   {
				   if (!bAvailable)
					   return false;
				   Out.Init(Height, X * Y);
				   return true;
			   }
		   } Source;
		   FSimulationBarrier Simulation;
		   Simulation.AllocateNative();
		   Simulation.SetTimeStep(1.0 / 60.0);
		   FTerrainBarrier Template;
		   FHeightFieldShapeBarrier HeightField;
		   HeightField.AllocateNative(4, 4, 100.0, 100.0);
		   Template.AllocateNative(HeightField, 200.0);
		   FTerrainPagerBarrier Pager;
		   Pager.AllocateNative(Template, 9, 2, 100.0, 200.0);
		   Pager.CreateTerrainDataSource(&Source, nullptr);
		   FRigidBodyBarrier Body;
		   Body.AllocateNative();
		   Body.SetMotionControl(EAGX_MotionControl::MC_KINEMATICS);
		   Simulation.Add(Body);
		   Pager.AddRigidBody(Body, 50.0, 50.0);
		   Simulation.Add(Pager);
		   TMap<FIntPoint, uint64> Known;
		   Simulation.Step();
		   TestEqual(
			   TEXT("Missing required data completes instead of deadlocking"),
			   Pager.GetTileHeightUpdates(Known).Num(), 0);
		   Source.bAvailable = true;
		   TArray<FTerrainTileHeights> Updates;
		   for (int32 Step = 0; Step < 10 && Updates.IsEmpty(); ++Step)
		   {
			   Simulation.Step();
			   Updates = Pager.GetTileHeightUpdates(Known);
		   }
		   TestFalse(TEXT("A previously missing source recovers"), Updates.IsEmpty());
		   for (const auto& Tile : Updates)
		   {
			   TestTrue(TEXT("New tile receives full refresh"), Tile.bFullRefresh);
			   TestEqual(
				   TEXT("Full tile includes neutral and unchanged vertices"), Tile.Heights.Num(),
				   81);
			   TestEqual(
				   TEXT("Pager-frame heights include tile vertical offset"), Tile.Heights[0],
				   123.f);
		   }
		   TArray<FIntPoint> InitialTiles;
		   Known.GenerateKeyArray(InitialTiles);
		   Source.Height = 321.f;
		   Body.SetPosition(FVector(50000.0, 0.0, 0.0));
		   for (int32 Step = 0; Step < 100; ++Step)
			   Simulation.Step();
		   Pager.GetTileHeightUpdates(Known);
		   for (const FIntPoint& Id : InitialTiles)
			   TestFalse(
				   TEXT("Unloaded tiles leave no renderer tracking history"), Known.Contains(Id));
		   TestTrue(TEXT("Tracking follows the active region"), Known.Num() <= 4);
		   Body.SetPosition(FVector::ZeroVector);
		   Updates.Reset();
		   for (int32 Step = 0; Step < 10 && Updates.IsEmpty(); ++Step)
		   {
			   Simulation.Step();
			   Updates = Pager.GetTileHeightUpdates(Known);
		   }
		   TestFalse(TEXT("Visited tile restores"), Updates.IsEmpty());
		   for (const auto& Tile : Updates)
		   {
			   TestTrue(
				   TEXT("Restoration triggers a full refresh without digging"), Tile.bFullRefresh);
			   TestEqual(TEXT("Restored heights come from AGX's cache"), Tile.Heights[0], 123.f);
		   }
		   Known.Empty();
		   TestFalse(
			   TEXT("Re-enabled rendering can refresh every active tile"),
			   Pager.GetTileHeightUpdates(Known).IsEmpty());
		   Simulation.Remove(Pager);
		   Pager.ReleaseNative();
	   });
}
