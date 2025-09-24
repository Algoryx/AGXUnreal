// Copyright 2025, Algoryx Simulation AB.

#include "Utilities/OpenPLXUtilities.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Environment.h"
#include "Utilities/PLXUtilitiesInternal.h"

// Unreal Engine includes.
#include "HAL/FileManager.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/Paths.h"

TArray<FString> FOpenPLXUtilities::GetBundlePaths()
{
	TArray<FString> Paths;

	Paths.Add(FPaths::Combine(
		FAGX_Environment::GetPluginSourcePath(), "ThirdParty", "agx", "openplxbundles"));
	Paths.Add(FPaths::Combine(
			FAGX_Environment::GetPluginSourcePath(), "ThirdParty", "agx", "data", "openplx", "agxBundle"));

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
		LogTemp, Error, TEXT("CreateUniqueModelDirectory: Failed to create directory: %s"),
		*UniqueModelDir);
	return "";
}

FString FOpenPLXUtilities::CopyAllDependenciesToProject(FString Filepath, const FString& Destination)
{
	const TArray<FString> Dependencies = FPLXUtilitiesInternal::GetFileDependencies(Filepath);
	if (Dependencies.Num() == 0)
		return ""; // Logging done in GetFileDependencies.

	// Ensure consistent formatting of / and \\ in the path.
	Filepath = FPaths::ConvertRelativePathToFull(Filepath);
	const FString SourceRoot = FPaths::GetPath(Filepath);
	FString CopiedMainFilePath;

	for (const FString& Dep : Dependencies)
	{
		if (!Dep.StartsWith(SourceRoot))
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("OpenPLX dependency '%s' is not located in the same directory, or a "
					 "subdirectory to the OpenPLX file '%s'. It cannot be copied."),
				*Dep, *Filepath);
		}

		const FString RelativePath = Dep.RightChop(SourceRoot.Len());
		const FString TargetPath = FPaths::Combine(Destination, RelativePath);

		const FString TargetDir = FPaths::GetPath(TargetPath);
		if (!FPaths::DirectoryExists(TargetDir))
			IFileManager::Get().MakeDirectory(*TargetDir, true);

		if (!FPlatformFileManager::Get().GetPlatformFile().CopyFile(*TargetPath, *Dep))
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to copy OpenPLX dependency: %s"), *Dep);
		}

		if (Dep == Filepath)
		{
			CopiedMainFilePath = TargetPath;
		}
	}

	return CopiedMainFilePath;
}
