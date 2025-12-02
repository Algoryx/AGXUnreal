// Copyright 2025, Algoryx Simulation AB.

#include "AgxAutomationCommon.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_Simulation.h"
#include "Shapes/AGX_TrimeshShapeComponent.h"
#include "Utilities/AGX_EditorUtilities.h"
#include "Utilities/AGX_ImportUtilities.h"

// Unreal Engine includes.
#include "Components/StaticMeshComponent.h"
#include "Editor.h"
#include "Engine/Blueprint.h"
#include "Engine/Engine.h"
#include "Engine/EngineTypes.h"
#include "Engine/SCS_Node.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Misc/SecureHash.h"

// Standard library includes.
#include <cmath>

UWorld* AgxAutomationCommon::GetTestWorld()
{
	// Based on GetAnyGameWorld() in AutomationCommon.cpp.
	// That implementation has the following comment:
	//
	// @todo this is a temporary solution. Once we know how to get test's hands on a proper world
	// this function should be redone/removed
	//
	// Keep an eye at the engine implementation and replace this once they provide a better way to
	// get the test world.

	if (GEngine == nullptr)
	{
		UE_LOG(LogAGX, Error, TEXT("Cannot get the test world because GEngine is nullptr."));
		return nullptr;
	}
	const TIndirectArray<FWorldContext>& WorldContexts = GEngine->GetWorldContexts();
	if (WorldContexts.Num() == 0)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Cannot get the test world because GEngine->GetWorldContexts() is empty."));
		return nullptr;
	}
	TArray<UWorld*> Candidates;
	for (const FWorldContext& Context : WorldContexts)
	{
		// It's not clear to me which worlds are OK to use when testing. Here, taken from
		// GetAnyGameWorld in the engine's AutomationCommon.cpp, we allow with PIE and Game.
		// However, in FLoadGameMapCommand, in the same AutomationCommon.cpp, only Game worlds, not
		// PIE, are accepted. So some things are allowed on both PIE and Game worlds, but map
		// loading is only allowed on Game worlds? Is there a way to create a hidden background
		// world used for the test only, detaching the entire test from the state of the rest of the
		// worlds?
		EWorldType::Type Type = Context.WorldType;
		bool bIsPieOrGame = Type == EWorldType::PIE || Type == EWorldType::Game;
		if (bIsPieOrGame && Context.World() != nullptr)
		{
			Candidates.Add(Context.World());
		}
	}
	if (Candidates.Num() == 0)
	{
		UE_LOG(
			LogAGX, Error, TEXT("None of the %d WorldContexts contain a PIE or Game world."),
			WorldContexts.Num());
		return nullptr;
	}
	else if (Candidates.Num() == 1)
	{
		return Candidates[0];
	}
	else if (Candidates.Num() > 1)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Got more than one world that looks like a test world. Don't know which one "
				 "should be used."))
		return nullptr;
	}
	checkNoEntry();
	return nullptr;
}

void AgxAutomationCommon::TestEqual(
	FAutomationTestBase& Test, const TCHAR* What, double Actual, double Expected, double Tolerance)
{
	if (std::isinf(Actual) && std::isinf(Expected) &&
		(std::signbit(Actual) != std::signbit(Expected)))
	{
		Test.AddError(FString::Printf(
			TEXT("Expected '%s' to be %f, but it was %f within tolerance %f."), What, Expected,
			Actual, Tolerance));
	}
}

void AgxAutomationCommon::TestEqual(
	FAutomationTestBase& Test, const TCHAR* What, const FVector4& Actual, const FVector4& Expected,
	float Tolerance)
{
	if (!Expected.Equals(Actual, Tolerance))
	{
		Test.AddError(FString::Printf(
			TEXT("Expected '%s' to be '%s' but it was '%s', with tolerance %f."), What,
			*Expected.ToString(), *Actual.ToString(), Tolerance));
	}
}

void AgxAutomationCommon::TestEqual(
	FAutomationTestBase& Test, const TCHAR* What, const FQuat& Actual, const FQuat& Expected,
	float Tolerance)
{
	if (!Expected.Equals(Actual, Tolerance))
	{
		Test.AddError(FString::Printf(
			TEXT("Expected '%s' to be '%s' but it was '%s', with tolerance %f."), What,
			*Expected.ToString(), *Actual.ToString(), Tolerance));
	}
}

void AgxAutomationCommon::TestEqual(
	FAutomationTestBase& Test, const TCHAR* What, const FRotator& Actual, const FRotator& Expected,
	float Tolerance)
{
	if (!Expected.Equals(Actual, Tolerance))
	{
		Test.AddError(FString::Printf(
			TEXT("Expected '%s' to be '%s' but it was '%s', with tolerance %f."), What,
			*Expected.ToString(), *Actual.ToString(), Tolerance));
	}
}

void AgxAutomationCommon::TestEqual(
	FAutomationTestBase& Test, const TCHAR* What, const FLinearColor& Actual,
	const FLinearColor& Expected, float Tolerance)
{
	if (!Expected.Equals(Actual, Tolerance))
	{
		Test.AddError(FString::Printf(
			TEXT("Expected '%s' to be '%s' but with was '%s', with tolerance %f."), What,
			*Expected.ToString(), *Actual.ToString(), Tolerance));
	}
}

bool AgxAutomationCommon::TestEqual(
	FAutomationTestBase& Test, const TCHAR* What, const FAGX_RealInterval& Actual,
	const FAGX_RealInterval& Expected, double Tolerance)
{
	if (!Expected.Equals(Actual, Tolerance))
	{
		Test.AddError(FString::Printf(
			TEXT("Expected '%s' to be '%s' but it was '%s', with tolerance %f."), What,
			*Expected.ToString(), *Actual.ToString(), Tolerance));
		return false;
	}
	return true;
}

void AgxAutomationCommon::TestLess(
	FAutomationTestBase& Test, const TCHAR* SmallerName, double Smaller, const TCHAR* LargerName,
	double Larger)
{
	if (!(Smaller < Larger))
	{
		Test.AddError(FString::Printf(
			TEXT("Expected '%s' (%f) to be smaller than '%s' (%f), but it was not."), SmallerName,
			Smaller, LargerName, Larger));
	}
}

void AgxAutomationCommon::TestAllLess(
	FAutomationTestBase& Test, const TCHAR* SmallerName, const FVector& Smaller,
	const TCHAR* LargerName, const FVector& Larger)
{
	for (int32 I = 0; I < 3; ++I)
	{
		TestLess(
			Test, *FString::Printf(TEXT("%s.%c"), SmallerName, 'X' + I), Smaller[I],
			*FString::Printf(TEXT("%s.%c"), LargerName, 'X' + I), Larger[I]);
	}
}

FString AgxAutomationCommon::WorldTypeToString(EWorldType::Type Type)
{
	switch (Type)
	{
		case EWorldType::None:
			return TEXT("None");
		case EWorldType::Game:
			return TEXT("Game");
		case EWorldType::Editor:
			return TEXT("Editor");
		case EWorldType::PIE:
			return TEXT("PIE");
		case EWorldType::EditorPreview:
			return TEXT("EditorPreview");
		case EWorldType::GamePreview:
			return TEXT("GamePreview");
		case EWorldType::GameRPC:
			return TEXT("GameRPC");
		case EWorldType::Inactive:
			return TEXT("Inactive");
	}

	return TEXT("Unknown");
}

FString AgxAutomationCommon::GetNoWorldTestsReasonText(NoWorldTestsReason Reason)
{
	switch (Reason)
	{
		case TooManyWorlds:
			return "Cannot run tests that need a world because there are too many world "
				   "contexts.";
		case IllegalWorldType:
			return FString::Printf(
				TEXT("Cannot run tests that need a world because the available world isn't a "
					 "'Game' world, it's a '%s' world."),
				*AgxAutomationCommon::WorldTypeToString(
					GEngine->GetWorldContexts()[0].WorldType.GetValue()));
	}
	return FString();
}

AgxAutomationCommon::NoWorldTestsReason AgxAutomationCommon::CanRunWorldTests()
{
	const TIndirectArray<FWorldContext>& Worlds = GEngine->GetWorldContexts();
	if (Worlds.Num() != 1)
	{
		return NoWorldTestsReason::TooManyWorlds;
	}
	if (GEngine->GetWorldContexts()[0].WorldType != EWorldType::Game)
	{
		return NoWorldTestsReason::IllegalWorldType;
	}
	return NoWorldTestsReason::NoReason;
}

FString AgxAutomationCommon::GetTestScenePath(const TCHAR* SceneName)
{
	FString ProjectDir = FPaths::ProjectDir();
	FPaths::CollapseRelativeDirectories(ProjectDir);
	ProjectDir = FPaths::ConvertRelativePathToFull(ProjectDir);
	const FString SceneDir = ProjectDir.Replace(
		TEXT("/AGXUnrealDev/"), TEXT("/TestScenes/"), ESearchCase::CaseSensitive);
	const FString ScenePath = FPaths::Combine(SceneDir, SceneName);
	if (FPaths::FileExists(ScenePath))
	{
		return ScenePath;
	}
	else
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Did not find full test scene path for test scene '%s'. Searched in '%s'. Does "
				 "not exist: '%s'"),
			SceneName, *SceneDir, *ScenePath);
		return FString();
	}
}

FString AgxAutomationCommon::GetTestScenePath(const FString& SceneName)
{
	return GetTestScenePath(*SceneName);
}

FString AgxAutomationCommon::GetTestSceneDirPath(const FString& SubDir)
{
	FString ProjectDir = FPaths::ProjectDir();
	FPaths::CollapseRelativeDirectories(ProjectDir);
	ProjectDir = FPaths::ConvertRelativePathToFull(ProjectDir);
	const FString SceneDir = ProjectDir.Replace(
		TEXT("/AGXUnrealDev/"), TEXT("/TestScenes/"), ESearchCase::CaseSensitive);
	const FString SceneDirPath = FPaths::Combine(SceneDir, SubDir);

	if (FPaths::DirectoryExists(SceneDirPath))
	{
		return SceneDirPath;
	}
	else
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Did not find full test scene directory path for test scene dir '%s'. Searched in "
				 "'%s'."),
			*SceneDirPath, *SceneDir)
		return FString();
	}
}

bool AgxAutomationCommon::CheckMapMD5Checksum(
	const FString& MapPath, const TCHAR* Expected, FAutomationTestBase& Test)
{
	const FString FilePath = FPaths::ConvertRelativePathToFull(
		FPackageName::LongPackageNameToFilename(MapPath, FPackageName::GetMapPackageExtension()));
	TArray<uint8> PackageBytes;
	FFileHelper::LoadFileToArray(PackageBytes, *FilePath, FILEREAD_None);
	// The documentation (and the code) for FFileHelper::LoadFileToArray says that it adds
	// two bytes of padding to the TArray, but that appears to be a lie. Not doing -2 here
	// and it seems to work. Not sure what's going on here.
	// https://docs.unrealengine.com/en-US/API/Runtime/Core/Misc/FFileHelper/LoadFileToArray/2/
	FString MD5Sum = FMD5::HashBytes(PackageBytes.GetData(), PackageBytes.Num());
	Test.TestEqual(TEXT("Map file MD5 checksum."), MD5Sum, Expected);
	return MD5Sum == Expected;
}

bool AgxAutomationCommon::CheckAssetMD5Checksum(
	const FString& PackagePath, const TCHAR* Expected, FAutomationTestBase& Test)
{
	const FString FilePath =
		FPaths::ConvertRelativePathToFull(FPackageName::LongPackageNameToFilename(
			PackagePath, FPackageName::GetAssetPackageExtension()));
	TArray<uint8> PackageBytes;
	FFileHelper::LoadFileToArray(PackageBytes, *FilePath, FILEREAD_None);
	// The documentation (and the code) for FFileHelper::LoadFileToArray says that it adds
	// two bytes of padding to the TArray, but that appears to be a lie. Not doing -2 here
	// and it seems to work. Not sure what's going on here.
	// https://docs.unrealengine.com/en-US/API/Runtime/Core/Misc/FFileHelper/LoadFileToArray/2/
	FString MD5Sum = FMD5::HashBytes(PackageBytes.GetData(), PackageBytes.Num());
	Test.TestEqual(TEXT("The asset file should have the expected MD5 checksum."), MD5Sum, Expected);
	return MD5Sum == Expected;
}

bool AgxAutomationCommon::CheckFileMD5Checksum(
	const FString& FilePath, const TCHAR* Expected, FAutomationTestBase& Test)
{
	const FString MD5Sum = GetFileMD5Checksum(FilePath);
	Test.TestEqual(TEXT("The asset file should have the expected MD5 checksum."), MD5Sum, Expected);
	return MD5Sum == Expected;
}

FString AgxAutomationCommon::GetFileMD5Checksum(const FString& FilePath)
{
	TArray<uint8> PackageBytes;
	if (!FFileHelper::LoadFileToArray(PackageBytes, *FilePath, FILEREAD_None))
	{
		UE_LOG(
			LogAGX, Warning, TEXT("Could not load file '%s' for MD5 Checksum calculation."),
			*FilePath);
		return FString();
	}
	// The documentation (and the code) for FFileHelper::LoadFileToArray says that it adds
	// two bytes of padding to the TArray, but that appears to be a lie. Not doing -2 here
	// and it seems to work. Not sure what's going on here.
	// https://docs.unrealengine.com/en-US/API/Runtime/Core/Misc/FFileHelper/LoadFileToArray/2/
	FString MD5Sum = FMD5::HashBytes(PackageBytes.GetData(), PackageBytes.Num());
	return MD5Sum;
}

bool AgxAutomationCommon::DeleteImportDirectory(
	const TCHAR* ArchiveName, const TArray<const TCHAR*>& ExpectedFileAndDirectoryNames)
{
	/// @todo There is probably a correct way to delete assets but I haven't been able to get it to
	/// work. See attempts below. For now we just delete the files.

	// The is the sledgehammer appraoch. I don't expect Unreal Engine to like this.
	// I have tried a few variantes (see below) to do this cleanly via the Engine API but I don't
	// know what I'm doing and it always crashes. Doing filesystem delete for now. Nothing is
	// referencing theses assets and nothing ever will again, and the engine will shut down shortly,
	// at least if the tests are being run from the command line.
	//
	// I'm just worried that the wrong directory may be deleted in some circumstances.

	const FString Root = FPaths::ProjectContentDir();
	const FString ImportsLocal =
		FPaths::Combine(FAGX_ImportUtilities::GetImportRootDirectoryName(), ArchiveName);
	const FString ImportsFull = FPaths::Combine(Root, ImportsLocal);
	const FString ImportsAbsolute = FPaths::ConvertRelativePathToFull(ImportsFull);
	if (ImportsFull == Root)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Cannot delete import directory for archive '%s': Directory path is the same as "
				 "the project content directory."),
			ArchiveName)
		return false;
	}
	if (ImportsAbsolute.IsEmpty())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Cannot delete import directory for archive '%s': Directory path is the empty "
				 "string."),
			ArchiveName);
		return false;
	}
	if (!FPaths::DirectoryExists(ImportsAbsolute))
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Cannot delete import directory for archive '%s': Directory '%s' does not exist."),
			ArchiveName, *ImportsAbsolute)
		return false;
	}

	TArray<FString> DirectoryContents;
	IFileManager::Get().FindFilesRecursive(
		DirectoryContents, *ImportsAbsolute, TEXT("*"), true, true);
	if (DirectoryContents.Num() != ExpectedFileAndDirectoryNames.Num())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT(
				"Cannot delete import directory for archive '%s': Directory '%s' contains "
				"unexpected files or directories. Expected %d files and directories but found %d."),
			ArchiveName, *ImportsAbsolute, ExpectedFileAndDirectoryNames.Num(),
			DirectoryContents.Num())
		return false;
	}
	for (const FString& Entry : DirectoryContents)
	{
		const FString Name = FPaths::GetCleanFilename(Entry);
		if (!ExpectedFileAndDirectoryNames.ContainsByPredicate([&Name](const TCHAR* E)
															   { return Name == E; }))
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Cannot delete import directory for archive '%s': Directory '%s' contains "
					 "unexpected files or directories. Found unexpected file or directory '%s'."),
				ArchiveName, *ImportsAbsolute, *Name)
			return false;
		}
	}
	return IFileManager::Get().DeleteDirectory(*ImportsAbsolute, true, true);

#if 0
	// An attempt at deleting the StaticMesh asset.
	// Crashes on GEditor->Something because GEditor is nullptr.
	/// @todo The path for this particular run may be different, may have a _# suffix. How do I find
	/// the path for this particular run?
	const TCHAR* MeshPath = TEXT(
								"StaticMesh'/Game/ImportedAGXModels/simple_trimesh_build/StaticMeshs/"
								"simple_trimesh.simple_trimesh'");
	UObject* Asset = StaticLoadObject(UStaticMesh::StaticClass(), nullptr, MeshPath);
	if (Asset == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Cannot delete imported asset '%s': The asset was not found by StaticLoadObject."),
			MeshPath)
		return true;
	}
	TArray<FAssetData> Assets;
	Assets.Add(FAssetData(Asset));
	ObjectTools::DeleteAssets(Assets, false);
#elif 0
	// An attempt at deleting the StaticMesh asset.
	// Crashes on the first run, and does nothing for subsequent runs.
	/// @todo The path for this particular run may be different, may have a _# suffix. How do I find
	/// the path for this particular run?
	const TCHAR* MeshPath = TEXT(
		"StaticMesh'/Game/ImportedAGXModels/simple_trimesh_build/StaticMeshs/"
		"simple_trimesh.simple_trimesh'");
	UObject* Asset = StaticLoadObject(UStaticMesh::StaticClass(), nullptr, MeshPath);
	if (Asset == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Cannot delete imported asset '%s': The asset was not found by StaticLoadObject."),
			MeshPath)
		return true;
	}
	Asset->MarkPendingKill();
#endif
}

TArray<FString> AgxAutomationCommon::GetReferencedStaticMeshAssets(
	const TArray<UActorComponent*>& Components)
{
	TArray<FString> Assets;

	for (const UActorComponent* Component : Components)
	{
		if (const UAGX_TrimeshShapeComponent* Trimesh = Cast<UAGX_TrimeshShapeComponent>(Component))
		{
			if (Trimesh->MeshSourceAsset != nullptr)
			{
				Assets.AddUnique(Trimesh->MeshSourceAsset->GetName() + ".uasset");
			}
		}
		else if (const UStaticMeshComponent* Mesh = Cast<UStaticMeshComponent>(Component))
		{
			if (Mesh->GetStaticMesh() != nullptr)
			{
				Assets.AddUnique(Mesh->GetStaticMesh()->GetName() + +".uasset");
			}
		}
	}

	return Assets;
}

void AgxAutomationCommon::AddExpectedError(FAutomationTestBase& Test, const FString& Error)
{
	Test.AddExpectedError(Error, EAutomationExpectedErrorFlags::Contains, 0);
	Test.AddError(Error);
}

bool AgxAutomationCommon::FCheckWorldsCommand::Update()
{
	UWorld* TestWorld = AgxAutomationCommon::GetTestWorld();
	UWorld* CurrentWorld = FAGX_EditorUtilities::GetCurrentWorld();
	UE_LOG(LogAGX, Warning, TEXT("TestWorld:    %p"), (void*) TestWorld);
	UE_LOG(LogAGX, Warning, TEXT("CurrentWorld: %p"), (void*) CurrentWorld);
	Test.TestEqual(TEXT("Worlds"), TestWorld, CurrentWorld);
	Test.TestNotNull("TestWorld", TestWorld);
	Test.TestNotNull("CurrentWorld", CurrentWorld);
	return true;
}

bool AgxAutomationCommon::FLogWarningAgxCommand::Update()
{
	UE_LOG(LogAGX, Warning, TEXT("%s"), *Message);
	return true;
}

bool AgxAutomationCommon::FLogErrorAgxCommand::Update()
{
	UE_LOG(LogAGX, Error, TEXT("%s"), *Message);
	return true;
}

bool AgxAutomationCommon::FWaitUntilPIEUpCommand::Update()
{
	UE_LOG(LogAGX, Display, TEXT("Polling for PIE up."));
	return GEditor->IsPlayingSessionInEditor() && GEditor->GetPIEWorldContext() != nullptr &&
		   GEditor->GetPIEWorldContext()->World() != nullptr;
}

bool AgxAutomationCommon::FWaitUntilPIEDownCommand::Update()
{
	UE_LOG(LogAGX, Display, TEXT("Polling for PIE down."));
	return !GEditor->IsPlayingSessionInEditor();
}

bool AgxAutomationCommon::FWaitNTicksCommand::Update()
{
	--NumTicks;
	return NumTicks <= 0;
}

bool AgxAutomationCommon::FWaitUntilTimeCommand::Update()
{
	return World->GetTimeSeconds() >= Time;
}

bool AgxAutomationCommon::FWaitUntilSimTime::Update()
{
	--MaxTicks;
	const UWorld* World = GEditor->GetPIEWorldContext()->World();
	const UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(World);
	const float CurrentTime = static_cast<float>(Simulation->GetTimeStamp());
	const float UnrealTime = World->GetTimeSeconds();
#if 0
	UE_LOG(
		LogAGX, Warning,
		TEXT("Polling for Simulation time: %f/%f. Ticks remaining: %d. Unreal time: %f"),
		CurrentTime, Time, MaxTicks, UnrealTime);
#endif
	if (MaxTicks <= 0)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Failed to step to simulation time %f before reaching max ticks. Current "
				 "simulation time is %f and Unreal time is %f."), Time, CurrentTime, UnrealTime);
	}
	return CurrentTime >= Time || MaxTicks <= 0;
}

AgxAutomationCommon::FWaitWorldDuration::FWaitWorldDuration(UWorld*& InWorld, float InDuration)
	: World(InWorld)
	, Duration(InDuration)
{
}

bool AgxAutomationCommon::FWaitWorldDuration::Update()
{
	if (EndTime < 0.0f)
	{
		// EndTime is only ever negative after construction, so we get here only on the very first
		// call to Update.
		EndTime = World->GetTimeSeconds() + Duration;
	}
	return World->GetTimeSeconds() >= EndTime;
}

AgxAutomationCommon::FCheckWorldsTest::FCheckWorldsTest()
	: FAutomationTestBase(TEXT("FCheckWorldsTest"), false)
{
}

#if UE_VERSION_OLDER_THAN(5, 5, 0)
uint32 AgxAutomationCommon::FCheckWorldsTest::GetTestFlags() const
#else
EAutomationTestFlags AgxAutomationCommon::FCheckWorldsTest::GetTestFlags() const
#endif
{
	return DefaultTestFlags;
}

uint32 AgxAutomationCommon::FCheckWorldsTest::GetRequiredDeviceNum() const
{
	return 1;
}

FString AgxAutomationCommon::FCheckWorldsTest::GetBeautifiedTestName() const
{
	return TEXT("AGXUnreal.CheckWorlds");
}

void AgxAutomationCommon::FCheckWorldsTest::GetTests(
	TArray<FString>& OutBeutifiedNames, TArray<FString>& OutTestCommands) const
{
	OutBeutifiedNames.Add(GetBeautifiedTestName());
	OutTestCommands.Add(FString());
}

bool AgxAutomationCommon::FCheckWorldsTest::RunTest(const FString& InParameter)
{
	UE_LOG(
		LogAGX, Warning, TEXT("Running test '%s' with parameter '%s'."), *GetTestName(),
		*InParameter);

	ADD_LATENT_AUTOMATION_COMMAND(AgxAutomationCommon::FCheckWorldsCommand(*this));
	return true;
}

// We must create an instantiate of the test class for the testing framework to find it.
namespace
{
	AgxAutomationCommon::FCheckWorldsTest CheckWorldsTest;
}

AgxAutomationCommon::FAgxAutomationTest::FAgxAutomationTest(
	const FString& InClassName, const FString& InBeautifiedName)
	: FAutomationTestBase(InClassName, false)
	, BeautifiedTestName(InBeautifiedName)
{
}

#if UE_VERSION_OLDER_THAN(5, 5, 0)
uint32 AgxAutomationCommon::FAgxAutomationTest::GetTestFlags() const
#else
EAutomationTestFlags AgxAutomationCommon::FAgxAutomationTest::GetTestFlags() const
#endif
{
	return DefaultTestFlags;
}

uint32 AgxAutomationCommon::FAgxAutomationTest::GetRequiredDeviceNum() const
{
	return 1;
}

FString AgxAutomationCommon::FAgxAutomationTest::GetBeautifiedTestName() const
{
	return BeautifiedTestName;
}

void AgxAutomationCommon::FAgxAutomationTest::GetTests(
	TArray<FString>& OutBeautifiedNames, TArray<FString>& OutTestCommands) const
{
	OutBeautifiedNames.Add(GetBeautifiedTestName());
	OutTestCommands.Add(FString());
};

USCS_Node* AgxAutomationCommon::GetNodeChecked(const UBlueprint& Blueprint, const FString& Name)
{
	USCS_Node* Node = Blueprint.SimpleConstructionScript->FindSCSNode(FName(Name));
	if (Node == nullptr)
	{
		UE_LOG(LogAGX, Error, TEXT("Did not find SCS Node '%s' in the Blueprint."), *Name);
		return nullptr;
	}

	return Node;
}

USCS_Node* AgxAutomationCommon::GetOnlyAttachChildChecked(USCS_Node* Node)
{
	if (Node == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("GetOnlyAttachChildChecked failed because the passed Node was nullptr."));
		return nullptr;
	}

	const auto& Children = Node->GetChildNodes();
	if (Children.Num() != 1)
	{
		UE_LOG(
			LogAGX, Error, TEXT("Number of children of node '%s' was expected to be 1 but was %d."),
			*Node->GetVariableName().ToString(), Children.Num());
		return nullptr;
	}

	return Children[0];
}
