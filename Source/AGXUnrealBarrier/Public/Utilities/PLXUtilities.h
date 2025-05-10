// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

class AGXUNREALBARRIER_API FPLXUtilities
{
public:
	static FString GetBundlePath();

	/**
	 * Returns the directory in which all OpenPLX files must reside in for the to be importable.
	 */
	static FString GetModelsDirectory();

	/**
	 * Given a OpenPLX file anywhere on the system, this function creates a directory with the same
	 * name as the OpenPLX model inside the Project/OpenPLXModels/ directory. If there already
	 * exists a directory with the same name, a _n suffix is added at the end where n goes from
	 * 1...inf such that it becomes unique.
	 * Returns the absolute path to the created directory.
	 */
	static FString CreateUniqueModelDirectory(const FString& Filepath);

	/**
	 * Given an OpenPLX file anywhere on the system, it, and all of it's dependencies are copied to
	 * the given destination path, with preserved relative paths from the OpenPLX file.
	 * All dependencies must be in the same directory or a subdirectory to the OpenPLX file.
	 * If there exists files with the same name in the directory, the files are overwritten.
	 * Returns the path to the copy of the main OpenPLX file.
	 */
	static FString CopyAllDependenciesToProject(
		FString Filepath, const FString& Destination);
};
