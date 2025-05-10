// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "Import/AGX_ImportEnums.h"

#include "AGX_ModelSourceComponent.generated.h"

/*
 * Component holding information about an imported archive.
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", Meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_ModelSourceComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "AGX Reimport Model Info")
	FString FilePath;

	UPROPERTY(EditAnywhere, Category = "AGX Reimport Model Info")
	bool bIgnoreDisabledTrimeshes = false;

	// The reason why these Guid maps are stored in this Component is
	// that we cannot store an ImportGuid into Static Mesh Components as we do for any imported
	// AGXUnreal Components.

	// Key is the name of the imported Static Mesh Component's SCS Node and the value is the guid
	// of the owning Trimesh.
	UPROPERTY(EditAnywhere, Category = "AGX Reimport Model Info")
	TMap<FString, FGuid> StaticMeshComponentToOwningTrimesh;

	// Key is the name of the imported Static Mesh Component's SCS Node and the value is the GUID
	// of the owning Shape.
	UPROPERTY(EditAnywhere, Category = "AGX Reimport Model Info")
	TMap<FString, FGuid> StaticMeshComponentToOwningShape;

	// Emulate an ImportGuid on Unreal's Materials.
	//
	// They key is the asset path relative to the root model directory, such as
	// 'my_model/RenderMaterial/Steel.Steel', to a Material Instance created
	// by the import pipeline. The value is the Guid, originally Uuid, of the source AGX Dynamics
	// Render Material.
	UPROPERTY(EditAnywhere, Category = "AGX Reimport Model Info")
	TMap<FString, FGuid> UnrealMaterialToImportGuid;

	/**
	 * Absolute file path to the original .agx archive, OpenPLX or Urdf file that was selected for
	 * Import or Reimport. In most cases this is the same as the FilePath property, but may differ
	 * in some cases, for example for OpenPLX models where the files are copied to the project dir
	 * (which is what FilePath points to in that case). In that case, this points to the original
	 * source file that was copied.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Reimport Model Info")
	FString SourceFilePath;

	UPROPERTY(EditAnywhere, Category = "AGX Reimport Model Info")
	bool bRuntimeImport {false};

	virtual void Serialize(FArchive& Archive) override;

	const TMap<FString, FGuid>& GetDeprecatedRenderDataTable() const;

	//~ Begin UActorComponent Interface
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	//~ End UActorComponent Interface

private:
#if WITH_EDITOR
	/**
	 * Upgrade entries in the deprecated Render Data table and insert the result into the Shape
	 * UUID based Render Data Table. This is required after loading a Blueprint saved prior to the
	 * introduction of support for Render Data being shared by many Shape Components, which was
	 * added in AGX Dynamics for Unreal 1.14.0.
	 *
	 * This is done automatically by the Serialize member function, but on some Unreal Engine
	 * versions, at least 5.4 and 5.5, this doesn't work because of limitations of Unreal Engine's
	 * Blueprint loading implementation.
	 */
	void UpgradeRenderDataTableFromRenderDataUuidToShapeUuid();

	void OnBlueprintLoaded(UObject* LoadedObject);
#endif


private:
	// Key is the name of the imported Static Mesh Component's SCS Node and the value is the guid
	// of the owning RenderData.
	UPROPERTY()
	TMap<FString, FGuid> StaticMeshComponentToOwningRenderData_DEPRECATED;
};
