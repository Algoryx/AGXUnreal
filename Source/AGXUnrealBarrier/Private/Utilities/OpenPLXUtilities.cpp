// Copyright 2025, Algoryx Simulation AB.

#include "Utilities/OpenPLXUtilities.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Environment.h"
#include "AGX_LogCategory.h"
#include "BarrierOnly/AGXTypeConversions.h"
#include "Utilities/PLXUtilitiesInternal.h"

// Unreal Engine includes.
#include "HAL/FileManager.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/Paths.h"

// OpenPLX includes.
#include "BeginAGXIncludes.h"
#include "agxOpenPLX/AgxOpenPlxApi.h"
#include "EndAGXIncludes.h"

TArray<FString> FOpenPLXUtilities::GetBundlePaths()
{
	TArray<FString> Paths;

	Paths.Add(FPaths::Combine(
		FAGX_Environment::GetPluginSourcePath(), "ThirdParty", "agx", "openplxbundles"));
	Paths.Add(FPaths::Combine(
		FAGX_Environment::GetPluginSourcePath(), "ThirdParty", "agx", "data", "openplx",
		"agxBundle"));

	const FString UserBundlePath = GetUserBundlePath();
	if (FPaths::DirectoryExists(UserBundlePath))
		Paths.Add(UserBundlePath);

	return Paths;
}

FString FOpenPLXUtilities::GetUserBundlePath()
{
	const FString ProjectPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
	return FPaths::Combine(ProjectPath, TEXT("OpenPLXUserBundles"));
}

FString FOpenPLXUtilities::GetModelsDirectory()
{
	const FString ProjectPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
	return FPaths::Combine(ProjectPath, TEXT("OpenPLXModels"));
}

FString FOpenPLXUtilities::CreateUniqueModelDirectory(const FString& Filepath)
{
	const FString ModelName = FPaths::GetBaseFilename(Filepath);
	const FString BaseModelDir = FPaths::Combine(GetModelsDirectory(), ModelName);

	FString UniqueModelDir = BaseModelDir;
	int32 Suffix = 1;
	while (FPaths::DirectoryExists(UniqueModelDir))
	{
		UniqueModelDir = FString::Printf(TEXT("%s_%d"), *BaseModelDir, Suffix++);
	}

	if (IFileManager::Get().MakeDirectory(*UniqueModelDir, true))
	{
		return UniqueModelDir;
	}

	UE_LOG(
		LogAGX, Error, TEXT("CreateUniqueModelDirectory: Failed to create directory: %s"),
		*UniqueModelDir);
	return "";
}

FString FOpenPLXUtilities::RebuildOpenPLXFilePath(FString Path)
{
	// Ensure consistent formatting of / and \\ in the path.
	const FString AbsolutePath = FPaths::ConvertRelativePathToFull(Path);

	const FString Marker = TEXT("OpenPLXModels/");
	const int32 Index = AbsolutePath.Find(Marker, ESearchCase::IgnoreCase, ESearchDir::FromEnd);

	if (Index == INDEX_NONE)
		return AbsolutePath;

	// Slice out everything after "OpenPLXModels/"
	const int32 Start = Index + Marker.Len();
	const FString RelativeSubPath = AbsolutePath.Mid(Start);

	// Combine with GetModelsDirectory()
	const FString ModelsDir = FOpenPLXUtilities::GetModelsDirectory();
	return FPaths::Combine(ModelsDir, RelativeSubPath);
}

FString FOpenPLXUtilities::CopyAllDependenciesToProject(
	FString Filepath, const FString& Destination)
{
	// Ensure consistent formatting of / and \\ in the path.
	Filepath = FPaths::ConvertRelativePathToFull(Filepath);
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	// First, we backup any existing file so that we can restore it if this operation fails.
	FString BackupDir("");
	if (FPaths::DirectoryExists(Destination))
	{
		const FString DestParentDir = FPaths::GetPath(Destination);
		BackupDir =
			FPaths::Combine(DestParentDir, FPaths::GetBaseFilename(Filepath) + FString("_backup_"));
		if (FPaths::DirectoryExists(BackupDir))
			PlatformFile.DeleteDirectoryRecursively(*BackupDir);

		PlatformFile.CopyDirectoryTree(*BackupDir, *Destination, true);

		// Delete any old files.
		PlatformFile.DeleteDirectoryRecursively(*Destination);
	}

	bool Result = false;
	try
	{
		const TArray<FString> BundlePaths = FOpenPLXUtilities::GetBundlePaths();
		Result = agxopenplx::bake_file(
			Convert(Filepath), Convert(Destination), /*bake_imports*/ false,
			ToStdStringVector(BundlePaths));
	}
	catch (const std::exception& Ex)
	{
		UE_LOG(LogAGX, Error, TEXT("agxopenplx::bake_file threw std::exception: %hs"), Ex.what());
	}
	catch (...)
	{
		UE_LOG(LogAGX, Error, TEXT("agxopenplx::bake_file threw an unknown exception"));
	}

	const FString OutputFile = FPaths::Combine(Destination, FPaths::GetCleanFilename(Filepath));
	if (!Result || !FPaths::FileExists(OutputFile))
	{
		// Clean up any partial result from bake_file before returning.
		if (PlatformFile.DirectoryExists(*Destination))
			PlatformFile.DeleteDirectoryRecursively(*Destination);

		// Restore (and then remove) backup if it exists.
		if (FPaths::DirectoryExists(BackupDir))
		{
			PlatformFile.CopyDirectoryTree(*Destination, *BackupDir, true);
			PlatformFile.DeleteDirectoryRecursively(*BackupDir);
		}

		return "";
	}

	if (FPaths::DirectoryExists(BackupDir))
		PlatformFile.DeleteDirectoryRecursively(*BackupDir);

	return OutputFile;
}
