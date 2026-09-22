// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "OpenPLX/OpenPLXMaterialBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_ImportContext.generated.h"

struct FAGX_ImportSettings;
struct FOpenPLXMaterialBarrier;

class UAGX_CableComponent;
class UAGX_CableProperties;
class UAGX_CollisionGroupDisablerComponent;
class UAGX_ConstraintComponent;
class UAGX_ContactMaterial;
class UAGX_ContactMaterialRegistrarComponent;
class UAGX_MergeSplitThresholdsBase;
class UAGX_ModelSourceComponent;
class UAGX_ObserverFrameComponent;
class UAGX_RigidBodyComponent;
class UAGX_ShapeComponent;
class UAGX_ShapeMaterial;
class UAGX_ShovelComponent;
class UAGX_ShovelProperties;
class UAGX_SteeringComponent;
class UAGX_SteeringParameters;
class UAGX_TerrainWheelComponent;
class UAGX_TerrainWheelDeformationProperties;
class UAGX_TerrainWheelSettings;
class UAGX_TrackComponent;
class UAGX_TrackInternalMergeProperties;
class UAGX_TrackProperties;
class UAGX_TwoBodyTireComponent;
class UAGX_WireComponent;
class UMaterialInstanceConstant;
class UMaterialInterface;
class UOpenPLX_SignalHandlerComponent;
class UStaticMesh;
class UStaticMeshComponent;
class UTexture2D;
class UWorld;

/**
 * This struct holds references to objects created during an Import process.
 */
USTRUCT()
struct AGXUNREAL_API FAGX_ImportContext
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	TMap<FGuid, TObjectPtr<UAGX_RigidBodyComponent>> RigidBodies;

	UPROPERTY(Transient)
	TMap<FGuid, TObjectPtr<UAGX_ShapeComponent>> Shapes;

	UPROPERTY(Transient)
	TMap<FGuid, TObjectPtr<UAGX_ConstraintComponent>> Constraints;

	UPROPERTY(Transient)
	TMap<FGuid, TObjectPtr<UAGX_TerrainWheelComponent>> TerrainWheels;

	UPROPERTY(Transient)
	TMap<FGuid, TObjectPtr<UAGX_TwoBodyTireComponent>> Tires;

	UPROPERTY(Transient)
	TMap<FGuid, TObjectPtr<UAGX_ShovelComponent>> Shovels;

	UPROPERTY(Transient)
	TMap<FGuid, TObjectPtr<UAGX_SteeringComponent>> Steerings;

	UPROPERTY(Transient)
	TMap<FGuid, TObjectPtr<UAGX_CableComponent>> Cables;

	UPROPERTY(Transient)
	TMap<FGuid, TObjectPtr<UAGX_CableProperties>> CableProperties;

	UPROPERTY(Transient)
	TMap<FGuid, TObjectPtr<UAGX_WireComponent>> Wires;

	UPROPERTY(Transient)
	TMap<FGuid, TObjectPtr<UAGX_TrackComponent>> Tracks;

	UPROPERTY(Transient)
	TMap<FGuid, TObjectPtr<UAGX_ObserverFrameComponent>> ObserverFrames;

	// The key is the GUID of the Shape for which the render data Static Mesh
	// Component has been created.
	UPROPERTY(Transient)
	TMap<FGuid, TObjectPtr<UStaticMeshComponent>> RenderStaticMeshCom;

	// The key is the GUID of the Trimesh Shape.
	UPROPERTY(Transient)
	TMap<FGuid, TObjectPtr<UStaticMeshComponent>> CollisionStaticMeshCom;

	// The key is the GUID of the RenderMaterial.
	UPROPERTY(Transient)
	TMap<FGuid, TObjectPtr<UMaterialInterface>> RenderMaterials;

	// The key is the GUID of the source texture.
	UPROPERTY(Transient)
	TMap<FGuid, TObjectPtr<UTexture2D>> Textures;

	// This is a holder of material overrides coming from OpenPLX.
	// Key is the GUID of a RenderMaterial in AGX, and the value is the
	// corresponing OpenPLX Visual Material that will replace the AGX RenderMaterial.
	UPROPERTY(Transient)
	TMap<FGuid, FOpenPLXMaterialBarrier> PLXMaterialOverrides;

	// For render meshes, the GUID is taken from the RenderData.
	UPROPERTY(Transient)
	TMap<FGuid, TObjectPtr<UStaticMesh>> RenderStaticMeshes;

	// The key is the GUID of the Trimesh Shape.
	UPROPERTY(Transient)
	TMap<FGuid, TObjectPtr<UStaticMesh>> CollisionStaticMeshes;

	UPROPERTY(Transient)
	TMap<FGuid, TObjectPtr<UAGX_MergeSplitThresholdsBase>> MSThresholds;

	UPROPERTY(Transient)
	TMap<FGuid, TObjectPtr<UAGX_ShapeMaterial>> ShapeMaterials;

	UPROPERTY(Transient)
	TMap<FGuid, TObjectPtr<UAGX_ContactMaterial>> ContactMaterials;

	UPROPERTY(Transient)
	TMap<FGuid, TObjectPtr<UAGX_ShovelProperties>> ShovelProperties;

	UPROPERTY(Transient)
	TMap<FGuid, TObjectPtr<UAGX_SteeringParameters>> SteeringParameters;

	UPROPERTY(Transient)
	TMap<FGuid, TObjectPtr<UAGX_TerrainWheelDeformationProperties>> TerrainWheelDeformationProperties;

	UPROPERTY(Transient)
	TMap<FGuid, TObjectPtr<UAGX_TerrainWheelSettings>> TerrainWheelSettings;

	UPROPERTY(Transient)
	TMap<FGuid, TObjectPtr<UAGX_TrackProperties>> TrackProperties;

	UPROPERTY(Transient)
	TMap<FGuid, TObjectPtr<UAGX_TrackInternalMergeProperties>> TrackMergeProperties;

	UPROPERTY(Transient)
	TObjectPtr<UAGX_ModelSourceComponent> ModelSourceComponent;

	UPROPERTY(Transient)
	TObjectPtr<UAGX_ContactMaterialRegistrarComponent> ContactMaterialRegistrar;

	UPROPERTY(Transient)
	TObjectPtr<UAGX_CollisionGroupDisablerComponent> CollisionGroupDisabler;

	UPROPERTY(Transient)
	TObjectPtr<UOpenPLX_SignalHandlerComponent> SignalHandler;

	UPROPERTY(Transient)
	FGuid SessionGuid;

	const FAGX_ImportSettings* Settings {nullptr};

	// TransientPackage for editor imports and UWorld for runtime imports.
	UPROPERTY(Transient)
	TObjectPtr<UObject> Outer;

	/**
	 * The root model name, not always set.
	 * For OpenPLX imports, this is set to the root System name.
	 */
	UPROPERTY(Transient)
	FString RootModelName = "";

	/**
	* Whether or not the TMap members of this struct is used during import / reimport.
	*/
	UPROPERTY(Transient)
	bool bStoreObjects {true};
};
