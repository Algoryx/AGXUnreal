// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Import/AGX_ImportEnums.h"

// Unreal Engine includes.
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include <type_traits>

class AActor;
class FShapeMaterialBarrier;
class UAGX_ShapeMaterial;
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

	/**
	 * In some cases, for example in the case of OpenPLX imports, many Barrier names will start with
	 * the root model name (the System name). This function returns the Barrier name with the root
	 * model name removed from the beginning (if applicable).
	 */
	template <typename ObjectType>
	static FString RemoveModelNameFromBarrierName(
		ObjectType& Object, const FString& BarrierName, FAGX_ImportContext* Context)
	{
		using ObjectT = std::remove_cv_t<std::remove_reference_t<ObjectType>>;
		static_assert(
			std::is_base_of_v<UActorComponent, ObjectT>,
			"RemoveModelNameFromBarrierName can only be called with UActorComponent-derived types.");
		(void) Object; // Unused warning fix.
		return RemoveModelNameFromBarrierNameImpl(BarrierName, Context);
	}

private:
	static FString RemoveModelNameFromBarrierNameImpl(
		const FString& BarrierName, FAGX_ImportContext* Context);
};
