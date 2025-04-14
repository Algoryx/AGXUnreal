// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "Containers/UnrealString.h"

struct FSimulationObjectCollection;

namespace FAGXSimObjectsReader
{
	/**
	 * Read the AGX Archive file pointed to by 'Filename' and for each
	 * supported object found, collect them into the passed in OutSimObjects.
	 * @param Filename Path to the .agx file to read.
	 * @param OutSimObjects - The object to which all read simulation objects will be collected
	 * into.
	 * @return True if the file was read successfully, false otherwise.
	 */
	AGXUNREALBARRIER_API bool ReadAGXArchive(
		const FString& Filename, FSimulationObjectCollection& OutSimObjects);

	/**
	 * Read the URDF file pointed to by 'Filename' and for each
	 * supported object found, collect them into the passed in OutSimObjects.
	 * @param UrdfFilePath - The path to the URDF file to read.
	 * @param UrdfPackagePath - The path to the package directory. Corresponds to the `package://`
	 * part of any filepath in the .urdf file, typically used for pointing at mesh files. Can be
	 * left empty if the URDF file does not have any file paths in it.
	 * @param OutSimObjects - The object to which all read simulation objects will be collected
	 * into.
	 * @return True if the file was read successfully, false otherwise.
	 */
	AGXUNREALBARRIER_API bool ReadUrdf(
		const FString& UrdfFilePath, const FString& UrdfPackagePath,
		const TArray<double>& InitJoints, FSimulationObjectCollection& OutSimObjects);

	AGXUNREALBARRIER_API bool ReadOpenPLXFile(
		const FString& OpenPLXFilePath, FSimulationObjectCollection& OutSimObjects);
};
