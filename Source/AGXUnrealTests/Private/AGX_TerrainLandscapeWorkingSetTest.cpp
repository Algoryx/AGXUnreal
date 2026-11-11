// Copyright 2026, Algoryx Simulation AB.

#if WITH_DEV_AUTOMATION_TESTS

// AGX Dynamics for Unreal includes.
#include "AGX_MotionControl.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_Simulation.h"
#include "Terrain/AGX_Terrain.h"
#include "Terrain/AGX_TerrainNaniteDisplacementData.h"
#include "Terrain/AGX_TerrainPartitionedLandscapeRendererComponent.h"

// Unreal Engine includes.
#include "DynamicRHI.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "HAL/PlatformTime.h"
#include "LandscapeNaniteComponent.h"
#include "LandscapeProxy.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/AutomationTest.h"
#include "RenderingThread.h"
#include "Tests/AutomationEditorCommon.h"
#include "RHICommandList.h"
#include "VirtualTexturing.h"
#include "VT/RuntimeVirtualTexture.h"

namespace
{
	class FCheckLandscapeWorkingSet final : public IAutomationLatentCommand
	{
	public:
		explicit FCheckLandscapeWorkingSet(FAutomationTestBase& InTest)
			: Test(InTest)
		{
		}
		virtual bool Update() override
		{
			if (Start == 0.0)
				Start = FPlatformTime::Seconds();
			if (FPlatformTime::Seconds() - Start > 120.0)
			{
				Test.AddError(TEXT("Terrain working set did not become ready within 120 s."));
				return true;
			}
			UWorld* World = nullptr;
			for (const auto& Context : GEngine->GetWorldContexts())
				if (Context.WorldType == EWorldType::PIE)
					World = Context.World();
			if (World == nullptr)
				return false;
			for (TActorIterator<AAGX_Terrain> It(World); It; ++It)
			{
				AAGX_Terrain& Terrain = **It;
				auto* Renderer =
					Terrain
						.FindComponentByClass<UAGX_TerrainPartitionedLandscapeRendererComponent>();
				if (Renderer == nullptr)
					continue;
				UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(&Terrain);
				if (Simulation == nullptr || Simulation->GetTimeStamp() < 4.0)
					return false;
				if (Phase >= 5)
				{
					const double Now = FPlatformTime::Seconds();
					if (++ProfileFrames > 30 && ProfileLast != 0.0)
					{
						ProfileWall += (Now - ProfileLast) * 1000.0;
						ProfileGPU += FPlatformTime::ToMilliseconds(RHIGetGPUFrameCycles());
					}
					ProfileLast = Now;
					if (ProfileFrames < 150)
						return false;
					Test.AddInfo(
						FString::Printf(
							TEXT(
								"Matched stationary profile %s: 120 frames, wall=%.3f ms/frame, "
								"GPU=%.3f ms/frame. Native stepping paused; unchanged camera and "
								"bodies."),
							Phase == 6 ? TEXT("disabled")
									   : (Phase == 5 ? TEXT("enabled") : TEXT("re-enabled")),
							ProfileWall / 120.0, ProfileGPU / 120.0));
					if (Phase == 7)
					{
						Simulation->StepMode = PreviousStepMode;
						return true;
					}
					Renderer->SetEnabled(Phase == 6);
					++Phase;
					ProfileFrames = 0;
					ProfileLast = ProfileWall = ProfileGPU = 0;
					return false;
				}
				if (Phase == 1)
				{
					if (Simulation->GetTimeStamp() < DisabledAt + 0.5)
						return false;
					Test.TestTrue(
						TEXT("Physics continues with rendering disabled"),
						Terrain.HasNativeTerrainPager());
					Renderer->SetEnabled(true);
					Phase = 2;
					return false;
				}
				if (Renderer->GetRetainedTileCount() == 0 || Renderer->GetGeneratedPageCount() == 0)
					return false;
				if (Phase == 2)
				{
					Test.TestTrue(
						TEXT("Renderer restores active heights"),
						Renderer->GetRetainedHeightBytes() > 0);
					if (World->IsPartitionedWorld())
						return true;
					for (TActorIterator<AActor> Actor(World); Actor; ++Actor)
					{
						TInlineComponentArray<UAGX_RigidBodyComponent*> Components(*Actor);
						if (Components.IsEmpty())
							continue;
						Actor->SetActorTickEnabled(false);
						TInlineComponentArray<UActorComponent*> ActorComponents(*Actor);
						for (auto* Component : ActorComponents)
							Component->SetComponentTickEnabled(false);
						for (auto* Body : Components)
							if (Body->HasNative())
							{
								Bodies.Add(Body);
								BodyPositions.Add(Body->GetPosition());
								InitialCenter += BodyPositions.Last();
								Body->SetMotionControl(MC_KINEMATICS);
								Body->SetVelocity(FVector::ZeroVector);
							}
					}
					Test.TestTrue(
						TEXT("Example has tracked bodies to travel with"), !Bodies.IsEmpty());
					InitialCenter /= FMath::Max(Bodies.Num(), 1);
					MoveToVisit(Terrain);
					TravelAt = Simulation->GetTimeStamp();
					Phase = 3;
					return false;
				}
				if (Phase == 3)
				{
					if (Simulation->GetTimeStamp() < TravelAt + 0.3)
						return false;
					const int32 Active =
						Terrain.GetNativeTerrainPager()->GetActiveTileTransforms().Num();
					Test.TestEqual(
						TEXT("Renderer retains exactly the active native tiles after travel"),
						Renderer->GetRetainedTileCount(), Active);
					PeakTiles = FMath::Max(PeakTiles, Active);
					PeakHeightBytes =
						FMath::Max(PeakHeightBytes, Renderer->GetRetainedHeightBytes());
					for (const auto& Tile :
						 Terrain.GetNativeTerrainPager()->GetTileHeightUpdates(TravelKnown))
						VisitedTiles.Add(Tile.Id);
					if (++Visit < 25)
						MoveToVisit(Terrain);
					else
					{
						for (int32 Index = 0; Index < Bodies.Num(); ++Index)
							if (Bodies[Index].IsValid())
								Bodies[Index]->SetPosition(
									BodyPositions[Index] + FVector(0, 0, 1000));
						Phase = 4;
					}
					TravelAt = Simulation->GetTimeStamp();
					return false;
				}
				if (Phase == 4)
				{
					if (Simulation->GetTimeStamp() < TravelAt + 0.5)
						return false;
					Test.TestTrue(
						TEXT("Visited history exceeds the retained active set"),
						VisitedTiles.Num() > PeakTiles);
					Test.TestTrue(
						TEXT("CPU rendering heights stay below 2 MiB after 25 regions"),
						PeakHeightBytes < 2 * 1024 * 1024);
					Test.TestTrue(
						TEXT("Revisiting restores excavation without another shovel contact"),
						Renderer->GetActiveDisplacementExtrema().X < -0.1);
					Test.AddInfo(
						FString::Printf(
							TEXT(
								"Travel: 25 regions, %d distinct tiles, peak active=%d, peak "
								"height bytes=%lld. Original excavation restored with all "
								"deformers raised 1000 cm."),
							VisitedTiles.Num(), PeakTiles, PeakHeightBytes));
					PreviousStepMode = Simulation->StepMode;
					Simulation->StepMode = SmNone;
					Phase = 5;
					return false;
				}
				int32 PreparedBindings = 0;
				for (TActorIterator<ALandscapeProxy> Proxy(World); Proxy; ++Proxy)
				{
					TInlineComponentArray<ULandscapeNaniteComponent*> Components(*Proxy);
					for (auto* Component : Components)
					{
						UStaticMesh* Mesh = Component->GetStaticMesh();
						auto* Data =
							Mesh == nullptr
								? nullptr
								: Cast<UAGX_TerrainNaniteDisplacementData>(
									  Mesh->GetAssetUserDataOfClass(
										  UAGX_TerrainNaniteDisplacementData::StaticClass()));
						Test.TestTrue(
							TEXT("Prepared Nanite metadata is current"),
							Data != nullptr && Data->IsCurrent(*Mesh));
						if (auto* Material =
								Cast<UMaterialInstanceDynamic>(Component->GetMaterial(0)))
						{
							Test.TestEqual(
								TEXT("Nanite material deformation is enabled"),
								Material->K2_GetScalarParameterValue(
									TEXT("AGX_DeformationEnabled")),
								1.f);
							++PreparedBindings;
						}
					}
				}
				Test.TestTrue(TEXT("Nanite materials are bound"), PreparedBindings > 0);
				Test.TestTrue(TEXT("Physics pager is active"), Terrain.HasNativeTerrainPager());
				const FVector2D Extrema = Renderer->GetActiveDisplacementExtrema();
				Test.TestTrue(
					TEXT("Excavation reaches the renderer"), Extrema.X < -0.1 || Extrema.Y > 0.1);
				Test.TestTrue(
					TEXT("CPU working set is bounded"),
					Renderer->GetRetainedHeightBytes() < 64 * 1024 * 1024);
				URuntimeVirtualTexture* Texture = Renderer->GetDisplacementVirtualTexture();
				uint64 PoolBytes = 0;
				EPixelFormat PoolFormat = PF_Unknown;
				ENQUEUE_RENDER_COMMAND(AGX_TestTerrainPool)(
					[Texture, &PoolBytes, &PoolFormat](FRHICommandListImmediate&)
					{
						if (auto* Allocated = Texture->GetAllocatedVirtualTexture())
						{
							const uint64 Side = Allocated->GetPhysicalTextureSize(0);
							PoolFormat = Allocated->GetPhysicalTexture(0)->GetFormat();
							PoolBytes = Side * Side * GPixelFormats[PoolFormat].BlockBytes;
						}
					});
				FlushRenderingCommands();
				Test.TestEqual(TEXT("Precise height/gradient pool"), PoolFormat, PF_A32B32G32R32F);
				Test.TestTrue(TEXT("Physical pool is allocated"), PoolBytes > 0);
				Test.AddInfo(TEXT(
					"Editor physical-pool size is diagnostic only: stock UE editor auto-growth may "
					"override the fixed budget. Validate the hard cap separately in a built "
					"application."));

				Test.AddInfo(
					FString::Printf(
						TEXT(
							"Terrain working set: tiles=%d, height bytes=%lld, pages=%lld, "
							"physical "
							"pool=%llu bytes, displacement=[%.3f,%.3f] cm"),
						Renderer->GetRetainedTileCount(), Renderer->GetRetainedHeightBytes(),
						Renderer->GetGeneratedPageCount(), PoolBytes, Extrema.X, Extrema.Y));
				Renderer->SetEnabled(false);
				Test.TestEqual(
					TEXT("Disable releases CPU height state"), Renderer->GetRetainedHeightBytes(),
					static_cast<int64>(0));
				DisabledAt = Simulation->GetTimeStamp();
				Phase = 1;
				return false;
			}
			return false;
		}

	private:
		void MoveToVisit(AAGX_Terrain& Terrain)
		{
			const FVector Offset(
				Terrain.GetActorLocation().X - InitialCenter.X - 800 + (Visit % 5) * 400,
				Terrain.GetActorLocation().Y - InitialCenter.Y - 800 + (Visit / 5) * 400, 1000);
			for (int32 Index = 0; Index < Bodies.Num(); ++Index)
				if (Bodies[Index].IsValid())
					Bodies[Index]->SetPosition(BodyPositions[Index] + Offset);
		}
		TArray<TWeakObjectPtr<UAGX_RigidBodyComponent>> Bodies;
		TArray<FVector> BodyPositions;
		FVector InitialCenter = FVector::ZeroVector;
		TMap<FIntPoint, uint64> TravelKnown;
		TSet<FIntPoint> VisitedTiles;
		int32 Visit = 0;
		int32 PeakTiles = 0;
		int64 PeakHeightBytes = 0;
		double TravelAt = 0;
		FAutomationTestBase& Test;
		double Start = 0.0;
		double DisabledAt = 0.0;
		EAGX_StepMode PreviousStepMode = SmNone;
		int32 ProfileFrames = 0;
		double ProfileLast = 0, ProfileWall = 0, ProfileGPU = 0;
		int32 Phase = 0;
	};
}

IMPLEMENT_COMPLEX_AUTOMATION_TEST(
	FAGX_LandscapeWorkingSetTest, "AGXUnreal.TerrainLandscape.RuntimeWorkingSet",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

void FAGX_LandscapeWorkingSetTest::GetTests(TArray<FString>& Names, TArray<FString>& Commands) const
{
	Names.Add(TEXT("OrdinaryLandscape"));
	Commands.Add(TEXT("/Game/Levels/TerrainLandscapePaging"));
	Names.Add(TEXT("WorldPartitionLandscape"));
	Commands.Add(TEXT("/Game/Levels/TerrainLandscapePaging_WP"));
}

bool FAGX_LandscapeWorkingSetTest::RunTest(const FString& Map)
{
	ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(Map));
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(FCheckLandscapeWorkingSet(*this));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

#endif
