// Copyright 2025, Algoryx Simulation AB.

/*
 * This file contains a number of Play In Editor unit tests.
 *
 * Each test loads a Level, starts a Play In Editor session, and then verifies that the simulation
 * behaves as expected. Each test is associated with a Level asset in /Game/Tests/, the Levels whose
 * name does not start with FTEST_, those are Blueprint Functional Tests.
 *
 * Each test follows the same structure:
 * - A Latent Automation Command named FCheckLEVEL_NAMECommand.
 * - A Simple Automation Test named AGXUnreal.Game.AGX_PlayInEditorTest.LEVEL_NAME.
 *   - Has Latent Commands to:
 *   - Load the level.
 *   - Start Play In Editor session.
 *   - Check the simulation state, i.e. the test-specific Latent Command.
 *   - End the Play In Editor session.
 *
 * Any state needed by the test is created in the Run Test function and copied into the test
 * Latent Command. Several tests need to do some setup in the beginning of the test. This is
 * typically done by passing in a container to store data in and by checking if the container is
 * empty we know if we are in the beginning of the test or not.
 */

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_PlayInEditorUtils.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_Simulation.h"
#include "AgxAutomationCommon.h"
#include "Constraints/AGX_BallConstraintComponent.h"
#include "Materials/AGX_ContactMaterialRegistrarComponent.h"
#include "Materials/AGX_ContactMaterial.h"
#include "Sensors/SensorEnvironmentBarrier.h"
#include "Shapes/AGX_BoxShapeComponent.h"
#include "Shapes/AGX_SphereShapeComponent.h"
#include "Terrain/AGX_Terrain.h"

// Unreal Engine includes.
#include "Components/StaticMeshComponent.h"
#include "Editor.h"
#include "Containers/Map.h"
#include "HAL/FileManager.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"

namespace AGX_PlayInEditorTest_helpers
{
	using namespace AGX_PlayInEditorUtils;

	template <typename T>
	T* GetComponentByName(UWorld* TestWorld, const FString& ActorName, const FString& ComponentName)
	{
		TMap<FString, AActor*> Actors = GetActorsByName(TestWorld, {ActorName});
		AActor* Actor = Actors.FindRef(ActorName);
		if (Actor == nullptr)
			return nullptr;

		for (const auto& Component : Actor->GetComponents())
		{
			if (Component->GetName().Equals(ComponentName))
			{
				return Cast<T>(Component);
			}
		}

		return nullptr;
	}
}

// For some reason, the DEFINE_LATENT_AUTOMATION_COMMAND macro fails when using TMap<FString,
// AActor*> directly.
using ActorMap = TMap<FString, AActor*>;
using ComponentMap = TMap<FString, UActorComponent*>;

//
// FallingBox test starts here.
//

DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(
	FCheckFallinBoxMovedCommand, double, SimTimeMax, ComponentMap, ComponentsOfInterest,
	FAutomationTestBase&, Test);

bool FCheckFallinBoxMovedCommand::Update()
{
	using namespace AgxAutomationCommon;
	using namespace AGX_PlayInEditorTest_helpers;

	if (!GEditor->IsPlayingSessionInEditor())
	{
		return false;
	}

	static int32 NumTicks = 0;
	UWorld* TestWorld = GEditor->GetPIEWorldContext()->World();
	if (ComponentsOfInterest.Num() == 0)
	{
		NumTicks = 0;
		ActorMap BoxActors = GetActorsByName(TestWorld, {"BoxActor"});

		Test.TestTrue("Found actor of interest", BoxActors.Contains("BoxActor"));
		if (!BoxActors.Contains("BoxActor"))
		{
			return true;
		}

		auto Body = GetRigidBodyByName(*BoxActors["BoxActor"], TEXT("BoxBody"));
		Test.TestNotNull("BoxBody", Body);
		if (Body == nullptr)
			return true;

		ComponentsOfInterest.Add("BoxBody", Body);
		Test.TestTrue("Body initial z pos", Body->GetComponentLocation().Z > 299.0);
	}

	UAGX_Simulation* Sim = UAGX_Simulation::GetFrom(TestWorld);
	Test.TestNotNull("Simulation", Sim);
	if (Sim == nullptr)
		return true;

	const auto SimTime = Sim->GetTimeStamp();
	{
		// Sanity check to avoid hanging forever if the Simulation is not ticking.
		NumTicks++;
		if (NumTicks > 1000 && FMath::IsNearlyZero(SimTime))
		{
			Test.AddError(FString::Printf(
				TEXT("SimTime too small: %f. The Simulation has not stepped as expected."),
				SimTime));
			return true;
		}
	}

	if (SimTime < SimTimeMax)
	{
		return false; // Continue ticking.
	}

	// At this point we have ticked to TickMax.
	auto Body = Cast<UAGX_RigidBodyComponent>(ComponentsOfInterest["BoxBody"]);
	Test.TestTrue("Body final z pos", Body->GetComponentLocation().Z < 299.0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFallingBoxTest, "AGXUnreal.Game.AGX_PlayInEditorTest.FallingBox",
	AgxAutomationCommon::ETF_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FFallingBoxTest::RunTest(const FString& Parameters)
{
	using namespace AGX_PlayInEditorTest_helpers;
	FString MapPath = FString("/Game/Tests/FallingBox");

	ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(MapPath))
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(true));

	ComponentMap ComponentsOfInterest;
	double SimTimeMax = 0.5;
	ADD_LATENT_AUTOMATION_COMMAND(
		FCheckFallinBoxMovedCommand(SimTimeMax, ComponentsOfInterest, *this));

	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand);
	return true;
}

//
// Ball Joint Twist Range Controller test starts here.
//
// The level contains a Blueprint instance containing two parallel body + constraint setups. One of
// them have had their properties set in the Blueprint editor's Details panel while the other have
// had their properties set by a Blueprint Visual Script executed on Begin Play.
//

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FCheckTwistRangeCommand, FAutomationTestBase&, Test);

bool FCheckTwistRangeCommand::Update()
{
	using namespace AGX_PlayInEditorTest_helpers;
	using namespace AgxAutomationCommon;

	if (!GEditor->IsPlayingSessionInEditor())
	{
		Test.AddError(TEXT("FCheckTwistRangeCommand is not in a Play In Editor session."));
		return true;
	}

	// Get the objects we need.
	UWorld* TestWorld = GEditor->GetPIEWorldContext()->World();
	if (!Test.TestNotNull(TEXT("PIE World"), TestWorld))
		return true;
	if (!Test.TestEqual(TEXT("Level name"), TestWorld->GetName(), TEXT("BallJointTwistRange")))
		return true;
	ActorMap Actors = GetActorsByName(TestWorld, {"Ball"});
	AActor* BallActor = Actors.FindRef(TEXT("Ball"));
	if (!Test.TestNotNull(TEXT("Actor 'Ball'"), BallActor))
		return true;
	UAGX_RigidBodyComponent* DetailsRod = GetRigidBodyByName(*BallActor, TEXT("Details Rod Body"));
	if (!Test.TestNotNull(TEXT("Details Rod Body"), DetailsRod))
		return true;
	UAGX_RigidBodyComponent* BpRod = GetRigidBodyByName(*BallActor, TEXT("BP Rod Body"));
	if (!Test.TestNotNull(TEXT("BP Rod Body"), BpRod))
		return true;
	UAGX_BallConstraintComponent* DetailsBall =
		GetBallConstraintByName(*BallActor, TEXT("Details Ball Constraint"));
	if (!Test.TestNotNull(TEXT("Details Ball Constraint"), DetailsBall))
		return true;
	UAGX_BallConstraintComponent* BpBall =
		GetBallConstraintByName(*BallActor, "BP Ball Constraint");
	if (!Test.TestNotNull(TEXT("BP Ball Constraint"), BpBall))
		return true;

	// Test the objects configured in the Details panel.
	Test.TestTrue(TEXT("Details Rod Body has native"), DetailsRod->HasNative());
	Test.TestEqual(TEXT("Details Rod Body roll"), DetailsRod->GetRotator().Roll, 20.0);
	Test.TestTrue(TEXT("Details Ball Constraint has native"), DetailsBall->HasNative());
	FAGX_TwistRangeController& DetailsTwist = DetailsBall->TwistRangeController;
	Test.TestEqual(TEXT("Ball Constraint RangeMin"), DetailsTwist.GetRangeMin(), -20.0);
	Test.TestEqual(TEXT("Ball Constraint RangeMax"), DetailsTwist.GetRangeMax(), 10.0);
	Test.TestEqual(TEXT("Ball Constraint Enable"), DetailsTwist.GetEnable(), true);
	Test.TestEqual(TEXT("Ball Constraint Compliance"), DetailsTwist.GetCompliance(), 1e-9);
	Test.TestEqual(TEXT("Ball Constraint SpookDamping"), DetailsTwist.GetSpookDamping(), 0.04);
	Test.TestEqual(TEXT("Ball Constraint ForceRangeMin"), DetailsTwist.GetForceRangeMin(), -1000.0);
	Test.TestEqual(TEXT("Ball Constraint ForceRangeMax"), DetailsTwist.GetForceRangeMax(), 1000.0);

	// Test the objects configured with Blueprint Visual Script.
	Test.TestTrue(TEXT("BP Rod Body has native"), BpRod->HasNative());
	Test.TestEqual(TEXT("BP Rod Body roll"), BpRod->GetRotator().Roll, 15.0);
	Test.TestTrue(TEXT("BP Ball Constraint has native"), BpBall->HasNative());
	FAGX_TwistRangeController& BpTwist = BpBall->TwistRangeController;
	Test.TestEqual(TEXT("Ball Constraint RangeMin"), BpTwist.GetRangeMin(), -15.0);
	Test.TestEqual(TEXT("Ball Constraint RangeMax"), BpTwist.GetRangeMax(), 20.0);
	Test.TestEqual(TEXT("Ball Constraint Enable"), BpTwist.GetEnable(), true);
	Test.TestEqual(TEXT("Ball Constraint Compliance"), BpTwist.GetCompliance(), 1e-9);
	Test.TestEqual(TEXT("Ball Constraint SpookDamping"), BpTwist.GetSpookDamping(), 0.035);
	Test.TestEqual(TEXT("Ball Constraint ForceRangeMin"), BpTwist.GetForceRangeMin(), -2000.0);
	Test.TestEqual(TEXT("Ball Constraint ForceRangeMax"), BpTwist.GetForceRangeMax(), 2000.0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FTwistRangeTest, "AGXUnreal.Game.AGX_PlayInEditorTest.TwistRange",
	AgxAutomationCommon::DefaultTestFlags)

bool FTwistRangeTest::RunTest(const FString& Parameters)
{
	using namespace AGX_PlayInEditorTest_helpers;
	using namespace AgxAutomationCommon;

	FString MapPath("/Game/Tests/BallJointTwistRange");
	ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(MapPath));
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(true));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilPIEUpCommand());
	const double SimTime = 1.0;
	const int32 MaxTicks = 1000;
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilSimTime(SimTime, MaxTicks));
	ADD_LATENT_AUTOMATION_COMMAND(FCheckTwistRangeCommand(*this));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand);
	ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilPIEDownCommand());
	return true;
}

//
// Terrain Paging test starts here.
//

DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(
	FCheckTerrainPagingStateCommand, double, SimTimeMax, ComponentMap, ComponentsOfInterest,
	FAutomationTestBase&, Test);

bool FCheckTerrainPagingStateCommand::Update()
{
	using namespace AGX_PlayInEditorTest_helpers;
	using namespace AgxAutomationCommon;

	if (!GEditor->IsPlayingSessionInEditor())
	{
		return false;
	}

	static int32 NumTicks = 0;
	UWorld* TestWorld = GEditor->GetPIEWorldContext()->World();
	if (ComponentsOfInterest.Num() == 0)
	{
		NumTicks = 0;
		auto Chassi = GetComponentByName<UAGX_RigidBodyComponent>(TestWorld, "Car", "Chassi");
		Test.TestNotNull("Chassi", Chassi);

		auto Untracked =
			GetComponentByName<UAGX_RigidBodyComponent>(TestWorld, "Untracked", "Untracked");
		Test.TestNotNull("Untracked", Untracked);

		if (Chassi == nullptr || Untracked == nullptr)
			return true;

		ComponentsOfInterest.Add("Chassi", Chassi);
		ComponentsOfInterest.Add("Untracked", Untracked);
		Test.TestTrue("Chassi body initial y pos", Chassi->GetComponentLocation().Y < 10.0);
		Test.TestTrue("Untracked body initial z pos", Chassi->GetComponentLocation().Z > 0.0);

		// Ensure ShapeMaterial is still assigned (sanity check that may fail due to backwards
		// compatibility breaks).
		auto ChassiBox = GetComponentByName<UAGX_BoxShapeComponent>(TestWorld, "Car", "ChassiBox");
		Test.TestNotNull("ChassiBox", ChassiBox);
		if (ChassiBox == nullptr)
			return true;

		Test.TestNotNull("ShapeMaterial of ChassiBox", ChassiBox->ShapeMaterial);
	}

	UAGX_Simulation* Sim = UAGX_Simulation::GetFrom(TestWorld);
	Test.TestNotNull("Simulation", Sim);
	if (Sim == nullptr)
		return true;

	const auto SimTime = Sim->GetTimeStamp();
	{
		// Sanity check to avoid hanging forever if the Simulation is not ticking.
		NumTicks++;
		if (NumTicks > 1000 && FMath::IsNearlyZero(SimTime))
		{
			Test.AddError(FString::Printf(
				TEXT("SimTime too small: %f. The Simulation has not stepped as expected."),
				SimTime));
			return true;
		}
	}

	if (SimTime < SimTimeMax)
	{
		return false; // Continue ticking..
	}

	// At this point we have ticked to TickMax.
	auto ChassiBody = Cast<UAGX_RigidBodyComponent>(ComponentsOfInterest["Chassi"]);
	auto UntrackedBody = Cast<UAGX_RigidBodyComponent>(ComponentsOfInterest["Untracked"]);

	// The "Car" is tracked by the Terrain Pager and should have moved forwards.
	Test.TestTrue("Chassi body final y pos", ChassiBody->GetComponentLocation().Y > 50.0);

	// We expect the untracked body to fall through the landscape towards negative infinity since no
	// Terrain Tile has been created for it.
	Test.TestTrue("Untracked body final z pos", UntrackedBody->GetComponentLocation().Z < 0.0);

	// Ensure we have spawned some particles (there is a shovel in the Level).
	ActorMap TerrainActors = GetActorsByName(TestWorld, {"AGX_Terrain_1"});
	AAGX_Terrain* TerrainActor = Cast<AAGX_Terrain>(TerrainActors.FindRef("AGX_Terrain_1"));
	Test.TestNotNull("Terrain actor", TerrainActor);
	if (TerrainActor == nullptr)
		return true;

	Test.TestTrue("Has spawned particles", TerrainActor->GetNumParticles() > 5);

	// Ensure TerrainMaterial is assigned (sanity check that may fail due to backwards
	// compatibility breaks).
	Test.TestNotNull("Terrain Material", TerrainActor->TerrainMaterial);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FTerrainPagingTest, "AGXUnreal.Game.AGX_PlayInEditorTest.TerrainPaging",
	AgxAutomationCommon::ETF_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTerrainPagingTest::RunTest(const FString& Parameters)
{
	using namespace AGX_PlayInEditorTest_helpers;
	FString MapPath = FString("/Game/Tests/Test_TerrainPaging");

	ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(MapPath))
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(true));

	ComponentMap ComponentsOfInterest;
	double SimTimeMax = 4.0;
	ADD_LATENT_AUTOMATION_COMMAND(
		FCheckTerrainPagingStateCommand(SimTimeMax, ComponentsOfInterest, *this));

	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand);
	return true;
}

//
// Playground test starts here.
//
// The Playground level contains a mixture of several different things/mechanisms/scenarios and
// exists to make it easy to add play-in-editor test scenarios in the future. Simply add the
// scenario in the level and add the test logic for it here.
//

DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(
	FCheckPlaygroundStateCommand, double, SimTimeMax, ComponentMap, ComponentsOfInterest,
	FAutomationTestBase&, Test);

bool FCheckPlaygroundStateCommand::Update()
{
	using namespace AGX_PlayInEditorTest_helpers;
	if (!GEditor->IsPlayingSessionInEditor())
	{
		return false;
	}

	UWorld* TestWorld = GEditor->GetPIEWorldContext()->World();
	static int32 NumTicks = 0;
	if (ComponentsOfInterest.Num() == 0)
	{
		NumTicks = 0;
		auto FallDynamicBody =
			GetComponentByName<UAGX_RigidBodyComponent>(TestWorld, "Collisions", "FallDynamicBody");
		Test.TestNotNull("FallDynamicBody", FallDynamicBody);

		auto ConstrainedBody = GetComponentByName<UAGX_RigidBodyComponent>(
			TestWorld, "Collisions", "ConstrainedBody2");
		Test.TestNotNull("ConstrainedBody", ConstrainedBody);

		auto SlideBody =
			GetComponentByName<UAGX_RigidBodyComponent>(TestWorld, "Slide", "SlideBody");
		Test.TestNotNull("SlideBody", SlideBody);

		if (FallDynamicBody == nullptr || ConstrainedBody == nullptr || SlideBody == nullptr)
			return true;

		ComponentsOfInterest.Add("FallDynamicBody", FallDynamicBody);
		ComponentsOfInterest.Add("ConstrainedBody", ConstrainedBody);
		ComponentsOfInterest.Add("SlideBody", SlideBody);

		Test.TestTrue(
			"FallDynamicBody initial z pos", FallDynamicBody->GetComponentLocation().Z > 0.0);
		Test.TestTrue(
			"ConstrainedBody initial z pos", ConstrainedBody->GetComponentLocation().Z > 0.0);
		Test.TestTrue("SlideBody initial z pos", SlideBody->GetComponentLocation().Z > 0.0);
	}

	UAGX_Simulation* Sim = UAGX_Simulation::GetFrom(TestWorld);
	Test.TestNotNull("Simulation", Sim);
	if (Sim == nullptr)
		return true;

	const auto SimTime = Sim->GetTimeStamp();
	{
		// Sanity check to avoid hanging forever if the Simulation is not ticking.
		NumTicks++;
		if (NumTicks > 1000 && FMath::IsNearlyZero(SimTime))
		{
			Test.AddError(FString::Printf(
				TEXT("SimTime too small: %f. The Simulation has not stepped as expected."),
				SimTime));
			return true;
		}
	}

	if (SimTime < SimTimeMax)
	{
		return false; // Continue ticking..
	}

	// At this point we have simulated to SimTimeMax. Check the final state.
	auto FallDynamicBody = Cast<UAGX_RigidBodyComponent>(ComponentsOfInterest["FallDynamicBody"]);
	auto ConstrainedBody = Cast<UAGX_RigidBodyComponent>(ComponentsOfInterest["ConstrainedBody"]);
	auto SlideBody = Cast<UAGX_RigidBodyComponent>(ComponentsOfInterest["SlideBody"]);
	Test.TestTrue(
		"FallDynamicBody final z pos", FallDynamicBody->GetComponentLocation().Z < -100.0);
	Test.TestTrue(
		"ConstrainedBody final z pos", ConstrainedBody->GetComponentLocation().Z < -100.0);
	Test.TestTrue("SlideBody final z pos", SlideBody->GetComponentLocation().Z < -100.0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlaygroundTest, "AGXUnreal.Game.AGX_PlayInEditorTest.Playground",
	AgxAutomationCommon::ETF_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPlaygroundTest::RunTest(const FString& Parameters)
{
	using namespace AGX_PlayInEditorTest_helpers;
	FString MapPath = FString("/Game/Tests/Playground/Playground");

	ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(MapPath))
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(true));

	ComponentMap ComponentsOfInterest;
	double SimTimeMax = 5.0;
	ADD_LATENT_AUTOMATION_COMMAND(
		FCheckPlaygroundStateCommand(SimTimeMax, ComponentsOfInterest, *this));

	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand);
	return true;
}

//
// StepExampleLevels starts here.
//

/**
 * A complex automation test lets us define GetTests() from where we can append to
 * OutBeautifiedNames and OutTestCommands. For each entry in OutBeautifiedNames, a test with that
 * name will be run, where RunTest is called and the parameter passed to RunTest is the
 * corresponding element in OutTestCommands.
 */
IMPLEMENT_COMPLEX_AUTOMATION_TEST(
	FStepExampleLevelsTest, "AGXUnreal.Game.AGX_PlayInEditorTest.StepExampleLevels",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Collect all levels inside Content/Levels.
void FStepExampleLevelsTest::GetTests(
	TArray<FString>& OutBeautifiedNames, TArray<FString>& OutTestCommands) const
{
	// ComponentGallery ignored because it produces several errors on Play: for example Constraints
	// without a Body.
	TArray<FString> IgnoreLevels {"ComponentGallery"};
	if (!FSensorEnvironmentBarrier::IsRaytraceSupported())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Skipping StepExampleLevels.Lidar test because Raytracing is not supported on "
				 "this computer."));
		IgnoreLevels.Add("Lidar");
	}

	const FString LevelsDir = FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Levels"));
	TArray<FString> FoundAssetes;
	IFileManager::Get().FindFiles(FoundAssetes, *LevelsDir, TEXT("umap"));

	for (const FString& LevelFullPath : FoundAssetes)
	{
		const FString LevelName = FPaths::GetBaseFilename(LevelFullPath);
		if (IgnoreLevels.Contains(LevelName))
			continue;

		OutBeautifiedNames.Add(LevelName);
		OutTestCommands.Add(FString::Printf(TEXT("/Game/Levels/%s"), *LevelName));
	}
}

bool FStepExampleLevelsTest::RunTest(const FString& Parameters)
{
	AgxAutomationCommon::AddExpectedError(
		*this, TEXT("Could not allocate resource for Landscape Displacement Map for AGX Terrain "
					".*. There may be rendering issues."));
	using namespace AGX_PlayInEditorTest_helpers;
	const FString LevelPath = Parameters;
	ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(LevelPath))
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(true));

	int TickCurrent = 0;
	int TickMax = 3;
	ADD_LATENT_AUTOMATION_COMMAND(FTickOnlyCommand(TickCurrent, TickMax));

	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand);

	return true;
}

//
// Material Library test starts here.
//

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FCheckMaterialLibraryStateCommand, FAutomationTestBase&, Test);

bool FCheckMaterialLibraryStateCommand::Update()
{
	using namespace AGX_PlayInEditorTest_helpers;

	static int32 NumTicks = 0;
	NumTicks++;
	if (NumTicks > 1000)
	{
		Test.AddError("Level never began play even after many attempts.");
		return true;
	}

	if (!GEditor->IsPlayingSessionInEditor())
	{
		return false;
	}

	UWorld* TestWorld = GEditor->GetPIEWorldContext()->World();
	auto Box = GetComponentByName<UAGX_BoxShapeComponent>(TestWorld, "Actor", "BoxShape");
	Test.TestNotNull("Box", Box);

	auto Sphere = GetComponentByName<UAGX_SphereShapeComponent>(TestWorld, "Actor", "SphereShape");
	Test.TestNotNull("Sphere", Sphere);

	auto CMRegistrar = GetComponentByName<UAGX_ContactMaterialRegistrarComponent>(
		TestWorld, "Actor", "CMRegistrar");
	Test.TestNotNull("CMRegistrar", CMRegistrar);

	if (Box == nullptr || Sphere == nullptr || CMRegistrar == nullptr)
		return true;

	// Sanity check: ensure the materials used from the Material Library are still assigned.
	Test.TestTrue("Aluminium Library Shape Material not null", Box->ShapeMaterial != nullptr);
	Test.TestTrue("Steel Library Shape Material not null", Sphere->ShapeMaterial != nullptr);
	Test.TestTrue(
		"Steel-Aluminium Library Contact Material not null and assigned material pair",
		CMRegistrar->ContactMaterials.Num() == 1 && CMRegistrar->ContactMaterials[0] != nullptr &&
			CMRegistrar->ContactMaterials[0]->Material1 != nullptr &&
			CMRegistrar->ContactMaterials[0]->Material2 != nullptr);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMaterialLibraryTest, "AGXUnreal.Game.AGX_PlayInEditorTest.MaterialLibrary",
	AgxAutomationCommon::ETF_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMaterialLibraryTest::RunTest(const FString& Parameters)
{
	using namespace AGX_PlayInEditorTest_helpers;
	FString MapPath = FString("/Game/Tests/Test_MaterialLibrary");

	ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(MapPath));
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(true));
	ADD_LATENT_AUTOMATION_COMMAND(FCheckMaterialLibraryStateCommand(*this));

	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand);
	return true;
}

//
// ROS2 test starts here.
//

DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(
	FCheckROS2MovedCommand, double, SimTimeMax, ComponentMap, ComponentsOfInterest,
	FAutomationTestBase&, Test);

bool FCheckROS2MovedCommand::Update()
{
	using namespace AGX_PlayInEditorTest_helpers;
	using namespace AgxAutomationCommon;

	if (!GEditor->IsPlayingSessionInEditor())
	{
		return false;
	}

	static int32 NumTicks = 0;
	UWorld* TestWorld = GEditor->GetPIEWorldContext()->World();
	if (ComponentsOfInterest.Num() == 0)
	{
		NumTicks = 0;
		ActorMap Actors = GetActorsByName(TestWorld, {"BP_ROS2"});

		Test.TestTrue("Found actor of interest", Actors.Contains("BP_ROS2"));
		if (!Actors.Contains("BP_ROS2"))
		{
			return true;
		}

		auto CubeComplexMsg =
			GetComponentByName<UStaticMeshComponent>(*Actors["BP_ROS2"], "Cube_ComplexMessage");
		Test.TestNotNull("CubeComplexMsg", CubeComplexMsg);
		if (CubeComplexMsg == nullptr)
			return true;

		auto CubeAnyMsg =
			GetComponentByName<UStaticMeshComponent>(*Actors["BP_ROS2"], "Cube_AnyMessage");
		Test.TestNotNull("CubeAnyMsg", CubeAnyMsg);
		if (CubeAnyMsg == nullptr)
			return true;

		ComponentsOfInterest.Add("CubeComplexMsg", CubeComplexMsg);
		ComponentsOfInterest.Add("CubeAnyMsg", CubeAnyMsg);
		Test.TestTrue(
			"CubeComplexMsg initial x pos", CubeComplexMsg->GetComponentLocation().X < 1.0);
		Test.TestTrue("CubeAnyMsg initial x pos", CubeAnyMsg->GetComponentLocation().X < 1.0);
	}

	UAGX_Simulation* Sim = UAGX_Simulation::GetFrom(TestWorld);
	Test.TestNotNull("Simulation", Sim);
	if (Sim == nullptr)
		return true;

	const auto SimTime = Sim->GetTimeStamp();
	{
		// Sanity check to avoid hanging forever if the Simulation is not ticking.
		NumTicks++;
		if (NumTicks > 1000 && FMath::IsNearlyZero(SimTime))
		{
			Test.AddError(FString::Printf(
				TEXT("SimTime too small: %f. The Simulation has not stepped as expected."),
				SimTime));
			return true;
		}
	}

	if (SimTime < SimTimeMax)
	{
		return false; // Continue ticking..
	}

	// At this point we have ticked to TickMax. In this test, the Cubes will be moved in +x >100cm
	// if the tests succeeded.
	auto CubeComplexMsg = Cast<UStaticMeshComponent>(ComponentsOfInterest["CubeComplexMsg"]);
	auto CubeAnyMsg = Cast<UStaticMeshComponent>(ComponentsOfInterest["CubeAnyMsg"]);
	Test.TestTrue("CubeComplexMsg final x pos", CubeComplexMsg->GetComponentLocation().X > 100.0);
	Test.TestTrue("CubeAnyMsg final x pos", CubeAnyMsg->GetComponentLocation().X > 100.0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FROS2Test, "AGXUnreal.Game.AGX_PlayInEditorTest.ROS2",
	AgxAutomationCommon::ETF_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FROS2Test::RunTest(const FString& Parameters)
{
	using namespace AGX_PlayInEditorTest_helpers;
	FString MapPath = FString("/Game/Tests/Test_ROS2");

	ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(MapPath))
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(true));

	ComponentMap ComponentsOfInterest;
	double SimTimeMax = 5.0;
	ADD_LATENT_AUTOMATION_COMMAND(FCheckROS2MovedCommand(SimTimeMax, ComponentsOfInterest, *this));

	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand);
	return true;
}

//
// Lidar test starts here.
//

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(
	FCheckLidarSteppedCommand, double, SimTimeMax, FAutomationTestBase&, Test);

bool FCheckLidarSteppedCommand::Update()
{
	if (!GEditor->IsPlayingSessionInEditor())
	{
		return false;
	}

	static int32 NumTicks = 0;
	UWorld* TestWorld = GEditor->GetPIEWorldContext()->World();
	UAGX_Simulation* Sim = UAGX_Simulation::GetFrom(TestWorld);
	Test.TestNotNull("Simulation", Sim);
	if (Sim == nullptr)
		return true;

	const auto SimTime = Sim->GetTimeStamp();
	{
		// Sanity check to avoid hanging forever if the Simulation is not ticking.
		NumTicks++;
		if (NumTicks > 1000 && FMath::IsNearlyZero(SimTime))
		{
			Test.AddError(FString::Printf(
				TEXT("SimTime too small: %f. The Simulation has not stepped as expected."),
				SimTime));
			return true;
		}
	}

	if (SimTime < SimTimeMax)
	{
		return false; // Continue ticking..
	}

	// At this point we have ticked to TickMax.
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLidarTest, "AGXUnreal.Game.AGX_PlayInEditorTest.Lidar",
	AgxAutomationCommon::ETF_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FLidarTest::RunTest(const FString& Parameters)
{
	if (!FSensorEnvironmentBarrier::IsRaytraceSupported())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Skipping AGX_PlayInEditorTest.Lidar test because Raytracing is not supported on "
				 "this computer."));
		return true;
	}
	using namespace AGX_PlayInEditorTest_helpers;
	FString MapPath = FString("/Game/Tests/Test_Lidar");

	ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(MapPath))
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(true));

	double SimTimeMax = 5.0;
	ADD_LATENT_AUTOMATION_COMMAND(FCheckLidarSteppedCommand(SimTimeMax, *this));

	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand);
	return true;
}
