// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Import/AGX_ImportEnums.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

class AActor;
class FShapeMaterialBarrier;
class UAGX_ShapeMaterial;
class UActorComponent;
struct FAGX_ImportContext;

class AGXUNREAL_API FAGX_ImportRuntimeUtilities
{
public:
	/**
	 * Write an import session Guid to the Component as a tag.
	 * The session Guid should be generated once per import or reimport, for example by the importer
	 * classes.
	 * This session guid can be used to identify which Components are part of a specific
	 * import/reimport session.
	 */
	static void WriteSessionGuid(UActorComponent& Component, const FGuid& SessionGuid);

	static void WriteSessionGuidToAssetType(UObject& Object, const FGuid& SessionGuid);

	static void OnComponentCreated(
		UActorComponent& OutComponent, AActor& Owner, const FGuid& SessionGuid);

	static void OnAssetTypeCreated(UObject& OutObject, const FGuid& SessionGuid);

	static UAGX_ShapeMaterial* GetOrCreateShapeMaterial(
		const FShapeMaterialBarrier& Barrier, FAGX_ImportContext* Context);

	static EAGX_ImportType GetImportTypeFrom(const FString& FilePath);

	/**
	 * Given an absolute path to an OpenPLX file in
	 * <project>/OpenPLXModels/<mymodel>/.../model.openplx, removes the <mymodel> directory and
	 * everything inside it. Returns the deleted directory if any.
	 */
	static FString RemoveImportedOpenPLXFiles(const FString& FilePath);
};
