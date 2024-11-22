// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "AGX_ModelSourceComponent.generated.h"

/*
 * Component holding information about an imported archive.
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", Meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_ModelSourceComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "AGX Synchronize Model Info")
	FString FilePath;

	UPROPERTY(EditAnywhere, Category = "AGX Synchronize Model Info")
	bool bIgnoreDisabledTrimeshes = false;

	// The reason why these Guid maps are stored in this Component is
	// that we cannot store an ImportGuid into Static Mesh Components as we do for any imported
	// AGXUnreal Components.

	// Key is the name of the imported Static Mesh Component's SCS Node and the value is the guid
	// of the owning Trimesh.
	UPROPERTY(EditAnywhere, Category = "AGX Synchronize Model Info")
	TMap<FString, FGuid> StaticMeshComponentToOwningTrimesh;

	// Key is the name of the imported Static Mesh Component's SCS Node and the value is the GUID
	// of the owning Shape.
	UPROPERTY(EditAnywhere, Category = "AGX Synchronize Model Info")
	TMap<FString, FGuid> StaticMeshComponentToOwningShape;

	// Emulate an ImportGuid on Unreal's Materials.
	//
	// They key is the asset path relative to the root model directory, such as
	// 'my_model/RenderMaterial/Steel.Steel', to a Material Instance created
	// by the import pipeline. The value is the Guid, originally Uuid, of the source AGX Dynamics
	// Render Material.
	UPROPERTY(EditAnywhere, Category = "AGX Synchronize Model Info")
	TMap<FString, FGuid> UnrealMaterialToImportGuid;
};
