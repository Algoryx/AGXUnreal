// Copyright 2024, Algoryx Simulation AB.

#if WITH_DEV_AUTOMATION_TESTS

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_Simulation.h"
#include "AgxAutomationCommon.h"
#include "CollisionGroups/AGX_CollisionGroupDisablerComponent.h"
#include "Constraints/AGX_ConstraintComponent.h"
#include "Import/AGX_ImporterToEditor.h"
#include "Import/AGX_ImportSettings.h"
#include "Materials/AGX_ContactMaterial.h"
#include "Materials/AGX_ContactMaterialRegistrarComponent.h"
#include "Materials/AGX_ShapeMaterial.h"
#include "Shapes/AGX_SphereShapeComponent.h"
#include "Shapes/AGX_BoxShapeComponent.h"
#include "Shapes/AGX_CapsuleShapeComponent.h"
#include "Shapes/AGX_CylinderShapeComponent.h"
#include "Shapes/AGX_TrimeshShapeComponent.h"
#include "Terrain/AGX_ShovelComponent.h"
#include "Terrain/AGX_ShovelProperties.h"
#include "Utilities/AGX_BlueprintUtilities.h"
#include "Utilities/AGX_EditorUtilities.h"
#include "Utilities/AGX_ImportUtilities.h"
#include "Vehicle/AGX_TrackComponent.h"
#include "Vehicle/AGX_TrackInternalMergeProperties.h"
#include "Vehicle/AGX_TrackProperties.h"
#include "Vehicle/AGX_TrackWheel.h"
#include "Wire/AGX_WireComponent.h"

// Unreal Engine includes.
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "HAL/FileManager.h"
#include "MaterialTypes.h"
#include "Materials/MaterialInterface.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"

/*
 * This file contains a set of tests for AGX_ImporterToBlueprint, which imports an AGX
 * Dynamics archive into the current world as a Blueprint that contains ActorComponents for each
 * imported object.
 *
 * Search for "test starts here." within this file to find the start of each test case.
 */

/**
 * Latent Command that imports an AGX Dynamics archive into a Blueprint. A pointer to the Blueprint
 * created to hold the imported objects is stored in the Contents parameter.
 * @param ArchiveName The AGX Dynamics archive to import.
 * @param Contents Pointer set to point to the Blueprint containing the imported objects.
 * @param Test The Automation test that contains this Latent Command.
 */
DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(
	FImportArchiveBlueprintCommand, FString, ArchiveName, UBlueprint*&, Contents,
	FAutomationTestBase&, Test);

bool FImportArchiveBlueprintCommand::Update()
{
	if (ArchiveName.IsEmpty())
	{
		Test.AddError(TEXT("FImportArchiveBlueprintCommand not given an archive to import."));
		return true;
	}
	FString ArchiveFilePath = AgxAutomationCommon::GetTestScenePath(ArchiveName);
	if (ArchiveFilePath.IsEmpty())
	{
		Test.AddError(FString::Printf(TEXT("Did not find an archive named '%s'."), *ArchiveName));
		return true;
	}

	FAGX_ImportSettings Settings;
	Settings.FilePath = ArchiveFilePath;
	Settings.bIgnoreDisabledTrimeshes = false;
	Settings.ImportType = EAGX_ImportType::Agx;
	Settings.bOpenBlueprintEditorAfterImport = false;

	FAGX_ImporterToEditor Importer;
	UBlueprint* ChildBlueprint = Importer.Import(Settings);
	Contents = FAGX_BlueprintUtilities::GetOutermostParent(ChildBlueprint);
	Test.TestNotNull(TEXT("Contents"), Contents);
	return true;
}

/**
 * Latent Command that imports an URDF model into a Blueprint. A pointer to the Blueprint
 * created to hold the imported objects is stored in the Contents parameter.
 * @param FileName The URDF file to import.
 * @param Contents Pointer set to point to the Blueprint containing the imported objects.
 * @param Test The Automation test that contains this Latent Command.
 */
DEFINE_LATENT_AUTOMATION_COMMAND_FOUR_PARAMETER(
	FImportURDFBlueprintCommand, FString, FileName, FString, PackagePath, UBlueprint*&, Contents,
	FAutomationTestBase&, Test);

bool FImportURDFBlueprintCommand::Update()
{
	if (FileName.IsEmpty())
	{
		Test.AddError(TEXT("FImportURDFBlueprintCommand not given a file to import."));
		return true;
	}
	FString UrdfFilePath = AgxAutomationCommon::GetTestScenePath(FileName);
	if (UrdfFilePath.IsEmpty())
	{
		Test.AddError(FString::Printf(TEXT("Did not find an URDF file named '%s'."), *FileName));
		return true;
	}

	FAGX_ImportSettings Settings;
	Settings.FilePath = UrdfFilePath;
	Settings.UrdfPackagePath = PackagePath;
	Settings.bIgnoreDisabledTrimeshes = false;
	Settings.ImportType = EAGX_ImportType::Urdf;
	Settings.bOpenBlueprintEditorAfterImport = false;

	FAGX_ImporterToEditor Importer;

	UBlueprint* ChildBlueprint = Importer.Import(Settings);
	Contents = FAGX_BlueprintUtilities::GetOutermostParent(ChildBlueprint);
	Test.TestNotNull(TEXT("Contents"), Contents);
	return true;
}

//
// EmptyScene test starts here.
//

/**
 * Latent Command testing that the empty scene was imported correctly.
 * @param Contents The Blueprint that was created by the archive importer to hold the imported
 * objects.
 * @param Test The Automation test that contains this Latent Command.
 */
DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(
	FCheckEmptySceneImportedCommand, UBlueprint*&, Contents, FAutomationTestBase&, Test);

bool FCheckEmptySceneImportedCommand::Update()
{
	using namespace AgxAutomationCommon;
	UWorld* World = GetTestWorld();
	if (World == nullptr || Contents == nullptr)
	{
		return true;
	}

	// The Blueprint's only component should be the root component.
	TArray<UActorComponent*> Components = FAGX_BlueprintUtilities::GetTemplateComponents(Contents);
	Test.TestEqual(TEXT("Number of imported components"), Components.Num(), 1);
	USceneComponent* SceneRoot = AgxAutomationCommon::GetByName<USceneComponent>(
		Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("DefaultSceneRoot"));
	Test.TestNotNull(TEXT("DefaultSceneRoot"), SceneRoot);

	// The Blueprint should have been created in the test world.
	Test.TestEqual(TEXT("The Blueprint should be in the test world."), Contents->GetWorld(), World);
	return true;
}

/**
 * Latent Command that removes everything that was created by the Import Empty Scene test. Actual
 * removal isn't done immediately by Unreal Engine so the first call to Update will return false
 * so that the removal is completed before the next Latent Command starts.
 */
DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FClearEmptySceneImportedCommand, UBlueprint*&, Contents);
bool FClearEmptySceneImportedCommand::Update()
{
	if (Contents == nullptr)
	{
		// The removal happened the previous tick so it's safe to return true and complete this
		// Latent Command now.
		return true;
	}

	UWorld* World = AgxAutomationCommon::GetTestWorld();
	if (World == nullptr || Contents == nullptr)
	{
		return true;
	}

	Contents = nullptr;

	// Return false so the engine get a tick to do the actual removal.
	return false;
}

/**
 * Test that an empty AGX Dynamics archive can be imported, that the archive Blueprint root is
 * created as it should, and that it is added to the world.
 */
class FImporterToBlueprint_EmptySceneTest final : public AgxAutomationCommon::FAgxAutomationTest
{
public:
	FImporterToBlueprint_EmptySceneTest()
		: AgxAutomationCommon::FAgxAutomationTest(
			  TEXT("FImporterToBlueprint_EmptySceneTest"),
			  TEXT("AGXUnreal.Game.ImporterToBlueprint.EmptyScene"))
	{
	}

protected:
	bool RunTest(const FString& Parameters) override
	{
		BAIL_TEST_IF_NO_WORLD(false)
		BAIL_TEST_IF_WORLDS_MISMATCH(false)

		/// @todo I would like to load a fresh map before doing the actual test, which it would seem
		/// one does with FLoadGameMapCommand and FWaitForMapToLoadCommand, but including them
		/// causes the world to stop ticking. Figure out why. For now I just hope that loading the
		/// map as a unit test launch command line parameter is good enough. Not sure how multiple
		/// import tests interact though. Some form of level cleanup Latent Command at the end of
		/// each test may be required. I really hope multiple tests don't run concurrently in the
		/// same world.
		///
		/// Some news on the above. Map loading seems to work if commands are done in the following
		/// order:
		///
		/// ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(EmptyMapPath))
		/// ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(true));
		/// ADD_LATENT_AUTOMATION_COMMAND(AgxAutomationCommon::FWaitUntilPIEUpCommand);
		///
		/// After these you can queue commands that spawn Actors and create Components in the Play
		/// In Editor session world, which you can get with
		///
		/// UWorld* World = GEditor->GetPIEWorldContext()->World();
		///
		/// To simulate until a specified time use
		///
		/// ADD_LATENT_AUTOMATION_COMMAND(FTickUntilTimeStamp(1.0));
		///
		/// Cleanup after the test with
		///
		/// ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand);
		/// ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(EmptyMapPath));
		///
		/// Note that here we load an empty map, then launch a Play In Editor session, then create
		/// our objects, then simulate a bit, then do our testing, then restore a known initial
		/// state. If you have an already populated map, e.g. Test_MyTestMap, stored as an asset
		/// then you can pass /Game/Tests/Test_MyTestMap instead of EmptyMapPath to FEditorLoadMap
		/// and don't need to create any objects in code.
#if 0
		// ADD_LATENT_AUTOMATION_COMMAND(FLoadGameMapCommand(TEXT("Test_ArchiveImport")));
		// ADD_LATENT_AUTOMATION_COMMAND(FWaitForMapToLoadCommand());
#endif

		ADD_LATENT_AUTOMATION_COMMAND(
			FImportArchiveBlueprintCommand("empty_scene.agx", Contents, *this));
		ADD_LATENT_AUTOMATION_COMMAND(FCheckEmptySceneImportedCommand(Contents, *this));
		ADD_LATENT_AUTOMATION_COMMAND(FClearEmptySceneImportedCommand(Contents));
		ADD_LATENT_AUTOMATION_COMMAND(AgxAutomationCommon::FWaitNTicksCommand(1));

		return true;
	}

private:
	UBlueprint* Contents = nullptr;
};

namespace
{
	FImporterToBlueprint_EmptySceneTest ImporterToBlueprint_EmptySceneTest;
}

//
// SingleSphere test starts here.
//

class FImporterToBlueprint_SingleSphereTest;

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FCheckSingleSphereImportedCommand, FImporterToBlueprint_SingleSphereTest&, Test);

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FStoreInitialTimes, FImporterToBlueprint_SingleSphereTest&, Test);

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FStoreResultingTimes, FImporterToBlueprint_SingleSphereTest&, Test);

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FCheckSphereHasMoved, FImporterToBlueprint_SingleSphereTest&, Test);

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FClearSingleSphereImportedCommand, UBlueprint*&, Contents);

class FImporterToBlueprint_SingleSphereTest final : public AgxAutomationCommon::FAgxAutomationTest
{
public:
	FImporterToBlueprint_SingleSphereTest()
		: AgxAutomationCommon::FAgxAutomationTest(
			  TEXT("FImporterToBlueprint_SingleSphereTest"),
			  TEXT("AGXUnreal.Game.ImporterToBlueprint.SingleSphere"))
	{
	}

public:
	UWorld* World = nullptr;
	UAGX_Simulation* Simulation = nullptr;
	UBlueprint* Contents = nullptr;
	UAGX_RigidBodyComponent* SphereBody = nullptr;
	FVector StartPosition;
	FVector StartVelocity;
	float StartAgxTime = -1.0f;
	float StartUnrealTime = -1.0f;
	float EndUnrealTime = -1.0f;
	float EndAgxTime = -1.0f;
	int32 NumTicks = 0;

protected:
	bool RunTest(const FString& Parameters) override
	{
		using namespace AgxAutomationCommon;
		BAIL_TEST_IF_CANT_SIMULATE(false)
		World = AgxAutomationCommon::GetTestWorld();
		if (World == nullptr)
		{
			AddError(TEXT("Do not have a test world, cannot test SingleSphere import."));
		}
		Simulation = UAGX_Simulation::GetFrom(World);
		if (Simulation == nullptr)
		{
			AddError(TEXT("Do not have a simulation, cannot test SingleSphere import."));
		}

		// See comment in FImporterToBlueprint_EmptySceneTest.
		// In short, loading a map stops world ticking.
#if 0
		ADD_LATENT_AUTOMATION_COMMAND(FLoadGameMapCommand(TEXT("Test_ArchiveImport")))
		ADD_LATENT_AUTOMATION_COMMAND(FWaitForMapToLoadCommand())
#endif

		ADD_LATENT_AUTOMATION_COMMAND(
			FImportArchiveBlueprintCommand("single_sphere_build.agx", Contents, *this))
		ADD_LATENT_AUTOMATION_COMMAND(FCheckSingleSphereImportedCommand(*this))
		ADD_LATENT_AUTOMATION_COMMAND(FStoreInitialTimes(*this))
		ADD_LATENT_AUTOMATION_COMMAND(FWaitWorldDuration(World, 1.0f))
		ADD_LATENT_AUTOMATION_COMMAND(FStoreResultingTimes(*this))
		ADD_LATENT_AUTOMATION_COMMAND(FCheckSphereHasMoved(*this))
		ADD_LATENT_AUTOMATION_COMMAND(FClearSingleSphereImportedCommand(Contents))
		ADD_LATENT_AUTOMATION_COMMAND(FWaitNTicksCommand(1))

		return true;
	}
};

namespace
{
	FImporterToBlueprint_SingleSphereTest ImporterToBlueprint_SingleSphereTest;
}

/*
 * Check that the expected state has been created during import.
 *
 * The object structure and all numbers tested here should match what is being set in the source
 * script single_sphere.agxPy.
 */
bool FCheckSingleSphereImportedCommand::Update()
{
	using namespace AgxAutomationCommon;

	if (Test.World == nullptr || Test.Simulation == nullptr || Test.Contents == nullptr)
	{
		return true;
	}

	// Get all the imported components.
	TArray<UActorComponent*> Components =
		FAGX_BlueprintUtilities::GetTemplateComponents(Test.Contents);
	Test.TestEqual(TEXT("Number of imported components"), Components.Num(), 3);

	// Get the components we know should be there.
	USceneComponent* SceneRoot = GetByName<USceneComponent>(
		Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("DefaultSceneRoot"));
	UAGX_RigidBodyComponent* SphereBody = GetByName<UAGX_RigidBodyComponent>(
		Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("SphereBody"));
	UAGX_SphereShapeComponent* SphereShape = GetByName<UAGX_SphereShapeComponent>(
		Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("SphereGeometry"));

	// Make sure we got the components we know should be there.
	Test.TestNotNull(TEXT("DefaultSceneRoot"), SceneRoot);
	Test.TestNotNull(TEXT("SphereBody"), SphereBody);
	Test.TestNotNull(TEXT("SphereShape"), SphereShape);

	if (SphereBody == nullptr || SphereShape == nullptr)
	{
		Test.AddError("No sphere body found in the level, cannot continue.");
		return true;
	}

	// Name.
	{
		Test.TestEqual("Sphere name", SphereBody->GetFName(), FName(TEXT("SphereBody")));
	}

	// Position.
	{
		FVector Actual = FAGX_BlueprintUtilities::GetTemplateComponentWorldLocation(SphereBody);
		// The position, in AGX Dynamics' units, that was given to the sphere when created.
		FVector ExpectedAgx(
			1.00000000000000000000e+01f, 2.00000000000000000000e+01f, 3.00000000000000000000e+01f);
		FVector Expected = AgxToUnrealDisplacement(ExpectedAgx);
		Test.TestEqual(TEXT("Sphere position"), Actual, Expected);
	}

	// Rotation.
	{
		FRotator Actual = FAGX_BlueprintUtilities::GetTemplateComponentWorldRotation(SphereBody);
		// The rotation, in AGX Dynamics' units, that was given to the sphere when created.
		FVector ExpectedAgx(
			1.01770284974289526581e+00f, -2.65482457436691521302e-01f,
			-1.54866776461627897454e+00f);
		FRotator Expected = AgxToUnrealEulerAngles(ExpectedAgx);
		TestEqual(Test, TEXT("Sphere rotation"), Actual, Expected);
	}

	// Velocity.
	{
		FVector Actual = SphereBody->Velocity;
		// The velocity, in AGX Dynamics' units, that was given to the sphere when created.
		FVector ExpectedAgx(
			1.00000000000000000000e+00f, 2.00000000000000000000e+00f, 3.00000000000000000000e+00f);
		FVector Expected = AgxToUnrealDisplacement(ExpectedAgx);
		Test.TestEqual(TEXT("Sphere linear velocity"), Actual, Expected);
	}

	// Angular velocity.
	{
		FVector Actual = SphereBody->AngularVelocity;
		// The angular velocity, in AGX Dynamics' units, that was given to the sphere when created.
		FVector ExpectedAgx(
			1.10000000000000000000e+01f, 1.20000000000000000000e+01f, 1.30000000000000000000e+01f);
		FVector Expected = AgxToUnrealAngularVelocity(ExpectedAgx);
		Test.TestEqual(TEXT("Sphere angular velocity"), Actual, Expected);
	}

	// Mass.
	{
		Test.TestEqual(TEXT("Sphere mass"), SphereBody->Mass, 5.00000000000000000000e+02f);
	}

	// Inertia tensor diagonal.
	{
		FVector Actual = SphereBody->GetPrincipalInertia();
		FVector Expected(
			1.00000000000000000000e+02f, 2.00000000000000000000e+02f, 3.00000000000000000000e+02f);
		Test.TestEqual(TEXT("Sphere inertia tensor diagonal"), Actual, Expected);
	}

	// Motion control.
	{
		EAGX_MotionControl Actual = SphereBody->MotionControl;
		EAGX_MotionControl Expected = EAGX_MotionControl::MC_DYNAMICS;
		Test.TestEqual(TEXT("Sphere motion control"), Actual, Expected);
	}

	// Transform root component.
	{
		Test.TestEqual(
			TEXT("Sphere transform target"), SphereBody->TransformTarget,
			EAGX_TransformTarget::TT_SELF);
	}

	// Radius.
	{
		float Actual = SphereShape->Radius;
		float ExpectedAgx = 5.00000000000000000000e-01f;
		float Expected = AgxToUnrealDistance(ExpectedAgx);
		Test.TestEqual(TEXT("Sphere radius"), Actual, Expected);
	}

	// Imported objects don't get a native AGX Dynamics representation immediately when imported
	// into Unreal Editor but this unit test is run in Game mode which means that BeginPlay is
	// called on an Actor as soon as it is created, and Actors which have had BeginPlay called will
	// call BeginPlay on any registered Component, with UActorComponent::RegisterComponent,
	// immediately. Which creates the AGX Dynamics native object.
	Test.TestTrue(TEXT("Sphere has native"), SphereBody->HasNative());

	// The body should have been created in the test world.
	Test.TestEqual(TEXT("Sphere world"), SphereBody->GetWorld(), Test.World);

	// Publish the important bits to the rest of the test.
	Test.SphereBody = SphereBody;
	Test.StartPosition = FAGX_BlueprintUtilities::GetTemplateComponentWorldLocation(SphereBody);
	Test.StartVelocity = SphereBody->Velocity;

	return true;
}

bool FStoreInitialTimes::Update()
{
	Test.StartUnrealTime = Test.World->GetTimeSeconds();
	Test.StartAgxTime = Test.StartUnrealTime;
	UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(Test.World);
	Simulation->SetTimeStamp(Test.StartUnrealTime);
	Simulation->StepMode = SmCatchUpImmediately;
	return true;
}

bool FStoreResultingTimes::Update()
{
	Test.EndUnrealTime = Test.World->GetTimeSeconds();
	Test.EndAgxTime = static_cast<float>(UAGX_Simulation::GetFrom(Test.World)->GetTimeStamp());
	return true;
}

bool FCheckSphereHasMoved::Update()
{
	using namespace AgxAutomationCommon;

	if (Test.SphereBody == nullptr)
	{
		return true;
	}

	FVector EndPosition =
		FAGX_BlueprintUtilities::GetTemplateComponentWorldLocation(Test.SphereBody);
	FVector EndVelocity = Test.SphereBody->Velocity;
	float Duration = Test.EndAgxTime - Test.StartAgxTime;

	// Velocity constant only for X and Y directions. Z has gravity.
	for (int32 I : {0, 1})
	{
		float ExpectedDisplacement = EndVelocity[I] * Duration;
		float ExpectedPosition = Test.StartPosition[I] + ExpectedDisplacement;
		float ActualPosition = EndPosition[I];
		Test.TestEqual(
			"Body should move according to velocity.", ActualPosition, ExpectedPosition,
			RelativeTolerance(ExpectedPosition, KINDA_SMALL_NUMBER));
	}

	// Position test for Z.
	{
		float StartVelocity = Test.StartVelocity.Z;
		float StartPosition = Test.StartPosition.Z;
		float Acceleration = UAGX_Simulation::GetFrom(Test.World)->UniformGravity.Z;
		// The familiar Xt = X0 + V0 * t + 1/2 * a * t^2.
		float ExpectedPosition =
			StartPosition + StartVelocity * Duration + 0.5f * Acceleration * Duration * Duration;
		float ActualPosition = EndPosition.Z;
		/// @todo Not sure why the relative tolerance must be so large here. Maybe gravity mismatch?
		/// There is also the SPOOK/leapfrog integration formulation that makes free-fall in gravity
		/// a bit less straight-forward. Perhaps that is enough to explain the difference.
		Test.TestEqual(
			"Velocity in the Z direction should be subject to gravity.", ActualPosition,
			ExpectedPosition, RelativeTolerance(ExpectedPosition, 0.003f));
	}

	return true;
}

bool FClearSingleSphereImportedCommand::Update()
{
	UWorld* World = AgxAutomationCommon::GetTestWorld();
	if (World == nullptr || Contents == nullptr)
	{
		return true;
	}
	Contents = nullptr;
	return true;
}

//
// MotionControl test starts here.
//

class FImporterToBlueprint_MotionControlTest;

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FCheckMotionControlImportedCommand, FImporterToBlueprint_MotionControlTest&, Test);

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FClearMotionControlImportedCommand, FImporterToBlueprint_MotionControlTest&, Test);

class FImporterToBlueprint_MotionControlTest final : public AgxAutomationCommon::FAgxAutomationTest
{
public:
	FImporterToBlueprint_MotionControlTest()
		: AgxAutomationCommon::FAgxAutomationTest(
			  TEXT("FImporterToBlueprint_MotionControlTest"),
			  TEXT("AGXUnreal.Editor.ImporterToBlueprint.MotionControl"))
	{
	}

public:
	UBlueprint* Contents = nullptr;

protected:
	virtual bool RunTest(const FString&) override
	{
		ADD_LATENT_AUTOMATION_COMMAND(
			FImportArchiveBlueprintCommand("motion_control_build.agx", Contents, *this))
		ADD_LATENT_AUTOMATION_COMMAND(FCheckMotionControlImportedCommand(*this))
		ADD_LATENT_AUTOMATION_COMMAND(FClearMotionControlImportedCommand(*this))
		return true;
	}
};

namespace
{
	FImporterToBlueprint_MotionControlTest FImporterToBlueprint_MotionControlTest;
}

bool FCheckMotionControlImportedCommand::Update()
{
	using namespace AgxAutomationCommon;
	if (Test.Contents == nullptr)
	{
		Test.AddError(TEXT("Could not import MotionControl test scene: No content created."));
		return true;
	}

	// Get all the imported components.
	TArray<UActorComponent*> Components =
		FAGX_BlueprintUtilities::GetTemplateComponents(Test.Contents);
	Test.TestEqual(TEXT("Number of imported components"), Components.Num(), 8);

	// Get the components we know should be there.
	USceneComponent* SceneRoot = GetByName<USceneComponent>(
		Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("DefaultSceneRoot"));
	UAGX_RigidBodyComponent* StaticBody = GetByName<UAGX_RigidBodyComponent>(
		Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("StaticBody"));
	UAGX_SphereShapeComponent* StaticShape = GetByName<UAGX_SphereShapeComponent>(
		Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("StaticShape"));
	UAGX_RigidBodyComponent* KinematicsBody = GetByName<UAGX_RigidBodyComponent>(
		Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("KinematicBody"));
	UAGX_SphereShapeComponent* KinematicsShape = GetByName<UAGX_SphereShapeComponent>(
		Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("KinematicShape"));
	UAGX_RigidBodyComponent* DynamicsBody = GetByName<UAGX_RigidBodyComponent>(
		Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("DynamicBody"));
	UAGX_SphereShapeComponent* DynamicsShape = GetByName<UAGX_SphereShapeComponent>(
		Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("DynamicShape"));

	// Make sure we got the components we know should be there.
	Test.TestNotNull(TEXT("DefaultSceneRoot"), SceneRoot);
	Test.TestNotNull(TEXT("StaticBody"), StaticBody);
	Test.TestNotNull(TEXT("StaticShape"), StaticShape);
	Test.TestNotNull(TEXT("KinematicBody"), KinematicsBody);
	Test.TestNotNull(TEXT("KinematicShape"), KinematicsShape);
	Test.TestNotNull(TEXT("DynamicBody"), DynamicsBody);
	Test.TestNotNull(TEXT("DynamicShape"), DynamicsShape);
	if (SceneRoot == nullptr || StaticBody == nullptr || StaticShape == nullptr ||
		KinematicsBody == nullptr || KinematicsShape == nullptr || DynamicsBody == nullptr ||
		DynamicsShape == nullptr)
	{
		Test.AddError(
			"A required component wasn't found in the imported Blueprint. Cannot continue");
		return true;
	}

	Test.TestEqual(
		TEXT("Static body motion control"), StaticBody->MotionControl,
		EAGX_MotionControl::MC_STATIC);
	Test.TestEqual(
		TEXT("Kinematic body motion control"), KinematicsBody->MotionControl,
		EAGX_MotionControl::MC_KINEMATICS);
	Test.TestEqual(
		TEXT("Dynamic body motion control"), DynamicsBody->MotionControl,
		EAGX_MotionControl::MC_DYNAMICS);

#if 0
	// We would like this to be Static, but setting it in UAGX_RigidBodyComponent::CopyFrom during
	// import of an AGX Dynamics archive causes the move widget in Unreal Editor to break. Static
	// bodies doesn't follow the Actor's RootComponent. For now we set Mobility to Movable even for
	// static bodies. This comes with a rendering performance cost since baked lighting can't be
	// used anymore, and possibly other reasons as well. I'm guessing that we're supposed to do
	// something when changing Mobility to make it update everywhere, or perhaps we're setting it
	// at the wrong time. Not sure, more investigation needed.
	Test.TestEqual(TEXT("Static body mobility"), StaticBody->Mobility, EComponentMobility::Static);
#else
	Test.TestEqual(TEXT("Static body mobility"), StaticBody->Mobility, EComponentMobility::Movable);
#endif
	Test.TestEqual(
		TEXT("Kinematic body mobility"), KinematicsBody->Mobility, EComponentMobility::Movable);
	Test.TestEqual(
		TEXT("Dynamic body mobility"), DynamicsBody->Mobility, EComponentMobility::Movable);

	return true;
}

bool FClearMotionControlImportedCommand::Update()
{
	if (Test.Contents == nullptr)
	{
		return true;
	}

#if defined(__linux__)
	/// @todo Workaround for internal issue #213.
	Test.AddExpectedError(
		TEXT("inotify_rm_watch cannot remove descriptor"), EAutomationExpectedErrorFlags::Contains,
		0);
	Test.AddError(TEXT("inotify_rm_watch cannot remove descriptor"));
#endif

	TArray<const TCHAR*> ExpectedFiles {TEXT("Blueprint"), TEXT("BP_motion_control_build.uasset")};

	const FString BaseBlueprintName = Test.Contents->GetName() + FString(".uasset");
	ExpectedFiles.Add(*BaseBlueprintName);

	AgxAutomationCommon::DeleteImportDirectory(TEXT("motion_control_build"), ExpectedFiles);

	return true;
}

//
// SimpleTrimesh test starts here.
//

class FImporterToBlueprint_SimpleTrimeshTest;

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FCheckSimpleTrimeshImportedCommand, FImporterToBlueprint_SimpleTrimeshTest&, Test);

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FClearSimpleTrimeshImportedCommand, FImporterToBlueprint_SimpleTrimeshTest&, Test);

class FImporterToBlueprint_SimpleTrimeshTest final : public AgxAutomationCommon::FAgxAutomationTest
{
public:
	FImporterToBlueprint_SimpleTrimeshTest()
		: AgxAutomationCommon::FAgxAutomationTest(
			  TEXT("FImporterToBlueprint_SimpleTrimeshTest"),
			  TEXT("AGXUnreal.Game.ImporterToBlueprint.SimpleTrimesh"))
	{
	}

public:
	UWorld* World = nullptr;
	UAGX_Simulation* Simulation = nullptr;
	UBlueprint* Contents = nullptr;
	UAGX_RigidBodyComponent* TrimeshBody = nullptr;

protected:
	virtual bool RunTest(const FString&) override
	{
		BAIL_TEST_IF_NO_AGX(false)
		BAIL_TEST_IF_NO_WORLD(false)
		BAIL_TEST_IF_WORLDS_MISMATCH(false)
		World = AgxAutomationCommon::GetTestWorld();
		Simulation = UAGX_Simulation::GetFrom(World);

		// See comment in FImporterToBlueprint_EmptySceneTest.
		// In short, loading a map stops world ticking.
#if 0
		ADD_LATENT_AUTOMATION_COMMAND(FLoadGameMapCommand(TEXT("Test_ArchiveImport")))
		ADD_LATENT_AUTOMATION_COMMAND(FWaitForMapToLoadCommand())
#endif

		ADD_LATENT_AUTOMATION_COMMAND(
			FImportArchiveBlueprintCommand("simple_trimesh_build.agx", Contents, *this))
		ADD_LATENT_AUTOMATION_COMMAND(FCheckSimpleTrimeshImportedCommand(*this))
		ADD_LATENT_AUTOMATION_COMMAND(FClearSimpleTrimeshImportedCommand(*this))
		return true;
	}
};

namespace
{
	FImporterToBlueprint_SimpleTrimeshTest ImporterToBlueprint_SimpleTrimeshTest;
}

/**
 * Check that the expected state was created during import.
 *
 * The object structure and all numbers tested here should match what is being set in the source
 * script simple_trimesh.agxPy.
 * @return true when the check is complete. Never returns false.
 */
bool FCheckSimpleTrimeshImportedCommand::Update()
{
	using namespace AgxAutomationCommon;
	if (Test.Contents == nullptr)
	{
		Test.AddError(TEXT("Could not import SimpleTrimesh test scene: No content created."));
		return true;
	}

	// Get all the imported components.
	TArray<UActorComponent*> Components =
		FAGX_BlueprintUtilities::GetTemplateComponents(Test.Contents);
	Test.TestEqual(TEXT("Number of imported components"), Components.Num(), 5);

	// Get the components we know should be there.
	USceneComponent* SceneRoot = GetByName<USceneComponent>(
		Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("DefaultSceneRoot"));
	UAGX_RigidBodyComponent* TrimeshBody = GetByName<UAGX_RigidBodyComponent>(
		Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("TrimeshBody"));
	UAGX_TrimeshShapeComponent* TrimeshShape = GetByName<UAGX_TrimeshShapeComponent>(
		Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("TrimeshGeometry"));
	UStaticMeshComponent* StaticMesh = GetByName<UStaticMeshComponent>(
		Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("simple_trimesh"));

	// Make sure we got the components we know should be there.
	Test.TestNotNull(TEXT("DefaultSceneRoot"), SceneRoot);
	Test.TestNotNull(TEXT("TrimeshBody"), TrimeshBody);
	Test.TestNotNull(TEXT("TrimeshShape"), TrimeshShape);
	Test.TestNotNull(TEXT("StaticMesh"), StaticMesh);
	if (TrimeshBody == nullptr || TrimeshShape == nullptr || StaticMesh == nullptr)
	{
		Test.AddError(
			"A required component wasn't found in the imported Blueprint. Cannot continue.");
		return true;
	}

	const TArray<USceneComponent*>& Children = TrimeshShape->GetAttachChildren();
	Test.TestEqual(TEXT("TrimeshShape child components"), Children.Num(), 1);
	if (Children.Num() != 1)
	{
		return true;
	}
	USceneComponent* Child = Children[0];
	Test.TestNotNull(TEXT("Child"), Child);
	if (Child == nullptr)
	{
		return true;
	}
	UStaticMeshComponent* Mesh = Cast<UStaticMeshComponent>(Child);
	Test.TestNotNull(TEXT("Trimesh asset"), Mesh);
	if (Mesh == nullptr)
	{
		return true;
	}
	Test.TestEqual(TEXT("The StaticMesh should be a child of the TrimeshShape"), Mesh, StaticMesh);

	return true;
}

/**
 * Remove everything created by the archive import.
 * @return true when the clearing is complete. Never returns false.
 */
bool FClearSimpleTrimeshImportedCommand::Update()
{
	if (Test.World == nullptr || Test.Contents == nullptr)
	{
		return true;
	}

#if defined(__linux__)
	/// @todo Workaround for internal issue #213.
	Test.AddExpectedError(
		TEXT("inotify_rm_watch cannot remove descriptor"), EAutomationExpectedErrorFlags::Contains,
		0);
	Test.AddError(TEXT("inotify_rm_watch cannot remove descriptor"));
#endif

	TArray<const TCHAR*> ExpectedFiles = {TEXT("StaticMesh"), TEXT("simple_trimesh.uasset")};

	const FString BaseBlueprintName = Test.Contents->GetName() + FString(".uasset");
	ExpectedFiles.Add(*BaseBlueprintName);

	AgxAutomationCommon::DeleteImportDirectory(TEXT("simple_trimesh_build"), ExpectedFiles);

	return true;
}

//
// RenderMaterial test starts here.
//

class FImporterToBlueprint_RenderMaterialTest;

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FCheckRenderMaterialImportedCommand, FImporterToBlueprint_RenderMaterialTest&, Test);

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FClearRenderMaterialImportedCommand, FImporterToBlueprint_RenderMaterialTest&, Test);

class FImporterToBlueprint_RenderMaterialTest final : public AgxAutomationCommon::FAgxAutomationTest
{
public:
	FImporterToBlueprint_RenderMaterialTest()
		: AgxAutomationCommon::FAgxAutomationTest(
			  TEXT("FImporterToBlueprint_RenderMaterialTest"),
			  TEXT("AGXUnreal.Editor.ImporterToBlueprint.RenderMaterial"))
	{
	}

public:
	UBlueprint* Contents = nullptr;

protected:
	virtual bool RunTest(const FString&) override
	{
		BAIL_TEST_IF_NOT_EDITOR(false)
		ADD_LATENT_AUTOMATION_COMMAND(
			FImportArchiveBlueprintCommand(TEXT("render_materials_build.agx"), Contents, *this))
		ADD_LATENT_AUTOMATION_COMMAND(FCheckRenderMaterialImportedCommand(*this))
		ADD_LATENT_AUTOMATION_COMMAND(FClearRenderMaterialImportedCommand(*this))
		return true;
	}
};

namespace
{
	FImporterToBlueprint_RenderMaterialTest ImporterToBlueprint_RenderMaterialTest;
}

namespace
{
	/// \todo These are also in AGX_ImportUtilities, but I get linker errors when using them even
	/// though AGXUnrealEditor is in the modules list in AGXUnrealTest.Build.cs. Copying the
	/// code here for now.

	FLinearColor SRGBToLinear(const FVector4& SRGB)
	{
		FColor SRGBBytes(
			static_cast<uint8>(SRGB.X * 255.0f), static_cast<uint8>(SRGB.Y * 255.0f),
			static_cast<uint8>(SRGB.Z * 255.0f), static_cast<uint8>(SRGB.W * 255.0f));
		return {SRGBBytes};
	}

	FVector4 LinearToSRGB(const FLinearColor& Linear)
	{
		FColor SRGBBytes = Linear.ToFColor(true);
		return FVector4(
			static_cast<float>(SRGBBytes.R) / 255.0f, static_cast<float>(SRGBBytes.G) / 255.0f,
			static_cast<float>(SRGBBytes.B) / 255.0f, static_cast<float>(SRGBBytes.A) / 255.0f);
	}
}

namespace CheckRenderMaterialImportedCommand_helpers
{
	void TestScalar(
		UMaterialInterface& Material, const TCHAR* ParameterName, float Expected,
		FAutomationTestBase& Test)
	{
		FMaterialParameterInfo Info;
		Info.Name = ParameterName;
		float Actual;
		if (!Material.GetScalarParameterValue(Info, Actual, false))
		{
			Test.AddError(FString::Printf(
				TEXT("Could not get parameter '%s' for material '%s'."), ParameterName,
				*Material.GetName()));
			return;
		}
		Test.TestEqual(
			*FString::Printf(TEXT("%s in %s"), ParameterName, *Material.GetName()), Actual,
			Expected);
	}

	void TestColor(
		UMaterialInterface& Material, const TCHAR* ParameterName, const FVector4& Expected,
		FAutomationTestBase& Test)
	{
		FMaterialParameterInfo Info;
		Info.Name = ParameterName;
		FLinearColor ActualLinear;
		if (!Material.GetVectorParameterValue(Info, ActualLinear, false))
		{
			Test.AddError(FString::Printf(
				TEXT("Could not get parameter '%s' from material '%s'."), ParameterName,
				*Material.GetName()));
			return;
		}

		FVector4 Actual = LinearToSRGB(ActualLinear);
		float Tolerance = 1.0f / 255.0f; // This is all the precision we have in a byte.
		AgxAutomationCommon::TestEqual(
			Test, *FString::Printf(TEXT("%s in %s"), ParameterName, *Material.GetName()), Actual,
			Expected, Tolerance);
	}

	struct FMaterialParameters
	{
		// These are the default material properties in AGX Dynamics. Each test override a subset of
		// these.
		FVector4 Ambient = LinearToSRGB({0.01f, 0.0028806f, 0.0f, 1.0f});
		FVector4 Diffuse = LinearToSRGB({0.822786f, 0.822786f, 0.822786f, 1.0f});
		FVector4 Emissive = LinearToSRGB({0.0f, 0.0f, 0.0f, 1.0f});
		float Shininess {0.6f};
	};

	void TestMaterial(
		UAGX_SphereShapeComponent& Sphere, const FMaterialParameters& Parameters,
		FAutomationTestBase& Test)
	{
		UMaterialInterface* Material = Sphere.GetMaterial(0);
		if (Material == nullptr)
		{
			Test.AddError(
				FString::Printf(TEXT("Sphere '%s' does not have a material."), *Sphere.GetName()));
			return;
		}
		TestColor(*Material, TEXT("Ambient"), Parameters.Ambient, Test);
		TestColor(*Material, TEXT("Diffuse"), Parameters.Diffuse, Test);
		TestColor(*Material, TEXT("Emissive"), Parameters.Emissive, Test);
		TestScalar(*Material, TEXT("Shininess"), Parameters.Shininess, Test);
	}
}

bool FCheckRenderMaterialImportedCommand::Update()
{
	using namespace AgxAutomationCommon;
	using namespace CheckRenderMaterialImportedCommand_helpers;
	if (Test.Contents == nullptr)
	{
		Test.AddError(TEXT("Could not import RenderMaterial test scene: No content created."));
		return true;
	}

	// Get all the imported components. The test for the number of components is a safety check.
	// It should be updated whenever the test scene is changed.
	TArray<UActorComponent*> Components =
		FAGX_BlueprintUtilities::GetTemplateComponents(Test.Contents);
	Test.TestEqual(TEXT("Number of imported components"), Components.Num(), 20);

// Enable this to see the names of the components that was imported. Useful when adding new stuff
// to the archive.
#if 0
	UE_LOG(LogAGX, Warning, TEXT("Imported the following components:"));
	for (const UActorComponent* Component : Components)
	{
		UE_LOG(LogAGX, Warning, TEXT("  %s"), *Component->GetName());
	}
#endif

	auto GetSphere = [&Components](const TCHAR* Name) -> UAGX_SphereShapeComponent*
	{ return GetByName<UAGX_SphereShapeComponent>(Components, Name); };

	// Get the components we know should be there.
	/// @todo Some of these get auto-generated names because of name conflicts. Happens every time a
	/// agxCollide::Geometry contains more than once agxCollide::Shape since the name lives in the
	/// Geometry. So far the generated names have been consistent between runs, but I'm not sure if
	/// we're guaranteed that. Especially if we run multiple tests in the same invocation of the
	/// editor. The fix is to fetch objects based on UUID/GUID instead of names.
	USceneComponent* SceneRoot = GetByName<USceneComponent>(
		Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("DefaultSceneRoot"));
	UAGX_RigidBodyComponent* Body = GetByName<UAGX_RigidBodyComponent>(
		Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("RenderMaterialBody"));
	UAGX_SphereShapeComponent* Ambient =
		GetSphere(*FAGX_BlueprintUtilities::ToTemplateComponentName("AmbientGeometry"));
	UAGX_SphereShapeComponent* Diffuse =
		GetSphere(*FAGX_BlueprintUtilities::ToTemplateComponentName("DiffuseGeometry"));
	UAGX_SphereShapeComponent* Emissive =
		GetSphere(*FAGX_BlueprintUtilities::ToTemplateComponentName("EmissiveGeometry"));
	UAGX_SphereShapeComponent* Shininess =
		GetSphere(*FAGX_BlueprintUtilities::ToTemplateComponentName("ShininessGeometry"));
	UAGX_SphereShapeComponent* AmbientDiffuse =
		GetSphere(*FAGX_BlueprintUtilities::ToTemplateComponentName("AmbientDiffuseGeometry"));
	UAGX_SphereShapeComponent* AmbientEmissive =
		GetSphere(*FAGX_BlueprintUtilities::ToTemplateComponentName("AmbientEmissiveGeometry"));
	UAGX_SphereShapeComponent* DiffuseShininessLow =
		GetSphere(*FAGX_BlueprintUtilities::ToTemplateComponentName("DiffuseShininessLowGeometry"));
	UAGX_SphereShapeComponent* DiffuseShininessHigh = GetSphere(
		*FAGX_BlueprintUtilities::ToTemplateComponentName("DiffuseShininessHighGeometry"));
	UAGX_SphereShapeComponent* SharedSphere1 =
		GetSphere(*FAGX_BlueprintUtilities::ToTemplateComponentName("SharedGeometry"));
	UAGX_SphereShapeComponent* SharedSphere2 =
		GetSphere(*FAGX_BlueprintUtilities::ToTemplateComponentName("SharedGeometry_0"));
	UAGX_SphereShapeComponent* NameConflictSphere1 =
		GetSphere(*FAGX_BlueprintUtilities::ToTemplateComponentName("MaterialNameConflict"));
	UAGX_SphereShapeComponent* NameConflictSphere2 =
		GetSphere(*FAGX_BlueprintUtilities::ToTemplateComponentName("MaterialNameConflict_1"));
	UAGX_SphereShapeComponent* VisibleSphere =
		GetSphere(*FAGX_BlueprintUtilities::ToTemplateComponentName("VisibleSphere"));
	UAGX_SphereShapeComponent* InvisibleSphere =
		GetSphere(*FAGX_BlueprintUtilities::ToTemplateComponentName("InvisibleSphere"));
	auto Trimesh = GetByName<UAGX_TrimeshShapeComponent>(
		Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("Trimesh"));

	// Make sure we got the components we know should be there.
	Test.TestNotNull(TEXT("DefaultSceneRoot"), SceneRoot);
	Test.TestNotNull(TEXT("Body"), Body);
	Test.TestNotNull(TEXT("Ambient"), Ambient);
	Test.TestNotNull(TEXT("Diffuse"), Diffuse);
	Test.TestNotNull(TEXT("Emissive"), Emissive);
	Test.TestNotNull(TEXT("Shininess"), Shininess);
	Test.TestNotNull(TEXT("AmbientDiffuse"), AmbientDiffuse);
	Test.TestNotNull(TEXT("AmbientEmissive"), AmbientEmissive);
	Test.TestNotNull(TEXT("DiffuseShininessLow"), DiffuseShininessLow);
	Test.TestNotNull(TEXT("DiffuseShininessHigh"), DiffuseShininessHigh);
	Test.TestNotNull(TEXT("SharedSphere1"), SharedSphere1);
	Test.TestNotNull(TEXT("SharedSphere2"), SharedSphere2);
	Test.TestNotNull(TEXT("NameConflictSphere1"), NameConflictSphere1);
	Test.TestNotNull(TEXT("NameConflictSphere2"), NameConflictSphere2);
	Test.TestNotNull(TEXT("VisibleSphere"), VisibleSphere);
	Test.TestNotNull(TEXT("InvisibleSphere"), InvisibleSphere);
	Test.TestNotNull(TEXT("Trimesh"), Trimesh);

	if (SceneRoot == nullptr || Body == nullptr || Ambient == nullptr || Diffuse == nullptr ||
		Emissive == nullptr || Shininess == nullptr || AmbientDiffuse == nullptr ||
		AmbientEmissive == nullptr || DiffuseShininessLow == nullptr ||
		DiffuseShininessHigh == nullptr || SharedSphere1 == nullptr || SharedSphere2 == nullptr ||
		NameConflictSphere1 == nullptr || NameConflictSphere2 == nullptr ||
		VisibleSphere == nullptr || InvisibleSphere == nullptr || Trimesh == nullptr)
	{
		Test.AddError(TEXT("At least one required object was nullptr, cannot continue."));
		return true;
	}

	// Ambient.
	{
		FMaterialParameters Parameters;
		Parameters.Ambient = FVector4(0.32f, 0.85f, 0.21f, 1.0f);
		TestMaterial(*Ambient, Parameters, Test);
	}
	// Diffuse.
	{
		FMaterialParameters Parameters;
		Parameters.Diffuse = FVector4(0.80f, 0.34f, 0.21f, 1.0f);
		TestMaterial(*Diffuse, Parameters, Test);
	}
	// Emissive.
	{
		FMaterialParameters Parameters;
		Parameters.Emissive = FVector4(0.98f, 0.94f, 0.76f, 1.0f);
		TestMaterial(*Emissive, Parameters, Test);
	}
	// AmbientDiffuse.
	{
		FMaterialParameters Parameters;
		Parameters.Ambient = FVector4(0.81f, 0.34f, 0.26f, 1.0f);
		Parameters.Diffuse = FVector4(0.32f, 0.28f, 0.67f, 1.0f);
		TestMaterial(*AmbientDiffuse, Parameters, Test);
	}
	// AmbientEmissive.
	{
		FMaterialParameters Parameters;
		Parameters.Ambient = FVector4(0.32f, 0.34f, 0.54f, 1.0f);
		Parameters.Emissive = FVector4(0.21f, 0.17f, 0.23f, 1.0f);
		TestMaterial(*AmbientEmissive, Parameters, Test);
	}
	// DiffuseShininessLow.
	{
		FMaterialParameters Parameters;
		Parameters.Diffuse = FVector4(0.65f, 0.74f, 0.48f, 1.0f);
		Parameters.Shininess = 0.0f;
		TestMaterial(*DiffuseShininessLow, Parameters, Test);
	}
	// DiffuseShininessHigh.
	{
		FMaterialParameters Parameters;
		Parameters.Diffuse = FVector4(0.65f, 0.74f, 0.48f, 1.0f);
		Parameters.Shininess = 1.0f;
		TestMaterial(*DiffuseShininessHigh, Parameters, Test);
	}
	// Shared.
	{
		const UMaterialInterface* const Material1 = SharedSphere1->GetMaterial(0);
		const UMaterialInterface* const Material2 = SharedSphere2->GetMaterial(0);
		Test.TestNotNull(TEXT("SharedSphere1 material"), Material1);
		Test.TestNotNull(TEXT("SharedSphere2 material"), Material2);
		Test.TestEqual(TEXT("SharedSphere materials"), Material1, Material2);
	}
	// NameConflict.
	{
		const UMaterialInterface* const Material1 = NameConflictSphere1->GetMaterial(0);
		const UMaterialInterface* const Material2 = NameConflictSphere2->GetMaterial(0);
		Test.TestNotEqual(TEXT("Name conflict materials"), Material1, Material2);
// AGX Dynamics does not currently store render material names in archives. The importer always
// reads empty strings. Enable these tests if/when render material names are added to serialization.
#if 0
		Test.TestEqual(TEXT("Conflict name"), Material1->GetName(), FString(TEXT("NameConflict")));
		Test.TestEqual(TEXT("Conflict name"), Material2->GetName(), FString(TEXT("NameConflict")));
#endif
		FMaterialParameters Parameters1;
		Parameters1.Shininess = 0.30f;
		TestMaterial(*NameConflictSphere1, Parameters1, Test);
		FMaterialParameters Parameters2;
		Parameters2.Shininess = 0.99f;
		TestMaterial(*NameConflictSphere2, Parameters2, Test);
	}
	// ShouldRender.
	{
		Test.TestTrue(TEXT("VisibleSphere Visible flag"), VisibleSphere->GetVisibleFlag());
		Test.TestFalse(TEXT("InvisibleSphere Visible flag"), InvisibleSphere->GetVisibleFlag());
	}

	// Static mesh assets render materials.
	USCS_Node* TrimeshNode = GetNodeChecked(*Test.Contents, "Trimesh");
	USCS_Node* CollisionMeshNode = GetOnlyAttachChildChecked(TrimeshNode);
	USCS_Node* RenderMeshNode = GetOnlyAttachChildChecked(CollisionMeshNode);
	if (RenderMeshNode == nullptr)
	{
		Test.AddError("Render Mesh SCS Node was nullptr. Cannot continue.");
		return true;
	}

	auto CollisionMeshComp = Cast<UStaticMeshComponent>(CollisionMeshNode->ComponentTemplate);
	auto RenderMeshComp = Cast<UStaticMeshComponent>(RenderMeshNode->ComponentTemplate);
	if (CollisionMeshComp == nullptr || RenderMeshComp == nullptr)
	{
		Test.AddError("Render or Collision Mesh Component was nullptr. Cannot continue.");
		return true;
	}

	auto CollisionMeshAsset = Cast<UStaticMesh>(CollisionMeshComp->GetStaticMesh());
	auto RenderMeshAsset = Cast<UStaticMesh>(RenderMeshComp->GetStaticMesh());
	if (CollisionMeshAsset == nullptr || RenderMeshAsset == nullptr)
	{
		Test.AddError("Render or Collision Mesh Assets was nullptr. Cannot continue.");
		return true;
	}

	if (RenderMeshAsset->GetMaterial(0) == nullptr)
	{
		Test.AddError("Render Mesh Assets material was nullptr. Cannot continue.");
		return true;
	}

	Test.TestTrue(
		"Render mesh material name",
		RenderMeshAsset->GetMaterial(0)->GetName().StartsWith("MI_RenderMaterial_"));
	Test.TestTrue(
		"Collision and Render mesh material same",
		CollisionMeshAsset->GetMaterial(0) == RenderMeshAsset->GetMaterial(0));

	return true;
}

bool FClearRenderMaterialImportedCommand::Update()
{
	if (Test.Contents == nullptr)
	{
		return true;
	}

	const FString Path =
		FAGX_ImportUtilities::GetDefaultModelImportDirectory("render_materials_build");
	if (!FPaths::DirectoryExists(Path))
	{
		Test.AddError(FString::Printf(
			TEXT("Unable to delete files directory '%s' because it does not exist."), *Path));
		return true;
	}

#if defined(__linux__)
	/// @todo Workaround for internal issue #213.
	Test.AddExpectedError(
		TEXT("inotify_rm_watch cannot remove descriptor"), EAutomationExpectedErrorFlags::Contains,
		0);
	Test.AddError(TEXT("inotify_rm_watch cannot remove descriptor"));
#endif

	// Doing a file system delete of the assets is a bit harsh. Works in this case since we know
	// nothing will use these assets the next time Unreal Editor is started since we don't use
	// the imported unit test assets for anything.
	if (!IFileManager::Get().DeleteDirectory(*Path, true, true))
	{
		Test.AddError(FString::Printf(
			TEXT("IFileManager::DeleteDirectory returned false trying to remove: '%s'"), *Path));
		return true;
	}

	return true;
}

//
// Render Data test starts here.
//

class FImporterToBlueprint_RenderDataTest;

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FCheckRenderDataImportedCommand, FImporterToBlueprint_RenderDataTest&, Test);

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FClearRenderDataImportedCommand, FImporterToBlueprint_RenderDataTest&, Test);

class FImporterToBlueprint_RenderDataTest final : public AgxAutomationCommon::FAgxAutomationTest
{
public:
	FImporterToBlueprint_RenderDataTest()
		: AgxAutomationCommon::FAgxAutomationTest(
			  TEXT("FImporterToBlueprint_RenderDataTest"),
			  TEXT("AGXUnreal.Editor.ImporterToBlueprint.RenderData"))
	{
	}

public:
	UBlueprint* Contents = nullptr;

protected:
	virtual bool RunTest(const FString&) override
	{
		BAIL_TEST_IF_NOT_EDITOR(false)
		ADD_LATENT_AUTOMATION_COMMAND(
			FImportArchiveBlueprintCommand(TEXT("render_data_build.agx"), Contents, *this))
		ADD_LATENT_AUTOMATION_COMMAND(FCheckRenderDataImportedCommand(*this))
		ADD_LATENT_AUTOMATION_COMMAND(FClearRenderDataImportedCommand(*this))
		return true;
	}
};

namespace
{
	FImporterToBlueprint_RenderDataTest ImporterToBlueprint_RenderDataTest;
}

bool FCheckRenderDataImportedCommand::Update()
{
	using namespace AgxAutomationCommon;

	if (Test.Contents == nullptr)
	{
		Test.AddError(TEXT("Could not import RenderMaterial test scene: No content created."));
		return true;
	}

	TArray<UActorComponent*> Components =
		FAGX_BlueprintUtilities::GetTemplateComponents(Test.Contents);
	// Root(1), Rigid Body(2), Shape(3), Static Mesh(4), Reimport(5).
	Test.TestEqual(TEXT("Number of imported components"), Components.Num(), 5);

	// Enable this to see the names of the components that was imported. Useful when adding new
	// stuff to the archive.
#if 0
	UE_LOG(LogAGX, Warning, TEXT("Imported the following components:"));
	for (const UActorComponent* Component : Components)
	{
		UE_LOG(LogAGX, Warning, TEXT("  %s"), *Component->GetName());
	}
#endif

	UAGX_SphereShapeComponent* Sphere = GetByName<UAGX_SphereShapeComponent>(
		Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("Render Data Geometry"));
	UStaticMeshComponent* Mesh = GetByName<UStaticMeshComponent>(
		Components, *FAGX_BlueprintUtilities::ToTemplateComponentName(
						"RenderMesh_944C2AF4E9279E2C61D073B86467F6BA"));

	Test.TestNotNull(TEXT("Sphere"), Sphere);
	Test.TestNotNull(TEXT("Mesh"), Mesh);

	if (IsAnyNullptr(Sphere, Mesh))
	{
		Test.AddError(TEXT("At least one required object was nullptr, cannot continue."));
		return true;
	}

	Test.TestTrue(
		TEXT("The mesh should be a child of the sphere"),
		FAGX_BlueprintUtilities::GetTemplateComponentAttachParent(Mesh) == Sphere);

	return true;
}

bool FClearRenderDataImportedCommand::Update()
{
	if (Test.Contents == nullptr)
	{
		return true;
	}

#if defined(__linux__)
	/// @todo Workaround for internal issue #213.
	Test.AddExpectedError(
		TEXT("inotify_rm_watch cannot remove descriptor"), EAutomationExpectedErrorFlags::Contains,
		0);
	Test.AddError(TEXT("inotify_rm_watch cannot remove descriptor"));
#endif

	// Files that are created by the test and thus safe to remove. The GUID values may make this
	// test cumbersome to update since they will change every time the AGX Dynamics archive is
	// regenerated. Consider either adding wildcard support to DeleteImportDirectory or assign
	// names to the render materials in the source .agxPy file.
	TArray<const TCHAR*> ExpectedFiles = {
		TEXT("Blueprint"), TEXT("BP_render_data_build.uasset"), TEXT("RenderMesh"),
		TEXT("SM_RenderMesh_944C2AF4E9279E2C61D073B86467F6BA.uasset")};

	const FString BaseBlueprintName = Test.Contents->GetName() + FString(".uasset");
	ExpectedFiles.Add(*BaseBlueprintName);

	AgxAutomationCommon::DeleteImportDirectory(TEXT("render_data_build"), ExpectedFiles);

	return true;
}

//
// CollisionGroups test starts here.
//

class FImporterToBlueprint_CollisionGroupsTest;

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FCheckCollisionGroupsImportedCommand, FImporterToBlueprint_CollisionGroupsTest&, Test);

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FClearCollisionGroupsImportedCommand, FImporterToBlueprint_CollisionGroupsTest&, Test);

class FImporterToBlueprint_CollisionGroupsTest final
	: public AgxAutomationCommon::FAgxAutomationTest
{
public:
	FImporterToBlueprint_CollisionGroupsTest()
		: AgxAutomationCommon::FAgxAutomationTest(
			  TEXT("FImporterToBlueprint_CollisionGroupsTest"),
			  TEXT("AGXUnreal.Editor.ImporterToBlueprint.CollisionGroups"))
	{
	}

public:
	UWorld* World = nullptr;
	UAGX_Simulation* Simulation = nullptr;
	UBlueprint* Contents = nullptr;
	UAGX_RigidBodyComponent* TrimeshBody = nullptr;

protected:
	virtual bool RunTest(const FString&) override
	{
		BAIL_TEST_IF_NOT_EDITOR(false)
		ADD_LATENT_AUTOMATION_COMMAND(
			FImportArchiveBlueprintCommand(TEXT("collision_groups_build.agx"), Contents, *this))
		ADD_LATENT_AUTOMATION_COMMAND(FCheckCollisionGroupsImportedCommand(*this))
		ADD_LATENT_AUTOMATION_COMMAND(FClearCollisionGroupsImportedCommand(*this))
		return true;
	}
};

namespace
{
	FImporterToBlueprint_CollisionGroupsTest ImporterToBlueprint_CollisionGroupsTest;
}

/**
 * Check that the expected state was created during import.
 *
 * The object structure and all numbers tested here should match what is being set in the source
 * script collision_groups.agxPy.
 * @return true when the check is complete. Never returns false.
 */
bool FCheckCollisionGroupsImportedCommand::Update()
{
	using namespace AgxAutomationCommon;
	if (Test.Contents == nullptr)
	{
		Test.AddError(TEXT("Could not import CollisionGroups test scene: No content created."));
		return true;
	}

	// Get all the imported components.
	TArray<UActorComponent*> Components =
		FAGX_BlueprintUtilities::GetTemplateComponents(Test.Contents);
	Test.TestEqual(TEXT("Number of imported components"), Components.Num(), 20);

	auto GetBox = [&Components](
					  const TCHAR* Name,
					  TArray<UAGX_BoxShapeComponent*>& OutArr) -> UAGX_BoxShapeComponent*
	{
		UAGX_BoxShapeComponent* Box = GetByName<UAGX_BoxShapeComponent>(Components, Name);
		OutArr.Add(Box);
		return Box;
	};

	auto GetBody = [&Components](
					   const TCHAR* Name,
					   TArray<UAGX_RigidBodyComponent*>& OutArr) -> UAGX_RigidBodyComponent*
	{
		UAGX_RigidBodyComponent* Rb = GetByName<UAGX_RigidBodyComponent>(Components, Name);
		OutArr.Add(Rb);
		return Rb;
	};

	TArray<UAGX_RigidBodyComponent*> RbArr;
	TArray<UAGX_BoxShapeComponent*> BoxArr;
	USceneComponent* SceneRoot = GetByName<USceneComponent>(
		Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("DefaultSceneRoot"));
	UAGX_RigidBodyComponent* rb_0_brown =
		GetBody(*FAGX_BlueprintUtilities::ToTemplateComponentName("rb_0_brown"), RbArr);
	UAGX_BoxShapeComponent* geom_0_brown =
		GetBox(*FAGX_BlueprintUtilities::ToTemplateComponentName("geom_0_brown"), BoxArr);
	UAGX_RigidBodyComponent* rb_left_1_brown =
		GetBody(*FAGX_BlueprintUtilities::ToTemplateComponentName("rb_left_1_brown"), RbArr);
	UAGX_BoxShapeComponent* geom_left_1_brown =
		GetBox(*FAGX_BlueprintUtilities::ToTemplateComponentName("geom_left_1_brown"), BoxArr);
	UAGX_RigidBodyComponent* rb_right_1_orange =
		GetBody(*FAGX_BlueprintUtilities::ToTemplateComponentName("rb_right_1_orange"), RbArr);
	UAGX_BoxShapeComponent* geom_right_1_orange =
		GetBox(*FAGX_BlueprintUtilities::ToTemplateComponentName("geom_right_1_orange"), BoxArr);
	UAGX_RigidBodyComponent* rb_left_2_orange =
		GetBody(*FAGX_BlueprintUtilities::ToTemplateComponentName("rb_left_2_orange"), RbArr);
	UAGX_BoxShapeComponent* geom_left_2_orange =
		GetBox(*FAGX_BlueprintUtilities::ToTemplateComponentName("geom_left_2_orange"), BoxArr);
	UAGX_RigidBodyComponent* rb_right_2_orange =
		GetBody(*FAGX_BlueprintUtilities::ToTemplateComponentName("rb_right_2_orange"), RbArr);
	UAGX_BoxShapeComponent* geom_right_2_orange =
		GetBox(*FAGX_BlueprintUtilities::ToTemplateComponentName("geom_right_2_orange"), BoxArr);
	UAGX_RigidBodyComponent* rb_left_3_brown =
		GetBody(*FAGX_BlueprintUtilities::ToTemplateComponentName("rb_left_3_brown"), RbArr);
	UAGX_BoxShapeComponent* geom_left_3_brown =
		GetBox(*FAGX_BlueprintUtilities::ToTemplateComponentName("geom_left_3_brown"), BoxArr);
	UAGX_RigidBodyComponent* rb_4_blue =
		GetBody(*FAGX_BlueprintUtilities::ToTemplateComponentName("rb_4_blue"), RbArr);
	UAGX_BoxShapeComponent* geom_4_blue =
		GetBox(*FAGX_BlueprintUtilities::ToTemplateComponentName("geom_4_blue"), BoxArr);
	UAGX_RigidBodyComponent* rb_left_5_blue =
		GetBody(*FAGX_BlueprintUtilities::ToTemplateComponentName("rb_left_5_blue"), RbArr);
	UAGX_BoxShapeComponent* geom_left_5_blue =
		GetBox(*FAGX_BlueprintUtilities::ToTemplateComponentName("geom_left_5_blue"), BoxArr);
	UAGX_WireComponent* Wire = GetByName<UAGX_WireComponent>(
		Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("Wire"));
	UAGX_CollisionGroupDisablerComponent* AGX_CollisionGroupDisabler =
		GetByName<UAGX_CollisionGroupDisablerComponent>(
			Components,
			*FAGX_BlueprintUtilities::ToTemplateComponentName("AGX_CollisionGroupDisabler"));

	Test.TestEqual(TEXT("Number of Rigid Bodies"), RbArr.Num(), 8);
	Test.TestEqual(TEXT("Number of Box Shapes"), BoxArr.Num(), 8);

	Test.TestNotNull(TEXT("Wire"), Wire);
	Test.TestNotNull(TEXT("DefaultSceneRoot"), SceneRoot);
	Test.TestNotNull(TEXT("AGX_CollisionGroupDisabler"), AGX_CollisionGroupDisabler);

	for (auto& R : RbArr)
	{
		Test.TestNotNull(TEXT("Rigid Body"), R);
		if (R == nullptr)
		{
			Test.AddError(TEXT("At least one required object was nullptr, cannot continue."));
			return true;
		}
	}

	for (auto& B : BoxArr)
	{
		Test.TestNotNull(TEXT("Box Shape"), B);
		if (B == nullptr)
		{
			Test.AddError(TEXT("At least one required object was nullptr, cannot continue."));
			return true;
		}

		Test.TestEqual(TEXT("Number of Collision groups"), B->CollisionGroups.Num(), 1);
		if (B->CollisionGroups.Num() != 1)
		{
			Test.AddError(TEXT("Wrong number of collision groups, cannot continue."));
			return true;
		}
	}

	if (IsAnyNullptr(Wire, AGX_CollisionGroupDisabler))
	{
		return true; // Not null tested above and will get reported in the test log.
	}

	Test.TestEqual(
		TEXT("Collision group name"), geom_0_brown->CollisionGroups[0].IsEqual("A"), true);
	Test.TestEqual(
		TEXT("Collision grp name"), geom_left_1_brown->CollisionGroups[0].IsEqual("A"), true);
	Test.TestEqual(
		TEXT("Collision grp name"), geom_right_1_orange->CollisionGroups[0].IsEqual("5"), true);
	Test.TestEqual(
		TEXT("Collision grp name"), geom_left_2_orange->CollisionGroups[0].IsEqual("5"), true);
	Test.TestEqual(
		TEXT("Collision grp name"), geom_right_2_orange->CollisionGroups[0].IsEqual("5"), true);
	Test.TestEqual(
		TEXT("Collision grp name"), geom_left_3_brown->CollisionGroups[0].IsEqual("A"), true);
	Test.TestEqual(TEXT("Collision grp name"), geom_4_blue->CollisionGroups[0].IsEqual("b"), true);
	Test.TestEqual(
		TEXT("Collision grp name"), geom_left_5_blue->CollisionGroups[0].IsEqual("b"), true);

	Test.TestEqual("Wire num collision groups", Wire->CollisionGroups.Num(), 1);
	if (Wire->CollisionGroups.Num() == 1)
		Test.TestEqual("Wire collision group", Wire->CollisionGroups[0], FName("w"));

	// Adding the Wire will automatically add 9 additional collision group pairs. Not clear why,
	// it's an AGX thing.
	Test.TestEqual(
		TEXT("Number of Collision group pairs"),
		AGX_CollisionGroupDisabler->DisabledCollisionGroupPairs.Num(), 12);

	Test.TestEqual(
		TEXT("Pair collision disabled"),
		AGX_CollisionGroupDisabler->IsCollisionGroupPairDisabled(FName("A"), FName("b")), true);

	Test.TestEqual(
		TEXT("Pair collision disabled"),
		AGX_CollisionGroupDisabler->IsCollisionGroupPairDisabled(FName("b"), FName("b")), true);

	Test.TestEqual(
		TEXT("Pair collision disabled"),
		AGX_CollisionGroupDisabler->IsCollisionGroupPairDisabled(FName("5"), FName("5")), true);

	Test.TestEqual(
		TEXT("Pair collision disabled"),
		AGX_CollisionGroupDisabler->IsCollisionGroupPairDisabled(FName("A"), FName("A")), false);

	return true;
}

/**
 * Remove everything created by the archive import.
 * @return true when the clearing is complete. Never returns false.
 */
bool FClearCollisionGroupsImportedCommand::Update()
{
	if (Test.Contents == nullptr)
	{
		return true;
	}

#if defined(__linux__)
	/// @todo Workaround for internal issue #213.
	Test.AddExpectedError(
		TEXT("inotify_rm_watch cannot remove descriptor"), EAutomationExpectedErrorFlags::Contains,
		0);
	Test.AddError(TEXT("inotify_rm_watch cannot remove descriptor"));
#endif

	TArray<const TCHAR*> ExpectedFiles {
		TEXT("Blueprint"), TEXT("BP_collision_groups_build.uasset"), TEXT("ShapeMaterial"),
		TEXT("defaultWireMaterial_93.uasset")};

	const FString BaseBlueprintName = Test.Contents->GetName() + FString(".uasset");
	ExpectedFiles.Add(*BaseBlueprintName);

	AgxAutomationCommon::DeleteImportDirectory(TEXT("collision_groups_build"), ExpectedFiles);
	return true;
}

//
// GeometrySensors test starts here.
//

class FImporterToBlueprint_GeometrySensorsTest;

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FCheckGeometrySensorsImportedCommand, FImporterToBlueprint_GeometrySensorsTest&, Test);

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FClearGeometrySensorsImportedCommand, FImporterToBlueprint_GeometrySensorsTest&, Test);

class FImporterToBlueprint_GeometrySensorsTest final
	: public AgxAutomationCommon::FAgxAutomationTest
{
public:
	FImporterToBlueprint_GeometrySensorsTest()
		: AgxAutomationCommon::FAgxAutomationTest(
			  TEXT("FImporterToBlueprint_GeometrySensorsTest"),
			  TEXT("AGXUnreal.Editor.ImporterToBlueprint.GeometrySensors"))
	{
	}

public:
	UWorld* World = nullptr;
	UAGX_Simulation* Simulation = nullptr;
	UBlueprint* Contents = nullptr;
	UAGX_RigidBodyComponent* TrimeshBody = nullptr;

protected:
	virtual bool RunTest(const FString&) override
	{
		BAIL_TEST_IF_NOT_EDITOR(false)
		ADD_LATENT_AUTOMATION_COMMAND(
			FImportArchiveBlueprintCommand(TEXT("geometry_sensors_build.agx"), Contents, *this))
		ADD_LATENT_AUTOMATION_COMMAND(FCheckGeometrySensorsImportedCommand(*this))
		ADD_LATENT_AUTOMATION_COMMAND(FClearGeometrySensorsImportedCommand(*this))
		return true;
	}
};

namespace
{
	FImporterToBlueprint_GeometrySensorsTest ImporterToBlueprint_GeometrySensorsTest;
}

/**
 * Check that the expected state was created during import.
 *
 * The object structure and all numbers tested here should match what is being set in the source
 * script geometry_sensors.agxPy.
 * @return true when the check is complete. Never returns false.
 */
bool FCheckGeometrySensorsImportedCommand::Update()
{
	using namespace AgxAutomationCommon;
	if (Test.Contents == nullptr)
	{
		Test.AddError(TEXT("Could not import GeometrySensors test scene: No content created."));
		return true;
	}

	// Get all the imported components.
	TArray<UActorComponent*> Components =
		FAGX_BlueprintUtilities::GetTemplateComponents(Test.Contents);

	// Three Rigid Bodies, three Geometries, one Default Scene Root, one Reimport Component.
	Test.TestEqual(TEXT("Number of imported components"), Components.Num(), 8);

	UAGX_SphereShapeComponent* BoolSensor = GetByName<UAGX_SphereShapeComponent>(
		Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("boolSensor"));

	UAGX_CylinderShapeComponent* ContactsSensor = GetByName<UAGX_CylinderShapeComponent>(
		Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("contactsSensor"));

	UAGX_BoxShapeComponent* NotASensor = GetByName<UAGX_BoxShapeComponent>(
		Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("notASensor"));

	Test.TestNotNull(TEXT("boolSensor"), BoolSensor);
	Test.TestNotNull(TEXT("contactsSensor"), ContactsSensor);
	Test.TestNotNull(TEXT("notASensor"), NotASensor);

	if (BoolSensor == nullptr || ContactsSensor == nullptr || NotASensor == nullptr)
	{
		Test.AddError(TEXT("At least one required object was nullptr, cannot continue."));
		return true;
	}

	// Test the bIsSensor property.
	Test.TestEqual(TEXT("Is Sensor property"), BoolSensor->bIsSensor, true);
	Test.TestEqual(TEXT("Is Sensor property"), ContactsSensor->bIsSensor, true);
	Test.TestEqual(TEXT("Is Sensor property"), NotASensor->bIsSensor, false);

	// Test the SensorType property (only relevant for sensor geometries).
	Test.TestEqual(
		TEXT("Sensor type property"), BoolSensor->SensorType == EAGX_ShapeSensorType::BooleanSensor,
		true);

	Test.TestEqual(
		TEXT("Sensor type property"),
		ContactsSensor->SensorType == EAGX_ShapeSensorType::ContactsSensor, true);

	// Test the Materials applied after import.
	const auto BoolSensorMaterials = BoolSensor->GetMaterials();
	Test.TestEqual(
		TEXT("Sensor Material"),
		(BoolSensorMaterials.Num() == 1 && BoolSensorMaterials[0] != nullptr &&
		 BoolSensorMaterials[0]->GetName() == "M_SensorMaterial"),
		true);

	const auto ContactsSensorMaterials = ContactsSensor->GetMaterials();
	Test.TestEqual(
		TEXT("Sensor Material"),
		(ContactsSensorMaterials.Num() == 1 && ContactsSensorMaterials[0] != nullptr &&
		 ContactsSensorMaterials[0]->GetName() == "M_SensorMaterial"),
		true);

	const auto NotASensorMaterials = NotASensor->GetMaterials();
	Test.TestEqual(
		TEXT("Default Material"),
		(NotASensorMaterials.Num() == 1 && NotASensorMaterials[0] != nullptr &&
		 NotASensorMaterials[0]->GetName() == "M_ImportedBase"),
		true);

	return true;
}

/**
 * Remove everything created by the archive import.
 * @return true when the clearing is complete. Never returns false.
 */
bool FClearGeometrySensorsImportedCommand::Update()
{
	if (Test.Contents == nullptr)
	{
		return true;
	}

#if defined(__linux__)
	/// @todo Workaround for internal issue #213.
	Test.AddExpectedError(
		TEXT("inotify_rm_watch cannot remove descriptor"), EAutomationExpectedErrorFlags::Contains,
		0);
	Test.AddError(TEXT("inotify_rm_watch cannot remove descriptor"));
#endif

	TArray<const TCHAR*> ExpectedFiles {
		TEXT("Blueprint"), TEXT("BP_geometry_sensors_build.uasset")};

	const FString BaseBlueprintName = Test.Contents->GetName() + FString(".uasset");
	ExpectedFiles.Add(*BaseBlueprintName);

	AgxAutomationCommon::DeleteImportDirectory(TEXT("geometry_sensors_build"), ExpectedFiles);

	return true;
}

//
// Wire test starts here.
//

class FImporterToBlueprint_WireTest;

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FCheckWireImportedCommand, FImporterToBlueprint_WireTest&, Test);

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FClearWireImportedCommand, FImporterToBlueprint_WireTest&, Test);

class FImporterToBlueprint_WireTest final : public AgxAutomationCommon::FAgxAutomationTest
{
public:
	FImporterToBlueprint_WireTest()
		: AgxAutomationCommon::FAgxAutomationTest(
			  TEXT("FImporterToBlueprint_WireTest"),
			  TEXT("AGXUnreal.Editor.ImporterToBlueprint.Wire"))
	{
	}

public:
	UWorld* World = nullptr;
	UAGX_Simulation* Simulation = nullptr;
	UBlueprint* Contents = nullptr;

protected:
	virtual bool RunTest(const FString&) override
	{
		BAIL_TEST_IF_NOT_EDITOR(false)
		ADD_LATENT_AUTOMATION_COMMAND(
			FImportArchiveBlueprintCommand(TEXT("wire_build.agx"), Contents, *this))
		ADD_LATENT_AUTOMATION_COMMAND(FCheckWireImportedCommand(*this))
		ADD_LATENT_AUTOMATION_COMMAND(FClearWireImportedCommand(*this))
		return true;
	}
};

namespace
{
	FImporterToBlueprint_WireTest ImporterToBlueprint_WireTest;
}

/**
 * Check that the expected state was created during import.
 *
 * The object structure and all numbers tested here should match what is being set in the source
 * script wire.agxPy.
 *
 * @return True when the check is complete. Never returns false.
 */
bool FCheckWireImportedCommand::Update()
{
	using namespace AgxAutomationCommon;
	if (Test.Contents == nullptr)
	{
		Test.AddError(TEXT("Could not import Wire test scene: No content created"));
		return true;
	}

	// Get all the imported components.
	TArray<UActorComponent*> Components =
		FAGX_BlueprintUtilities::GetTemplateComponents(Test.Contents);

	// A Wire (1) three Rigid Bodies (4), three Shapes (7), a Collision Group
	// Disabler (8), a Default Scene Root (9) and one ModelSourceComponent (10).
	Test.TestEqual(TEXT("Number of imported components"), Components.Num(), 10);
	if (Components.Num() != 10)
	{
		UE_LOG(LogAGX, Warning, TEXT("Found the following components:"));
		for (auto Component : Components)
		{
			UE_LOG(
				LogAGX, Warning, TEXT("  %s: %s"), *Component->GetName(),
				*Component->GetClass()->GetName());
		}
	}

	UAGX_WireComponent* Wire = GetByName<UAGX_WireComponent>(
		Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("Wire"));
	UAGX_RigidBodyComponent* WinchBody = GetByName<UAGX_RigidBodyComponent>(
		Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("Winch Body"));
	UAGX_CylinderShapeComponent* WinchShape = GetByName<UAGX_CylinderShapeComponent>(
		Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("Winch Shape"));
	UAGX_RigidBodyComponent* EyeBody = GetByName<UAGX_RigidBodyComponent>(
		Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("Eye Body"));
	UAGX_SphereShapeComponent* EyeShape = GetByName<UAGX_SphereShapeComponent>(
		Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("Eye Shape"));
	UAGX_RigidBodyComponent* BeadBody = GetByName<UAGX_RigidBodyComponent>(
		Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("Bead Body"));
	UAGX_CapsuleShapeComponent* BeadShape = GetByName<UAGX_CapsuleShapeComponent>(
		Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("Bead Shape"));

	Test.TestNotNull(TEXT("Wire"), Wire);
	Test.TestNotNull(TEXT("Winch Body"), WinchBody);
	Test.TestNotNull(TEXT("Winch Shape"), WinchShape);
	Test.TestNotNull(TEXT("Eye Body"), EyeBody);
	Test.TestNotNull(TEXT("Eye Shape"), EyeShape);
	Test.TestNotNull(TEXT("Bead Body"), BeadBody);
	Test.TestNotNull(TEXT("Bead Shape"), BeadShape);

	if (IsAnyNullptr(Wire, WinchBody, WinchShape, EyeBody, EyeShape, BeadBody, BeadShape))
	{
		Test.AddError(TEXT("At least one required object was nullptr, cannot continue."));
		return true;
	}

	const float Radius = CentimeterToMeter(Wire->Radius);
	const float Resolution = 1.0f / CentimeterToMeter(Wire->MinSegmentLength);
	FAGX_WireWinch& Winch = Wire->OwnedBeginWinch;
	const double PulledInLength = CentimeterToMeter(Winch.GetPulledInLength());
	const double Speed = CentimeterToMeter(Winch.GetTargetSpeed());
	const FVector WinchLocation {
		CentimeterToMeter(Winch.Location.X), CentimeterToMeter(Winch.Location.Y),
		CentimeterToMeter(Winch.Location.Z)};

	// Note negated Y, since AGX Dynamics is right-handed and Unreal Engine is left-handed.
	const FVector ExpectedDirection = FVector(0.696526, -0.398015, 0.597022).GetUnsafeNormal();
	const FRotator ExpectedRotation = ExpectedDirection.Rotation();

	Test.TestEqual(TEXT("Radius"), Radius, 0.05f);
	Test.TestEqual(TEXT("Resolution"), Resolution, 3.0f);
	Test.TestEqual(TEXT("Begin winch type"), Wire->BeginWinchType, EWireWinchOwnerType::Wire);
	Test.TestEqual(TEXT("Pulled in length"), PulledInLength, 10.0);
	Test.TestEqual(TEXT("Speed"), Speed, 0.5);
	Test.TestEqual(TEXT("Num route nodes"), Wire->RouteNodes.Num(), 23);

	Test.TestEqual(TEXT("GetMotorForceRangeMin"), Winch.GetMotorForceRangeMin(), -10.0);
	Test.TestEqual(TEXT("GetMotorForceRangeMax"), Winch.GetMotorForceRangeMax(), 20.0);
	Test.TestEqual(TEXT("GetBrakeForceRangeMin"), Winch.GetBrakeForceRangeMin(), -100.0);
	Test.TestEqual(TEXT("GetBrakeForceRangeMax"), Winch.GetBrakeForceRangeMax(), 200.0);
	Test.TestEqual(TEXT("IsMotorEnabled"), Winch.IsMotorEnabled(), true);
	Test.TestEqual(TEXT("IsBrakeEnabled"), Winch.IsBrakeEnabled(), true);
	Test.TestEqual(TEXT("Location"), WinchLocation, FVector(0.6f, 0.0f, 0.0f));
	// Somewhat unclear why we need such a big tolerance here. The value is created through
	// computation, both in Python, during import, and in this test, but still 1e-3 is very big.
	AgxAutomationCommon::TestEqual(Test, TEXT("Rotation"), Winch.Rotation, ExpectedRotation, 1e-3f);
	Test.TestEqual(TEXT("bAutoFeed"), Winch.bAutoFeed, false);

	auto RangeHasType =
		[this, &Nodes = Wire->RouteNodes](int32 Begin, int32 End, EWireNodeType Type)
	{
		for (int32 I = Begin; I < End; ++I)
		{
			Test.TestEqual(TEXT("NodeType"), Nodes[I].NodeType, Type);
		}
	};

	// The original route nodes created by the Python script were lost during the wire
	// initialization right before the scene was stored to the AGX Dynamics archive. We can only see
	// the initialized nodes.
	RangeHasType(0, 11, EWireNodeType::Free);
	RangeHasType(11, 12, EWireNodeType::Eye);
	RangeHasType(12, 22, EWireNodeType::Free);
	RangeHasType(22, 23, EWireNodeType::BodyFixed);

	const FString WinchBodyName = Winch.BodyAttachment.Name.ToString();
	Test.TestEqual(TEXT("Winch Body Name"), WinchBodyName, FString("Winch Body"));

	auto RangeHasBody =
		[this, &Nodes = Wire->RouteNodes](int32 Begin, int32 End, const FString& BodyName)
	{
		for (int32 I = Begin; I < End; ++I)
		{
			const FString NodeBodyName = Nodes[I].RigidBody.Name.ToString();
			Test.TestEqual(TEXT("NodeBodyName"), NodeBodyName, BodyName);
		}
	};

	RangeHasBody(0, 11, "None");
	RangeHasBody(11, 12, "Eye Body");
	RangeHasBody(12, 22, "None");
	RangeHasBody(22, 23, "Bead Body");

	return true;
}

/**
 * Remove eveything created by the archive import.
 *
 * @return True when the clearing is complete. Never returns false.
 */
bool FClearWireImportedCommand::Update()
{
	if (Test.Contents == nullptr)
	{
		return true;
	}

#if defined(__linux__)
	/// @todo Workaround for internal issue #213.
	Test.AddExpectedError(
		TEXT("inotify_rm_watch cannot remove descriptor"), EAutomationExpectedErrorFlags::Contains,
		0);
	Test.AddError(TEXT("inotify_rm_watch cannot remove descriptor"));
#endif

	TArray<const TCHAR*> ExpectedFiles {
		TEXT("Blueprint"), TEXT("BP_wire_build.uasset"), TEXT("ShapeMaterial"),
		TEXT("defaultWireMaterial_57.uasset")};

	const FString BaseBlueprintName = Test.Contents->GetName() + FString(".uasset");
	ExpectedFiles.Add(*BaseBlueprintName);

	AgxAutomationCommon::DeleteImportDirectory(TEXT("wire_build"), ExpectedFiles);

	return true;
}

//
// Constraint Dynamic Parameters test starts here.
//

class FImporterToBlueprint_ConstraintDynamicParametersTest;

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FCheckConstraintDynamicParametersImportedCommand,
	FImporterToBlueprint_ConstraintDynamicParametersTest&, Test);

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FClearConstraintDynamicParametersImportedCommand,
	FImporterToBlueprint_ConstraintDynamicParametersTest&, Test);

class FImporterToBlueprint_ConstraintDynamicParametersTest final
	: public AgxAutomationCommon::FAgxAutomationTest
{
public:
	FImporterToBlueprint_ConstraintDynamicParametersTest()
		: AgxAutomationCommon::FAgxAutomationTest(
			  TEXT("FImporterToBlueprint_ConstraintDynamicParametersTest"),
			  TEXT("AGXUnreal.Editor.ImporterToBlueprint.ConstraintDynamicParameters"))
	{
	}

public:
	UWorld* World = nullptr;
	UAGX_Simulation* Simulation = nullptr;
	UBlueprint* Contents = nullptr;
	UAGX_RigidBodyComponent* TrimeshBody = nullptr;

protected:
	virtual bool RunTest(const FString&) override
	{
		BAIL_TEST_IF_NOT_EDITOR(false)
		ADD_LATENT_AUTOMATION_COMMAND(FImportArchiveBlueprintCommand(
			TEXT("constraint_dynamic_parameters_build.agx"), Contents, *this))
		ADD_LATENT_AUTOMATION_COMMAND(FCheckConstraintDynamicParametersImportedCommand(*this))
		ADD_LATENT_AUTOMATION_COMMAND(FClearConstraintDynamicParametersImportedCommand(*this))
		return true;
	}
};

namespace
{
	FImporterToBlueprint_ConstraintDynamicParametersTest
		ImporterToBlueprint_ConstraintDynamicParametersTest;
}

/**
 * Check that the expected state was created during import.
 *
 * The object structure and all numbers tested here should match what is being set in the source
 * script constraint_dynamic_parameters.agxPy.
 * @return true when the check is complete. Never returns false.
 */
bool FCheckConstraintDynamicParametersImportedCommand::Update()
{
	using namespace AgxAutomationCommon;
	if (Test.Contents == nullptr)
	{
		Test.AddError(
			TEXT("Could not import ConstraintDynamicParameters test scene: No content created."));
		return true;
	}

	// Get all the imported components.
	TArray<UActorComponent*> Components =
		FAGX_BlueprintUtilities::GetTemplateComponents(Test.Contents);

	// Two Rigid Bodies, one Hinge constraint, one Default Scene Root and one ModelSourceComponent.
	Test.TestEqual(TEXT("Number of imported components"), Components.Num(), 5);

	UAGX_ConstraintComponent* Constraint = GetByName<UAGX_ConstraintComponent>(
		Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("constraint"));

	// Compliance.
	Test.TestEqual(
		TEXT("Translational 1 compliance"),
		Constraint->GetCompliance(EGenericDofIndex::Translational1), 100.0);
	Test.TestEqual(
		TEXT("Translational 2 compliance"),
		Constraint->GetCompliance(EGenericDofIndex::Translational2), 101.0);
	Test.TestEqual(
		TEXT("Translational 3 compliance"),
		Constraint->GetCompliance(EGenericDofIndex::Translational3), 102.0);
	Test.TestEqual(
		TEXT("Rotational 1 compliance"), Constraint->GetCompliance(EGenericDofIndex::Rotational1),
		103.0);
	Test.TestEqual(
		TEXT("Rotational 2 compliance"), Constraint->GetCompliance(EGenericDofIndex::Rotational2),
		104.0);
	// Rotational 3 is not supported for AGX::Hinge.

	// SpookDamping.
	Test.TestEqual(
		TEXT("Translational 1 damping"), Constraint->SpookDamping.Translational_1, 200.0);
	Test.TestEqual(
		TEXT("Translational 2 damping"), Constraint->SpookDamping.Translational_2, 201.0);
	Test.TestEqual(
		TEXT("Translational 3 damping"), Constraint->SpookDamping.Translational_3, 202.0);
	Test.TestEqual(TEXT("Rotational 1 damping"), Constraint->SpookDamping.Rotational_1, 203.0);
	Test.TestEqual(TEXT("Rotational 2 damping"), Constraint->SpookDamping.Rotational_2, 204.0);
	// Rotational 3 is not supported for AGX::Hinge.

	// Force range.
	Test.TestEqual(
		TEXT("Translational 1 force range min"), Constraint->ForceRange.Translational_1.Min, 300.0);
	Test.TestEqual(
		TEXT("Translational 1 force range max"), Constraint->ForceRange.Translational_1.Max, 301.0);
	Test.TestEqual(
		TEXT("Translational 2 force range min"), Constraint->ForceRange.Translational_2.Min, 302.0);
	Test.TestEqual(
		TEXT("Translational 2 force range max"), Constraint->ForceRange.Translational_2.Max, 303.0);
	Test.TestEqual(
		TEXT("Translational 3 force range min"), Constraint->ForceRange.Translational_3.Min, 304.0);
	Test.TestEqual(
		TEXT("Translational 3 force range max"), Constraint->ForceRange.Translational_3.Max, 305.0);
	Test.TestEqual(
		TEXT("Rotational 1 force range min"), Constraint->ForceRange.Rotational_1.Min, 306.0);
	Test.TestEqual(
		TEXT("Rotational 1 force range min"), Constraint->ForceRange.Rotational_1.Max, 307.0);
	Test.TestEqual(
		TEXT("Rotational 2 force range min"), Constraint->ForceRange.Rotational_2.Min, 308.0);
	Test.TestEqual(
		TEXT("Rotational 2 force range min"), Constraint->ForceRange.Rotational_2.Max, 309.0);
	// Rotational 3 is not supported for AGX::Hinge.

	Test.TestEqual(TEXT("Solve type"), Constraint->SolveType, EAGX_SolveType::StDirectAndIterative);
	Test.TestEqual(TEXT("Enabled"), Constraint->bEnable, true);

	return true;
}

/**
 * Remove everything created by the archive import.
 * @return true when the clearing is complete. Never returns false.
 */
bool FClearConstraintDynamicParametersImportedCommand::Update()
{
	if (Test.Contents == nullptr)
	{
		return true;
	}

#if defined(__linux__)
	/// @todo Workaround for internal issue #213.
	Test.AddExpectedError(
		TEXT("inotify_rm_watch cannot remove descriptor"), EAutomationExpectedErrorFlags::Contains,
		0);
	Test.AddError(TEXT("inotify_rm_watch cannot remove descriptor"));
#endif

	TArray<const TCHAR*> ExpectedFiles {
		TEXT("Blueprint"), TEXT("BP_constraint_dynamic_parameters_build.uasset")};

	const FString BaseBlueprintName = Test.Contents->GetName() + FString(".uasset");
	ExpectedFiles.Add(*BaseBlueprintName);

	AgxAutomationCommon::DeleteImportDirectory(
		TEXT("constraint_dynamic_parameters_build"), ExpectedFiles);

	return true;
}

//
// Rigid Body properties test starts here.
//

class FImporterToBlueprint_RigidBodyPropertiesTest;

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FCheckRigidBodyPropertiesImportedCommand, FImporterToBlueprint_RigidBodyPropertiesTest&, Test);

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FClearRigidBodyPropertiesImportedCommand, FImporterToBlueprint_RigidBodyPropertiesTest&, Test);

class FImporterToBlueprint_RigidBodyPropertiesTest final
	: public AgxAutomationCommon::FAgxAutomationTest
{
public:
	FImporterToBlueprint_RigidBodyPropertiesTest()
		: AgxAutomationCommon::FAgxAutomationTest(
			  TEXT("FImporterToBlueprint_RigidBodyPropertiesTest"),
			  TEXT("AGXUnreal.Editor.ImporterToBlueprint.RigidBodyProperties"))
	{
	}

public:
	UWorld* World = nullptr;
	UAGX_Simulation* Simulation = nullptr;
	UBlueprint* Contents = nullptr;
	UAGX_RigidBodyComponent* TrimeshBody = nullptr;

protected:
	virtual bool RunTest(const FString&) override
	{
		BAIL_TEST_IF_NOT_EDITOR(false)
		ADD_LATENT_AUTOMATION_COMMAND(
			FImportArchiveBlueprintCommand(TEXT("rigidbody_properties_build.agx"), Contents, *this))
		ADD_LATENT_AUTOMATION_COMMAND(FCheckRigidBodyPropertiesImportedCommand(*this))
		ADD_LATENT_AUTOMATION_COMMAND(FClearRigidBodyPropertiesImportedCommand(*this))
		return true;
	}
};

namespace
{
	FImporterToBlueprint_RigidBodyPropertiesTest ImporterToBlueprint_RigidBodyPropertiesTest;
}

/**
 * Check that the expected state was created during import.
 *
 * The object structure and all numbers tested here should match what is being set in the source
 * script rigidbody_properties.agxPy.
 * @return true when the check is complete. Never returns false.
 */
bool FCheckRigidBodyPropertiesImportedCommand::Update()
{
	using namespace AgxAutomationCommon;
	if (Test.Contents == nullptr)
	{
		Test.AddError(TEXT("Could not import RigidBodyProperties test scene: No content created."));
		return true;
	}

	// Get all the imported components.
	TArray<UActorComponent*> Components =
		FAGX_BlueprintUtilities::GetTemplateComponents(Test.Contents);

	// One Rigid Bodies, one Geometry, one Default Scene Root and one Reimport Component.
	Test.TestEqual(TEXT("Number of imported components"), Components.Num(), 4);

	UAGX_RigidBodyComponent* SphereBody = GetByName<UAGX_RigidBodyComponent>(
		Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("SphereBody"));

	// Name.
	{
		Test.TestEqual(
			"Sphere name", SphereBody->GetName(),
			FAGX_BlueprintUtilities::ToTemplateComponentName("SphereBody"));
	}

	// Position.
	{
		FVector Actual = FAGX_BlueprintUtilities::GetTemplateComponentWorldLocation(SphereBody);
		// The position, in AGX Dynamics' units, that was given to the sphere when created.
		FVector ExpectedAgx(10.f, 20.f, 30.f);
		FVector Expected = AgxToUnrealDisplacement(ExpectedAgx);
		Test.TestEqual(TEXT("Sphere position"), Actual, Expected);
	}

	// Rotation.
	{
		FRotator Actual = FAGX_BlueprintUtilities::GetTemplateComponentWorldRotation(SphereBody);
		// The rotation, in AGX Dynamics' units, that was given to the sphere when created.
		FVector ExpectedAgx(0.1f, 0.2f, 0.3f);
		FRotator Expected = AgxToUnrealEulerAngles(ExpectedAgx);
		TestEqual(Test, TEXT("Sphere rotation"), Actual, Expected);
	}

	// Velocity.
	{
		FVector Actual = SphereBody->Velocity;
		// The velocity, in AGX Dynamics' units, that was given to the sphere when created.
		FVector ExpectedAgx(1.f, 2.f, 3.f);
		FVector Expected = AgxToUnrealDisplacement(ExpectedAgx);
		Test.TestEqual(TEXT("Sphere linear velocity"), Actual, Expected);
	}

	// Angular velocity.
	{
		FVector Actual = SphereBody->AngularVelocity;
		// The angular velocity, in AGX Dynamics' units, that was given to the sphere when created.
		FVector ExpectedAgx(1.1f, 1.2f, 1.3f);
		FVector Expected = AgxToUnrealAngularVelocity(ExpectedAgx);
		Test.TestEqual(TEXT("Sphere angular velocity"), Actual, Expected);
	}

	// Linear velocity damping.
	{
		FVector Actual = SphereBody->LinearVelocityDamping;
		FVector Expected(1.0, 2.0, 3.0);
		Test.TestEqual(TEXT("Sphere linear velocity damping"), Actual, Expected);
	}

	// Angular velocity damping.
	{
		FVector Actual = SphereBody->AngularVelocityDamping;
		FVector Expected(4.0, 5.0, 6.0);
		Test.TestEqual(TEXT("Sphere angular velocity damping"), Actual, Expected);
	}

	// Mass.
	{
		Test.TestEqual(TEXT("Sphere mass"), SphereBody->Mass, 500.f);
	}

	// Mass properties automatic generation.
	{
		Test.TestEqual(TEXT("Auto generate mass"), SphereBody->GetAutoGenerateMass(), false);
		Test.TestEqual(
			TEXT("Auto generate CoM offset"), SphereBody->GetAutoGenerateCenterOfMassOffset(),
			true);
		Test.TestEqual(
			TEXT("Auto generate inertia"), SphereBody->GetAutoGeneratePrincipalInertia(), false);
	}

	// Inertia tensor diagonal.
	{
		FVector Actual = SphereBody->GetPrincipalInertia();
		FVector Expected(100.f, 200.f, 300.f);
		Test.TestEqual(TEXT("Sphere inertia tensor diagonal"), Actual, Expected);
	}

	// Motion control.
	{
		EAGX_MotionControl Actual = SphereBody->MotionControl;
		EAGX_MotionControl Expected = EAGX_MotionControl::MC_DYNAMICS;
		Test.TestEqual(TEXT("Sphere motion control"), Actual, Expected);
	}

	// Transform root component.
	{
		Test.TestEqual(
			TEXT("Sphere transform target"), SphereBody->TransformTarget,
			EAGX_TransformTarget::TT_SELF);
	}

	return true;
}

/**
 * Remove everything created by the archive import.
 * @return true when the clearing is complete. Never returns false.
 */
bool FClearRigidBodyPropertiesImportedCommand::Update()
{
	if (Test.Contents == nullptr)
	{
		return true;
	}

#if defined(__linux__)
	/// @todo Workaround for internal issue #213.
	Test.AddExpectedError(
		TEXT("inotify_rm_watch cannot remove descriptor"), EAutomationExpectedErrorFlags::Contains,
		0);
	Test.AddError(TEXT("inotify_rm_watch cannot remove descriptor"));
#endif

	TArray<const TCHAR*> ExpectedFiles {
		TEXT("Blueprint"), TEXT("BP_rigidbody_properties_build.uasset")};

	const FString BaseBlueprintName = Test.Contents->GetName() + FString(".uasset");
	ExpectedFiles.Add(*BaseBlueprintName);

	AgxAutomationCommon::DeleteImportDirectory(TEXT("rigidbody_properties_build"), ExpectedFiles);

	return true;
}

//
// Simple geometries test starts here.
//

class FImporterToBlueprint_SimpleGeometriesTest;

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FCheckSimpleGeometriesImportedCommand, FImporterToBlueprint_SimpleGeometriesTest&, Test);

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FClearSimpleGeometriesImportedCommand, FImporterToBlueprint_SimpleGeometriesTest&, Test);

class FImporterToBlueprint_SimpleGeometriesTest final
	: public AgxAutomationCommon::FAgxAutomationTest
{
public:
	FImporterToBlueprint_SimpleGeometriesTest()
		: AgxAutomationCommon::FAgxAutomationTest(
			  TEXT("FImporterToBlueprint_SimpleGeometriesTest"),
			  TEXT("AGXUnreal.Editor.ImporterToBlueprint.SimpleGeometries"))
	{
	}

public:
	UWorld* World = nullptr;
	UAGX_Simulation* Simulation = nullptr;
	UBlueprint* Contents = nullptr;
	UAGX_RigidBodyComponent* TrimeshBody = nullptr;

protected:
	virtual bool RunTest(const FString&) override
	{
		BAIL_TEST_IF_NOT_EDITOR(false)
		ADD_LATENT_AUTOMATION_COMMAND(
			FImportArchiveBlueprintCommand(TEXT("single_geometries_build.agx"), Contents, *this))
		ADD_LATENT_AUTOMATION_COMMAND(FCheckSimpleGeometriesImportedCommand(*this))
		ADD_LATENT_AUTOMATION_COMMAND(FClearSimpleGeometriesImportedCommand(*this))
		return true;
	}
};

namespace
{
	FImporterToBlueprint_SimpleGeometriesTest ImporterToBlueprint_SimpleGeometriesTest;
}

/**
 * Check that the expected state was created during import.
 *
 * The object structure and all numbers tested here should match what is being set in the source
 * script single_geometries.agxPy.
 * @return true when the check is complete. Never returns false.
 */
bool FCheckSimpleGeometriesImportedCommand::Update()
{
	using namespace AgxAutomationCommon;
	if (Test.Contents == nullptr)
	{
		Test.AddError(TEXT("Could not import SimpleGeometries test scene: No content created."));
		return true;
	}

	auto testShape = [this](USceneComponent* c, const FVector& ExpectedAGXWorldPos)
	{
		Test.TestNotNull(TEXT("Component exists"), c);
		if (c == nullptr)
		{
			return;
		}

		const FVector ExpectedUnrealPos = AgxToUnrealDisplacement(ExpectedAGXWorldPos);
		Test.TestEqual(
			TEXT("Component position"),
			FAGX_BlueprintUtilities::GetTemplateComponentWorldLocation(c), ExpectedUnrealPos);
	};

	// Get all the imported components.
	TArray<UActorComponent*> Components =
		FAGX_BlueprintUtilities::GetTemplateComponents(Test.Contents);

	// 5 Rigid Bodies, 10 Geometries, 2 Static Meshes, one Default Scene Root and one Reimport
	// Component
	Test.TestEqual(TEXT("Number of imported components"), Components.Num(), 19);

	testShape(
		GetByName<UAGX_SphereShapeComponent>(
			Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("sphereGeometry")),
		FVector(0.f, 0.f, 0.f));

	testShape(
		GetByName<UAGX_BoxShapeComponent>(
			Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("boxGeometry")),
		FVector(2.f, 0.f, 0.f));

	testShape(
		GetByName<UAGX_CylinderShapeComponent>(
			Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("cylinderGeometry")),
		FVector(4.f, 0.f, 0.f));

	testShape(
		GetByName<UAGX_CapsuleShapeComponent>(
			Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("capsuleGeometry")),
		FVector(6.f, 0.f, 0.f));

	testShape(
		GetByName<UAGX_TrimeshShapeComponent>(
			Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("trimeshGeometry")),
		FVector(8.f, 0.f, 0.f));

	testShape(
		GetByName<UAGX_SphereShapeComponent>(
			Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("sphereGeometryFree")),
		FVector(0.f, 2.f, 0.f));

	testShape(
		GetByName<UAGX_BoxShapeComponent>(
			Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("boxGeometryFree")),
		FVector(2.f, 2.f, 0.f));

	testShape(
		GetByName<UAGX_CylinderShapeComponent>(
			Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("cylinderGeometryFree")),
		FVector(4.f, 2.f, 0.f));

	testShape(
		GetByName<UAGX_CapsuleShapeComponent>(
			Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("capsuleGeometryFree")),
		FVector(6.f, 2.f, 0.f));

	testShape(
		GetByName<UAGX_TrimeshShapeComponent>(
			Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("trimeshGeometryFree")),
		FVector(8.f, 2.f, 0.f));

	return true;
}

/**
 * Remove everything created by the archive import.
 * @return true when the clearing is complete. Never returns false.
 */
bool FClearSimpleGeometriesImportedCommand::Update()
{
	if (Test.Contents == nullptr)
	{
		return true;
	}

#if defined(__linux__)
	/// @todo Workaround for internal issue #213.
	Test.AddExpectedError(
		TEXT("inotify_rm_watch cannot remove descriptor"), EAutomationExpectedErrorFlags::Contains,
		0);
	Test.AddError(TEXT("inotify_rm_watch cannot remove descriptor"));
#endif

	TArray<const TCHAR*> ExpectedFiles = {
		TEXT("Blueprint"), TEXT("BP_single_geometries_build.uasset"), TEXT("StaticMesh"),
		TEXT("SM_trimeshShape.uasset"), TEXT("SM_trimeshShapeFree.uasset")};

	const FString BaseBlueprintName = Test.Contents->GetName() + FString(".uasset");
	ExpectedFiles.Add(*BaseBlueprintName);

	AgxAutomationCommon::DeleteImportDirectory(TEXT("single_geometries_build"), ExpectedFiles);

	return true;
}

//
// Contact materials test starts here.
//

class FArchiveImporterToBlueprint_ContactMaterialsTest;

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FCheckContactMaterialsImportedCommand, FArchiveImporterToBlueprint_ContactMaterialsTest&, Test);

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FClearContactMaterialsImportedCommand, FArchiveImporterToBlueprint_ContactMaterialsTest&, Test);

class FArchiveImporterToBlueprint_ContactMaterialsTest final
	: public AgxAutomationCommon::FAgxAutomationTest
{
public:
	FArchiveImporterToBlueprint_ContactMaterialsTest()
		: AgxAutomationCommon::FAgxAutomationTest(
			  TEXT("FArchiveImporterToBlueprint_ContactMaterialsTest"),
			  TEXT("AGXUnreal.Editor.ImporterToBlueprint.ContactMaterials"))
	{
	}

public:
	UBlueprint* Contents = nullptr;

protected:
	virtual bool RunTest(const FString&) override
	{
		BAIL_TEST_IF_NOT_EDITOR(false)
		ADD_LATENT_AUTOMATION_COMMAND(
			FImportArchiveBlueprintCommand(TEXT("contact_materials_build.agx"), Contents, *this))
		ADD_LATENT_AUTOMATION_COMMAND(FCheckContactMaterialsImportedCommand(*this))
		ADD_LATENT_AUTOMATION_COMMAND(FClearContactMaterialsImportedCommand(*this))
		return true;
	}
};

namespace
{
	FArchiveImporterToBlueprint_ContactMaterialsTest
		ArchiveImporterToBlueprint_ContactMaterialsTest;
}

/**
 * Check that the expected state was created during import.
 *
 * The object structure and all numbers tested here should match what is being set in the source
 * script contact_materials.agxPy.
 * @return true when the check is complete. Never returns false.
 */
bool FCheckContactMaterialsImportedCommand::Update()
{
	using namespace AgxAutomationCommon;
	if (Test.Contents == nullptr)
	{
		Test.AddError(TEXT("Could not import ContactMaterials test scene: No content created."));
		return true;
	}

	// Get all the imported components.
	TArray<UActorComponent*> Components =
		FAGX_BlueprintUtilities::GetTemplateComponents(Test.Contents);

	// 4 Rigid Bodies, 4 Geometries, 1 Contact Material Registrar, one Default Scene Root and one
	// Reimport Component.
	Test.TestEqual(TEXT("Number of imported components"), Components.Num(), 11);

	UAGX_ContactMaterialRegistrarComponent* Registrar =
		GetByName<UAGX_ContactMaterialRegistrarComponent>(
			Components,
			*FAGX_BlueprintUtilities::ToTemplateComponentName("AGX_ContactMaterialRegistrar"));

	Test.TestNotNull("Contact Material Registrar", Registrar);
	if (Registrar == nullptr)
	{
		// Abort the test. It will fail since TestNotNull above will have failed.
		return true;
	}

	Test.TestEqual("Num Contact Materials in Registrar", Registrar->ContactMaterials.Num(), 2);

	UAGX_ContactMaterial** Cm1 = Registrar->ContactMaterials.FindByPredicate(
		[](UAGX_ContactMaterial* Cm) { return Cm->GetName() == "CM_Mat1_Mat2"; });
	Test.TestNotNull("Cm1", Cm1);
	if (Cm1 == nullptr)
	{
		// Abort the test. It will fail since TestNotNull above will have failed.
		return true;
	}

	Test.TestNotNull("Cm1 Material1", (*Cm1)->Material1);
	if ((*Cm1)->Material1 == nullptr)
		return true;

	Test.TestNotNull("Cm1 Material2", (*Cm1)->Material2);
	if (IsAnyNullptr((*Cm1)->Material1, (*Cm1)->Material2))
	{
		// Abort the test. It will fail since TestNotNull above will have failed.
		return true;
	}

	Test.TestEqual("Cm1 Material1 name", (*Cm1)->Material1->GetName(), "Mat1");
	Test.TestEqual("Cm1 Material2 name", (*Cm1)->Material2->GetName(), "Mat2");
	Test.TestEqual("Cm1 contact solver", (*Cm1)->ContactSolver, EAGX_ContactSolver::Iterative);
	Test.TestEqual(
		"Cm1 friction model", (*Cm1)->FrictionModel,
		EAGX_FrictionModel::IterativeProjectedConeFriction);
	Test.TestEqual(
		"Cm1 contact reduction bin resolution", (*Cm1)->ContactReduction.ContactReductionLevel,
		EAGX_ContactReductionLevel::Moderate);
	Test.TestEqual("Cm1 friction coefficient", (*Cm1)->FrictionCoefficient, 0.11);
	Test.TestEqual("Cm1 surface viscosity", (*Cm1)->SurfaceViscosity, 0.12);
	Test.TestEqual("Cm1 restitution", (*Cm1)->Restitution, 0.13);
	Test.TestEqual("Cm1 youngs modulus", (*Cm1)->YoungsModulus, 123451234.0);
	Test.TestEqual("Cm1 damping", (*Cm1)->SpookDamping, 0.14);
	Test.TestEqual("Cm1 adhesive force", (*Cm1)->AdhesiveForce, 0.15);
	Test.TestEqual("Cm1 adhesive overlap", (*Cm1)->AdhesiveOverlap, AgxToUnrealDistance(0.16));

	UAGX_ContactMaterial** Cm2 = Registrar->ContactMaterials.FindByPredicate(
		[](UAGX_ContactMaterial* Cm) { return Cm->GetName() == "CM_Mat3_Mat4"; });
	Test.TestNotNull("Cm2", Cm2);
	if (Cm2 == nullptr)
	{
		// Abort the test. It will fail since TestNotNull above will have failed.
		return true;
	}

	Test.TestNotNull("Cm2 Material1", (*Cm2)->Material1);
	if ((*Cm2)->Material1 == nullptr)
		return true;

	Test.TestNotNull("Cm2 Material2", (*Cm2)->Material2);
	if (IsAnyNullptr((*Cm2)->Material1, (*Cm2)->Material2))
	{
		// Abort the test. It will fail since TestNotNull above will have failed.
		return true;
	}

	Test.TestEqual("Cm2 Material1 name", (*Cm2)->Material1->GetName(), "Mat3");
	Test.TestEqual("Cm2 Material2 name", (*Cm2)->Material2->GetName(), "Mat4");
	Test.TestEqual("Cm2 contact solver", (*Cm2)->ContactSolver, EAGX_ContactSolver::Direct);
	Test.TestEqual("Cm2 friction model", (*Cm2)->FrictionModel, EAGX_FrictionModel::BoxFriction);
	Test.TestEqual(
		"Cm2 contact reduction bin resolution", (*Cm2)->ContactReduction.ContactReductionLevel,
		EAGX_ContactReductionLevel::Minimal);
	Test.TestEqual("Cm2 friction coefficient", (*Cm2)->FrictionCoefficient, 0.21);
	Test.TestEqual("Cm2 surface viscosity", (*Cm2)->SurfaceViscosity, 0.22);
	Test.TestEqual("Cm2 restitution", (*Cm2)->Restitution, 0.23);
	Test.TestEqual("Cm2 youngs modulus", (*Cm2)->YoungsModulus, 101010101.0);
	Test.TestEqual("Cm2 damping", (*Cm2)->SpookDamping, 0.24);
	Test.TestEqual("Cm2 adhesive force", (*Cm2)->AdhesiveForce, 0.25);
	Test.TestEqual("Cm2 adhesive overlap", (*Cm2)->AdhesiveOverlap, AgxToUnrealDistance(0.26));

	return true;
}

/**
 * Remove everything created by the archive import.
 * @return true when the clearing is complete. Never returns false.
 */
bool FClearContactMaterialsImportedCommand::Update()
{
	if (Test.Contents == nullptr)
	{
		return true;
	}

#if defined(__linux__)
	/// @todo Workaround for internal issue #213.
	Test.AddExpectedError(
		TEXT("inotify_rm_watch cannot remove descriptor"), EAutomationExpectedErrorFlags::Contains,
		0);
	Test.AddError(TEXT("inotify_rm_watch cannot remove descriptor"));
#endif

	TArray<const TCHAR*> ExpectedFiles = {
		TEXT("Blueprint"),			 TEXT("BP_contact_materials_build.uasset"),
		TEXT("ContactMaterial"),	 TEXT("CM_Mat1_Mat2.uasset"),
		TEXT("CM_Mat3_Mat4.uasset"), TEXT("ShapeMaterial"),
		TEXT("Mat1.uasset"),		 TEXT("Mat2.uasset"),
		TEXT("Mat3.uasset"),		 TEXT("Mat4.uasset")};

	const FString BaseBlueprintName = Test.Contents->GetName() + FString(".uasset");
	ExpectedFiles.Add(*BaseBlueprintName);

	AgxAutomationCommon::DeleteImportDirectory(TEXT("contact_materials_build"), ExpectedFiles);

	return true;
}

//
// Observer Frame test starts here.
//

class FArchiveImporterToBlueprint_ObserverFramesTest;

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FCheckObserverFramesImportedCommand, FArchiveImporterToBlueprint_ObserverFramesTest&, Test);

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FClearObserverFramesImportedCommand, FArchiveImporterToBlueprint_ObserverFramesTest&, Test);

class FArchiveImporterToBlueprint_ObserverFramesTest final
	: public AgxAutomationCommon::FAgxAutomationTest
{
public:
	FArchiveImporterToBlueprint_ObserverFramesTest()
		: AgxAutomationCommon::FAgxAutomationTest(
			  TEXT("FArchiveImpoterToBlueprint_ObserverFramesTest"),
			  TEXT("AGXUnreal.Editor.ImporterToBlueprint.ObserverFrames"))
	{
	}

public:
	UBlueprint* Contents = nullptr;

protected:
	virtual bool RunTest(const FString&) override
	{
		BAIL_TEST_IF_NOT_EDITOR(false)
		ADD_LATENT_AUTOMATION_COMMAND(
			FImportArchiveBlueprintCommand(TEXT("observer_frames_build.agx"), Contents, *this))
		ADD_LATENT_AUTOMATION_COMMAND(FCheckObserverFramesImportedCommand(*this))
		ADD_LATENT_AUTOMATION_COMMAND(FClearObserverFramesImportedCommand(*this))
		return true;
	}
};

namespace
{
	FArchiveImporterToBlueprint_ObserverFramesTest ArchiveImpoterterToBlueprint_ObserverFramesTest;
}

/**
 * Check that the expected state was created during import.
 *
 * The object structure and all numbers tested here should match what is being set in the source
 * script TestScenes/observer_frames.agxPy.
 */
bool FCheckObserverFramesImportedCommand::Update()
{
	using namespace AgxAutomationCommon;
	if (Test.Contents == nullptr)
	{
		Test.AddError(TEXT("Could not import ObserverFrames test scene: No content created."));
		return true;
	}

	// Get all the imported Components.
	TArray<UActorComponent*> Components =
		FAGX_BlueprintUtilities::GetTemplateComponents(Test.Contents);

	// 1 Default Scene Root, 4 groups each containing a Rigid Body, a Shape, a Scene and one
	// Reimport Component.
	Test.TestEqual(TEXT("Number of imported Components"), Components.Num(), 14);

	auto TestGroup =
		[this, &Components](int32 Id, const FVector& BodyLocation, const FVector& ObserverLocation)
	{
		const FString BodyName = *FString::Printf(TEXT("Body_%d"), Id);
		const FString GeometryName = *FString::Printf(TEXT("Geometry_%d"), Id);
		const FString ObserverName = *FString::Printf(TEXT("Observer_%d"), Id);
		UAGX_RigidBodyComponent* Body = GetByName<UAGX_RigidBodyComponent>(
			Components, *FAGX_BlueprintUtilities::ToTemplateComponentName(BodyName));
		UAGX_BoxShapeComponent* Geometry = GetByName<UAGX_BoxShapeComponent>(
			Components, *FAGX_BlueprintUtilities::ToTemplateComponentName(GeometryName));
		USceneComponent* Observer = GetByName<USceneComponent>(
			Components, *FAGX_BlueprintUtilities::ToTemplateComponentName(ObserverName));

		Test.TestNotNull(*BodyName, Body);
		Test.TestNotNull(*GeometryName, Geometry);
		Test.TestNotNull(*ObserverName, Observer);
		if (IsAnyNullptr(Body, Geometry, Observer))
		{
			return;
		}

		USceneComponent* BodyAsComponent = static_cast<USceneComponent*>(Body);
		Test.TestEqual(
			*FString::Printf(TEXT("%s parent"), *GeometryName),
			Cast<USceneComponent>(
				FAGX_BlueprintUtilities::GetTemplateComponentAttachParent(Geometry)),
			BodyAsComponent);
		Test.TestEqual(
			*FString::Printf(TEXT("%s parent"), *ObserverName),
			Cast<USceneComponent>(
				FAGX_BlueprintUtilities::GetTemplateComponentAttachParent(Observer)),
			BodyAsComponent);

		Test.TestEqual(
			*FString::Printf(TEXT("%s location"), *BodyName), Body->GetRelativeLocation(),
			BodyLocation);

		Test.TestEqual(
			*FString::Printf(TEXT("%s location"), *ObserverName), Observer->GetRelativeLocation(),
			ObserverLocation);
	};

	TestGroup(1, AgxToUnrealDisplacement(0.0, 0.0, 0.0), AgxToUnrealDisplacement(0.0, 0.0, 0.0));
	TestGroup(2, AgxToUnrealDisplacement(1.0, 0.0, 0.0), AgxToUnrealDisplacement(0.3, 0.3, 0.3));
	TestGroup(3, AgxToUnrealDisplacement(2.0, 0.0, 0.0), AgxToUnrealDisplacement(0.3, 0.3, 0.3));

	FRotator Rotation = AgxToUnrealEulerAngles(PI / 10, 0.0, 0.0);
	FVector ObserverLocation = Rotation.RotateVector(AgxToUnrealDisplacement(0.3, 0.3, 0.3));
	TestGroup(4, AgxToUnrealDisplacement(3.0, 0.0, 0.0), ObserverLocation);

	// Tests/Test_ArchiveImport.umap

	return true;
}

/**
 * Remove everything created by the archive import.
 * @return true when the clearing is complete. Never returns false.
 */
bool FClearObserverFramesImportedCommand::Update()
{
	if (Test.Contents == nullptr)
	{
		return true;
	}

#if defined(__linux__)
	/// @todo Workaround for internal issue #213.
	Test.AddExpectedError(
		TEXT("inotify_rm_watch cannot remove descriptor"), EAutomationExpectedErrorFlags::Contains,
		0);
	Test.AddError(TEXT("inotify_rm_watch cannot remove descriptor"));
#endif

	TArray<const TCHAR*> ExpectedFiles {TEXT("Blueprint"), TEXT("BP_observer_frames_build.uasset")};

	const FString BaseBlueprintName = Test.Contents->GetName() + FString(".uasset");
	ExpectedFiles.Add(*BaseBlueprintName);

	AgxAutomationCommon::DeleteImportDirectory(TEXT("observer_frames_build"), ExpectedFiles);

	return true;
}
//
// URDF link with meshes test starts here.
//

class FImporterToBlueprint_URDFLinkWithMeshesTest;

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FCheckURDFLinkWithMeshesImportedCommand, FImporterToBlueprint_URDFLinkWithMeshesTest&, Test);

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FClearURDFLinkWithMeshesImportedCommand, FImporterToBlueprint_URDFLinkWithMeshesTest&, Test);

class FImporterToBlueprint_URDFLinkWithMeshesTest final
	: public AgxAutomationCommon::FAgxAutomationTest
{
public:
	FImporterToBlueprint_URDFLinkWithMeshesTest()
		: AgxAutomationCommon::FAgxAutomationTest(
			  TEXT("FImporterToBlueprint_URDFLinkWithMeshesTest"),
			  TEXT("AGXUnreal.Editor.ImporterToBlueprint.URDFLinkWithMeshes"))
	{
	}

public:
	UWorld* World = nullptr;
	UAGX_Simulation* Simulation = nullptr;
	UBlueprint* Contents = nullptr;

protected:
	virtual bool RunTest(const FString&) override
	{
		BAIL_TEST_IF_NOT_EDITOR(false)
		ADD_LATENT_AUTOMATION_COMMAND(FImportURDFBlueprintCommand(
			TEXT("link_with_meshes.urdf"), AgxAutomationCommon::GetTestSceneDirPath(), Contents,
			*this))
		ADD_LATENT_AUTOMATION_COMMAND(FCheckURDFLinkWithMeshesImportedCommand(*this))
		ADD_LATENT_AUTOMATION_COMMAND(FClearURDFLinkWithMeshesImportedCommand(*this))
		return true;
	}
};

namespace
{
	FImporterToBlueprint_URDFLinkWithMeshesTest ImporterToBlueprint_URDFLinkWithMeshesTest;
}

/**
 * Check that the expected state was created during import.
 *
 * The object structure and all numbers tested here should match what is being set in the source
 * script link_with_meshes.agxPy.
 * @return true when the check is complete. Never returns false.
 */
bool FCheckURDFLinkWithMeshesImportedCommand::Update()
{
	using namespace AgxAutomationCommon;
	if (Test.Contents == nullptr)
	{
		Test.AddError(TEXT("Could not import URDFLinkWithMeshes test scene: No content created."));
		return true;
	}

	// Get all the imported components.
	TArray<UActorComponent*> Components =
		FAGX_BlueprintUtilities::GetTemplateComponents(Test.Contents);

	// One DefaultSceneRoot, one Rigid Body, one Trimesh with a render mesh and a collision mesh,
	// one Trimesh with only one collision mesh and one Reimport Component.
	Test.TestEqual("Number of components", Components.Num(), 8);

	UAGX_TrimeshShapeComponent* Urdfmeshvisual = GetByName<UAGX_TrimeshShapeComponent>(
		Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("urdfmeshvisual"));
	UAGX_TrimeshShapeComponent* Urdfmeshcollision = GetByName<UAGX_TrimeshShapeComponent>(
		Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("urdfmeshcollision"));

	Test.TestNotNull(TEXT("Urdfmeshvisual"), Urdfmeshvisual);
	Test.TestNotNull(TEXT("Urdfmeshcollision"), Urdfmeshcollision);
	if (IsAnyNullptr(Urdfmeshvisual, Urdfmeshcollision))
	{
		// Abort the test. It will fail since TestNotNull above will have failed.
		return true;
	}

	Test.TestFalse("Urdfmeshvisual collide", Urdfmeshvisual->bCanCollide);
	Test.TestTrue("Urdfmeshcollision collide", Urdfmeshcollision->bCanCollide);

	return true;
}

/**
 * Remove everything created by the archive import.
 * @return true when the clearing is complete. Never returns false.
 */
bool FClearURDFLinkWithMeshesImportedCommand::Update()
{
	using namespace AgxAutomationCommon;

	if (Test.Contents == nullptr)
	{
		return true;
	}

	TArray<UActorComponent*> Components =
		FAGX_BlueprintUtilities::GetTemplateComponents(Test.Contents);
	TArray<FString> Assets = GetReferencedStaticMeshAssets(Components);
	if (Assets.Num() != 3)
	{
		Test.AddError(TEXT("Unexpected number of assets found."));
		return true;
	}

	TArray<const TCHAR*> FilesAndDirsToRemove;
	FilesAndDirsToRemove.Add(TEXT("Blueprint"));
	FilesAndDirsToRemove.Add(TEXT("BP_link_with_meshes.uasset"));
	FilesAndDirsToRemove.Add(TEXT("RenderMesh"));
	FilesAndDirsToRemove.Add(TEXT("StaticMesh"));
	for (const FString& Asset : Assets)
	{
		FilesAndDirsToRemove.Add(*Asset);
	}

	const FString BaseBlueprintName = Test.Contents->GetName() + FString(".uasset");
	FilesAndDirsToRemove.Add(*BaseBlueprintName);

#if defined(__linux__)
	/// @todo Workaround for internal issue #213.
	Test.AddExpectedError(
		TEXT("inotify_rm_watch cannot remove descriptor"), EAutomationExpectedErrorFlags::Contains,
		0);
	Test.AddError(TEXT("inotify_rm_watch cannot remove descriptor"));
#endif

	AgxAutomationCommon::DeleteImportDirectory(TEXT("link_with_meshes"), FilesAndDirsToRemove);

	return true;
}

//
// URDF links geometries constraints test starts here.
//

class FImporterToBlueprint_URDFLinksGeometriesConstraintsTest;

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FCheckURDFLinksGeometriesConstraintsImportedCommand,
	FImporterToBlueprint_URDFLinksGeometriesConstraintsTest&, Test);

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FClearURDFLinksGeometriesConstraintsImportedCommand,
	FImporterToBlueprint_URDFLinksGeometriesConstraintsTest&, Test);

class FImporterToBlueprint_URDFLinksGeometriesConstraintsTest final
	: public AgxAutomationCommon::FAgxAutomationTest
{
public:
	FImporterToBlueprint_URDFLinksGeometriesConstraintsTest()
		: AgxAutomationCommon::FAgxAutomationTest(
			  TEXT("FImporterToBlueprint_URDFLinksGeometriesConstraintsTest"),
			  TEXT("AGXUnreal.Editor.ImporterToBlueprint.URDFLinksGeometriesConstraints"))
	{
	}

public:
	UWorld* World = nullptr;
	UAGX_Simulation* Simulation = nullptr;
	UBlueprint* Contents = nullptr;

protected:
	virtual bool RunTest(const FString&) override
	{
		BAIL_TEST_IF_NOT_EDITOR(false)
		ADD_LATENT_AUTOMATION_COMMAND(FImportURDFBlueprintCommand(
			TEXT("links_geometries_constraints.urdf"), "", Contents, *this))
		ADD_LATENT_AUTOMATION_COMMAND(FCheckURDFLinksGeometriesConstraintsImportedCommand(*this))
		ADD_LATENT_AUTOMATION_COMMAND(FClearURDFLinksGeometriesConstraintsImportedCommand(*this))
		return true;
	}
};

namespace
{
	FImporterToBlueprint_URDFLinksGeometriesConstraintsTest
		ImporterToBlueprint_URDFLinksGeometriesConstraintsTest;
}

/**
 * Check that the expected state was created during import.
 *
 * The object structure and all numbers tested here should match what is being set in the source
 * script links_geometries_constraints.agxPy.
 * @return true when the check is complete. Never returns false.
 */
bool FCheckURDFLinksGeometriesConstraintsImportedCommand::Update()
{
	using namespace AgxAutomationCommon;
	if (Test.Contents == nullptr)
	{
		Test.AddError(TEXT(
			"Could not import URDFLinksGeometriesConstraints test scene: No content created."));
		return true;
	}

	// Get all the imported components.
	TArray<UActorComponent*> Components =
		FAGX_BlueprintUtilities::GetTemplateComponents(Test.Contents);

	// 1 DefaultSceneRoot, 4 Rigid Bodies, 4 Shape Components, 2 Constraints and one Reimport
	// Component.
	Test.TestEqual("Number of components", Components.Num(), 12);

	UAGX_RigidBodyComponent* Boxlink = GetByName<UAGX_RigidBodyComponent>(
		Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("boxlink"));
	UAGX_RigidBodyComponent* Shperelink = GetByName<UAGX_RigidBodyComponent>(
		Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("spherelink"));
	UAGX_RigidBodyComponent* Cylinderlink = GetByName<UAGX_RigidBodyComponent>(
		Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("cylinderlink"));
	UAGX_RigidBodyComponent* Freefallinglink = GetByName<UAGX_RigidBodyComponent>(
		Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("freefallinglink"));

	Test.TestNotNull(TEXT("Boxlink"), Boxlink);
	Test.TestNotNull(TEXT("Shperelink"), Shperelink);
	Test.TestNotNull(TEXT("Cylinderlink"), Cylinderlink);
	Test.TestNotNull(TEXT("Freefallinglink"), Freefallinglink);
	if (IsAnyNullptr(Boxlink, Shperelink, Cylinderlink, Freefallinglink))
	{
		Test.AddError("At least one Rigid Body was nullptr, cannot continue.");
		return true;
	}

	if (Boxlink == nullptr || Shperelink == nullptr || Cylinderlink == nullptr ||
		Freefallinglink == nullptr)
	{
		Test.AddError("At least one Rigid Body was nullptr, cannot continue.");
		return true;
	}

	Test.TestEqual(
		TEXT("Boxlink position"),
		FAGX_BlueprintUtilities::GetTemplateComponentWorldLocation(Boxlink),
		AgxToUnrealDisplacement({0.f, 0.f, 0.f}));

	Test.TestEqual(
		TEXT("Shperelink position"),
		FAGX_BlueprintUtilities::GetTemplateComponentWorldLocation(Shperelink),
		AgxToUnrealDisplacement({1.f, 0.f, 0.f}));

	Test.TestEqual(
		TEXT("Cylinderlink position"),
		FAGX_BlueprintUtilities::GetTemplateComponentWorldLocation(Cylinderlink),
		AgxToUnrealDisplacement({2.f, 0.f, 0.f}));

	Test.TestEqual(
		TEXT("Freefallinglink position"),
		FAGX_BlueprintUtilities::GetTemplateComponentWorldLocation(Freefallinglink),
		AgxToUnrealDisplacement({0.f, 0.f, 0.f}));

	return true;
}

/**
 * Remove everything created by the archive import.
 * @return true when the clearing is complete. Never returns false.
 */
bool FClearURDFLinksGeometriesConstraintsImportedCommand::Update()
{
	if (Test.Contents == nullptr)
	{
		return true;
	}

#if defined(__linux__)
	/// @todo Workaround for internal issue #213.
	Test.AddExpectedError(
		TEXT("inotify_rm_watch cannot remove descriptor"), EAutomationExpectedErrorFlags::Contains,
		0);
	Test.AddError(TEXT("inotify_rm_watch cannot remove descriptor"));
#endif

	TArray<const TCHAR*> ExpectedFiles {
		TEXT("Blueprint"), TEXT("BP_links_geometries_constraints.uasset")};

	const FString BaseBlueprintName = Test.Contents->GetName() + FString(".uasset");
	ExpectedFiles.Add(*BaseBlueprintName);

	AgxAutomationCommon::DeleteImportDirectory(TEXT("links_geometries_constraints"), ExpectedFiles);

	return true;
}

//
// Track test starts here.
//

class FImporterToBlueprint_TrackTest;

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FCheckTrackImportedCommand, FImporterToBlueprint_TrackTest&, Test);

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FClearTrackImportedCommand, FImporterToBlueprint_TrackTest&, Test);

class FImporterToBlueprint_TrackTest final : public AgxAutomationCommon::FAgxAutomationTest
{
public:
	FImporterToBlueprint_TrackTest()
		: AgxAutomationCommon::FAgxAutomationTest(
			  TEXT("FImporterToBlueprint_TrackTest"),
			  TEXT("AGXUnreal.Editor.ImporterToBlueprint.Track"))
	{
	}

public:
	UWorld* World = nullptr;
	UAGX_Simulation* Simulation = nullptr;
	UBlueprint* Contents = nullptr;

protected:
	virtual bool RunTest(const FString&) override
	{
		BAIL_TEST_IF_NOT_EDITOR(false)
		ADD_LATENT_AUTOMATION_COMMAND(
			FImportArchiveBlueprintCommand(TEXT("track_build.agx"), Contents, *this))
		ADD_LATENT_AUTOMATION_COMMAND(FCheckTrackImportedCommand(*this))
		ADD_LATENT_AUTOMATION_COMMAND(FClearTrackImportedCommand(*this))
		return true;
	}
};

namespace
{
	FImporterToBlueprint_TrackTest ImporterToBlueprint_TrackTest;
}

/**
 * Check that the expected state was created during import.
 *
 * The object structure and all numbers tested here should match what is being set in the source
 * script track.agxPy.
 *
 * @return True when the check is complete. Never returns false.
 */
bool FCheckTrackImportedCommand::Update()
{
	using namespace AgxAutomationCommon;
	if (Test.Contents == nullptr)
	{
		Test.AddError(TEXT("Could not import Track test scene: No content created"));
		return true;
	}

	// Get all the imported components.
	TArray<UActorComponent*> Components =
		FAGX_BlueprintUtilities::GetTemplateComponents(Test.Contents);

	// 24 Hinge Constraints (24), 25 Rigid Bodies (49), 20 Sphere Shapes
	// (69), 24 Cylinder Shapes (93), 3 Box Shapes (96), a Collision Group Disabler (97), a
	// Contact Material Registrar (98), a Default Scene Root (99), two Tracks (101) and one Reimport
	// Component(102).
	Test.TestEqual(TEXT("Number of imported components"), Components.Num(), 102);
	if (Components.Num() != 102)
	{
		UE_LOG(LogAGX, Warning, TEXT("Found the following components:"));
		for (auto Component : Components)
		{
			UE_LOG(
				LogAGX, Warning, TEXT("  %s: %s"), *Component->GetName(),
				*Component->GetClass()->GetName());
		}
	}

	auto TestTrack = [&](UAGX_TrackComponent* Track) -> bool
	{
		Test.TestNotNull("Track Component", Track);
		if (Track == nullptr)
		{
			return true;
		}

		Test.TestEqual("Number Of Nodes", Track->NumberOfNodes, 120);
		Test.TestEqual("Width", Track->Width, 35.f);
		Test.TestEqual("Thickness", Track->Thickness, 2.5f);
		Test.TestEqual("Initial Distance Tension", Track->InitialDistanceTension, 0.1f);
		Test.TestNotNull("Shape Material", Track->ShapeMaterial);

		// Track Properties.
		Test.TestNotNull("Track Properties", Track->TrackProperties);
		if (Track->TrackProperties == nullptr)
		{
			return true;
		}

		FString BeautifiedTrackName = Track->GetName();
		BeautifiedTrackName.RemoveFromEnd(UActorComponent::ComponentTemplateNameSuffix);
		Test.TestEqual(
			"Track Properties Name", Track->TrackProperties->GetName(),
			FString("AGX_TP_") + BeautifiedTrackName);
		Test.TestEqual(
			"Hinge Compliance Translational X",
			Track->TrackProperties->HingeComplianceTranslational_X, 2e-10);
		Test.TestEqual(
			"Hinge Compliance Translational Y",
			Track->TrackProperties->HingeComplianceTranslational_Y, 3e-10);
		Test.TestEqual(
			"Hinge Compliance Translational Z",
			Track->TrackProperties->HingeComplianceTranslational_Z, 4e-10);
		Test.TestEqual(
			"Hinge Compliance Rotational X", Track->TrackProperties->HingeComplianceRotational_X,
			5e-10);
		Test.TestEqual(
			"Hinge Compliance Rotational Y", Track->TrackProperties->HingeComplianceRotational_Y,
			6e-10);
		Test.TestEqual(
			"Hinge Damping Translational X",
			Track->TrackProperties->HingeSpookDampingTranslational_X, 0.01);
		Test.TestEqual(
			"Hinge Damping Translational Y",
			Track->TrackProperties->HingeSpookDampingTranslational_Y, 0.02);
		Test.TestEqual(
			"Hinge Damping Translational Z",
			Track->TrackProperties->HingeSpookDampingTranslational_Z, 0.03);
		Test.TestEqual(
			"Hinge Damping Rotational X", Track->TrackProperties->HingeSpookDampingRotational_X,
			0.04);
		Test.TestEqual(
			"Hinge Damping Rotational Y", Track->TrackProperties->HingeSpookDampingRotational_Y,
			0.05);
		Test.TestEqual("Hinge Range Enabled", Track->TrackProperties->bEnableHingeRange, false);
		Test.TestEqual("Hinge Range Min", Track->TrackProperties->HingeRange.Min, -120.0);
		Test.TestEqual("Hinge Range Max", Track->TrackProperties->HingeRange.Max, 20.0);
		Test.TestEqual(
			"On Initialize Merge Nodes to Wheels Enabled",
			Track->TrackProperties->bEnableOnInitializeMergeNodesToWheels, false);
		Test.TestEqual(
			"On Initialize Transform Nodes to Wheels Enabled",
			Track->TrackProperties->bEnableOnInitializeTransformNodesToWheels, true);
		Test.TestEqual(
			"Transform Nodes to Wheels Overlap",
			Track->TrackProperties->TransformNodesToWheelsOverlap, 0.2);
		Test.TestEqual(
			"Nodes to Wheels Merge Threshold", Track->TrackProperties->NodesToWheelsMergeThreshold,
			-0.14);
		Test.TestEqual(
			"Nodes to Wheels Split Threshold", Track->TrackProperties->NodesToWheelsSplitThreshold,
			-0.17);
		Test.TestEqual(
			"Num Nodes Included in Average Direction",
			Track->TrackProperties->NumNodesIncludedInAverageDirection, 5);
		Test.TestEqual(
			"Min Stabilizing Hinge Normal Force",
			Track->TrackProperties->MinStabilizingHingeNormalForce, 110.0);
		Test.TestEqual(
			"Stabilizing Hinge Friction Parameter",
			Track->TrackProperties->StabilizingHingeFrictionParameter, 0.005);

		// Internal Merge Properties.
		Test.TestNotNull("Internal Merge Properties", Track->InternalMergeProperties);
		if (Track->InternalMergeProperties == nullptr)
		{
			return true;
		}

		Test.TestEqual(
			"Internal Merge Properties Name", Track->InternalMergeProperties->GetName(),
			FString("AGX_TIMP_") + BeautifiedTrackName);
		Test.TestEqual("Enable Merge", Track->InternalMergeProperties->bEnableMerge, true);
		Test.TestEqual(
			"Num Nodes Per Merge Segment", Track->InternalMergeProperties->NumNodesPerMergeSegment,
			3);
		Test.TestEqual(
			"Contact Reduction", Track->InternalMergeProperties->ContactReduction,
			EAGX_MergedTrackNodeContactReduction::Moderate);
		Test.TestEqual(
			"Enable Lock to Reach Merge Condition",
			Track->InternalMergeProperties->bEnableLockToReachMergeCondition, true);
		Test.TestEqual(
			"Lock to Reach Merge Condition Compliance",
			Track->InternalMergeProperties->LockToReachMergeConditionCompliance, 1e-11);
		Test.TestEqual(
			"Lock to Reach Merge Condition Damping",
			Track->InternalMergeProperties->LockToReachMergeConditionSpookDamping, 0.05);
		Test.TestEqual(
			"Max Angle Merge Condition", Track->InternalMergeProperties->MaxAngleMergeCondition,
			FMath::RadiansToDegrees(0.00001));

		// Wheels
		Test.TestEqual("Number of Wheels", Track->Wheels.Num(), 12);
		for (const FAGX_TrackWheel& Wheel : Track->Wheels)
		{
			Test.TestEqual("Rigid Body Name", Wheel.RigidBody.Name.IsNone(), false);
		}

		Test.TestEqual(
			"Number of Roller Wheels",
			Track->Wheels
				.FilterByPredicate([](const FAGX_TrackWheel& Wheel)
								   { return Wheel.Model == EAGX_TrackWheelModel::Roller; })
				.Num(),
			10);
		Test.TestEqual(
			"Number of Sprocket Wheels",
			Track->Wheels
				.FilterByPredicate([](const FAGX_TrackWheel& Wheel)
								   { return Wheel.Model == EAGX_TrackWheelModel::Sprocket; })
				.Num(),
			1);
		Test.TestEqual(
			"Number of Idler Wheels",
			Track->Wheels
				.FilterByPredicate([](const FAGX_TrackWheel& Wheel)
								   { return Wheel.Model == EAGX_TrackWheelModel::Idler; })
				.Num(),
			1);
		Test.TestEqual(
			"Number of Wheels with Radius 30",
			Track->Wheels
				.FilterByPredicate([](const FAGX_TrackWheel& Wheel)
								   { return Wheel.Radius == 30.f; })
				.Num(),
			2);
		Test.TestEqual(
			"Number of Wheels with Radius 20",
			Track->Wheels
				.FilterByPredicate([](const FAGX_TrackWheel& Wheel)
								   { return Wheel.Radius == 20.f; })
				.Num(),
			2);
		Test.TestEqual(
			"Number of Wheels with Radius 15",
			Track->Wheels
				.FilterByPredicate([](const FAGX_TrackWheel& Wheel)
								   { return Wheel.Radius == 15.f; })
				.Num(),
			8);
		Test.TestEqual(
			"Number of Wheels with MOVE_NODES_TO_ROTATION_PLANE",
			Track->Wheels
				.FilterByPredicate([](const FAGX_TrackWheel& Wheel)
								   { return Wheel.bMoveNodesToRotationPlane; })
				.Num(),
			2);
		Test.TestEqual(
			"Number of Wheels with SPLIT_SEGMENTS",
			Track->Wheels
				.FilterByPredicate([](const FAGX_TrackWheel& Wheel)
								   { return Wheel.bSplitSegments; })
				.Num(),
			2);
		Test.TestEqual(
			"Number of Wheels with MOVE_NODES_TO_WHEELS",
			Track->Wheels
				.FilterByPredicate([](const FAGX_TrackWheel& Wheel)
								   { return Wheel.bMoveNodesToWheel; })
				.Num(),
			0);

		return true;
	};

	UAGX_TrackComponent* TrackRight = GetByName<UAGX_TrackComponent>(
		Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("track_right"));
	UAGX_TrackComponent* TrackLeft = GetByName<UAGX_TrackComponent>(
		Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("track_left"));

	if (!TestTrack(TrackRight))
	{
		Test.AddError(TEXT("TestTrack given TrackRight returned false."));
	}

	if (!TestTrack(TrackLeft))
	{
		Test.AddError(TEXT("TestTrack given TrackLeft returned false."));
	}

	return true;
}

/**
 * Remove everything created by the archive import.
 *
 * @return True when the clearing is complete. Never returns false.
 */
bool FClearTrackImportedCommand::Update()
{
	if (Test.Contents == nullptr)
	{
		return true;
	}

#if defined(__linux__)
	/// @todo Workaround for internal issue #213.
	Test.AddExpectedError(
		TEXT("inotify_rm_watch cannot remove descriptor"), EAutomationExpectedErrorFlags::Contains,
		0);
	Test.AddError(TEXT("inotify_rm_watch cannot remove descriptor"));
#endif

	TArray<const TCHAR*> ExpectedFiles {
		TEXT("Blueprint"),
		TEXT("BP_track_build.uasset"),
		TEXT("ContactMaterial"),
		TEXT("CM_track_ground.uasset"),
		TEXT("CM_track_wheel.uasset"),
		TEXT("ShapeMaterial"),
		TEXT("ground.uasset"),
		TEXT("track.uasset"),
		TEXT("wheel.uasset"),
		TEXT("TrackInternalMergeProperties"),
		TEXT("AGX_TIMP_track_left.uasset"),
		TEXT("AGX_TIMP_track_right.uasset"),
		TEXT("TrackProperties"),
		TEXT("AGX_TP_track_left.uasset"),
		TEXT("AGX_TP_track_right.uasset")};

	const FString BaseBlueprintName = Test.Contents->GetName() + FString(".uasset");
	ExpectedFiles.Add(*BaseBlueprintName);

	AgxAutomationCommon::DeleteImportDirectory(TEXT("track_build"), ExpectedFiles);

	return true;
}

//
// AMOR test starts here.
//

class FImporterToBlueprint_AmorTest;

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FSupressAmorWireImportErrorCommand, FImporterToBlueprint_AmorTest&, Test);

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FCheckAmorImportedCommand, FImporterToBlueprint_AmorTest&, Test);

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FClearAmorImportedCommand, FImporterToBlueprint_AmorTest&, Test);

class FImporterToBlueprint_AmorTest final : public AgxAutomationCommon::FAgxAutomationTest
{
public:
	FImporterToBlueprint_AmorTest()
		: AgxAutomationCommon::FAgxAutomationTest(
			  TEXT("FImporterToBlueprint_AmorTest"),
			  TEXT("AGXUnreal.Editor.ImporterToBlueprint.Amor"))
	{
	}

public:
	UBlueprint* Contents = nullptr;

protected:
	virtual bool RunTest(const FString&) override
	{
		BAIL_TEST_IF_NOT_EDITOR(false)
		ADD_LATENT_AUTOMATION_COMMAND(FSupressAmorWireImportErrorCommand(*this))
		ADD_LATENT_AUTOMATION_COMMAND(
			FImportArchiveBlueprintCommand(TEXT("amor_build.agx"), Contents, *this))
		ADD_LATENT_AUTOMATION_COMMAND(FCheckAmorImportedCommand(*this))
		ADD_LATENT_AUTOMATION_COMMAND(FClearAmorImportedCommand(*this))
		return true;
	}
};

namespace
{
	FImporterToBlueprint_AmorTest ImporterToBlueprint_AmorTest;
}

bool FSupressAmorWireImportErrorCommand::Update()
{
	// The .agx file about to be imported contains Wires which will generate error printouts from
	// AGX Dynamics when no AGX Dynamics license is available. Here, we suppress that error
	// printout.
	Test.AddExpectedError(
		TEXT("License for AgX-Wires not valid"), EAutomationExpectedErrorFlags::Contains, 0);
	Test.AddError(TEXT("License for AgX-Wires not valid"));

	return true;
}

bool FCheckAmorImportedCommand::Update()
{
	using namespace AgxAutomationCommon;
	if (Test.Contents == nullptr)
	{
		Test.AddError(TEXT("Could not import Amor test scene: No content created."));
		return true;
	}

	// Get all the imported components. The test for the number of components is a safety check.
	// It should be updated whenever the test scene is changed.
	TArray<UActorComponent*> Components =
		FAGX_BlueprintUtilities::GetTemplateComponents(Test.Contents);
	// Two Rigid Bodies (2), one Shape (3), two Wires (5), one Constraint (6),
	// one Collision Group Disabler (7), one Default Scene Root (8), one ModelSourceComponent (9).
	const int32 ExpectedNumComponents = 9;
	Test.TestEqual(TEXT("Number of imported components"), Components.Num(), ExpectedNumComponents);

	UAGX_RigidBodyComponent* Body = GetByName<UAGX_RigidBodyComponent>(
		Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("Body"));
	Test.TestTrue("Body Enable Merge", Body->MergeSplitProperties.bEnableMerge);
	Test.TestFalse("Body Enable Merge", Body->MergeSplitProperties.bEnableSplit);
	Test.TestNotNull("Body Thresholds", Body->MergeSplitProperties.Thresholds);
	if (Body->MergeSplitProperties.Thresholds == nullptr)
	{
		return true;
	}

	Test.TestEqual(
		"Body Thresholds MaxImpactSpeed", Body->MergeSplitProperties.Thresholds->MaxImpactSpeed,
		AgxToUnrealDistance(13.0));
	Test.TestEqual(
		"Body Thresholds MaxRelativeNormalSpeed",
		Body->MergeSplitProperties.Thresholds->MaxRelativeNormalSpeed, AgxToUnrealDistance(14.0));
	Test.TestEqual(
		"Body Thresholds MaxRelativeTangentSpeed",
		Body->MergeSplitProperties.Thresholds->MaxRelativeTangentSpeed, AgxToUnrealDistance(15.0));
	Test.TestEqual(
		"Body Thresholds MaxRollingSpeed", Body->MergeSplitProperties.Thresholds->MaxRollingSpeed,
		AgxToUnrealDistance(16.0));
	Test.TestTrue(
		"Body Thresholds MaySplitInGravityField",
		Body->MergeSplitProperties.Thresholds->bMaySplitInGravityField);
	Test.TestEqual(
		"Body Thresholds NormalAdhesion", Body->MergeSplitProperties.Thresholds->NormalAdhesion,
		17.0);
	Test.TestFalse(
		"Body Thresholds SplitOnLogicalImpact",
		Body->MergeSplitProperties.Thresholds->bSplitOnLogicalImpact);
	Test.TestEqual(
		"Body Thresholds TangentialAdhesion",
		Body->MergeSplitProperties.Thresholds->TangentialAdhesion, 18.0);

	UAGX_ShapeComponent* Geometry = GetByName<UAGX_ShapeComponent>(
		Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("GeometrySharingThresholds"));
	Test.TestTrue("Geometry Enable Merge", Geometry->MergeSplitProperties.bEnableMerge);
	Test.TestTrue("Geometry Enable Merge", Geometry->MergeSplitProperties.bEnableSplit);
	Test.TestNotNull("Geometry Thresholds", Geometry->MergeSplitProperties.Thresholds);
	if (Geometry->MergeSplitProperties.Thresholds == nullptr)
	{
		return true;
	}

	Test.TestEqual(
		"Geometry share Thresholds", Geometry->MergeSplitProperties.Thresholds,
		Body->MergeSplitProperties.Thresholds);

	UAGX_WireComponent* Wire = GetByName<UAGX_WireComponent>(
		Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("Wire"));
	Test.TestNotNull(TEXT("Wire Component"), Wire);
	if (Wire == nullptr)
	{
		return true;
	}
	Test.TestFalse("Wire Enable Merge", Wire->MergeSplitProperties.bEnableMerge);
	Test.TestTrue("Wire Enable Merge", Wire->MergeSplitProperties.bEnableSplit);
	Test.TestNotNull("Wire Thresholds", Wire->MergeSplitProperties.Thresholds);
	if (Wire->MergeSplitProperties.Thresholds == nullptr)
	{
		return true;
	}

	Test.TestEqual(
		"Wire Thresholds ForcePropagationDecayScale",
		Wire->MergeSplitProperties.Thresholds->ForcePropagationDecayScale, 1.1);
	Test.TestEqual(
		"Wire Thresholds ForcePropagationDecayScale",
		Wire->MergeSplitProperties.Thresholds->MergeTensionScale, 1.2);

	UAGX_WireComponent* WireNoThresholds = GetByName<UAGX_WireComponent>(
		Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("WireNoThresholds"));
	Test.TestTrue(
		"WireNoThresholds Enable Merge", WireNoThresholds->MergeSplitProperties.bEnableMerge);
	Test.TestTrue(
		"WireNoThresholds Enable Merge", WireNoThresholds->MergeSplitProperties.bEnableSplit);
	Test.TestNull("WireNoThresholds Thresholds", WireNoThresholds->MergeSplitProperties.Thresholds);

	UAGX_ConstraintComponent* Constraint = GetByName<UAGX_ConstraintComponent>(
		Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("Hinge"));
	Test.TestFalse("Constraint Enable Merge", Constraint->MergeSplitProperties.bEnableMerge);
	Test.TestFalse("Constraint Enable Merge", Constraint->MergeSplitProperties.bEnableSplit);
	Test.TestNotNull("Constraint Thresholds", Constraint->MergeSplitProperties.Thresholds);
	if (Constraint->MergeSplitProperties.Thresholds == nullptr)
	{
		return true;
	}

	Test.TestEqual(
		"Constraint Thresholds MaxDesiredForceRangeDiff",
		Constraint->MergeSplitProperties.Thresholds->MaxDesiredForceRangeDiff, 4.0);
	Test.TestEqual(
		"Constraint Thresholds MaxDesiredLockAngleDiff",
		Constraint->MergeSplitProperties.Thresholds->MaxDesiredLockAngleDiff,
		FMath::RadiansToDegrees(5.0));
	Test.TestEqual(
		"Constraint Thresholds MaxDesiredRangeAngleDiff",
		Constraint->MergeSplitProperties.Thresholds->MaxDesiredRangeAngleDiff,
		FMath::RadiansToDegrees(6.0));
	Test.TestEqual(
		"Constraint Thresholds MaxDesiredSpeedDiff",
		Constraint->MergeSplitProperties.Thresholds->MaxDesiredSpeedDiff,
		FMath::RadiansToDegrees(7.0));
	Test.TestEqual(
		"Constraint Thresholds MaxRelativeSpeed",
		Constraint->MergeSplitProperties.Thresholds->MaxRelativeSpeed,
		FMath::RadiansToDegrees(8.0));

// Enable this to see the names of the components that was imported. Useful when adding new stuff
// to the archive.
#if 0
	UE_LOG(LogAGX, Warning, TEXT("Imported the following components:"));
	for (const UActorComponent* Component : Components)
	{
		UE_LOG(LogAGX, Warning, TEXT("  %s"), *Component->GetName());
	}
#endif

	return true;
}

bool FClearAmorImportedCommand::Update()
{
	if (Test.Contents == nullptr)
	{
		return true;
	}

#if defined(__linux__)
	/// @todo Workaround for internal issue #213.
	Test.AddExpectedError(
		TEXT("inotify_rm_watch cannot remove descriptor"), EAutomationExpectedErrorFlags::Contains,
		0);
	Test.AddError(TEXT("inotify_rm_watch cannot remove descriptor"));
#endif

	// Files that are created by the test and thus safe to remove. The GUID values may make this
	// test cumbersome to update since they will change every time the AGX Dynamics archive is
	// regenerated. Consider either adding wildcard support to DeleteImportDirectory or assign
	// names to the render materials in the source .agxPy file.
	TArray<const TCHAR*> ExpectedFiles = {
		TEXT("Blueprint"),
		TEXT("BP_amor_build.uasset"),
		TEXT("ShapeMaterial"),
		TEXT("AGX_WMST_D04D33D7E7E4D113ABE604FB9454AFA3.uasset"),
		TEXT("defaultWireMaterial_40.uasset"),
		TEXT("defaultWireMaterial_550.uasset"),
		TEXT("MergeSplitThresholds"),
		TEXT("AGX_CMST_364174E1BCA7613B6CA7C137E4B90E9E.uasset"),
		TEXT("AGX_SMST_284C7B5646806D96703D520DDF31A09E.uasset")};

	const FString BaseBlueprintName = Test.Contents->GetName() + FString(".uasset");
	ExpectedFiles.Add(*BaseBlueprintName);

	AgxAutomationCommon::DeleteImportDirectory(TEXT("amor_build"), ExpectedFiles);

	return true;
}

//
// Shovel test starts here.
//

class FImporterToBlueprint_ShovelTest;

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FCheckShovelImportedCommand, FImporterToBlueprint_ShovelTest&, Test);

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FClearShovelImportedCommand, FImporterToBlueprint_ShovelTest&, Test);

class FImporterToBlueprint_ShovelTest final : public AgxAutomationCommon::FAgxAutomationTest
{
public:
	FImporterToBlueprint_ShovelTest()
		: AgxAutomationCommon::FAgxAutomationTest(
			  TEXT("FImporterToBlueprint_ShovelTest"),
			  TEXT("AGXUnreal.Editor.ImporterToBlueprint.Shovel"))
	{
	}

	UBlueprint* Contents = nullptr;

protected:
	virtual bool RunTest(const FString&) override
	{
		BAIL_TEST_IF_NOT_EDITOR(false)
		ADD_LATENT_AUTOMATION_COMMAND(
			FImportArchiveBlueprintCommand(TEXT("terrain_build.agx"), Contents, *this));
		ADD_LATENT_AUTOMATION_COMMAND(FCheckShovelImportedCommand(*this));
		ADD_LATENT_AUTOMATION_COMMAND(FClearShovelImportedCommand(*this));
		return true;
	}
};

namespace
{
	FImporterToBlueprint_ShovelTest ImporterToBlueprint_ShovelTest;
}

bool FCheckShovelImportedCommand::Update()
{
	using namespace AgxAutomationCommon;
	if (Test.Contents == nullptr)
	{
		Test.AddError(TEXT("Could not import Shovel test scene: No content created."));
		return true;
	}

	// Get all the imported Components.
	TArray<UActorComponent*> Components =
		FAGX_BlueprintUtilities::GetTemplateComponents(Test.Contents);
	// One Rigid Body (1), two Shapes (3), one Shovel (4), one Model Source (5), and one default
	// scene root (6).
	const int32 ExpectedNumComponents {6};
	Test.TestEqual(TEXT("Number of imported Components"), Components.Num(), ExpectedNumComponents);

	UAGX_RigidBodyComponent* ShovelBody = GetByName<UAGX_RigidBodyComponent>(
		Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("Shovel Body"));
	Test.TestNotNull(TEXT("Shovel Body"), ShovelBody);
	if (ShovelBody == nullptr)
		return true;
	Test.TestEqual(
		TEXT("Body Import GUID"),
		ShovelBody->ImportGuid.ToString(EGuidFormats::DigitsWithHyphensLower),
		TEXT("8c48a356-d44b-1f4a-134c-e7e7a1f4c003"));
	Test.TestEqual(
		TEXT("Shovel Body Position"), ShovelBody->GetRelativeLocation(), FVector(0.0, 0.0, 100.0));

	UAGX_BoxShapeComponent* VerticalBox = GetByName<UAGX_BoxShapeComponent>(
		Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("Vertical Box"));
	Test.TestNotNull(TEXT("Vertical Box"), VerticalBox);
	if (VerticalBox == nullptr)
		return true;
	Test.TestEqual(
		TEXT("Vertical Box Import GUID"),
		VerticalBox->ImportGuid.ToString(EGuidFormats::DigitsWithHyphensLower),
		TEXT("8c48a356-d44b-1f4a-134c-e7e7a1f4c004"));
	Test.TestEqual(
		TEXT("Vertical Box Half Extent"), VerticalBox->HalfExtent, FVector(10.0, 100.0, 100.0));

	UAGX_ShovelComponent* Shovel = GetByName<UAGX_ShovelComponent>(
		Components, *FAGX_BlueprintUtilities::ToTemplateComponentName(TEXT("Shovel_Shovel Body")));
	Test.TestNotNull(TEXT("Shovel Component"), Shovel);
	if (Shovel == nullptr)
		return true;
	Test.TestEqual(
		TEXT("Shovel Import GUID"),
		Shovel->ImportGuid.ToString(EGuidFormats::DigitsWithHyphensLower),
		"8c48a356-d44b-1f4a-134c-e7e7a1f4c008");
	Test.TestEqual(TEXT("bEnable"), Shovel->bEnabled, false);
	Test.TestEqual(
		TEXT("Top Edge Start Location"), Shovel->TopEdge.Start.LocalLocation,
		AgxToUnrealDisplacement(0.1, -1.0, 1.0));
	Test.TestEqual(
		TEXT("Top Edge Start Parent Name"), Shovel->TopEdge.Start.Parent.Name,
		FName(TEXT("Shovel Body")));
	Test.TestEqual(
		TEXT("Top Edge End Location"), Shovel->TopEdge.End.LocalLocation,
		AgxToUnrealDisplacement(0.1, 1.0, 1.0));
	Test.TestEqual(
		TEXT("Top Edge End Parent Name"), Shovel->TopEdge.End.Parent.Name,
		FName(TEXT("Shovel Body")));
	Test.TestEqual(
		TEXT("Cutting Edge Start Location"), Shovel->CuttingEdge.Start.LocalLocation,
		AgxToUnrealDisplacement(1.0, -1.0, 0.1));
	Test.TestEqual(
		TEXT("Cutting Edge Start Parent Name"), Shovel->CuttingEdge.Start.Parent.Name,
		FName(TEXT("Shovel Body")));
	Test.TestEqual(
		TEXT("Cutting Edge End Location"), Shovel->CuttingEdge.End.LocalLocation,
		AgxToUnrealDisplacement(1.0, 1.0, 0.1));
	Test.TestEqual(
		TEXT("Cutting Edge End Parent Name"), Shovel->CuttingEdge.End.Parent.Name,
		FName(TEXT("Shovel Body")));
	Test.TestEqual(
		TEXT("Cutting Direction"), Shovel->CuttingDirection.LocalRotation,
		FRotator(ForceInitToZero));

	Test.TestNotNull(TEXT("Shovel Properties"), Shovel->ShovelProperties);
	if (Shovel->ShovelProperties == nullptr)
		return true;
	Test.TestTrue(TEXT("Shovel Properties is asset"), Shovel->ShovelProperties->IsAsset());
	Test.TestFalse(TEXT("Shovel Properties is instance"), Shovel->ShovelProperties->IsInstance());
	Test.TestEqual(
		TEXT("ToothLength"), Shovel->ShovelProperties->ToothLength, AgxToUnrealDistance(1.0));
	Test.TestEqual(
		TEXT("ToothMinimumRadius"), Shovel->ShovelProperties->ToothMinimumRadius,
		AgxToUnrealDistance(2.0));
	Test.TestEqual(
		TEXT("ToothMaximumRadius"), Shovel->ShovelProperties->ToothMaximumRadius,
		AgxToUnrealDistance(3.0));
	Test.TestEqual(TEXT("NumberOfTeeth"), Shovel->ShovelProperties->NumberOfTeeth, 4);
	Test.TestEqual(
		TEXT("NoMergeExtensionDistance"), Shovel->ShovelProperties->NoMergeExtensionDistance,
		AgxToUnrealDistance(5.0));
	Test.TestEqual(
		TEXT("MinimumSubmergedContactLengthFraction"),
		Shovel->ShovelProperties->MinimumSubmergedContactLengthFraction, 6.0);
	Test.TestEqual(
		TEXT("VerticalBladeSoilMergeDistance"),
		Shovel->ShovelProperties->VerticalBladeSoilMergeDistance, AgxToUnrealDistance(7.0));
	Test.TestEqual(
		TEXT("SecondarySeparationDeadloadLimit"),
		Shovel->ShovelProperties->SecondarySeparationDeadloadLimit, 8.0);
	Test.TestEqual(
		TEXT("PenetrationDepthThreshold"), Shovel->ShovelProperties->PenetrationDepthThreshold,
		AgxToUnrealDistance(9.0));
	Test.TestEqual(
		TEXT("PenetrationForceScaling"), Shovel->ShovelProperties->PenetrationForceScaling, 10.0);
	Test.TestEqual(
		TEXT("EnableParticleFreeDeformers"), Shovel->ShovelProperties->bEnableParticleFreeDeformers,
		true);
	Test.TestEqual(
		TEXT("AlwaysRemoveShovelContacts"), Shovel->ShovelProperties->bAlwaysRemoveShovelContacts,
		true);
	Test.TestEqual(
		TEXT("MaxPenetrationForce"), Shovel->ShovelProperties->MaximumPenetrationForce, 11.0);
	Test.TestEqual(
		TEXT("ContactRegionThreshold"), Shovel->ShovelProperties->ContactRegionThreshold,
		AgxToUnrealDistance(12.0));
	Test.TestEqual(
		TEXT("ContactRegionVerticalLimit"), Shovel->ShovelProperties->ContactRegionVerticalLimit,
		AgxToUnrealDistance(13.0));
	Test.TestEqual(
		TEXT("EnableInnerShapeCreateDynamicMass"),
		Shovel->ShovelProperties->bEnableInnerShapeCreateDynamicMass, false);
	Test.TestEqual(
		TEXT("EnableParticleForceFeedback"), Shovel->ShovelProperties->bEnableParticleForceFeedback,
		true);
	Test.TestEqual(
		TEXT("ParicleInclusionMultiplier"), Shovel->ShovelProperties->ParticleInclusionMultiplier,
		/*14.0*/ 1.0 /* AGX Dynamics 2.37.0.1 does not store/restore this value. */);

	Test.TestEqual(
		TEXT("Primary enabled"), Shovel->ShovelProperties->PrimaryExcavationSettings.bEnabled,
		false);
	Test.TestEqual(
		TEXT("Primary create dynamic mass"),
		Shovel->ShovelProperties->PrimaryExcavationSettings.bEnableCreateDynamicMass, false);
	Test.TestEqual(
		TEXT("Primary force feedback"),
		Shovel->ShovelProperties->PrimaryExcavationSettings.bEnableForceFeedback, false);

	Test.TestEqual(
		TEXT("Deform back  enabled"),
		Shovel->ShovelProperties->DeformBackExcavationSettings.bEnabled, false);
	Test.TestEqual(
		TEXT("Deform back create dynamic mass"),
		Shovel->ShovelProperties->DeformBackExcavationSettings.bEnableCreateDynamicMass, false);
	Test.TestEqual(
		TEXT("Deform back force feedback"),
		Shovel->ShovelProperties->DeformBackExcavationSettings.bEnableForceFeedback, true);

	Test.TestEqual(
		TEXT("Deform right enabled"),
		Shovel->ShovelProperties->DeformRightExcavationSettings.bEnabled, false);
	Test.TestEqual(
		TEXT("Deform right create dynamic mass"),
		Shovel->ShovelProperties->DeformRightExcavationSettings.bEnableCreateDynamicMass, true);
	Test.TestEqual(
		TEXT("Deform right force feedback"),
		Shovel->ShovelProperties->DeformRightExcavationSettings.bEnableForceFeedback, false);

	Test.TestEqual(
		TEXT("Deform left enabled"),
		Shovel->ShovelProperties->DeformLeftExcavationSettings.bEnabled, true);
	Test.TestEqual(
		TEXT("Deform left create dynamic mass"),
		Shovel->ShovelProperties->DeformLeftExcavationSettings.bEnableCreateDynamicMass, false);
	Test.TestEqual(
		TEXT("Deform left force feedback"),
		Shovel->ShovelProperties->DeformLeftExcavationSettings.bEnableForceFeedback, false);

	return true;
}

bool FClearShovelImportedCommand::Update()
{
	if (Test.Contents == nullptr)
	{
		return true;
	}

	// clang-format off
	const FString BaseBlueprintName = Test.Contents->GetName() + FString(".uasset");
	TArray<const TCHAR*> ExpectedFiles = {
		TEXT("BP_terrain_build.uasset"),
			*BaseBlueprintName,
		TEXT("Blueprint"),
		TEXT("ShovelProperties"),
			TEXT("AGX_SP_ShovelBody.uasset")
	};
	// clang-format on

	AgxAutomationCommon::DeleteImportDirectory(TEXT("terrain_build"), ExpectedFiles);

	return true;
}

// WITH_DEV_AUTOMATION_TESTS
#endif
