// Copyright 2024, Algoryx Simulation AB.

/*
 * This files contains simulation unit tests for Shovel Component.
 */

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_PlayInEditorUtils.h"
#include "AGX_Simulation.h"
#include "AgxAutomationCommon.h"
#include "Terrain/AGX_Terrain.h"

// Unreal Engine includes.
#include "Editor.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"

//
// Shovel digging test starts here.
//

struct FShovelDiggingState
{
	AActor* Actor {nullptr};
};

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FCheckShovelDiggingCommand, FAutomationTestBase&, Test);

bool FCheckShovelDiggingCommand::Update()
{
	check(GEditor != nullptr);
	FWorldContext* WorldContext = GEditor->GetPIEWorldContext();
	check(WorldContext != nullptr);
	UWorld* World = WorldContext->World();
	check(World != nullptr);
	UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(World);
	FAGX_Statistics Statistics = Simulation->GetStatistics();
	Test.TestTrue(TEXT("Number of particles in statistics."), Statistics.NumParticles > 0);
	TArray<AActor*> AllTerrains;
	UGameplayStatics::GetAllActorsOfClass(World, AAGX_Terrain::StaticClass(), AllTerrains);
	Test.TestEqual(TEXT("Number of terrains"), AllTerrains.Num(), 1);
	if (AllTerrains.Num() != 1)
		return true;
	AAGX_Terrain* Terrain = Cast<AAGX_Terrain>(AllTerrains[0]);
	Test.TestNotNull(TEXT("Terrain"), Terrain);
	if (Terrain == nullptr)
		return true;
	Test.TestTrue(TEXT("Number of particles in Terrain."), Terrain->GetNumParticles() > 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FShovelDiggingTest, "AGXUnreal.Game.AGX_Shovel.Digging",
	AgxAutomationCommon::ETF_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FShovelDiggingTest::RunTest(const FString& Parameters)
{
	using namespace AGX_PlayInEditorUtils;

	AddExpectedError(
		TEXT("Could not allocate resource for Landscape Displacement Map for AGX Terrain"),
		EAutomationExpectedErrorFlags::Contains, 0);

	// Setup initial state.
	const FString MapPath {"/Game/Tests/Test_ShovelComponent"};
	ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(MapPath));
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(true));
	ADD_LATENT_AUTOMATION_COMMAND(AgxAutomationCommon::FWaitUntilPIEUpCommand);
	ADD_LATENT_AUTOMATION_COMMAND(FTickUntilTimeStamp(5.0));

	// Run the checks.
	ADD_LATENT_AUTOMATION_COMMAND(FCheckShovelDiggingCommand(*this));

	// Restore clean state.
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand);
	ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(EmptyMapPath));

	return true;
}
//
// Shovel properties setters test starts here.
//
