// Copyright 2025, Algoryx Simulation AB.

/*
 * A collection of tests that test AGX Dynamics archive import and model reimport.
 *
 * There are a number of Python scripts that build a scene, saves it to a .agx file, modifies the
 * scene, and finally saves it to another .agx file. These tests import the first .agx file, checks
 * that the contents is as expected, reimports with the second .agx file, and again checks that
 * the contents is as expected.
 *
 * A base class, FReimportModelTest, does import and reimport interspersed with calls
 * to virtual functions implemented by derived classes where the actual testing happens.
 */

// AGX Dynamics for Unreal includes.
#include "Import/AGX_ImportSettings.h"
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"
#include "CollisionGroups/AGX_CollisionGroupDisablerComponent.h"
#include "Constraints/AGX_BallConstraintComponent.h"
#include "Constraints/AGX_CylindricalConstraintComponent.h"
#include "Constraints/AGX_HingeConstraintComponent.h"
#include "Constraints/AGX_PrismaticConstraintComponent.h"
#include "Import/AGX_ImporterToEditor.h"
#include "Materials/AGX_ContactMaterial.h"
#include "Materials/AGX_ContactMaterialRegistrarComponent.h"
#include "Materials/AGX_ShapeMaterial.h"
#include "Shapes/AGX_BoxShapeComponent.h"
#include "Shapes/AGX_CylinderShapeComponent.h"
#include "Shapes/AGX_SphereShapeComponent.h"
#include "Shapes/AGX_TrimeshShapeComponent.h"
#include "AgxAutomationCommon.h"
#include "Utilities/AGX_BlueprintUtilities.h"
#include "Utilities/AGX_EditorUtilities.h"
#include "Utilities/AGX_ImportUtilities.h"
#include "Utilities/AGX_ObjectUtilities.h"

// Unreal Engine includes.
#include "Components/StaticMeshComponent.h"
#include "HAL/FileManager.h"
#include "Misc/AutomationTest.h"
#include "MaterialTypes.h"
#include "Materials/MaterialInterface.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"

namespace AGX_ReimportModelTest_helpers
{
	// Child Blueprints, like the one produced after an Import, does not necessarily contain any SCS
	// Nodes themselves. Instead one have to get the SCS Nodes from the base Blueprint, then get the
	// template Components from them, and go through the archetype instances to find the Components
	// of interest.

#if 0
	// todo: important; the found template Components from the GetTemplateComponents
	// call below are retrieved as expected. But calling GetArchetypeInstances on any of those
	// components gives nothing, which is really unexpected. It is just as if it is only from this
	// test that the issue exists, doing the anywhere in the Editor module of the plugin works as
	// expected.
	//
	// Update: it seems that the archetype instances of the base Blueprint are created on-demand
	// when the Blueprint Editor is opened. This was confirmed by printing out the number of
	// archetype instances right after a regular Import but before the Blueprint Editor was opened.
	// The number was then zero.
	TArray<UActorComponent*> GetComponentsFromChildBlueprint(
		UBlueprint& BaseBp, UBlueprint& ChildBlueprint)
	{
		TArray<UActorComponent*> BaseComponents =
			FAGX_BlueprintUtilities::GetTemplateComponents(&BaseBp);

		TArray<UActorComponent*> ChildComponents;
		ChildComponents.Reserve(BaseComponents.Num());
		for (UActorComponent* BaseComponent : BaseComponents)
		{
			if (auto MatchedComponent =
					FAGX_ObjectUtilities::GetMatchedInstance(BaseComponent, &ChildBlueprint))
			{
				ChildComponents.Add(MatchedComponent);
			}
		}

		return ChildComponents;
	}
#endif

	//
	// High-level functions, working on entire models or worlds.
	//

	UBlueprint* Import(const FString& ArchiveFileName, bool IgnoreDisabledTrimeshes)
	{
		FString ArchiveFilePath = AgxAutomationCommon::GetTestScenePath(
			FPaths::Combine(FString("ReimportModel"), ArchiveFileName));
		if (ArchiveFilePath.IsEmpty())
		{
			UE_LOG(LogAGX, Error, TEXT("Did not find an archive named '%s'."), *ArchiveFileName);
			return nullptr;
		}

		FAGX_ImportSettings ImportSettings;
		ImportSettings.bIgnoreDisabledTrimeshes = IgnoreDisabledTrimeshes;
		ImportSettings.bOpenBlueprintEditorAfterImport = false;
		ImportSettings.FilePath = ArchiveFilePath;
		ImportSettings.ImportType = EAGX_ImportType::Agx;

		FAGX_ImporterToEditor Importer;
		return Importer.Import(ImportSettings);
	}

	bool ReimportModel(
		UBlueprint& BaseBp, const FString& ArchiveFileName, bool IgnoreDisabledTrimeshes)
	{
		FString ArchiveFilePath = AgxAutomationCommon::GetTestScenePath(
			FPaths::Combine(FString("ReimportModel"), ArchiveFileName));
		if (ArchiveFilePath.IsEmpty())
		{
			UE_LOG(LogAGX, Error, TEXT("Did not find an archive named '%s'."), *ArchiveFileName);
			return false;
		}

		FAGX_ReimportSettings Settings;
		Settings.bForceOverwriteProperties = false;
		Settings.bForceReassignRenderMaterials = false;
		Settings.bIgnoreDisabledTrimeshes = IgnoreDisabledTrimeshes;
		Settings.FilePath = ArchiveFilePath;
		Settings.ImportType = EAGX_ImportType::Agx;
		Settings.bOpenBlueprintEditorAfterImport = false;
		FAGX_ImporterToEditor Importer;

		return Importer.Reimport(BaseBp, Settings);
	}

	/**
	 * Delete all asset files created for the given mode.
	 *
	 * @param ArchiveName The name, not file name, of the model to delete assets for.
	 * @return True if the directory does not exist, regardless of if it was deleted or did not
	 * exist in the first place.
	 */
	bool DeleteImportedAssets(const FString& ArchiveName, FAutomationTestBase& Test)
	{
		const FString Root = FPaths::ProjectContentDir();
		const FString ImportsLocal =
			FPaths::Combine(FAGX_ImportUtilities::GetImportRootDirectoryName(), ArchiveName);
		const FString ImportsFull = FPaths::Combine(Root, ImportsLocal);
		const FString ImportsAbsolute = FPaths::ConvertRelativePathToFull(ImportsFull);
		if (!FPaths::DirectoryExists(ImportsAbsolute))
		{
			return true;
		}

#if defined(__linux__)
		/// @todo Workaround for internal issue #213.
		Test.AddExpectedError(
			TEXT("inotify_rm_watch cannot remove descriptor"),
			EAutomationExpectedErrorFlags::Contains, 0);
		Test.AddError(TEXT("inotify_rm_watch cannot remove descriptor"));
#endif

		const bool Deleted = IFileManager::Get().DeleteDirectory(*ImportsAbsolute, true, true);
		if (!Deleted)
		{
			Test.AddError(FString::Printf(
				TEXT("%s: IFileManager::DeleteDirectory returned false trying to delete: '%s'"),
				*Test.GetTestName(), *ImportsAbsolute));
			return true;
		}
		return Deleted;
	}
}

//
// Functions operating on the SCS node tree.
//

bool CheckNodeNonExisting(const UBlueprint& Blueprint, const FString& Name)
{
	USCS_Node* Node = Blueprint.SimpleConstructionScript->FindSCSNode(FName(Name));
	if (Node != nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Found SCS Node '%s' that was expected not to exist in the Blueprint."), *Name);
		return false;
	}

	return true;
}

bool CheckNodeNameAndEnsureNoParent(const UBlueprint& Blueprint, const FString& Name)
{
	USCS_Node* Node = AgxAutomationCommon::GetNodeChecked(Blueprint, Name);
	if (Node == nullptr)
		return false;

	if (USCS_Node* Parent = Blueprint.SimpleConstructionScript->FindParentNode(Node))
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("SCS Node '%s' has parent '%s' but was expected to not have a parent."), *Name,
			*Parent->GetName());
		return false;
	}

	return true;
}

bool CheckNodeNoChild(const UBlueprint& Blueprint, const FString& Name)
{
	USCS_Node* Node = AgxAutomationCommon::GetNodeChecked(Blueprint, Name);
	if (Node == nullptr)
		return false; // Logging/error done in GetNodeChecked.

	if (Node->GetChildNodes().Num() != 0)
	{
		UE_LOG(
			LogAGX, Error, TEXT("Expected node '%s' not to have zero children, but it had %d"),
			*Name, Node->GetChildNodes().Num());
		return false;
	}

	return true;
}

bool CheckNodeNameAndParent(
	const UBlueprint& Blueprint, const FString& Name, const FString& ParentNodeName,
	bool EnsureNoChild)
{
	USCS_Node* Node = AgxAutomationCommon::GetNodeChecked(Blueprint, Name);
	if (Node == nullptr)
		return false;

	USCS_Node* Parent = Blueprint.SimpleConstructionScript->FindParentNode(Node);
	if (Parent == nullptr)
	{
		UE_LOG(LogAGX, Error, TEXT("The SCS Node '%s' does not have a parent."), *Name);
		return false;
	}

	if (Parent->GetVariableName().ToString() != ParentNodeName)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("The SCS Node '%s' has a parent Node named '%s', expected it to be '%s'."), *Name,
			*Parent->GetVariableName().ToString(), *ParentNodeName);
		return false;
	}

	if (EnsureNoChild)
	{
		return CheckNodeNoChild(Blueprint, Name);
	}

	return true;
}

//
// Functions operating on template components.
//

void PrintComponentNames(TArray<UActorComponent*> Components, const TCHAR* Title)
{
	UE_LOG(LogAGX, Warning, TEXT("%s"), Title);
	for (UActorComponent* Component : Components)
	{
		UE_LOG(LogAGX, Warning, TEXT("  %s"), *GetNameSafe(Component));
	}
}

template <typename UObject>
UObject* GetTemplateComponentByName(TArray<UActorComponent*>& Components, const TCHAR* Name)
{
	return AgxAutomationCommon::GetByName<UObject>(
		Components, *FAGX_BlueprintUtilities::ToTemplateComponentName(Name));
}

template <typename UAsset>
FString GetAssetPath(const FString& ArchiveName, const FString& AssetName)
{
	const FString Path = FPaths::ConvertRelativePathToFull(FPaths::Combine(
		FPaths::ProjectContentDir(), FAGX_ImportUtilities::GetImportRootDirectoryName(),
		FPaths::GetBaseFilename(ArchiveName),
		FAGX_ImportUtilities::GetImportAssetDirectoryName<UAsset>(), AssetName));
	return Path;
}

//
// Misc. functions.
//

template <typename F>
F FromRad(F radians)
{
	return FMath::RadiansToDegrees(radians);
}

//
// Common functionality
//
DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(
	FDeleteImportedAssets, FString, ArchiveName, FAutomationTestBase&, Test);

bool FDeleteImportedAssets::Update()
{
	const FString Path = FAGX_ImportUtilities::GetDefaultModelImportDirectory(ArchiveName);
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

class FReimportModelTest;

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FCallReimportModelTest, FReimportModelTest&, Test);

/**
 * Base class for tests that load a model and then reimports with an updated version of the
 * same model. Add model-specific test code by overriding the pure-virtual member functions.
 */
class FReimportModelTest : public AgxAutomationCommon::FAgxAutomationTest
{
public:
	/**
	 * @param InTestName The name of the test, passed to FAutomationTestBase.
	 * @param InTestPath The hierarchical name of the test, often starting with AGXUnreal.Editor.
	 * @param InInitialModelFileName The name of the model file to import.
	 * @param InUpdatedModelFileName Model file to reimport with.
	 */
	FReimportModelTest(
		const TCHAR* InTestName, const TCHAR* InTestPath, const TCHAR* InInitialModelFileName,
		const TCHAR* InUpdatedModelFileName)
		: AgxAutomationCommon::FAgxAutomationTest(InTestName, InTestPath)
		, InitialModelFileName(InInitialModelFileName)
		, InitialModelName(FPaths::GetBaseFilename(InitialModelFileName))
		, UpdatedModelFileName(InUpdatedModelFileName)
		, UpdatedModelName(FPaths::GetBaseFilename(UpdatedModelFileName))
		, ActorName(InitialModelName + "_Instance")
	{
	}

	/**
	 * Called by the Unreal Engine test framework. Sets up a latent call to the parameter-less
	 * RunTest.
	 */
	virtual bool RunTest(const FString&) override
	{
		World = FAGX_EditorUtilities::GetCurrentWorld();
		if (!TestNotNull(TEXT("World"), World))
		{
			return false;
		}

		// Delete any old assets that may remain from previous test runs.
		AGX_ReimportModelTest_helpers::DeleteImportedAssets(InitialModelName, *this);

		ADD_LATENT_AUTOMATION_COMMAND(FCallReimportModelTest(*this));
		ADD_LATENT_AUTOMATION_COMMAND(FDeleteImportedAssets(InitialModelName, *this));
		return true;
	}

	/**
	 * Main test flow implemented here. Handles model loading and reimport, and calls the
	 * pure-virtual member functions at the appropriate times.
	 */
	void RunTest()
	{
		using namespace AGX_ReimportModelTest_helpers;

		// Import initial state.
		ChildBlueprint = Import(InitialModelFileName, true);
		if (!TestNotNull(TEXT("Imported child Blueprint before reimport"), ChildBlueprint))
		{
			return;
		}

		// The Components live in the parent Blueprint, so get that.
		ParentBlueprint = FAGX_BlueprintUtilities::GetOutermostParent(ChildBlueprint);
		if (!TestNotNull(TEXT("Imported parent Blueprint before reimport"), ParentBlueprint))
		{
			return;
		}

		InitialTemplateComponents = FAGX_BlueprintUtilities::GetTemplateComponents(ParentBlueprint);

		// Spawn an instance of the imported Blueprint in the level.
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Name = FName(ActorName);
		InitialBlueprintInstance =
			World->SpawnActor<AActor>(ChildBlueprint->GeneratedClass, SpawnParameters);
		if (!TestNotNull(TEXT("Blueprint instance before reimport"), InitialBlueprintInstance))
		{
			return;
		}
		if (!TestEqual(
				TEXT("Actor found by GetActorByName and Actor created by SpawnActor before "
					 "reimport."),
				GetActorInstanceFromWorld(), InitialBlueprintInstance))
		{
			return;
		}

		// Initial setup complete, call the first test callback.
		if (!PostImport())
		{
			Cleanup();
			return;
		}

		ReimportModel(*ParentBlueprint, UpdatedModelFileName, true);

		if (!TestTrue(TEXT("Child Blueprint is valid after reimport"), IsValid(ChildBlueprint)))
		{
			return;
		}
		if (!TestTrue(TEXT("Parent Blueprint is valid after reimport"), IsValid(ParentBlueprint)))
		{
			return;
		}

		UpdatedTemplateComponents = FAGX_BlueprintUtilities::GetTemplateComponents(ParentBlueprint);

		UpdatedBlueprintInstance = GetActorInstanceFromWorld();
		if (!TestNotNull(TEXT("Blueprint instance after reimport"), UpdatedBlueprintInstance))
		{
			return;
		}

// This test fails from time to time, mostly on Unreal Engine 5. Not sure why its non-deterministic,
// or what the guarantees from Unreal Engine really is. It doesn't matter from the test's or the
// model reimport's point of view, both will work either way, but it is a sign that there are
// still things in Unreal engine that we do not understand.
#if 0
		if (!TestNotEqual(
				TEXT("Initial and updated Blueprint instances"), InitialBlueprintInstance,
				UpdatedBlueprintInstance))
		{
			// We don't actually assume or depend on the instance being replaced during model
			// reimport, but it would be good to know if a future version of Unreal Editor
			// changes this behavior. It may be a signal that more has changed behind the scenes.
			UE_LOG(
				LogAGX, Warning,
				TEXT("Expected the Blueprint instance in the level to be replaced during model "
					 "reimport."));
			return;
		}
#endif

		// The initial instance is now gone, use UpdatedBlueprintInstance from now on.
		InitialBlueprintInstance = nullptr;

		// Model reimport complete, call the second test callback.
		PostReimport();

		// All testing complete, call the cleanup callback.
		Cleanup();

		// Cleanup our own state.
		UpdatedBlueprintInstance->Destroy();
		UpdatedBlueprintInstance = nullptr;
	}

	// Model-specific work should be done by implementing these pure-virtual member functions in a
	// derived class. Calls to the Test-functions should be placed in PostImport and
	// PostReimport. Return true if all tests pass, and false otherwise. Cleanup is used to
	// destroy anything that the test creates in addition to what this base class creates, if
	// anything is created.
	virtual bool PostImport() = 0;
	virtual bool PostReimport() = 0;
	virtual void Cleanup()
	{
		//  By default there is nothing to clean up since mandatory cleanup is done by RunTest.
	}

	AActor* GetActorInstanceFromWorld()
	{
		return FAGX_ObjectUtilities::GetActorByName(*World, ActorName);
	}

	// Members valid throughout the life-time of the test.
	const FString InitialModelFileName;
	const FString InitialModelName;
	const FString UpdatedModelFileName;
	const FString UpdatedModelName;
	const FString ActorName;
	UWorld* World = nullptr;

	// Members valid from after the initial import to the end of the test.
	UBlueprint* ChildBlueprint = nullptr;
	UBlueprint* ParentBlueprint = nullptr;

	// Members valid from the initial import to model reimport.
	// While some of the Components may survive template reconstruction, a particular model may
	// add or delete components freely.
	AActor* InitialBlueprintInstance = nullptr;
	TArray<UActorComponent*> InitialTemplateComponents;

	// Members valid after model reimport.
	AActor* UpdatedBlueprintInstance = nullptr;
	TArray<UActorComponent*> UpdatedTemplateComponents;
};

bool FCallReimportModelTest::Update()
{
	Test.RunTest();
	return true;
}

//
// Reimport same twice test starts here.
//

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(
	FReimportSameCommand, FString, ArchiveFileName, FAutomationTestBase&, Test);

bool FReimportSameCommand::Update()
{
	using namespace AGX_ReimportModelTest_helpers;
	using namespace AgxAutomationCommon;

	UBlueprint* Blueprint = Import(ArchiveFileName, false);
	if (Blueprint == nullptr)
	{
		Test.AddError("Imported Blueprint was nullptr.");
		return true;
	}

	UBlueprint* BlueprintBase = FAGX_BlueprintUtilities::GetOutermostParent(Blueprint);
	if (BlueprintBase == nullptr)
	{
		Test.AddError(
			"Could not get Blueprint parent (base) from the returned Blueprint after import.");
		return true;
	}

	const int NumNodesFirstImport = BlueprintBase->SimpleConstructionScript->GetAllNodes().Num();

	auto NameEndsWithGuid = [](const FString& Name, const FString& StartToOmit)
	{
		FString GuidPart = Name;
		GuidPart.RemoveFromStart(StartToOmit);
		FGuid TestGuid(GuidPart);
		return TestGuid.IsValid();
	};

	// Ensure a GUID is part of the Node name for the collision Mesh as well as the render Mesh.
	// This is an important detail for the Model Reimport pipeline. See comment in free
	// function SetUnnamedNameForPossibleCollisions in AGX_ImporterToBlueprint.cpp for more details.
	USCS_Node* TrimeshGeomToChangeNode = GetNodeChecked(*BlueprintBase, "TrimeshGeomToChange");
	USCS_Node* TGTCCollisionMeshNode = GetOnlyAttachChildChecked(TrimeshGeomToChangeNode);
	USCS_Node* TGTCRenderMeshNode = GetOnlyAttachChildChecked(TGTCCollisionMeshNode);
	if (TGTCRenderMeshNode == nullptr)
	{
		Test.AddError("Render Mesh Component was nullptr. Cannot continue.");
		return true;
	}

	const FString CollisionMeshNamePreSync = TGTCCollisionMeshNode->GetVariableName().ToString();
	const FString RenderMeshNamePreSync = TGTCRenderMeshNode->GetVariableName().ToString();
	Test.TestTrue(
		"Collision Mesh Guid Naming", NameEndsWithGuid(CollisionMeshNamePreSync, "CollisionMesh_"));
	Test.TestTrue(
		"Render Mesh Guid Naming", NameEndsWithGuid(RenderMeshNamePreSync, "RenderMesh_"));

	if (!ReimportModel(*BlueprintBase, ArchiveFileName, false))
	{
		Test.AddError("ReimportModel returned false.");
		return true;
	}

	// Ensure the names for collision mesh / render mesh are still the same. These are tested
	// explicitly here to ensure naming conventions for those types are not changed without taking
	// into account the details outlined in SetUnnamedNameForPossibleCollisions in
	// AGX_ImporterToBlueprint.cpp.
	if (AgxAutomationCommon::IsAnyNullptr(TGTCCollisionMeshNode, TGTCRenderMeshNode))
	{
		Test.AddError(
			"Collision or render mesh Components was removed unexpectedly after model "
			"reimport.");
		return true;
	}

	Test.TestEqual(
		"Collision Mesh name", TGTCCollisionMeshNode->GetVariableName().ToString(),
		CollisionMeshNamePreSync);

	Test.TestEqual(
		"Render Mesh name", TGTCRenderMeshNode->GetVariableName().ToString(),
		RenderMeshNamePreSync);

	// Ensure we have the same number of nodes after the model Reimport as before.
	const int NumNodesReimportModel = BlueprintBase->SimpleConstructionScript->GetAllNodes().Num();
	if (NumNodesReimportModel != NumNodesFirstImport)
	{
		Test.AddError(FString::Printf(
			TEXT("Number of nodes was %d after import and %d after model reimport. "
				 "They are expected to be the same."),
			NumNodesFirstImport, NumNodesReimportModel));
		return true;
	}

	// Ensure no nodes have the name "AGX_Import_Unnamed..." name that is set in the beginning of a
	// model reimport. Note that a random GUID is appended after this, so we need to check
	// against the first part of the name.
	const FString UnnamedName = "AGX_Import_Unnamed";
	const FString UnsetUniqueImportName = FAGX_ImportUtilities::GetUnsetUniqueImportName();
	Test.TestTrue(
		"Unexpected Unset Unique Import Name unexpected",
		UnsetUniqueImportName.StartsWith(UnnamedName));
	for (USCS_Node* Node : BlueprintBase->SimpleConstructionScript->GetAllNodes())
	{
		Test.TestFalse("Invalid name", Node->GetVariableName().ToString().StartsWith(UnnamedName));
	}

	return true;
}

/**
 * Import a model and simply reimport against the same file as the original import. This is
 * somewhat a sanity-check test.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FReimportSameTest, "AGXUnreal.Editor.AGX_ReimportModelTest.SyncronizeSame",
	EAutomationTestFlags::ProductFilter | AgxAutomationCommon::ETF_ApplicationContextMask)

bool FReimportSameTest::RunTest(const FString& Parameters)
{
	const FString ArchiveFileName = "synchronize_same_build.agx";
	ADD_LATENT_AUTOMATION_COMMAND(FReimportSameCommand(ArchiveFileName, *this));
	ADD_LATENT_AUTOMATION_COMMAND(
		FDeleteImportedAssets(FPaths::GetBaseFilename(ArchiveFileName), *this));

	return true;
}

//
// Reimport large model test starts here.
//

DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(
	FReimportLargeModelCommand, FString, ArchiveFileName, FString, UpdatedArchiveFileName,
	FAutomationTestBase&, Test);

bool FReimportLargeModelCommand::Update()
{
	using namespace AGX_ReimportModelTest_helpers;
	using namespace AgxAutomationCommon;

	UBlueprint* Blueprint = Import(ArchiveFileName, false);
	if (Blueprint == nullptr)
	{
		Test.AddError("Imported Blueprint was nullptr.");
		return true;
	}

	UBlueprint* BlueprintBase = FAGX_BlueprintUtilities::GetOutermostParent(Blueprint);
	if (BlueprintBase == nullptr)
	{
		Test.AddError(
			"Could not get Blueprint parent (base) from the returned Blueprint after import.");
		return true;
	}

	if (!ReimportModel(*BlueprintBase, UpdatedArchiveFileName, false))
	{
		Test.AddError("ReimportModel returned false.");
		return true;
	}

	// Remember: Components in Blueprints have no attach parents setup. We need to check the SCS
	// Node tree for that information. We can do this by using the helper functions in the
	// AGX_ReimportModelTest_helpers namespace.

	// Important: we would like to get the components from the Blueprint child using the
	// GetComponentsFromChildBlueprint function in this file. Unfortunately that does not work while
	// Testing for some reason (see comment above that function). So we will have to look at the
	// base blueprint only.
	TArray<UActorComponent*> Components =
		FAGX_BlueprintUtilities::GetTemplateComponents(BlueprintBase);

	Test.TestTrue("Reimportd Components found.", Components.Num() > 0);

	// BodyToBeRenamed.
	{
		if (!CheckNodeNonExisting(*BlueprintBase, "BodyToBeRenamed"))
			return true; // Logging done in CheckNodeNonExisting.
	}

	// BodyWithNewName.
	{
		if (!CheckNodeNameAndParent(*BlueprintBase, "BodyWithNewName", "DefaultSceneRoot", true))
			return true; // Logging done in CheckNodeNameAndParent.
	}

	// SphereBodyToBeRemoved and any children.
	{
		if (!CheckNodeNonExisting(*BlueprintBase, "SphereBodyToBeRemoved"))
			return true; // Logging done in CheckNodeNonExisting.

		if (!CheckNodeNonExisting(*BlueprintBase, "SphereGeometryToBeRemoved"))
			return true; // Logging done in CheckNodeNonExisting.
	}

	// SphereBodyToChange and any children.
	{
		if (!CheckNodeNameAndParent(
				*BlueprintBase, "SphereBodyToChange", "DefaultSceneRoot", false))
			return true; // Logging done in CheckNodeNameAndParent.

		if (!CheckNodeNameAndParent(
				*BlueprintBase, "SphereGeometryToChange", "SphereBodyToChange", true))
			return true; // Logging done in CheckNodeNameAndParent.

		auto BodyToChange = AgxAutomationCommon::GetByName<UAGX_RigidBodyComponent>(
			Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("SphereBodyToChange"));

		auto ShapeToChange = AgxAutomationCommon::GetByName<UAGX_SphereShapeComponent>(
			Components,
			*FAGX_BlueprintUtilities::ToTemplateComponentName("SphereGeometryToChange"));

		auto ShapeMat = ShapeToChange != nullptr ? ShapeToChange->ShapeMaterial : nullptr;

		auto MergeSplitThresholds =
			BodyToChange != nullptr ? BodyToChange->MergeSplitProperties.Thresholds : nullptr;

		if (IsAnyNullptr(BodyToChange, ShapeToChange, ShapeMat, MergeSplitThresholds))
		{
			Test.AddError(
				TEXT("At least one required objects owned by SphereBodyToChange was nullptr, "
					 "cannot continue."));
			return true;
		}

		Test.TestEqual(
			"SphereBodyToChange location", AgxToUnrealDisplacement(-10.0, 1.0, 0.0),
			BodyToChange->GetRelativeLocation());
		Test.TestEqual("SphereBodyToChange mass", BodyToChange->GetMass(), 9.f);
		Test.TestEqual("SphereBodyToChange MST", MergeSplitThresholds->GetNormalAdhesion(), 132.0);

		Test.TestEqual(
			"SphereGeometryToChange location", AgxToUnrealDisplacement(0.0, 1.5, 0.0),
			ShapeToChange->GetRelativeLocation());
		Test.TestEqual(
			"SphereGeometryToChange radius", ShapeToChange->GetRadius(),
			AgxToUnrealDistance(0.25f));
		Test.TestEqual("SharedMaterial roughness", ShapeMat->GetRoughness(), 0.23);
		Test.TestEqual(
			"SphereGeometryToChange num collision groups", ShapeToChange->CollisionGroups.Num(), 3);
		Test.TestTrue(
			"SphereGeometryToChange collision groups",
			ShapeToChange->CollisionGroups.Contains(FName("Sphere1")) &&
				ShapeToChange->CollisionGroups.Contains(FName("Sphere2")) &&
				ShapeToChange->CollisionGroups.Contains(FName("Sphere3")));
	}

	// BoxBodyToLooseGeom
	{
		if (!CheckNodeNameAndParent(
				*BlueprintBase, "BoxBodyToLooseGeom", "DefaultSceneRoot", false))
			return true; // Logging done in CheckNodeNameAndParent.

		if (!CheckNodeNameAndParent(
				*BlueprintBase, "ObserverToChangeOwner", "BoxBodyToLooseGeom", true))
			return true; // Logging done in CheckNodeNameAndParent.
	}

	// BoxBodyToGainGeom and any children.
	{
		if (!CheckNodeNameAndParent(*BlueprintBase, "BoxBodyToGainGeom", "DefaultSceneRoot", false))
			return true; // Logging done in CheckNodeNameAndParent.

		if (!CheckNodeNameAndParent(*BlueprintBase, "BoxGeometryToMove", "BoxBodyToGainGeom", true))
			return true; // Logging done in CheckNodeNameAndParent.

		auto Body = AgxAutomationCommon::GetByName<UAGX_RigidBodyComponent>(
			Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("BoxBodyToGainGeom"));
		auto Shape = AgxAutomationCommon::GetByName<UAGX_BoxShapeComponent>(
			Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("BoxGeometryToMove"));
		auto ShapeMat = Shape != nullptr ? Shape->ShapeMaterial : nullptr;

		if (IsAnyNullptr(Body, Shape, ShapeMat))
		{
			Test.AddError(
				TEXT("At least one required objects owned by BoxBodyToGainGeom was nullptr, "
					 "cannot continue."));
			return true;
		}

		Test.TestEqual("BoxGeometryToMove num collision groups", Shape->CollisionGroups.Num(), 1);
		Test.TestTrue(
			"BoxGeometryToMove collision groups", Shape->CollisionGroups.Contains(FName("Box2")));
		Test.TestEqual("SharedMaterial roughness", ShapeMat->GetRoughness(), 0.23);

		// We also want to ensure that the BoxGeometryToMove and the SphereGeometryToChange Shape
		// Materials point to the same asset.
		auto ShapeToChange = AgxAutomationCommon::GetByName<UAGX_SphereShapeComponent>(
			Components,
			*FAGX_BlueprintUtilities::ToTemplateComponentName("SphereGeometryToChange"));
		auto ShapeMat2 = ShapeToChange != nullptr ? ShapeToChange->ShapeMaterial : nullptr;
		if (IsAnyNullptr(ShapeToChange, ShapeMat2))
		{
			Test.AddError(
				TEXT("At least one required objects for testing BoxBodyToGainGeom was nullptr, "
					 "cannot continue."));
			return true;
		}
		Test.TestEqual("Shared Shape Material same", ShapeMat, ShapeMat2);
	}

	// StandaloneCylToChange
	{
		if (!CheckNodeNameAndParent(
				*BlueprintBase, "StandaloneCylToChange", "DefaultSceneRoot", true))
			return true; // Logging done in CheckNodeNameAndParent.

		auto Shape = AgxAutomationCommon::GetByName<UAGX_CylinderShapeComponent>(
			Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("StandaloneCylToChange"));
		auto ShapeMat = Shape != nullptr ? Shape->ShapeMaterial : nullptr;

		if (IsAnyNullptr(Shape, ShapeMat))
		{
			Test.AddError(
				TEXT("At least one required objects owned by StandaloneCylToChange was nullptr, "
					 "cannot continue."));
			return true;
		}

		Test.TestEqual("CylinderMaterial roughness", ShapeMat->GetRoughness(), 0.24);
		Test.TestEqual(
			"StandaloneCylToChange height", Shape->GetHeight(), AgxToUnrealDistance(0.3f));
		Test.TestEqual(
			"StandaloneCylToChange location", AgxToUnrealDisplacement(-6.0, 1.0, 0.0),
			Shape->GetRelativeLocation());
		Test.TestEqual(
			"StandaloneCylToChange Shape Material Roughness", ShapeMat->GetRoughness(), 0.24);
		Test.TestTrue(
			"StandaloneCylToChange MSP",
			!Shape->MergeSplitProperties.bEnableMerge && !Shape->MergeSplitProperties.bEnableSplit);
	}

	// TrimeshBody and any children.
	{
		if (!CheckNodeNameAndParent(*BlueprintBase, "TrimeshBody", "DefaultSceneRoot", false))
			return true; // Logging done in CheckNodeNameAndParent.

		if (!CheckNodeNameAndParent(*BlueprintBase, "TrimeshGeomToChange", "TrimeshBody", false))
			return true; // Logging done in CheckNodeNameAndParent.

		// Ensure correct attach parent/child tree under the Trimesh node.
		USCS_Node* TrimeshGeomToChangeNode = GetNodeChecked(*BlueprintBase, "TrimeshGeomToChange");
		USCS_Node* TGTCCollisionMeshNode = GetOnlyAttachChildChecked(TrimeshGeomToChangeNode);
		USCS_Node* TGTCRenderMeshNode = GetOnlyAttachChildChecked(TGTCCollisionMeshNode);
		if (TGTCRenderMeshNode == nullptr)
		{
			Test.AddError("TGTCRenderMeshNode was nullptr");
			return true;
		}

		if (TGTCRenderMeshNode->GetChildNodes().Num() != 0)
		{
			Test.AddError(FString::Printf(
				TEXT("Expected TGTCRenderMeshNode to have zero children but it has %d."),
				TGTCRenderMeshNode->GetChildNodes().Num()));
			return true;
		}

		auto Shape = AgxAutomationCommon::GetByName<UAGX_TrimeshShapeComponent>(
			Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("TrimeshGeomToChange"));
		auto ShapeMat = Shape != nullptr ? Shape->ShapeMaterial : nullptr;
		auto RenderMesh = Cast<UStaticMeshComponent>(TGTCRenderMeshNode->ComponentTemplate);
		UMaterialInterface* RenderMat =
			RenderMesh != nullptr ? RenderMesh->GetMaterial(0) : nullptr;
		if (IsAnyNullptr(Shape, ShapeMat, RenderMesh, RenderMat))
		{
			Test.AddError(
				TEXT("At least one required objects owned by TrimeshBody was nullptr, "
					 "cannot continue."));
			return true;
		}

		Test.TestEqual(
			"TrimeshGeomToChange location", AgxToUnrealDisplacement(0.0, 1.0, 0.0),
			Shape->GetRelativeLocation());

		// Check the Diffuse Color.
		FVector4 ExpectedLinear(0.6f, 0.14f, 0.01f, 1.0f);
		FMaterialParameterInfo Info;
		Info.Name = FName("Diffuse");
		FLinearColor ActualLinear;
		if (!RenderMat->GetVectorParameterValue(Info, ActualLinear, false))
		{
			Test.AddError(FString::Printf(
				TEXT("Could not get diffuse color from RenderMaterial '%s'."),
				*RenderMat->GetName()));
			return true;
		}

		FVector4 Actual = FAGX_ImportUtilities::LinearToSRGB(ActualLinear);
		float Tolerance = 1.0f / 255.0f; // This is all the precision we have in a byte.
		AgxAutomationCommon::TestEqual(
			Test, *FString::Printf(TEXT("%s in %s"), *Info.Name.ToString(), *RenderMat->GetName()),
			Actual, ExpectedLinear, Tolerance);
	}

	// TrimeshBodyToLooseGeom.
	{
		if (!CheckNodeNameAndParent(
				*BlueprintBase, "TrimeshBodyToLooseGeom", "DefaultSceneRoot", true))
			return true; // Logging done in CheckNodeNameAndParent.
	}

	// TrimeshBodyToGainGeom and any children.
	{
		if (!CheckNodeNameAndParent(
				*BlueprintBase, "TrimeshBodyToGainGeom", "DefaultSceneRoot", false))
			return true; // Logging done in CheckNodeNameAndParent.

		if (!CheckNodeNameAndParent(
				*BlueprintBase, "TrimeshGeomToMove", "TrimeshBodyToGainGeom", false))
			return true; // Logging done in CheckNodeNameAndParent.

		USCS_Node* TrimeshGeomToMoveNode = GetNodeChecked(*BlueprintBase, "TrimeshGeomToMove");
		USCS_Node* TGTMCollisionMeshNode = GetOnlyAttachChildChecked(TrimeshGeomToMoveNode);
		USCS_Node* TGTMRenderMeshNode = GetOnlyAttachChildChecked(TGTMCollisionMeshNode);
		if (TGTMRenderMeshNode == nullptr)
		{
			Test.AddError("TGTMRenderMeshNode was nullptr");
			return true;
		}

		if (!CheckNodeNoChild(*BlueprintBase, TGTMRenderMeshNode->GetVariableName().ToString()))
			return true; // Logging done in CheckNodeNoChild.
	}

	// HingeToChange
	{
		if (!CheckNodeNameAndParent(*BlueprintBase, "HingeToChange", "DefaultSceneRoot", true))
			return true; // Logging done in CheckNodeNameAndParent.

		auto Constraint = AgxAutomationCommon::GetByName<UAGX_HingeConstraintComponent>(
			Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("HingeToChange"));
		if (Constraint == nullptr)
		{
			Test.AddError("HingeToChange was nullptr, cannot continue.");
			return true;
		}

		Test.TestEqual(
			"HingeToChange Compliance", Constraint->GetCompliance(EGenericDofIndex::Translational1),
			102.0);
		Test.TestEqual(
			"HingeToChange Body1", Constraint->BodyAttachment1.RigidBody.Name,
			FName("BodyWithNewName"));
	}

	// PrismaticToChange
	{
		if (!CheckNodeNameAndParent(*BlueprintBase, "PrismaticToChange", "DefaultSceneRoot", true))
			return true; // Logging done in CheckNodeNameAndParent.

		auto Constraint = AgxAutomationCommon::GetByName<UAGX_PrismaticConstraintComponent>(
			Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("PrismaticToChange"));
		if (Constraint == nullptr)
		{
			Test.AddError("PrismaticToChange was nullptr, cannot continue.");
			return true;
		}

		Test.TestEqual(
			"PrismaticToChange Damping", Constraint->GetSpookDamping(EGenericDofIndex::Rotational2),
			202.0);
	}

	// BallCToBeRemoved
	{
		if (!CheckNodeNonExisting(*BlueprintBase, "BallCToBeRemoved"))
			return true; // Logging done in CheckNodeNonExisting.
	}

	// NewHinge
	{
		if (!CheckNodeNameAndParent(*BlueprintBase, "NewHinge", "DefaultSceneRoot", true))
			return true; // Logging done in CheckNodeNameAndParent.

		auto Constraint = AgxAutomationCommon::GetByName<UAGX_HingeConstraintComponent>(
			Components, *FAGX_BlueprintUtilities::ToTemplateComponentName("NewHinge"));
		if (Constraint == nullptr)
		{
			Test.AddError("NewHinge was nullptr, cannot continue.");
			return true;
		}

		Test.TestEqual(
			"NewHinge Body1", Constraint->BodyAttachment1.RigidBody.Name,
			FName("TrimeshBodyToGainGeom"));

		Test.TestEqual(
			"NewHinge Body2", Constraint->BodyAttachment2.RigidBody.Name,
			FName("TrimeshBodyToLooseGeom"));
	}

	// AGX_ModelSource.
	{
		if (!CheckNodeNameAndEnsureNoParent(*BlueprintBase, "AGX_ModelSource"))
			return true; // Logging done in CheckNodeNameAndEnsureNoParent.
	}

	// AGX_ContactmaterialRegistrar.
	{
		const FString CMRName = FAGX_ImportUtilities::GetContactMaterialRegistrarDefaultName();
		if (!CheckNodeNameAndEnsureNoParent(*BlueprintBase, CMRName))
			return true; // Logging done in CheckNodeNameAndEnsureNoParent.

		auto CMRegistrar = AgxAutomationCommon::GetByName<UAGX_ContactMaterialRegistrarComponent>(
			Components, *FAGX_BlueprintUtilities::ToTemplateComponentName(CMRName));
		if (CMRegistrar == nullptr)
		{
			Test.AddError("Contact Material Registrar was nullptr. Cannot continue.");
			return true;
		}

		Test.TestEqual("CMRegistrar num CM", CMRegistrar->ContactMaterials.Num(), 2);
		if (CMRegistrar->ContactMaterials.Num() == 2)
		{
			auto CmSharedCyl = CMRegistrar->ContactMaterials.FindByPredicate(
				[](auto* Cm) { return Cm->GetName().Contains("Shared"); });
			auto CmCylTri = CMRegistrar->ContactMaterials.FindByPredicate(
				[](auto* Cm) { return Cm->GetName().Contains("Trimesh"); });

			if (IsAnyNullptr(CmSharedCyl, CmCylTri))
			{
				Test.AddError(
					TEXT("At least one required Contact Material could not be found in the Contact "
						 "Material Registrar, cannot continue."));
				return true;
			}

			Test.TestEqual(
				"CmSharedCyl Friction Coefficient", (*CmSharedCyl)->GetFrictionCoefficient(), 0.12);
			Test.TestEqual(
				"CmCylTri Friction Coefficient", (*CmCylTri)->GetFrictionCoefficient(), 0.23);
		}
	}

	// AGX_CollisionGroupDisabler
	{
		const FString CGDName = FAGX_ImportUtilities::GetCollisionGroupDisablerDefaultName();
		if (!CheckNodeNameAndEnsureNoParent(*BlueprintBase, *CGDName))
			return true; // Logging done in CheckNodeNameAndEnsureNoParent.

		auto CGDisabler = AgxAutomationCommon::GetByName<UAGX_CollisionGroupDisablerComponent>(
			Components, *FAGX_BlueprintUtilities::ToTemplateComponentName(CGDName));
		if (CGDisabler == nullptr)
		{
			Test.AddError("Collision Group Disabler was nullptr. Cannot continue.");
			return true;
		}

		Test.TestEqual("CGDisabler num groups", CGDisabler->DisabledCollisionGroupPairs.Num(), 2);
		const TArray<FAGX_CollisionGroupPair> ExpectedGroups = {
			{"Sphere1", "Box2"}, {"Sphere2", "Sphere2"}};

		if (CGDisabler->DisabledCollisionGroupPairs.Num() == 2)
		{
			Test.TestTrue(
				"CGDisabler group 0",
				CGDisabler->DisabledCollisionGroupPairs[0].IsIn(ExpectedGroups));

			Test.TestTrue(
				"CGDisabler group 1",
				CGDisabler->DisabledCollisionGroupPairs[1].IsIn(ExpectedGroups));
		}
	}

	return true;
}

/**
 * Imports the large_model.agx file and then reimports against large_model_updated.agx where many
 * things are changed from the original.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FReimportLargeModelTest, "AGXUnreal.Editor.AGX_ReimportModelTest.ReimportLargeModel",
	EAutomationTestFlags::ProductFilter | AgxAutomationCommon::ETF_ApplicationContextMask)

bool FReimportLargeModelTest::RunTest(const FString& Parameters)
{
	const FString ArchiveFileName = "large_model_build.agx";
	const FString UpdatedArchiveFileName = "large_model_updated_build.agx";
	ADD_LATENT_AUTOMATION_COMMAND(
		FReimportLargeModelCommand(ArchiveFileName, UpdatedArchiveFileName, *this));
	ADD_LATENT_AUTOMATION_COMMAND(
		FDeleteImportedAssets(FPaths::GetBaseFilename(ArchiveFileName), *this));

	return true;
}

//
// IgnoreDisabledTrimesh false then true test starts here.
//

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(
	FIgnoreDisabledTrimeshFTCommand, FString, ArchiveFileName, FAutomationTestBase&, Test);

bool FIgnoreDisabledTrimeshFTCommand::Update()
{
	using namespace AGX_ReimportModelTest_helpers;
	using namespace AgxAutomationCommon;

	// Import with IgnoreDisabledTrimesh set to false.
	UBlueprint* Blueprint = Import(ArchiveFileName, false);
	if (Blueprint == nullptr)
	{
		Test.AddError("Imported Blueprint was nullptr.");
		return true;
	}

	UBlueprint* BlueprintBase = FAGX_BlueprintUtilities::GetOutermostParent(Blueprint);
	if (BlueprintBase == nullptr)
	{
		Test.AddError(
			"Could not get Blueprint parent (base) from the returned Blueprint after import.");
		return true;
	}

	// Pre-reimport.
	{
		if (!CheckNodeNameAndParent(*BlueprintBase, "TrimeshBody", "DefaultSceneRoot", false))
			return true; // Logging done in CheckNodeNameAndParent.

		if (!CheckNodeNameAndParent(*BlueprintBase, "TrimeshGeomDisabled", "TrimeshBody", false))
			return true; // Logging done in CheckNodeNameAndParent.

		// Ensure correct attach parent/child tree under the Trimesh node.
		USCS_Node* TrimeshGeomToChangeNode = GetNodeChecked(*BlueprintBase, "TrimeshGeomDisabled");
		USCS_Node* CollisionMeshNode = GetOnlyAttachChildChecked(TrimeshGeomToChangeNode);
		USCS_Node* RenderMeshNode = GetOnlyAttachChildChecked(CollisionMeshNode);
		if (RenderMeshNode == nullptr)
		{
			Test.AddError("RenderMeshNode was nullptr");
			return true;
		}

		if (RenderMeshNode->GetChildNodes().Num() != 0)
		{
			Test.AddError(FString::Printf(
				TEXT("Expected RenderMeshNode to have zero children but it has %d."),
				RenderMeshNode->GetChildNodes().Num()));
			return true;
		}
	}

	// Reimport with the IgnoreDisabledTrimesh setting true.
	if (!ReimportModel(*BlueprintBase, ArchiveFileName, true))
	{
		Test.AddError("ReimportModel returned false.");
		return true;
	}

	// Post-reimport. Now the render mesh should be attached immediately under
	// the body.
	{
		if (!CheckNodeNameAndParent(*BlueprintBase, "TrimeshBody", "DefaultSceneRoot", false))
			return true; // Logging done in CheckNodeNameAndParent.

		if (!CheckNodeNonExisting(*BlueprintBase, "TrimeshGeomDisabled"))
			return true; // Logging done in CheckNodeNonExisting.

		// Ensure correct attach parent/child tree under the Body node.
		USCS_Node* TrimeshBodyNode = GetNodeChecked(*BlueprintBase, "TrimeshBody");
		USCS_Node* RenderMeshNode = GetOnlyAttachChildChecked(TrimeshBodyNode);
		if (RenderMeshNode == nullptr)
		{
			Test.AddError("RenderMeshNode was nullptr");
			return true;
		}

		if (RenderMeshNode->GetChildNodes().Num() != 0)
		{
			Test.AddError(FString::Printf(
				TEXT("Expected RenderMeshNode to have zero children but it has %d."),
				RenderMeshNode->GetChildNodes().Num()));
			return true;
		}
	}

	return true;
}

/**
 * Import model with a disabled Trimesh first with IgnoreDisabledTrimesh false, then reimport
 * against the same model but this time with IgnoreDisabledTrimesh true.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIgnoreDisabledTrimeshFTTest, "AGXUnreal.Editor.AGX_ReimportModelTest.IgnoreDisabledTrimeshFT",
	EAutomationTestFlags::ProductFilter | AgxAutomationCommon::ETF_ApplicationContextMask)

bool FIgnoreDisabledTrimeshFTTest::RunTest(const FString& Parameters)
{
	const FString ArchiveFileName = "disabled_trimesh_ft.agx";
	ADD_LATENT_AUTOMATION_COMMAND(FIgnoreDisabledTrimeshFTCommand(ArchiveFileName, *this));
	ADD_LATENT_AUTOMATION_COMMAND(
		FDeleteImportedAssets(FPaths::GetBaseFilename(ArchiveFileName), *this));

	return true;
}

//
// IgnoreDisabledTrimesh true then false test starts here.
//

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(
	FIgnoreDisabledTrimeshTFCommand, FString, ArchiveFileName, FAutomationTestBase&, Test);

bool FIgnoreDisabledTrimeshTFCommand::Update()
{
	using namespace AGX_ReimportModelTest_helpers;
	using namespace AgxAutomationCommon;

	// Import with IgnoreDisabledTrimesh true.
	UBlueprint* Blueprint = Import(ArchiveFileName, true);
	if (Blueprint == nullptr)
	{
		Test.AddError("Imported Blueprint was nullptr.");
		return true;
	}

	UBlueprint* BlueprintBase = FAGX_BlueprintUtilities::GetOutermostParent(Blueprint);
	if (BlueprintBase == nullptr)
	{
		Test.AddError(
			"Could not get Blueprint parent (base) from the returned Blueprint after import.");
		return true;
	}

	// Pre-reimport. Now the collision mesh + render mesh should be attached immediately under
	// the body.
	{
		if (!CheckNodeNameAndParent(*BlueprintBase, "TrimeshBody", "DefaultSceneRoot", false))
			return true; // Logging done in CheckNodeNameAndParent.

		if (!CheckNodeNonExisting(*BlueprintBase, "TrimeshGeomDisabled"))
			return true; // Logging done in CheckNodeNonExisting.

		// Ensure correct attach parent/child tree under the Body node.
		USCS_Node* TrimeshBodyNode = GetNodeChecked(*BlueprintBase, "TrimeshBody");
		USCS_Node* RenderMeshNode = GetOnlyAttachChildChecked(TrimeshBodyNode);
		if (RenderMeshNode == nullptr)
		{
			Test.AddError("RenderMeshNode was nullptr");
			return true;
		}

		if (RenderMeshNode->GetChildNodes().Num() != 0)
		{
			Test.AddError(FString::Printf(
				TEXT("Expected RenderMeshNode to have zero children but it has %d."),
				RenderMeshNode->GetChildNodes().Num()));
			return true;
		}
	}

	// Reimport with the the IgnoreDisabledTrimesh setting false.
	if (!ReimportModel(*BlueprintBase, ArchiveFileName, false))
	{
		Test.AddError("ReimportModel returned false.");
		return true;
	}

	// Post-reimport. The Disabled Trimesh should now exist again.
	{
		if (!CheckNodeNameAndParent(*BlueprintBase, "TrimeshBody", "DefaultSceneRoot", false))
			return true; // Logging done in CheckNodeNameAndParent.

		if (!CheckNodeNameAndParent(*BlueprintBase, "TrimeshGeomDisabled", "TrimeshBody", false))
			return true; // Logging done in CheckNodeNameAndParent.

		// Ensure correct attach parent/child tree under the Trimesh node.
		USCS_Node* TrimeshGeomToChangeNode = GetNodeChecked(*BlueprintBase, "TrimeshGeomDisabled");
		USCS_Node* CollisionMeshNode = GetOnlyAttachChildChecked(TrimeshGeomToChangeNode);
		USCS_Node* RenderMeshNode = GetOnlyAttachChildChecked(CollisionMeshNode);
		if (RenderMeshNode == nullptr)
		{
			Test.AddError("RenderMeshNode was nullptr");
			return true;
		}

		if (RenderMeshNode->GetChildNodes().Num() != 0)
		{
			Test.AddError(FString::Printf(
				TEXT("Expected RenderMeshNode to have zero children but it has %d."),
				RenderMeshNode->GetChildNodes().Num()));
			return true;
		}
	}

	return true;
}

/**
 * Import model with a disabled Trimesh first with IgnoreDisabledTrimesh true, then reimport
 * against the same model but this time with IgnoreDisabledTrimesh false.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIgnoreDisabledTrimeshTFTest, "AGXUnreal.Editor.AGX_ReimportModelTest.IgnoreDisabledTrimeshTF",
	EAutomationTestFlags::ProductFilter | AgxAutomationCommon::ETF_ApplicationContextMask)

bool FIgnoreDisabledTrimeshTFTest::RunTest(const FString& Parameters)
{
	const FString ArchiveFileName = "disabled_trimesh_tf.agx";
	ADD_LATENT_AUTOMATION_COMMAND(FIgnoreDisabledTrimeshTFCommand(ArchiveFileName, *this));
	ADD_LATENT_AUTOMATION_COMMAND(
		FDeleteImportedAssets(FPaths::GetBaseFilename(ArchiveFileName), *this));

	return true;
}

//
// Merge Split Thresholds reimport tests start here.
//

/**
 * Import model with a Rigid Body, a Hinge, and a Constraint Merge Split Thresholds. Then
 * reimport with an updated model where the thresholds has been removed from the hinge. The
 * Constraint Merge Split Thresholds asset should be deleted and the reference to it cleared.
 */
class FRemoveConstraintMergeSplitThresholdsTest final : public FReimportModelTest
{
public:
	FRemoveConstraintMergeSplitThresholdsTest()
		: FReimportModelTest(
			  TEXT("FRemoveConstraintMergeSplitThresholdsTest"),
			  TEXT("AGXUnreal.Editor.AGX_ReimportModelTest.RemoveConstraintMergeSplitThresholds"),
			  TEXT("thresholds_remove__initial.agx"), TEXT("thresholds_remove__updated.agx"))
	{
	}

	FString AssetPath;

	virtual bool PostImport() override
	{
		using namespace AGX_ReimportModelTest_helpers;

		// Make sure we got the template Components we expect.
		// 1 Default Scene Root, 1 Model Source, 1 Rigid Body, 1 Hinge.
		if (!TestEqual(
				TEXT("Number of imported components before reimport"),
				InitialTemplateComponents.Num(), 4))
		{
			PrintComponentNames(InitialTemplateComponents, TEXT("Initial template components"));
			return false;
		}

		// Check the Blueprint.
		UAGX_HingeConstraintComponent* TemplateHinge =
			GetTemplateComponentByName<UAGX_HingeConstraintComponent>(
				InitialTemplateComponents, TEXT("Hinge"));
		if (!TestNotNull(TEXT("Template Hinge before reimport"), TemplateHinge))
		{
			return false;
		}
		UAGX_ConstraintMergeSplitThresholds* TemplateThresholds =
			TemplateHinge->MergeSplitProperties.Thresholds;
		if (!TestNotNull(TEXT("Template Hinge Thresholds before reimport"), TemplateThresholds))
		{
			return false;
		}

		// Check the Blueprint instance.
		UAGX_HingeConstraintComponent* HingeInstance =
			FAGX_ObjectUtilities::GetComponentByName<UAGX_HingeConstraintComponent>(
				*InitialBlueprintInstance, TEXT("Hinge"));
		if (!TestNotNull(TEXT("Hinge instance before reimport"), HingeInstance))
		{
			return false;
		}
		UAGX_ConstraintMergeSplitThresholds* ThresholdsInstance =
			HingeInstance->MergeSplitProperties.Thresholds;
		if (!TestNotNull(TEXT("Hinge instance thresholds before reimport"), ThresholdsInstance))
		{
			return false;
		}

		// The template and the instance should point to the same asset.
		if (!TestEqual(TEXT("Template and instance asset"), TemplateThresholds, ThresholdsInstance))
		{
			return false;
		}

		// Check the asset file.
		const FString AssetName =
			FString::Printf(TEXT("AGX_CMST_%s.uasset"), *TemplateThresholds->ImportGuid.ToString());
		AssetPath =
			GetAssetPath<UAGX_ConstraintMergeSplitThresholds>(InitialModelFileName, AssetName);
		if (!TestTrue(
				TEXT("Thresholds asset exists before reimport"), FPaths::FileExists(AssetPath)))
		{
			return false;
		}

		return true;
	}

	virtual bool PostReimport() override
	{
		using namespace AGX_ReimportModelTest_helpers;

		// There are three places where the thresholds should have been removed:
		// - Reference in the Hinge template should have been cleared to nullptr.
		// - Reference in the Hinge instance should have been cleared to nullptr.
		// - Asset file on drive should have been deleted.

		// Reference in the Hinge template should have been cleared to nullptr.
		UAGX_HingeConstraintComponent* TemplateHinge =
			GetTemplateComponentByName<UAGX_HingeConstraintComponent>(
				UpdatedTemplateComponents, TEXT("Hinge"));
		if (!TestNotNull(TEXT("Template Hinge after reimport"), TemplateHinge))
		{
			return false;
		}
		const UAGX_ConstraintMergeSplitThresholds* TemplateThresholds =
			TemplateHinge->MergeSplitProperties.Thresholds;
		if (!TestNull(TEXT("Template Hinge Thresholds after reimport"), TemplateThresholds))
		{
			return false;
		}

		// Reference in the Hinge instance should have been cleared to nullptr.
		UAGX_HingeConstraintComponent* HingeInstance =
			FAGX_ObjectUtilities::GetComponentByName<UAGX_HingeConstraintComponent>(
				*UpdatedBlueprintInstance, TEXT("Hinge"));
		if (!TestNotNull(TEXT("Hinge instance after reimport"), HingeInstance))
		{
			return false;
		}
		const UAGX_ConstraintMergeSplitThresholds* ThresholdsInstance =
			HingeInstance->MergeSplitProperties.Thresholds;
		if (!TestNull(TEXT("Hinge instance thresholds after reimport"), ThresholdsInstance))
		{
			return false;
		}

		// Asset file on drive should have been deleted.
		if (!TestFalse(
				TEXT("Thresholds asset removed after reimport"), FPaths::FileExists(AssetPath)))
		{
			return false;
		}

		return true;
	}
};

namespace
{
	FRemoveConstraintMergeSplitThresholdsTest RemoveConstraintMergeSplitThresholdsTest;
}

/**
 * Import model with a Rigid Body, a Hinge, and a Constraint Merge Split Thresholds. Then
 * reimport with an updated model where all values held by the thresholds has been modified.
 */
class FModifyConstraintMergeSplitThresholdsTest final : public FReimportModelTest
{
public:
	FModifyConstraintMergeSplitThresholdsTest()
		: FReimportModelTest(
			  TEXT("FModifyConstraintMergeSplitThresholdsTest"),
			  TEXT("AGXUnreal.Editor.AGX_ReimportModelTest.ModifyConstraintMergeSplitThresholds"),
			  TEXT("thresholds_modify__initial.agx"), TEXT("thresholds_modify__updated.agx"))
	{
	}

	FString AssetPath;

	virtual bool PostImport() override
	{
		using namespace AGX_ReimportModelTest_helpers;

		// Make sure we got the template Components we expect.
		// 1 Default Scene Root, 1 Model Source, 1 Rigid Body, 1 Hinge.
		if (!TestEqual(
				TEXT("Number of imported components before reimport"),
				InitialTemplateComponents.Num(), 4))
		{
			PrintComponentNames(InitialTemplateComponents, TEXT("Initial template components"));
			return false;
		}

		// Check the Blueprint.
		UAGX_HingeConstraintComponent* TemplateHinge =
			GetTemplateComponentByName<UAGX_HingeConstraintComponent>(
				InitialTemplateComponents, TEXT("Hinge"));
		if (!TestNotNull(TEXT("Template Hinge before reimport"), TemplateHinge))
		{
			return false;
		}
		UAGX_ConstraintMergeSplitThresholds* TemplateThresholds =
			TemplateHinge->MergeSplitProperties.Thresholds;
		if (!TestNotNull(TEXT("Template Hinge Thresholds before reimport"), TemplateThresholds))
		{
			return false;
		}
		if (!CheckThresholds(TemplateThresholds, 1.0))
		{
			return false;
		}

		// Check the Blueprint instance.
		UAGX_HingeConstraintComponent* HingeInstance =
			FAGX_ObjectUtilities::GetComponentByName<UAGX_HingeConstraintComponent>(
				*InitialBlueprintInstance, TEXT("Hinge"));
		if (!TestNotNull(TEXT("Hinge instance before reimport"), HingeInstance))
		{
			return false;
		}
		UAGX_ConstraintMergeSplitThresholds* ThresholdsInstance =
			HingeInstance->MergeSplitProperties.Thresholds;
		if (!TestNotNull(TEXT("Hinge instance thresholds before reimport"), ThresholdsInstance))
		{
			return false;
		}
		if (!CheckThresholds(ThresholdsInstance, 1.0))
		{
			return false;
		}

		// Check asset on drive.
		const FString AssetName =
			FString::Printf(TEXT("AGX_CMST_%s"), *TemplateThresholds->ImportGuid.ToString());

		const FString ImportDirPath = FString::Printf(
			TEXT("/Game/%s/%s/"), *FAGX_ImportUtilities::GetImportRootDirectoryName(),
			*InitialModelName);

		AssetPath = FString::Printf(
			TEXT("%s%s"),
			*FAGX_ImportUtilities::CreatePackagePath(
				ImportDirPath, FAGX_ImportUtilities::GetImportMergeSplitThresholdsDirectoryName()),
			*AssetName);

		UAGX_ConstraintMergeSplitThresholds* Asset =
			LoadObject<UAGX_ConstraintMergeSplitThresholds>(nullptr, *AssetPath);
		if (!TestNotNull(TEXT("Thresholds asset before reimport"), Asset))
		{
			return false;
		}
		if (!CheckThresholds(Asset, 1.0))
		{
			return false;
		}
		return true;
	}

	virtual bool PostReimport() override
	{
		using namespace AGX_ReimportModelTest_helpers;

		// There are three places where the thresholds should have been modified:
		// - Thresholds in the Hinge template.
		// - Thresholds in the Hinge instance.
		// - Asset file on drive.

		// Check the Hinge template
		UAGX_HingeConstraintComponent* TemplateHinge =
			GetTemplateComponentByName<UAGX_HingeConstraintComponent>(
				UpdatedTemplateComponents, TEXT("Hinge"));
		if (!TestNotNull(TEXT("Template Hinge after reimport"), TemplateHinge))
		{
			return false;
		}
		const UAGX_ConstraintMergeSplitThresholds* TemplateThresholds =
			TemplateHinge->MergeSplitProperties.Thresholds;
		if (!TestNotNull(TEXT("Template Hinge Thresholds after reimport"), TemplateThresholds))
		{
			return false;
		}
		if (!CheckThresholds(TemplateThresholds, 10.0))
		{
			return false;
		}

		// Reference in the Hinge instance should have been cleared to nullptr.
		UAGX_HingeConstraintComponent* HingeInstance =
			FAGX_ObjectUtilities::GetComponentByName<UAGX_HingeConstraintComponent>(
				*UpdatedBlueprintInstance, TEXT("Hinge"));
		if (!TestNotNull(TEXT("Hinge instance after reimport"), HingeInstance))
		{
			return false;
		}
		const UAGX_ConstraintMergeSplitThresholds* ThresholdsInstance =
			HingeInstance->MergeSplitProperties.Thresholds;
		if (!TestNotNull(TEXT("Hinge instance thresholds after reimport"), ThresholdsInstance))
		{
			return false;
		}
		if (!CheckThresholds(ThresholdsInstance, 10.0))
		{
			return false;
		}

		// Check asset on drive.
		UAGX_ConstraintMergeSplitThresholds* Asset =
			LoadObject<UAGX_ConstraintMergeSplitThresholds>(nullptr, *AssetPath);
		if (!TestNotNull(TEXT("Thresholds asset before reimport"), Asset))
		{
			return false;
		}
		if (!CheckThresholds(Asset, 10.0))
		{
			return false;
		}

		return true;
	}

	bool CheckThresholds(const UAGX_ConstraintMergeSplitThresholds* Thresholds, double Factor)
	{
		bool Success = true;
		Success &= TestEqual(
			TEXT("MaxDesiredForceRangeDiff"), Thresholds->MaxDesiredForceRangeDiff, Factor * 1.0);
		Success &= TestEqual(
			TEXT("MaxDesiredLockAngleDiff"), Thresholds->MaxDesiredLockAngleDiff,
			FromRad(Factor * 2.0));
		Success &= TestEqual(
			TEXT("MaxDesiredRangeAngleDiff"), Thresholds->MaxDesiredRangeAngleDiff,
			FromRad(Factor * 3.0));
		Success &= TestEqual(
			TEXT("MaxDesiredSpeedDiff"), Thresholds->MaxDesiredSpeedDiff, FromRad(Factor * 4.0));
		Success &= TestEqual(
			TEXT("MaxRelativeSpeed"), Thresholds->MaxRelativeSpeed, FromRad(Factor * 5.0));
		return Success;
	}
};

namespace
{
	FModifyConstraintMergeSplitThresholdsTest ModifyConstraintMergeSplitThresholdsTest;
}

//
// Cylindrical Constraint test starts here.
//

class FModifyCylindricalConstraintTest final : public FReimportModelTest
{
public:
	FModifyCylindricalConstraintTest()
		: FReimportModelTest(
			  TEXT("ModifyCylindricalConstraint"),
			  TEXT("AGXUnreal.Editor.AGX_ReimportModelTest.ModifyCylindricalConstraint"),
			  TEXT("cylindrical_constraint__initial.agx"),
			  TEXT("cylindrical_constraint__updated.agx"))
	{
	}

	bool CheckCylindricalConstraint(
		UAGX_CylindricalConstraintComponent& Cylindrical, double Scale, EAGX_SolveType SolveType,
		bool bEnable)
	{
		bool AllCorrect = true;
		AllCorrect &= TestEqual(TEXT("Template Cylindrical enabled"), Cylindrical.bEnable, bEnable);
		AllCorrect &=
			TestEqual(TEXT("Template Cylindrical solve type"), Cylindrical.SolveType, SolveType);
		AllCorrect &= TestEqual(
			TEXT("Template Cylindrical compliance rotat 1"), Cylindrical.Compliance.Rotational_1,
			Scale * 1.0);
		AllCorrect &= TestEqual(
			TEXT("Template Cylindrical compliance rotat 1"), Cylindrical.Compliance.Rotational_2,
			Scale * 1.1);
		AllCorrect &= TestEqual(
			TEXT("Template Cylindrical compliance trans 1"), Cylindrical.Compliance.Translational_1,
			Scale * 1.2);
		AllCorrect &= TestEqual(
			TEXT("Template Cylindrical compliance trans 2"), Cylindrical.Compliance.Translational_2,
			Scale * 1.3);
		AllCorrect &= TestEqual(
			TEXT("Template Cylindrical Spook damping rotat 1"),
			Cylindrical.SpookDamping.Rotational_1, Scale * 2.0);
		AllCorrect &= TestEqual(
			TEXT("Template Cylindrical Spook damping rotat 2"),
			Cylindrical.SpookDamping.Rotational_2, Scale * 2.1);
		AllCorrect &= TestEqual(
			TEXT("Template Cylindrical Spook damping trans 1"),
			Cylindrical.SpookDamping.Translational_1, Scale * 2.2);
		AllCorrect &= TestEqual(
			TEXT("Template Cylindrical Spook damping trans 2"),
			Cylindrical.SpookDamping.Translational_2, Scale * 2.3);
		AllCorrect &= AgxAutomationCommon::TestEqual(
			*this, TEXT("Template Cylindrical rotational 1 force range"),
			Cylindrical.ForceRange.Rotational_1, FAGX_RealInterval(Scale * 3.0, Scale * 4.0));
		AllCorrect &= AgxAutomationCommon::TestEqual(
			*this, TEXT("Template Cylindrical rotational 2 force range"),
			Cylindrical.ForceRange.Rotational_2, FAGX_RealInterval(Scale * 3.1, Scale * 4.1));
		AllCorrect &= AgxAutomationCommon::TestEqual(
			*this, TEXT("Template Cylindrical translational 1 force range"),
			Cylindrical.ForceRange.Translational_1, FAGX_RealInterval(Scale * 3.2, Scale * 4.2));
		AllCorrect &= AgxAutomationCommon::TestEqual(
			*this, TEXT("Template Cylindrical translational 2 force range"),
			Cylindrical.ForceRange.Translational_2, FAGX_RealInterval(Scale * 3.3, Scale * 4.3));
		// Cylindrical Constraint does not have rotational compliance, damping, or force range since
		// all rotational degrees of freedom are free.
		AllCorrect &= TestEqual(
			TEXT("Template Cylindrical compute forces"), Cylindrical.bComputeForces, bEnable);
		AllCorrect &= TestEqual(
			TEXT("Template Cylindrical twist range enabled"), Cylindrical.ScrewController.bEnable,
			bEnable);
		AllCorrect &= TestEqual(
			TEXT("Template Cylindrical twist range compliance"),
			Cylindrical.ScrewController.Compliance, Scale * 5.0);
		AllCorrect &= TestEqual(
			TEXT("Template Cylindrical twist range damping"),
			Cylindrical.ScrewController.SpookDamping, Scale * 6.0);
		AllCorrect &= AgxAutomationCommon::TestEqual(
			*this, TEXT("Template Cylindrical twist range force range"),
			Cylindrical.ScrewController.ForceRange, FAGX_RealInterval(Scale * 7.0, Scale * 8.0));
		AllCorrect &= TestEqual(
			TEXT("Template Cylindrical lead"), (Cylindrical.ScrewController.Lead),
			Scale * AgxAutomationCommon::AgxToUnrealDistance(9.0));

		return AllCorrect;
	}

	virtual bool PostImport() override
	{
		// Make sure we got the template Components we expect.
		// 1 Default Scene Root, 1 Model Source, 1 Rigid Body, 1 Cylindrical Constraint.
		if (!TestEqual(
				TEXT("Number of imported components before reimport"),
				InitialTemplateComponents.Num(), 4))
		{
			PrintComponentNames(InitialTemplateComponents, TEXT("Initial template components"));
			return false;
		}

		// Check the Blueprint.
		UAGX_CylindricalConstraintComponent* CylindricalTemplate =
			GetTemplateComponentByName<UAGX_CylindricalConstraintComponent>(
				InitialTemplateComponents, TEXT("Cylindrical"));
		if (!TestNotNull(TEXT("Template Cylindrical before reimport"), CylindricalTemplate))
		{
			return false;
		}
		if (!CheckCylindricalConstraint(
				*CylindricalTemplate, 1.0, EAGX_SolveType::StDirectAndIterative, true))
		{
			return false;
		}

		// Check the Blueprint instance.
		UAGX_CylindricalConstraintComponent* CylindricalInstance =
			FAGX_ObjectUtilities::GetComponentByName<UAGX_CylindricalConstraintComponent>(
				*InitialBlueprintInstance, TEXT("Cylindrical"));
		if (!TestNotNull(
				TEXT("Cylindrical Constraint instance before reimport"), CylindricalInstance))
		{
			return false;
		}
		if (!CheckCylindricalConstraint(
				*CylindricalInstance, 1.0, EAGX_SolveType::StDirectAndIterative, true))
		{
			return false;
		}

		return true;
	}

	virtual bool PostReimport() override
	{
		// Make sure we got the template Components we expect.
		// 1 Default Scene Root, 1 Model Source, 1 Rigid Body, 1 Cylindrical Constraint.
		if (!TestEqual(
				TEXT("Number of imported components before reimport"),
				UpdatedTemplateComponents.Num(), 4))
		{
			PrintComponentNames(UpdatedTemplateComponents, TEXT("Updated template components"));
			return false;
		}

		// Check the Blueprint.
		UAGX_CylindricalConstraintComponent* CylindricalTemplate =
			GetTemplateComponentByName<UAGX_CylindricalConstraintComponent>(
				UpdatedTemplateComponents, TEXT("Cylindrical"));
		if (!TestNotNull(TEXT("Template Cylindrical before reimport"), CylindricalTemplate))
		{
			return false;
		}
		if (!CheckCylindricalConstraint(
				*CylindricalTemplate, 10.0, EAGX_SolveType::StDirect, false))
		{
			return false;
		}

		// Check the Blueprint instance.
		UAGX_CylindricalConstraintComponent* CylindricalInstance =
			FAGX_ObjectUtilities::GetComponentByName<UAGX_CylindricalConstraintComponent>(
				*UpdatedBlueprintInstance, TEXT("Cylindrical"));
		if (!TestNotNull(
				TEXT("Cylindrical Constraint instance before reimport"), CylindricalInstance))
		{
			return false;
		}
		if (!CheckCylindricalConstraint(
				*CylindricalInstance, 10.0, EAGX_SolveType::StDirect, false))
		{
			return false;
		}

		return true;
	}
};

namespace
{
	FModifyCylindricalConstraintTest ModifyCylindricalConstraintTest;
}

//
// Ball Constraint modified test starts here.
//

class FModifyBallConstraintTest final : public FReimportModelTest
{
public:
	FModifyBallConstraintTest()
		: FReimportModelTest(
			  TEXT("ModifyBallConstraint"),
			  TEXT("AGXUnreal.Editor.AGX_ReimportModelTest.ModifyBallConstraint"),
			  TEXT("ball_constraint__initial1.agx"), TEXT("ball_constraint__updated.agx"))
	{
	}

	bool CheckBallConstraint(
		UAGX_BallConstraintComponent& Ball, double Scale, EAGX_SolveType SolveType, bool bEnable)
	{
		bool AllCorrect = true;
		AllCorrect &= TestEqual(TEXT("Enabled"), Ball.bEnable, bEnable);
		AllCorrect &= TestEqual(TEXT("Solve type"), Ball.SolveType, SolveType);
		AllCorrect &=
			TestEqual(TEXT("Compliance trans 1"), Ball.Compliance.Translational_1, Scale * 1.0);
		AllCorrect &=
			TestEqual(TEXT("Compliance trans 2"), Ball.Compliance.Translational_2, Scale * 1.1);
		AllCorrect &=
			TestEqual(TEXT("Compliance trans 3"), Ball.Compliance.Translational_3, Scale * 1.2);
		AllCorrect &= TestEqual(
			TEXT("Spook damping trans 1"), Ball.SpookDamping.Translational_1, Scale * 2.0);
		AllCorrect &= TestEqual(
			TEXT("Spook damping trans 2"), Ball.SpookDamping.Translational_2, Scale * 2.1);
		AllCorrect &= TestEqual(
			TEXT("Spook damping trans 3"), Ball.SpookDamping.Translational_3, Scale * 2.2);
		AllCorrect &= AgxAutomationCommon::TestEqual(
			*this, TEXT("Force range 1"), Ball.ForceRange.Translational_1,
			FAGX_RealInterval(Scale * 3.0, Scale * 4.0));
		AllCorrect &= AgxAutomationCommon::TestEqual(
			*this, TEXT("Force range 2"), Ball.ForceRange.Translational_2,
			FAGX_RealInterval(Scale * 3.1, Scale * 4.1));
		AllCorrect &= AgxAutomationCommon::TestEqual(
			*this, TEXT("Force range 3"), Ball.ForceRange.Translational_3,
			FAGX_RealInterval(Scale * 3.2, Scale * 4.2));
		// Ball Constraint does not have rotational compliance, damping, or force range since all
		// rotational degrees of freedom are free.
		AllCorrect &= TestEqual(TEXT("Compute forces"), Ball.bComputeForces, bEnable);
		AllCorrect &=
			TestEqual(TEXT("Twist range enabled"), Ball.TwistRangeController.bEnable, bEnable);
		AllCorrect &= TestEqual(
			TEXT("Twist range compliance"), Ball.TwistRangeController.Compliance, Scale * 5.0);
		AllCorrect &= TestEqual(
			TEXT("Twist range damping"), Ball.TwistRangeController.SpookDamping, Scale * 6.0);
		AllCorrect &= AgxAutomationCommon::TestEqual(
			*this, TEXT("Twist range force range"), Ball.TwistRangeController.ForceRange,
			FAGX_RealInterval(Scale * 7.0, Scale * 8.0));
		AllCorrect &= AgxAutomationCommon::TestEqual(
			*this, TEXT("Twist range range"), Ball.TwistRangeController.Range,
			FAGX_RealInterval(FromRad(Scale * -0.1), FromRad(Scale * 0.1)));

		return AllCorrect;
	}

	virtual bool PostImport() override
	{
		// Make sure we got the template Components we expect.
		// 1 Default Scene Root, 1 Model Source, 1 Rigid Body, 1 Ball Constraint.
		if (!TestEqual(
				TEXT("Number of imported components before reimport"),
				InitialTemplateComponents.Num(), 4))
		{
			PrintComponentNames(InitialTemplateComponents, TEXT("Initial template components"));
			return false;
		}

		// Check the Blueprint.
		UAGX_BallConstraintComponent* BallTemplate =
			GetTemplateComponentByName<UAGX_BallConstraintComponent>(
				InitialTemplateComponents, TEXT("Ball"));
		if (!TestNotNull(TEXT("Template Ball before reimport"), BallTemplate))
		{
			return false;
		}
		if (!CheckBallConstraint(*BallTemplate, 1.0, EAGX_SolveType::StDirectAndIterative, true))
		{
			return false;
		}

		// Check the Blueprint instance.
		UAGX_BallConstraintComponent* BallInstance =
			FAGX_ObjectUtilities::GetComponentByName<UAGX_BallConstraintComponent>(
				*InitialBlueprintInstance, TEXT("Ball"));
		if (!TestNotNull(TEXT("Ball Constraint instance before reimport"), BallInstance))
		{
			return false;
		}
		if (!CheckBallConstraint(*BallInstance, 1.0, EAGX_SolveType::StDirectAndIterative, true))
		{
			return false;
		}

		return true;
	}

	virtual bool PostReimport() override
	{
		// Make sure we got the template Components we expect.
		// 1 Default Scene Root, 1 Model Source, 1 Rigid Body, 1 Ball Constraint.
		if (!TestEqual(
				TEXT("Number of imported components after reimport"),
				UpdatedTemplateComponents.Num(), 4))
		{
			PrintComponentNames(UpdatedTemplateComponents, TEXT("Updated template components"));
			return false;
		}

		// Check the Blueprint.
		UAGX_BallConstraintComponent* BallTemplate =
			GetTemplateComponentByName<UAGX_BallConstraintComponent>(
				UpdatedTemplateComponents, TEXT("Ball"));
		if (!TestNotNull(TEXT("Template Ball after reimport"), BallTemplate))
		{
			return false;
		}
		if (!CheckBallConstraint(*BallTemplate, 10.0, EAGX_SolveType::StDirect, false))
		{
			return false;
		}

		// Check the Blueprint instance.
		UAGX_BallConstraintComponent* BallInstance =
			FAGX_ObjectUtilities::GetComponentByName<UAGX_BallConstraintComponent>(
				*UpdatedBlueprintInstance, TEXT("Ball"));
		if (!TestNotNull(TEXT("Ball Constraint instance after reimport"), BallInstance))
		{
			return false;
		}
		if (!CheckBallConstraint(*BallInstance, 10.0, EAGX_SolveType::StDirect, false))
		{
			return false;
		}

		return true;
	}
};

namespace
{
	FModifyBallConstraintTest ModifyBallConstraintTest;
}

//
// Ball Constraint removed test starts here.
//

class FRemoveBallConstraintTest final : public FReimportModelTest
{
public:
	FRemoveBallConstraintTest()
		: FReimportModelTest(
			  TEXT("RemoveBallConstraint"),
			  TEXT("AGXUnreal.Editor.AGX_ReimportModelTest.RemoveBallConstraint"),
			  TEXT("ball_constraint__initial2.agx"), TEXT("ball_constraint__removed.agx"))
	{
	}

	virtual bool PostImport() override
	{
		// Make sure we got the template Components we expect.
		// 1 Default Scene Root, 1 Model Source, 1 Rigid Body, 1 Ball Constraint.
		if (!TestEqual(
				TEXT("Number of imported components before reimport"),
				InitialTemplateComponents.Num(), 4))
		{
			PrintComponentNames(InitialTemplateComponents, TEXT("Initial template components"));
			return false;
		}

		// Check the Blueprint.
		UAGX_BallConstraintComponent* BallTemplate =
			GetTemplateComponentByName<UAGX_BallConstraintComponent>(
				InitialTemplateComponents, TEXT("Ball"));
		if (!TestNotNull(TEXT("Template Ball before reimport"), BallTemplate))
		{
			return false;
		}

		// Check the Blueprint instance.
		UAGX_BallConstraintComponent* BallInstance =
			FAGX_ObjectUtilities::GetComponentByName<UAGX_BallConstraintComponent>(
				*InitialBlueprintInstance, TEXT("Ball"));
		if (!TestNotNull(TEXT("Ball Constraint instance before reimport"), BallInstance))
		{
			return false;
		}

		return true;
	}

	virtual bool PostReimport() override
	{
		// Make sure we got the template Components we expect.
		// 1 Default Scene Root, 1 Model Source, 1 Rigid Body.
		if (!TestEqual(
				TEXT("Number of imported components after reimport"),
				UpdatedTemplateComponents.Num(), 3))
		{
			PrintComponentNames(UpdatedTemplateComponents, TEXT("Updated template components"));
			return false;
		}

		// Check the Blueprint.
		UAGX_BallConstraintComponent* BallTemplate =
			GetTemplateComponentByName<UAGX_BallConstraintComponent>(
				UpdatedTemplateComponents, TEXT("Ball"));
		if (!TestNull(TEXT("Template Ball after reimport"), BallTemplate))
		{
			return false;
		}

		// Check the Blueprint instance.
		UAGX_BallConstraintComponent* BallInstance =
			FAGX_ObjectUtilities::GetComponentByName<UAGX_BallConstraintComponent>(
				*UpdatedBlueprintInstance, TEXT("Ball"));
		if (!TestNull(TEXT("Ball Constraint instance after reimport"), BallInstance))
		{
			return false;
		}

		return true;
	}
};

namespace
{
	FRemoveBallConstraintTest RemoveBallConstraintTest;
}

//
// Twist Range Controller-less Ball Constraint test starts here.
//
// Some AGX Dynamics archives, those created before 2.37, have Ball Constraints without a Twist
// Range Controller. This means that we will get an empty Barrier object. It is important
// that we do not try to use such a Barrier during import or model reimport. Once the Ball
// Constraint Component has been created there will always be a Twist Range Controller, it is only
// missing for Barriers wrapping objects in the temporary agxSDK::Simulation that exists during
// import.
//

class FBallConstraintNoTwistRangeTest final : public FReimportModelTest
{
public:
	FBallConstraintNoTwistRangeTest()
		: FReimportModelTest(
			  TEXT("BallConstraintNoTwistRangeTest"),
			  TEXT("AGXUnreal.Editor.AGX_ReimportModelTest.BallConstraintNoTwistRangeTest"),
			  // Same file for both since we want to test the no-twist-controller code path for
			  // both import and reimport. No need for a second archive since there is nothing
			  // to change for the reimport.
			  TEXT("ball_constraint__no_twist_range_agx-2_36_1_5.agx"),
			  TEXT("ball_constraint__no_twist_range_agx-2_36_1_5.agx"))
	{
	}

	bool CheckTwistRangeController(UAGX_BallConstraintComponent& Ball)
	{
		// The Twist Range Controller in the Component should be set to its default configuration
		// when the Barrier doesn't have a native AGX Dynamics object to read from.
		bool AllCorrect = true;
		AllCorrect &=
			TestEqual(TEXT("Twist range enabled"), Ball.TwistRangeController.bEnable, false);
		AllCorrect &= TestEqual(
			TEXT("Twist range compliance"), Ball.TwistRangeController.Compliance,
			ConstraintConstants::DefaultCompliance());
		AllCorrect &= TestEqual(
			TEXT("Twist range damping"), Ball.TwistRangeController.SpookDamping,
			ConstraintConstants::DefaultSpookDamping());
		AllCorrect &= AgxAutomationCommon::TestEqual(
			*this, TEXT("Twist range force range"), Ball.TwistRangeController.ForceRange,
			ConstraintConstants::DefaultForceRange());
		AllCorrect &= AgxAutomationCommon::TestEqual(
			*this, TEXT("Twist range range"), Ball.TwistRangeController.Range,
			FAGX_RealInterval(0.0, 0.0));
		return AllCorrect;
	}

	bool DoChecks(const TCHAR* StageName, TArray<UActorComponent*>& Components)
	{
		FStringFormatNamedArguments FormatArgs;
		FormatArgs.Add(TEXT("StageName"), StageName);

		// Make sure we got the template Components we expect.
		// 1 Default Scene Root, 1 Model Source, 1 Rigid Body, 1 Ball Constraint.
		if (!TestEqual(
				*FString::Format(
					TEXT("Number of imported components at stage {StageName}"), FormatArgs),
				Components.Num(), 4))
		{
			return false;
		}

		// Check the Twist Range Controller.
		UAGX_BallConstraintComponent* Ball =
			GetTemplateComponentByName<UAGX_BallConstraintComponent>(Components, TEXT("Ball"));
		if (!TestNotNull(
				*FString::Format(TEXT("Template Ball at stage {StageName}"), FormatArgs), Ball))
		{
			return false;
		}
		return CheckTwistRangeController(*Ball);
	}

	virtual bool PostImport() override
	{
		// Make sure the AGX Dynamics archive hasn't accidentally been re-generated, possibly with
		// a new AGX Dynamics version that has Twist Range Controller.
		//
		// Would like to do this in the constructor instead, but
		FString ArchiveFilePath = AgxAutomationCommon::GetTestScenePath(
			FPaths::Combine(FString("ReimportModel"), InitialModelFileName));
		AgxAutomationCommon::CheckFileMD5Checksum(
			ArchiveFilePath, TEXT("4d72dcda03b7e204da05a9b479208317"), *this);

		return DoChecks(TEXT("PostImport"), InitialTemplateComponents);
	}

	virtual bool PostReimport() override
	{
		return DoChecks(TEXT("PostReimport"), UpdatedTemplateComponents);
	}
};

namespace
{
	FBallConstraintNoTwistRangeTest BallConstraintNoTwistRangeTest;
}

//
// Surface velocity test starts here.
//

class FSurfaceVelocityTest final : public FReimportModelTest
{
public:
	FVector InitialSurfaceVelocity {AgxAutomationCommon::AgxToUnrealDisplacement(1.0, 2.0, 3.0)};
	FVector UpdatedSurfaceVelocity {AgxAutomationCommon::AgxToUnrealDisplacement(10.0, 20.0, 30.0)};

	FSurfaceVelocityTest()
		: FReimportModelTest(
			  TEXT("SurfaceVelocityTest"),
			  TEXT("AGXUnreal.Editor.AGX_ReimportModelTest.SurfaceVelocity"),
			  TEXT("surface_velocity__initial.agx"), TEXT("surface_velocity__updated.agx"))
	{
	}

	virtual bool PostImport() override
	{
		// Make sure we got the template Components we expect.
		// 1 Default Scene Root, 1 Model Source, 1 Shape.
		if (!TestEqual(
				TEXT("Number of imported components before reimport"),
				InitialTemplateComponents.Num(), 3))
		{
			PrintComponentNames(InitialTemplateComponents, TEXT("Initial template components"));
			return false;
		}

		// Check the Blueprint.
		UAGX_ShapeComponent* ShapeTemplate = GetTemplateComponentByName<UAGX_ShapeComponent>(
			InitialTemplateComponents, TEXT("SurfaceVelocity"));
		if (!TestNotNull(TEXT("Surface velocity geometry template before reimport"), ShapeTemplate))
		{
			return false;
		}
		if (!TestEqual(
				TEXT("Surface Velocity"), ShapeTemplate->SurfaceVelocity, InitialSurfaceVelocity))
		{
			return false;
		}

		// Check the Blueprint instance.
		UAGX_ShapeComponent* ShapeInstance =
			AgxAutomationCommon::GetComponentByName<UAGX_ShapeComponent>(
				*InitialBlueprintInstance, TEXT("SurfaceVelocity"));
		if (!TestNotNull(TEXT("Surface velocity geometry instance before reimport"), ShapeInstance))
		{
			return false;
		}
		if (!TestEqual(
				TEXT("Surface velocity on instance before reimport"),
				ShapeInstance->SurfaceVelocity, InitialSurfaceVelocity))
		{
			return false;
		}

		return true;
	}

	virtual bool PostReimport() override
	{
		// Make sure we got the template Components we expect.
		// 1 Default Scene Root, 1 Model Source, 1 Shape.
		if (!TestEqual(
				TEXT("Number of imported components after reimport"),
				UpdatedTemplateComponents.Num(), 3))
		{
			PrintComponentNames(UpdatedTemplateComponents, TEXT("Updated template components"));
			return false;
		}

		// Check the Blueprint.
		UAGX_ShapeComponent* ShapeTemplate = GetTemplateComponentByName<UAGX_ShapeComponent>(
			UpdatedTemplateComponents, TEXT("SurfaceVelocity"));
		if (!TestNotNull(TEXT("Surface velocity geometry template after reimport"), ShapeTemplate))
		{
			return false;
		}
		if (!TestEqual(
				TEXT("Surface Velocity"), ShapeTemplate->SurfaceVelocity, UpdatedSurfaceVelocity))
		{
			return false;
		}

		// Check the Blueprint instance.
		UAGX_ShapeComponent* ShapeInstance =
			AgxAutomationCommon::GetComponentByName<UAGX_ShapeComponent>(
				*UpdatedBlueprintInstance, TEXT("SurfaceVelocity"));
		if (!TestNotNull(TEXT("Surface velocity geometry instance after reimport"), ShapeInstance))
		{
			return false;
		}
		if (!TestEqual(
				TEXT("Surface velocity on instance after reimport"), ShapeInstance->SurfaceVelocity,
				UpdatedSurfaceVelocity))
		{
			return false;
		}

		return true;
	}
};

namespace
{
	FSurfaceVelocityTest SurfaceVelocityTest;
}

//
// Shared Render Material test starts here.
// This is mostly a sanity check test.
// This reimport used to cause a crash in StaticMesh.cpp saying that the member KnownMesh
// was not the expected one. It is unclear why. A fix for this crash was added in ImporterToEditor,
// calling PostApplyToComponent on the Meshes. This test is mostly to ensure this fix always works
// and we don't crash.
//

class FSharedRenderMaterialTest final : public FReimportModelTest
{
public:
	FSharedRenderMaterialTest()
		: FReimportModelTest(
			  TEXT("SharedRenderMaterial"),
			  TEXT("AGXUnreal.Editor.AGX_ReimportModelTest.SharedRenderMaterial"),
			  TEXT("shared_render_material_initial.agx"), TEXT("shared_render_material_updated.agx"))
	{
	}

	virtual bool PostImport() override
	{
		// Make sure we got the template Components we expect.
		// 1 Default Scene Root, 1 Model Source, 1 Rigid Body, 2 Shapes.
		if (!TestEqual(
				TEXT("Number of imported components before reimport"),
				InitialTemplateComponents.Num(), 5))
		{
			PrintComponentNames(InitialTemplateComponents, TEXT("Initial template components"));
			return false;
		}

		return true;
	}

	virtual bool PostReimport() override
	{
		// Make sure we got the template Components we expect.
		// 1 Default Scene Root, 1 Model Source, 1 Rigid Body, 2 shapes, one render static mesh.
		if (!TestEqual(
				TEXT("Number of imported components after reimport"),
				UpdatedTemplateComponents.Num(), 6))
		{
			PrintComponentNames(UpdatedTemplateComponents, TEXT("Updated template components"));
			return false;
		}

		UAGX_SphereShapeComponent* Sphere1 = GetTemplateComponentByName<UAGX_SphereShapeComponent>(
				UpdatedTemplateComponents, TEXT("SphereGeom1"));
		if (!TestNotNull(TEXT("Template SphereGeom1 after reimport"), Sphere1))
			return false;

		UAGX_SphereShapeComponent* Sphere2 = GetTemplateComponentByName<UAGX_SphereShapeComponent>(
			UpdatedTemplateComponents, TEXT("SphereGeom2"));
		if (!TestNotNull(TEXT("Template SphereGeom2 after reimport"), Sphere2))
			return false;

		UMaterialInterface* Mat1 = Sphere1->GetMaterial(0);
		UMaterialInterface* Mat2 = Sphere2->GetMaterial(0);

		if (!TestNotNull(TEXT("Material1 after reimport"), Mat1))
			return false;

		if (!TestEqual(TEXT("Material1 and Material2 equal"), Mat1, Mat2))
			return false;

		return true;
	}
};

namespace
{
	FSharedRenderMaterialTest SharedRenderMaterialTest;
}
