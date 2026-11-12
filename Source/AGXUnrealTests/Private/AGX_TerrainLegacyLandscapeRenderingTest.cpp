// Copyright 2026, Algoryx Simulation AB.

// AGX Dynamics for Unreal includes.
#include "AGX_Simulation.h"
#include "Terrain/AGX_Terrain.h"
#include "Terrain/AGX_TerrainLandscapeRendererComponent.h"
#include "Terrain/AGX_TerrainRVTLandscapeRendererComponent.h"

// Unreal Engine includes.
#include "Editor.h"
#include "Engine/TextureRenderTarget2D.h"
#include "EngineUtils.h"
#include "HAL/PlatformTime.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"

namespace
{
	class FConfigureLegacyTerrain final : public IAutomationLatentCommand
	{
	public:
		explicit FConfigureLegacyTerrain(bool bInPaged)
			: bPaged(bInPaged)
		{
		}
		virtual bool Update() override
		{
			for (TActorIterator<AAGX_Terrain> It(GEditor->GetEditorWorldContext().World()); It;
				 ++It)
			{
				It->SetEnableTerrainPaging(bPaged);
				// This released fixture intentionally saved displacement rendering off.
				It->bEnableDisplacementRendering = true;
				It->DefaultLandscapeRenderer->SetEnabled(true);
			}
			return true;
		}

	private:
		bool bPaged;
	};

	class FConfigureRejectedLegacyTerrain final : public IAutomationLatentCommand
	{
	public:
		virtual bool Update() override
		{
			for (TActorIterator<AAGX_Terrain> It(GEditor->GetEditorWorldContext().World()); It;
				 ++It)
			{
				auto* Renderer = It->DefaultLandscapeRenderer;
				Renderer->SetEnabled(true);
				auto* Texture = NewObject<UTextureRenderTarget2D>(Renderer);
				Texture->ClearColor = FLinearColor(7, 0, 0, 1);
				Texture->InitCustomFormat(8, 8, PF_R16F, true);
				Texture->UpdateResourceImmediate(true);
				Renderer->LandscapeDisplacementMap = Texture;
				It->bEnableDisplacementRendering = true;
				It->LandscapeDisplacementMap = Texture;
				TInlineComponentArray<UAGX_TerrainRVTLandscapeRendererComponent*> Others(*It);
				for (auto* Other : Others)
					Other->SetEnabled(false);
			}
			return true;
		}
	};

	class FCheckRejectedLegacyTerrain final : public IAutomationLatentCommand
	{
	public:
		explicit FCheckRejectedLegacyTerrain(FAutomationTestBase& InTest)
			: Test(InTest)
		{
		}
		virtual bool Update() override
		{
			if (Start == 0)
				Start = FPlatformTime::Seconds();
			if (FPlatformTime::Seconds() - Start > 30)
			{
				Test.AddError(TEXT("Partitioned Terrain physics did not start."));
				return true;
			}
			if (GEditor->PlayWorld == nullptr)
				return false;
			for (TActorIterator<AAGX_Terrain> It(GEditor->PlayWorld); It; ++It)
			{
				auto* Simulation = UAGX_Simulation::GetFrom(*It);
				if (Simulation == nullptr || Simulation->GetTimeStamp() < 1)
					return false;
				Test.TestTrue(
					TEXT("Unsupported rendering leaves physics and paging running"),
					It->HasNativeTerrainPager());
				auto* Renderer = It->DefaultLandscapeRenderer;
				// A rejected component must stay rejected without re-notifying each step.
				Renderer->Initialize(**It);
				Renderer->Initialize(**It);
				Renderer->Update();
				UTextureRenderTarget2D* Texture = Renderer->LandscapeDisplacementMap;
				Test.TestEqual(
					TEXT("Rejected renderer does not resize its target"), Texture->SizeX, 8);
				Test.TestEqual(
					TEXT("Rejected renderer does not resize its target"), Texture->SizeY, 8);
				TArray<FLinearColor> Pixels;
				Test.TestTrue(
					TEXT("Read sentinel target"),
					Texture->GameThread_GetRenderTargetResource()->ReadLinearColorPixels(Pixels));
				Test.TestEqual(TEXT("Sentinel target retains its dimensions"), Pixels.Num(), 64);
				for (const auto& Pixel : Pixels)
					Test.TestEqual(
						TEXT("Rejected renderer does not clear or upload its target"), Pixel.R,
						7.f);
				return true;
			}
			return false;
		}

	private:
		FAutomationTestBase& Test;
		double Start = 0;
	};

	class FCheckLegacyTerrainRendering final : public IAutomationLatentCommand
	{
	public:
		explicit FCheckLegacyTerrainRendering(FAutomationTestBase& InTest)
			: Test(InTest)
		{
		}
		virtual bool Update() override
		{
			if (Start == 0.0)
				Start = FPlatformTime::Seconds();
			if (FPlatformTime::Seconds() - Start > 45.0)
			{
				Test.AddError(TEXT("Released Landscape renderer did not become ready."));
				return true;
			}
			UWorld* World = GEditor->PlayWorld;
			if (World == nullptr)
				return false;
			for (TActorIterator<AAGX_Terrain> It(World); It; ++It)
			{
				auto* Simulation = UAGX_Simulation::GetFrom(*It);
				auto* Renderer = It->DefaultLandscapeRenderer;
				if (Simulation == nullptr || Simulation->GetTimeStamp() < 4.0 ||
					Renderer == nullptr)
					return false;
				auto* Texture = Renderer->LandscapeDisplacementMap;
				if (Texture == nullptr || Texture->GameThread_GetRenderTargetResource() == nullptr)
					return false;
				if (Phase == 1 && Simulation->GetTimeStamp() < ChangedAt + 0.2)
					return false;
				if (Phase == 2 && Simulation->GetTimeStamp() < ChangedAt + 0.2)
					return false;
				TArray<FLinearColor> Pixels;
				if (!Texture->GameThread_GetRenderTargetResource()->ReadLinearColorPixels(Pixels))
				{
					Test.AddError(TEXT("Cannot read released displacement target."));
					return true;
				}
				double Largest = 0;
				for (const auto& Pixel : Pixels)
					Largest = FMath::Max(Largest, FMath::Abs(static_cast<double>(Pixel.R)));
				if (Phase == 0 && Largest < 0.1)
					return false; // Wait for the example shovel's first contact.
				Test.TestTrue(TEXT("Terrain physics continues"), It->HasNative());
				if (Phase == 1)
				{
					Test.TestTrue(TEXT("Disabling clears the released target"), Largest < 0.01);
					Renderer->SetEnabled(true);
					ChangedAt = Simulation->GetTimeStamp();
					Phase = 2;
					return false;
				}
				Test.TestTrue(
					Phase == 0 ? TEXT("Released renderer displays excavation")
							   : TEXT("Re-enabling restores existing excavation"),
					Largest > 0.1);
				if (Phase == 2)
					return true;
				Renderer->SetEnabled(false);
				ChangedAt = Simulation->GetTimeStamp();
				Phase = 1;
				return false;
			}
			return false;
		}

	private:
		FAutomationTestBase& Test;
		double Start = 0, ChangedAt = 0;
		int32 Phase = 0;
	};
}

IMPLEMENT_COMPLEX_AUTOMATION_TEST(
	FAGX_LegacyTerrainRenderingTest, "AGXUnreal.Game.TerrainLegacyLandscapeRendering",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

void FAGX_LegacyTerrainRenderingTest::GetTests(
	TArray<FString>& Names, TArray<FString>& Commands) const
{
	Names.Add(TEXT("NonPaged"));
	Commands.Add(TEXT("NonPaged"));
	Names.Add(TEXT("Paged"));
	Commands.Add(TEXT("Paged"));
	Names.Add(TEXT("PartitionedRejected"));
	Commands.Add(TEXT("PartitionedRejected"));
}

bool FAGX_LegacyTerrainRenderingTest::RunTest(const FString& Mode)
{
	if (Mode == TEXT("PartitionedRejected"))
	{
		AddExpectedMessage(
			TEXT("supports only non-partitioned Landscapes"), ELogVerbosity::Warning,
			EAutomationExpectedMessageFlags::Contains, 1);
		ADD_LATENT_AUTOMATION_COMMAND(
			FEditorLoadMap(TEXT("/Game/Levels/TerrainLandscapePaging_WP")));
		ADD_LATENT_AUTOMATION_COMMAND(FConfigureRejectedLegacyTerrain());
		ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
		ADD_LATENT_AUTOMATION_COMMAND(FCheckRejectedLegacyTerrain(*this));
		ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
		return true;
	}
	ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(TEXT("/Game/Tests/Test_TerrainPaging")));
	ADD_LATENT_AUTOMATION_COMMAND(FConfigureLegacyTerrain(Mode == TEXT("Paged")));
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(FCheckLegacyTerrainRendering(*this));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}
