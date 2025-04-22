// Copyright 2024, Algoryx Simulation AB.

#pragma once

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

	/**
	 * Copies the Components and their properties from the Template Actor to the OutActor.
	 * Components in the OutActor that is not found in the Template Actor are removed.
	 * Component lookup is completely based on Components names, so renamed objects may be handled
	 * as new objects.
	 *
	 * 
	 * It is not recommended to store references to Components in the Actor that is reimported since
	 * Components may be removed during the Reimport process.
	 */
	static bool Reimport(const AActor& Template, AActor& OutActor);

	static FString GetUnsetUniqueImportName();
};
