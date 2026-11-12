// Copyright 2026, Algoryx Simulation AB.

// AGX Dynamics for Unreal includes.
#include "AGX_Simulation.h"
#include "AGX_TerrainLandscapeTestUtilities.h"
#include "AgxAutomationCommon.h"
#include "Terrain/AGX_HeightFieldBoundsComponent.h"
#include "Terrain/AGX_LandscapeMaterialLibrary.h"
#include "Terrain/AGX_Terrain.h"
#include "Terrain/AGX_TerrainHeightFetcher.h"
#include "Terrain/AGX_TerrainLandscapeRendererComponent.h"
#include "Terrain/AGX_TerrainNaniteDisplacementData.h"
#include "Terrain/AGX_TerrainRVTLandscapeRendererComponent.h"
#include "Terrain/TerrainPagerBarrier.h"

// Unreal Engine includes.
#include "Components/PrimitiveComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/WorldPartitionStreamingSourceComponent.h"
#include "DeviceProfiles/DeviceProfile.h"
#include "DeviceProfiles/DeviceProfileManager.h"
#include "Editor.h"
#include "Engine/SceneCapture2D.h"
#include "Engine/StaticMesh.h"
#include "Engine/TargetPoint.h"
#include "Engine/TextureLODSettings.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "HAL/PlatformTime.h"
#include "Kismet/GameplayStatics.h"
#include "Landscape.h"
#include "LandscapeComponent.h"
#include "LandscapeEdit.h"
#include "LandscapeEditLayer.h"
#include "LandscapeInfo.h"
#include "LandscapeNaniteComponent.h"
#include "LandscapeStreamingProxy.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/AutomationTest.h"
#include "Modules/ModuleManager.h"
#include "RendererInterface.h"
#include "RenderingThread.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"
#include "VT/RuntimeVirtualTexture.h"
#include "WorldPartition/WorldPartition.h"

namespace
{
	class FConfigureNonPagedTerrain final : public IAutomationLatentCommand
	{
	public:
		explicit FConfigureNonPagedTerrain(bool bInRectangular)
			: bRectangular(bInRectangular)
		{
		}
		virtual bool Update() override
		{
			for (TActorIterator<AAGX_Terrain> It(GEditor->GetEditorWorldContext().World()); It;
				 ++It)
			{
				It->SetEnableTerrainPaging(false);
				It->SimulationResolution = 0;
				if (bRectangular)
					It->TerrainBounds->HalfExtent = FVector(1900, 1300, 100);
			}
			return true;
		}

	private:
		bool bRectangular;
	};

	class FScopedTileCountBias
	{
	public:
		explicit FScopedTileCountBias(int32 Bias)
			: Group(
				  UDeviceProfileManager::Get()
					  .GetActiveProfile()
					  ->GetTextureLODSettings()
					  ->GetTextureLODGroup(TEXTUREGROUP_World))
			, Original(Group.VirtualTextureTileCountBias)
		{
			Group.VirtualTextureTileCountBias = Bias;
		}
		~FScopedTileCountBias()
		{
			Group.VirtualTextureTileCountBias = Original;
		}

	private:
		FTextureLODGroup& Group;
		int32 Original;
	};

	class FRestoreTileCountBias final : public IAutomationLatentCommand
	{
	public:
		explicit FRestoreTileCountBias(TSharedPtr<FScopedTileCountBias> InBias)
			: Bias(InBias)
		{
		}
		virtual bool Update() override
		{
			Bias.Reset();
			return true;
		}

	private:
		TSharedPtr<FScopedTileCountBias> Bias;
	};

	class FPrepareSlopedLandscape final : public IAutomationLatentCommand
	{
	public:
		explicit FPrepareSlopedLandscape(FAutomationTestBase& InTest)
			: Test(InTest)
		{
		}
		virtual bool Update() override
		{
			UWorld* World = GEditor->GetEditorWorldContext().World();
			AAGX_Terrain* Terrain = nullptr;
			for (TActorIterator<AAGX_Terrain> It(World); It; ++It)
				Terrain = *It;
			if (Terrain == nullptr || Terrain->SourceLandscape == nullptr)
			{
				Test.AddError(TEXT("Sloped fixture has no source Landscape."));
				return true;
			}
			ALandscape* Landscape = Terrain->SourceLandscape;
			ULandscapeInfo* Info = Landscape->GetLandscapeInfo();
			int32 X1, Y1, X2, Y2;
			if (Info == nullptr || !Info->GetLandscapeExtent(X1, Y1, X2, Y2))
				return true;
			FVector Anchor = Terrain->GetActorLocation();
			for (TActorIterator<AActor> It(World); It; ++It)
				if (It->GetActorLabel() == TEXT("Shovel"))
					Anchor = It->GetActorLocation();
			const FVector LocalAnchor =
				Landscape->GetActorTransform().InverseTransformPosition(Anchor);
			const float BaseHeight = Landscape->GetHeightAtLocation(Anchor).Get(0.f);
			const FVector Scale = Landscape->GetActorScale3D();
			TArray<uint16> Heights;
			for (int32 Y = Y1; Y <= Y2; ++Y)
				for (int32 X = X1; X <= X2; ++X)
				{
					const double LocalHeight = (BaseHeight - Landscape->GetActorLocation().Z +
												0.5 * (Y - LocalAnchor.Y) * Scale.Y) /
											   Scale.Z;
					Heights.Add(
						static_cast<uint16>(
							FMath::Clamp(FMath::RoundToInt(32768 + LocalHeight * 128), 0, 65535)));
				}
			{
				const auto Layers = Landscape->GetLayersConst();
				const FGuid Layer = Layers.IsEmpty() ? FGuid() : Layers[0].EditLayer->GetGuid();
				FLandscapeEditDataInterface Edit(Info, Layer);
				Edit.SetHeightData(X1, Y1, X2, Y2, Heights.GetData(), 0, true);
			}
			Landscape->RequestLayersContentUpdateForceAll();
			Landscape->ForceLayersFullUpdate();
			Landscape->RecreateCollisionComponents();
			const double Rise =
				Landscape->GetHeightAtLocation(Anchor + FVector(0, 100, 0)).Get(0.f) -
				Landscape->GetHeightAtLocation(Anchor - FVector(0, 100, 0)).Get(0.f);
			Test.AddInfo(
				FString::Printf(TEXT("Inclined source collision: %.3f cm rise over 200 cm"), Rise));
			Test.TestTrue(
				TEXT("Source collision has the intended 1:2 slope"),
				FMath::Abs(Rise - 100.0) < 1.0);
			Test.TestTrue(
				TEXT("Sloped Landscape Nanite build"),
				UAGX_LandscapeMaterialLibrary::BuildLandscapeNanite(Landscape));
			return true;
		}

	private:
		FAutomationTestBase& Test;
	};

	class FTranslateLandscapeFixture final : public IAutomationLatentCommand
	{
	public:
		virtual bool Update() override
		{
			UWorld* World = GEditor->GetEditorWorldContext().World();
			for (TActorIterator<AActor> It(World); It; ++It)
				It->ApplyWorldOffset(FVector(20000000, -30000000, 5000000.123), false);
			return true;
		}
	};

	class FCheckProxyStreaming final : public IAutomationLatentCommand
	{
	public:
		explicit FCheckProxyStreaming(FAutomationTestBase& InTest)
			: Test(InTest)
		{
		}
		virtual bool Update() override
		{
			UWorld* World = GEditor->PlayWorld;
			if (World == nullptr)
				return false;
			AAGX_Terrain* Terrain = nullptr;
			for (TActorIterator<AAGX_Terrain> It(World); It; ++It)
				Terrain = *It;
			if (Terrain == nullptr)
				return false;
			int32 Loaded = 0;
			for (TActorIterator<ALandscapeStreamingProxy> It(World); It; ++It)
				if (!It->LandscapeComponents.IsEmpty() &&
					It->LandscapeComponents[0]->IsRegistered())
					++Loaded;
			if (Phase == 0)
			{
				Test.TestTrue(TEXT("Spatial Landscape proxies initially loaded"), Loaded > 0);
				Test.AddInfo(
					FString::Printf(
						TEXT("WP streaming enabled=%d, server=%d, can stream=%d"),
						World->GetWorldPartition()->IsStreamingEnabled(),
						World->GetWorldPartition()->IsServer(),
						World->GetWorldPartition()->CanStream()));
				PreviousStepMode = UAGX_Simulation::GetFrom(Terrain)->StepMode;
				UAGX_Simulation::GetFrom(Terrain)->StepMode = SmNone;
				for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It;
					 ++It)
					if (It->IsValid())
						It->Get()->bEnableStreamingSource = false;
				SourceActor = World->SpawnActor<ATargetPoint>();
				SourceActor->SetActorLocation(FVector(100000, 100000, 5000));
				Source = NewObject<UWorldPartitionStreamingSourceComponent>(SourceActor);
				Source->RegisterComponent();
				Source->EnableStreamingSource();
				Start = FPlatformTime::Seconds();
				Phase = 1;
				return false;
			}
			if (FPlatformTime::Seconds() - Start > 30)
			{
				Test.AddError(
					FString::Printf(
						TEXT("Proxy streaming timed out in phase %d with %d registered proxies."),
						Phase, Loaded));
				UAGX_Simulation::GetFrom(Terrain)->StepMode = PreviousStepMode;
				return true;
			}
			if (Phase == 1)
			{
				if (Loaded != 0 || !Source->IsStreamingCompleted())
					return false;
				Test.AddInfo(
					TEXT("All spatial Landscape proxies unloaded while Terrain remained alive."));
				Test.TestTrue(
					TEXT("Native cached physics survives proxy unload"),
					Terrain->HasNativeTerrainPager());
				SourceActor->SetActorLocation(Terrain->GetActorLocation());
				Phase = 2;
				return false;
			}
			if (Phase == 2)
			{
				if (Loaded == 0 || !Source->IsStreamingCompleted())
					return false;
				UAGX_Simulation::GetFrom(Terrain)->StepMode = PreviousStepMode;
				ReadyAt = UAGX_Simulation::GetFrom(Terrain)->GetTimeStamp();
				Phase = 3;
				return false;
			}
			if (UAGX_Simulation::GetFrom(Terrain)->GetTimeStamp() < ReadyAt + 0.1)
				return false;
			int32 Bound = 0;
			for (TActorIterator<ALandscapeStreamingProxy> It(World); It; ++It)
				for (ULandscapeComponent* Component : It->LandscapeComponents)
					if (Component->IsRegistered() && !Component->MaterialInstancesDynamic.IsEmpty())
						++Bound;
			Test.TestTrue(TEXT("Late proxies receive runtime deformation materials"), Bound > 0);
			Test.AddInfo(
				FString::Printf(
					TEXT("Reloaded %d spatial Landscape proxies; %d ordinary components rebound."),
					Loaded, Bound));
			SourceActor->Destroy();
			return true;
		}

	private:
		FAutomationTestBase& Test;
		ATargetPoint* SourceActor = nullptr;
		UWorldPartitionStreamingSourceComponent* Source = nullptr;
		double Start = 0;
		double ReadyAt = 0;
		TEnumAsByte<EAGX_StepMode> PreviousStepMode = SmDropImmediately;
		int32 Phase = 0;
	};

	class FCheckLandscapeRendering final : public IAutomationLatentCommand
	{
	public:
		explicit FCheckLandscapeRendering(
			FAutomationTestBase& InTest, bool InCheckNormals = false, bool bInPaged = true)
			: Test(InTest)
			, bCheckNormals(InCheckNormals)
			, bPaged(bInPaged)
		{
		}
		virtual bool Update() override
		{
			if (Start == 0.0)
				Start = FPlatformTime::Seconds();
			if (FPlatformTime::Seconds() - Start > 120.0)
			{
				Test.AddError(
					TEXT("Landscape rendering did not reach its expected state within 120 s."));
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
			auto* Renderer =
				Terrain->FindComponentByClass<UAGX_TerrainRVTLandscapeRendererComponent>();
			if (Renderer == nullptr)
			{
				Test.AddError(TEXT("Example is missing its RVT Landscape renderer."));
				return true;
			}
			UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(Terrain);
			if (Simulation == nullptr || Simulation->GetTimeStamp() < 4.0)
				return false;
			if (Phase == 0)
			{
				if (Renderer->GetRetainedTileCount() == 0 || Renderer->GetGeneratedPageCount() == 0)
					return false;
				Test.TestTrue(TEXT("Terrain physics is active"), Terrain->HasNative());
				Test.TestEqual(
					TEXT("Expected paging mode"), Terrain->HasNativeTerrainPager(), bPaged);
				Test.TestFalse(
					TEXT("Legacy renderer is disabled"),
					Terrain->DefaultLandscapeRenderer->IsEnabled());
				Test.TestEqual(
					TEXT("Small pages are the default"),
					Renderer->GetDisplacementVirtualTexture()->GetTileSize(), 64);
				const FVector2D Extrema = Renderer->GetActiveDisplacementExtrema();
				Test.TestTrue(
					TEXT("Excavation reaches the renderer"), Extrema.X < -0.1 || Extrema.Y > 0.1);
				Test.TestTrue(
					TEXT("Heights use a matching source baseline"),
					Extrema.X > -1000.0 && Extrema.Y < 1000.0);
				Test.TestTrue(
					TEXT("CPU working set is finite"),
					Renderer->GetRetainedHeightBytes() < 64 * 1024 * 1024);
				int32 NaniteBindings = 0;
				int32 LandscapeBindings = 0;
				for (TActorIterator<ALandscapeProxy> It(World); It; ++It)
				{
					if (It->GetLandscapeGuid() != Terrain->SourceLandscape->GetLandscapeGuid())
						continue;
					TInlineComponentArray<ULandscapeNaniteComponent*> Nanite(*It);
					for (auto* Component : Nanite)
					{
						Test.TestTrue(TEXT("Nanite rendering is active"), Component->IsEnabled());
						const auto& Sources = Component->GetSourceLandscapeComponents();
						for (int32 Index = 0; Index < Sources.Num(); ++Index)
							if (Sources[Index] != nullptr &&
								!Sources[Index]->MaterialInstancesDynamic.IsEmpty())
							{
								Test.TestEqual(
									TEXT("Nanite shares its source component's live material"),
									Component->GetMaterial(Index),
									static_cast<UMaterialInterface*>(
										Sources[Index]->MaterialInstancesDynamic[0].Get()));
								It->SetLandscapeMaterialScalarParameterValue(
									TEXT("AGX_TestRuntimeParameter"), 0.375f);
								auto* MID =
									Cast<UMaterialInstanceDynamic>(Component->GetMaterial(Index));
								Test.TestTrue(
									TEXT("Gameplay material parameters reach Nanite"),
									MID != nullptr && MID->K2_GetScalarParameterValue(TEXT(
														  "AGX_TestRuntimeParameter")) == 0.375f);
							}
						UStaticMesh* Mesh = Component->GetStaticMesh();
						auto* Metadata =
							Mesh == nullptr
								? nullptr
								: Cast<UAGX_TerrainNaniteDisplacementData>(
									  Mesh->GetAssetUserDataOfClass(
										  UAGX_TerrainNaniteDisplacementData::StaticClass()));
						Test.TestTrue(
							TEXT("Prepared Nanite data survives loading and PIE"),
							Metadata != nullptr && Metadata->IsCurrent(*Mesh));
						if (auto* Material =
								Cast<UMaterialInstanceDynamic>(Component->GetMaterial(0)))
						{
							Test.TestEqual(
								TEXT("Prepared Nanite deformation is enabled"),
								Material->K2_GetScalarParameterValue(
									TEXT("AGX_DeformationEnabled")),
								1.f);
							++NaniteBindings;
						}
					}
					for (ULandscapeComponent* Component : It->LandscapeComponents)
						if (!Component->MaterialInstancesDynamic.IsEmpty())
							++LandscapeBindings;
				}
				Test.TestTrue(
					TEXT("Ordinary Landscape materials are bound"), LandscapeBindings > 0);
				Test.TestTrue(TEXT("Nanite materials are bound"), NaniteBindings > 0);
				Test.AddInfo(
					FString::Printf(
						TEXT(
							"Active tiles=%d, retained height bytes=%lld, generated pages=%lld, "
							"last CPU page batch=%.3f ms, displacement=[%.3f,%.3f] cm"),
						Renderer->GetRetainedTileCount(), Renderer->GetRetainedHeightBytes(),
						Renderer->GetGeneratedPageCount(),
						Renderer->GetLastPageProductionMilliseconds(), Extrema.X, Extrema.Y));
				TMap<FIntPoint, uint64> Unseen;
				CaptureTiles = Terrain->GetLandscapeRenderUpdates(Unseen);
				Frame = bPaged ? FTransform(
									 Terrain->GetNativeTerrainPager()->GetReferenceRotation(),
									 Terrain->GetNativeTerrainPager()->GetReferencePoint())
							   : FTransform(
									 Terrain->GetNative()->GetRotation(),
									 Terrain->GetNative()->GetPosition());
				double LargestChange = 0.0;
				for (const auto& Tile : CaptureTiles)
				{
					auto Baseline = Terrain->GetSourceTileSnapshot(Tile.Corner, Tile.SideVertices);
					if (!Baseline.IsValid())
						continue;
					CaptureBaselines.Add(Tile.Id, Baseline);
					for (int32 Index = 0; Index < Tile.Heights.Num(); ++Index)
					{
						const double Change =
							FMath::Abs(Tile.Heights[Index] - Baseline->Heights[Index]);
						if (Change > LargestChange)
						{
							LargestChange = Change;
							CaptureCenter =
								Tile.Corner +
								Frame.TransformVectorNoScale(FVector(
									(Index % Tile.SideVertices) * Tile.Spacing,
									(Tile.SideVertices - 1 - Index / Tile.SideVertices) *
										Tile.Spacing,
									0.0));
							CaptureCenter.Z = Frame.GetTranslation().Z + Tile.Heights[Index];
						}
					}
				}
				if (LargestChange < 0.1)
					return false; // A source snapshot may have aged out; service the new request
								  // first.
				Capture = World->SpawnActor<ASceneCapture2D>();
				Capture->SetActorLocation(CaptureCenter + FVector(0, 0, 10000));
				Capture->SetActorRotation(FRotator(-90, 0, 0));
				auto* Camera = Capture->GetCaptureComponent2D();
				Camera->bCaptureEveryFrame = false;
				Camera->bCaptureOnMovement = false;
				Camera->bAlwaysPersistRenderingState = true;
				Camera->ProjectionType = ECameraProjectionMode::Orthographic;
				Camera->OrthoWidth = CaptureWidth;
				Camera->bAutoCalculateOrthoPlanes = false;
				Camera->CaptureSource = ESceneCaptureSource::SCS_SceneDepth;
				Camera->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
				for (TActorIterator<ALandscapeProxy> It(World); It; ++It)
				{
					Camera->ShowOnlyActorComponents(*It);
					TInlineComponentArray<UPrimitiveComponent*> Components(*It);
					for (auto* Component : Components)
						Component->SetVisibleInSceneCaptureOnly(true);
				}
				Camera->TextureTarget = NewObject<UTextureRenderTarget2D>(Capture);
				Camera->TextureTarget->InitCustomFormat(
					CapturePixels, CapturePixels, PF_A32B32G32R32F, true);
				Camera->TextureTarget->UpdateResourceImmediate();
				UGameplayStatics::SetGamePaused(World, true);
				CaptureStart = FPlatformTime::Seconds();
				Phase = 1;
				return false;
			}
			if (Phase == 1)
			{
				Capture->GetCaptureComponent2D()->CaptureScene();
				if (FPlatformTime::Seconds() - CaptureStart < 3.0)
					return false;
				TArray<FLinearColor> Depth;
				Capture->GetCaptureComponent2D()
					->TextureTarget->GameThread_GetRenderTargetResource()
					->ReadLinearColorPixels(Depth);
				if (Depth.Num() != CapturePixels * CapturePixels)
				{
					Test.AddError(TEXT("Cannot read Landscape GPU depth."));
					UGameplayStatics::SetGamePaused(World, false);
					return true;
				}
				double MaximumError = 0.0;
				double ErrorSum = 0.0;
				int32 Checked = 0;
				for (int32 Row = 0; Row < CapturePixels; ++Row)
					for (int32 Column = 0; Column < CapturePixels; ++Column)
					{
						const FVector Position =
							CaptureCenter +
							FVector(
								(0.5 - (Row + 0.5) / CapturePixels) * CaptureWidth,
								((Column + 0.5) / CapturePixels - 0.5) * CaptureWidth, 0);
						for (const auto& Tile : CaptureTiles)
						{
							const FVector Local =
								Frame.InverseTransformVectorNoScale(Position - Tile.Corner) /
								Tile.Spacing;
							if (Local.X < 1 || Local.Y < 1 || Local.X >= Tile.SideVertices - 2 ||
								Local.Y >= Tile.SideVertices - 2)
								continue;
							const int32 X = FMath::FloorToInt(Local.X);
							const int32 Y = FMath::FloorToInt(Local.Y);
							auto Height = [&Tile](int32 I, int32 J)
							{
								return Tile
									.Heights[I + (Tile.SideVertices - 1 - J) * Tile.SideVertices];
							};
							const double Expected =
								Frame.GetTranslation().Z +
								FMath::Lerp(
									FMath::Lerp(Height(X, Y), Height(X + 1, Y), Local.X - X),
									FMath::Lerp(
										Height(X, Y + 1), Height(X + 1, Y + 1), Local.X - X),
									Local.Y - Y);
							const double Actual =
								CaptureCenter.Z + 10000 - Depth[Column + Row * CapturePixels].R;
							const double Error = FMath::Abs(Expected - Actual);
							MaximumError = FMath::Max(MaximumError, Error);
							ErrorSum += Error;
							++Checked;
							break;
						}
					}
				Test.TestTrue(TEXT("Depth capture covers active physics samples"), Checked > 100);
				Test.AddInfo(
					FString::Printf(
						TEXT("GPU depth vs AGX: %d samples, mean=%.3f cm, max=%.3f cm"), Checked,
						ErrorSum / FMath::Max(Checked, 1), MaximumError));
				Test.TestTrue(
					TEXT("Rendered terrain agrees with AGX within 2 cm"), MaximumError < 2.0);
				if (!bEvicted)
				{
					PagesBeforeEviction = Renderer->GetGeneratedPageCount();
					URuntimeVirtualTexture* Texture = Renderer->GetDisplacementVirtualTexture();
					ENQUEUE_RENDER_COMMAND(AGX_TestEvictTerrainPages)(
						[Texture](FRHICommandListImmediate&)
						{
							FModuleManager::GetModuleChecked<IRendererModule>(TEXT("Renderer"))
								.FlushVirtualTextureCache(
									Texture->GetAllocatedVirtualTexture(), FVector2f(0.f),
									FVector2f(1.f));
						});
					CaptureStart = FPlatformTime::Seconds();
					bEvicted = true;
					return false;
				}
				Test.TestTrue(
					TEXT("Evicted active pages regenerate without another physics edit"),
					Renderer->GetGeneratedPageCount() > PagesBeforeEviction);
				if (bCheckNormals)
				{
					Capture->GetCaptureComponent2D()->CaptureSource =
						ESceneCaptureSource::SCS_Normal;
					CaptureStart = FPlatformTime::Seconds();
					Phase = 4;
					return false;
				}
				Capture->Destroy();
				UGameplayStatics::SetGamePaused(World, false);
				Renderer->SetEnabled(false);
				Test.TestEqual(
					TEXT("Disable releases CPU tile state"), Renderer->GetRetainedTileCount(), 0);
				Test.TestEqual(
					TEXT("Disable releases height buffers"), Renderer->GetRetainedHeightBytes(),
					static_cast<int64>(0));
				TimeDisabled = Simulation->GetTimeStamp();
				Phase = 2;
				return false;
			}
			if (Phase >= 4 && Phase <= 6)
			{
				Capture->GetCaptureComponent2D()->CaptureScene();
				if (FPlatformTime::Seconds() - CaptureStart < 1.0)
					return false;
				TArray<FLinearColor> Pixels;
				Capture->GetCaptureComponent2D()
					->TextureTarget->GameThread_GetRenderTargetResource()
					->ReadLinearColorPixels(Pixels);
				double AngleSum = 0;
				int32 Checked = 0;
				const FVector Detail =
					Phase == 4 ? FVector::UpVector : FVector(0.2, 0.15, 1).GetSafeNormal();
				for (int32 Row = 0; Row < CapturePixels; ++Row)
					for (int32 Column = 0; Column < CapturePixels; ++Column)
					{
						const FVector Position =
							CaptureCenter +
							FVector(
								(0.5 - (Row + 0.5) / CapturePixels) * CaptureWidth,
								((Column + 0.5) / CapturePixels - 0.5) * CaptureWidth, 0);
						for (const auto& Tile : CaptureTiles)
						{
							const FVector Local =
								Frame.InverseTransformVectorNoScale(Position - Tile.Corner) /
								Tile.Spacing;
							if (Local.X < 1 || Local.Y < 1 || Local.X >= Tile.SideVertices - 3 ||
								Local.Y >= Tile.SideVertices - 3)
								continue;
							const int32 X = FMath::FloorToInt(Local.X);
							const int32 Y = FMath::FloorToInt(Local.Y);
							const TArray<float>* Heights = &Tile.Heights;
							if (Phase == 6)
							{
								const auto* Baseline = CaptureBaselines.Find(Tile.Id);
								if (Baseline == nullptr)
									continue;
								Heights = &(*Baseline)->Heights;
							}
							auto Height = [&Tile, Heights](int32 I, int32 J)
							{
								return (
									*Heights)[I + (Tile.SideVertices - 1 - J) * Tile.SideVertices];
							};
							auto Gradient = [&](int32 I, int32 J)
							{
								return FVector2D(
										   Height(I + 1, J) - Height(I, J),
										   Height(I, J + 1) - Height(I, J)) /
									   Tile.Spacing;
							};
							const FVector2D G = FMath::Lerp(
								FMath::Lerp(Gradient(X, Y), Gradient(X + 1, Y), Local.X - X),
								FMath::Lerp(
									Gradient(X, Y + 1), Gradient(X + 1, Y + 1), Local.X - X),
								Local.Y - Y);
							const FVector Surface =
								Frame.TransformVectorNoScale(FVector(-G.X, -G.Y, 1))
									.GetSafeNormal();
							const FVector Expected =
								FQuat::FindBetweenNormals(FVector::UpVector, Surface)
									.RotateVector(Detail);
							if (!Pixels.IsValidIndex(Column + Row * CapturePixels))
								continue;
							const FLinearColor Pixel = Pixels[Column + Row * CapturePixels];
							const FVector Actual(Pixel.R, Pixel.G, Pixel.B);
							if (Actual.SizeSquared() < 0.5)
								continue;
							AngleSum += FMath::RadiansToDegrees(
								FMath::Acos(
									FMath::Clamp(
										FVector::DotProduct(Expected, Actual.GetSafeNormal()), -1.0,
										1.0)));
							++Checked;
							break;
						}
					}
				Test.TestTrue(TEXT("GPU shading-normal capture covers terrain"), Checked > 100);
				Test.AddInfo(
					FString::Printf(
						TEXT("GPU shading normals%s: %d samples, mean angular error %.3f degrees"),
						Phase == 6 ? TEXT(" outside Terrain Bounds")
								   : (Phase == 5 ? TEXT(" with existing normal input") : TEXT("")),
						Checked, AngleSum / FMath::Max(Checked, 1)));
				Test.TestTrue(
					TEXT("Shading normals agree with the native surface and preserve normal input"),
					AngleSum / FMath::Max(Checked, 1) < 5.0);
				if (Phase == 4)
				{
					for (TActorIterator<ALandscapeProxy> It(World); It; ++It)
						It->SetLandscapeMaterialVectorParameterValue(
							TEXT("AGX_TestNormal"),
							FLinearColor(DetailNormal.X, DetailNormal.Y, DetailNormal.Z));
					Phase = 5;
					CaptureStart = FPlatformTime::Seconds();
					return false;
				}
				if (Phase == 5)
				{
					for (TActorIterator<ALandscapeProxy> It(World); It; ++It)
						It->SetLandscapeMaterialVectorParameterValue(
							TEXT("AGX_TerrainHalfExtent"), FLinearColor::Transparent);
					Phase = 6;
					CaptureStart = FPlatformTime::Seconds();
					return false;
				}
				Capture->Destroy();
				UGameplayStatics::SetGamePaused(World, false);
				return true;
			}
			if (Phase == 2)
			{
				if (Simulation->GetTimeStamp() < TimeDisabled + 0.5)
					return false;
				Test.TestTrue(
					TEXT("Disabling rendering does not stop physics"), Terrain->HasNative());
				Test.TestEqual(
					TEXT("Paging mode survives renderer disablement"),
					Terrain->HasNativeTerrainPager(), bPaged);
				if (bPaged)
					Test.TestTrue(
						TEXT("Active native tiles survive renderer disablement"),
						!Terrain->GetNativeTerrainPager()->GetActiveTileTransforms().IsEmpty());
				for (TActorIterator<ALandscapeProxy> It(World); It; ++It)
				{
					It->bUseDynamicMaterialInstance = true;
					for (ULandscapeComponent* Component : It->LandscapeComponents)
					{
						if (Component->MaterialInstancesDynamic.IsEmpty())
							for (UMaterialInstanceConstant* Parent : Component->MaterialInstances)
								Component->MaterialInstancesDynamic.Add(
									UMaterialInstanceDynamic::Create(Parent, Component));
						for (UMaterialInstanceDynamic* Material :
							 Component->MaterialInstancesDynamic)
							if (Material != nullptr)
								Material->SetScalarParameterValue(
									TEXT("AGX_TestExistingOverride"), 42.5f);
					}
				}
				Renderer->SetEnabled(true);
				Phase = 3;
				return false;
			}
			if (Renderer->GetRetainedTileCount() == 0 || Renderer->GetGeneratedPageCount() == 0)
				return false;
			Test.TestTrue(
				TEXT("Re-enabled rendering restores the working set"),
				Renderer->GetRetainedHeightBytes() > 0);
			const FVector2D Restored = Renderer->GetActiveDisplacementExtrema();
			if (!bPaged)
				Test.TestTrue(
					TEXT("Rebinding retains the original deformation baseline"),
					Restored.X < -0.1 || Restored.Y > 0.1);
			for (TActorIterator<ALandscapeProxy> It(World); It; ++It)
				for (ULandscapeComponent* Component : It->LandscapeComponents)
					for (UMaterialInstanceDynamic* Material : Component->MaterialInstancesDynamic)
						if (Material != nullptr)
							Test.TestEqual(
								TEXT("Binding preserves existing runtime overrides"),
								Material->K2_GetScalarParameterValue(
									TEXT("AGX_TestExistingOverride")),
								42.5f);
			return true;
		}

	private:
		FAutomationTestBase& Test;
		bool bCheckNormals = false;
		bool bPaged = true;
		const FVector DetailNormal = FVector(0.2, 0.15, 1).GetSafeNormal();
		double Start = 0.0;
		double TimeDisabled = 0.0;
		bool bEvicted = false;
		int64 PagesBeforeEviction = 0;
		int32 Phase = 0;
		ASceneCapture2D* Capture = nullptr;
		FVector CaptureCenter;
		FTransform Frame;
		TArray<FTerrainTileHeights> CaptureTiles;
		TMap<FIntPoint, TSharedPtr<const FAGX_TerrainSourceTile, ESPMode::ThreadSafe>>
			CaptureBaselines;
		double CaptureStart = 0;
		static constexpr int32 CapturePixels = 128;
		static constexpr double CaptureWidth = 160;
	};
}

IMPLEMENT_COMPLEX_AUTOMATION_TEST(
	FAGX_TerrainLandscapeRenderingTest, "AGXUnreal.Game.TerrainLandscapeRendering",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

void FAGX_TerrainLandscapeRenderingTest::GetTests(
	TArray<FString>& Names, TArray<FString>& Commands) const
{
	Names.Add(TEXT("OrdinaryLandscape"));
	Commands.Add(TEXT("/Game/Levels/TerrainLandscapePaging"));
	Names.Add(TEXT("TranslatedLandscape"));
	Commands.Add(TEXT("Translated"));
	Names.Add(TEXT("SlopedLandscape"));
	Commands.Add(TEXT("Slope"));
	Names.Add(TEXT("WorldPartitionLandscape"));
	Commands.Add(TEXT("/Game/Levels/TerrainLandscapePaging_WP"));
	Names.Add(TEXT("CoarseSourceFinePhysics"));
	Commands.Add(TEXT("Coarse"));
	Names.Add(TEXT("CoarseCollisionMip"));
	Commands.Add(TEXT("CoarseMip"));
	Names.Add(TEXT("ShadingNormalsFlat"));
	Commands.Add(TEXT("Normals"));
	Names.Add(TEXT("ShadingNormalsSlope"));
	Commands.Add(TEXT("NormalsSlope"));
	Names.Add(TEXT("NegativeDeviceProfileBias"));
	Commands.Add(TEXT("ProfileNegative"));
	Names.Add(TEXT("PositiveDeviceProfileBias"));
	Commands.Add(TEXT("ProfilePositive"));
	Names.Add(TEXT("NonPagedLandscape"));
	Commands.Add(TEXT("NonPaged"));
	Names.Add(TEXT("NonPagedExample"));
	Commands.Add(TEXT("/Game/Levels/TerrainLandscapeRVT_NonPaged"));
	Names.Add(TEXT("NonPagedSlopedLandscape"));
	Commands.Add(TEXT("NonPagedSlope"));
	Names.Add(TEXT("NonPagedRectangularLandscape"));
	Commands.Add(TEXT("NonPagedRectangular"));
	Names.Add(TEXT("NonPagedTranslatedLandscape"));
	Commands.Add(TEXT("NonPagedTranslated"));
	Names.Add(TEXT("NonPagedShadingNormalsSlope"));
	Commands.Add(TEXT("NonPagedNormalsSlope"));
}

bool FAGX_TerrainLandscapeRenderingTest::RunTest(const FString& Map)
{
	const bool bNonPaged = Map.StartsWith(TEXT("NonPaged")) || Map.EndsWith(TEXT("_NonPaged"));
	TSharedPtr<FScopedTileCountBias> ProfileBias;
	if (Map.StartsWith(TEXT("Profile")))
		ProfileBias = MakeShared<FScopedTileCountBias>(Map == TEXT("ProfileNegative") ? -2 : 1);
	ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(
		(!Map.StartsWith(TEXT("/Game/"))) ? TEXT("/Game/Levels/TerrainLandscapePaging") : Map));
	if (Map.StartsWith(TEXT("Coarse")) || Map.Contains(TEXT("Normals")))
		ADD_LATENT_AUTOMATION_COMMAND(FAGX_PrepareLandscapeScenario(
			*this, Map.StartsWith(TEXT("Coarse")), Map.Contains(TEXT("Normals")), false,
			Map == TEXT("CoarseMip") ? 1 : 0));
	if (Map.Contains(TEXT("Slope")) || Map.StartsWith(TEXT("Profile")))
		ADD_LATENT_AUTOMATION_COMMAND(FPrepareSlopedLandscape(*this));
	if (Map.Contains(TEXT("Translated")))
		ADD_LATENT_AUTOMATION_COMMAND(FTranslateLandscapeFixture());
	if (bNonPaged && !Map.EndsWith(TEXT("_NonPaged")))
		ADD_LATENT_AUTOMATION_COMMAND(
			FConfigureNonPagedTerrain(Map == TEXT("NonPagedRectangular")));
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(
		FCheckLandscapeRendering(*this, Map.Contains(TEXT("Normals")), !bNonPaged));
	if (Map.EndsWith(TEXT("_WP")))
		ADD_LATENT_AUTOMATION_COMMAND(FCheckProxyStreaming(*this));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	if (ProfileBias.IsValid())
		ADD_LATENT_AUTOMATION_COMMAND(FRestoreTileCountBias(ProfileBias));
	return true;
}
