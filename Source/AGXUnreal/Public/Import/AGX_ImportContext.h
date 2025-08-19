// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

struct FAGX_ImportSettings;

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
class UWorld;

/**
 * This struct holds references to objects created during an Import process.
 */
struct AGXUNREAL_API FAGX_ImportContext
{
	TUniquePtr<TMap<FGuid, UAGX_RigidBodyComponent*>> RigidBodies;
	TUniquePtr<TMap<FGuid, UAGX_ShapeComponent*>> Shapes;
	TUniquePtr<TMap<FGuid, UAGX_ConstraintComponent*>> Constraints;
	TUniquePtr<TMap<FGuid, UAGX_TwoBodyTireComponent*>> Tires;
	TUniquePtr<TMap<FGuid, UAGX_ShovelComponent*>> Shovels;
	TUniquePtr<TMap<FGuid, UAGX_WireComponent*>> Wires;
	TUniquePtr<TMap<FGuid, UAGX_TrackComponent*>> Tracks;
	TUniquePtr<TMap<FGuid, UAGX_ObserverFrameComponent*>> ObserverFrames;

	// The key is the GUID of the Shape for which the render data Static Mesh
	// Component has been created.
	TUniquePtr<TMap<FGuid, UStaticMeshComponent*>> RenderStaticMeshCom;

	// The key is the GUID of the Trimesh Shape.
	TUniquePtr<TMap<FGuid, UStaticMeshComponent*>> CollisionStaticMeshCom;

	// The key is the GUID of the RenderMaterial.
	TUniquePtr<TMap<FGuid, UMaterialInterface*>> RenderMaterials;

	// For render meshes, the GUID is taken from the RenderData.
	TUniquePtr<TMap<FGuid, UStaticMesh*>> RenderStaticMeshes;

	// The key is the GUID of the Trimesh Shape.
	TUniquePtr<TMap<FGuid, UStaticMesh*>> CollisionStaticMeshes;

	TUniquePtr<TMap<FGuid, UAGX_MergeSplitThresholdsBase*>> MSThresholds;
	TUniquePtr<TMap<FGuid, UAGX_ShapeMaterial*>> ShapeMaterials;
	TUniquePtr<TMap<FGuid, UAGX_ContactMaterial*>> ContactMaterials;
	TUniquePtr<TMap<FGuid, UAGX_ShovelProperties*>> ShovelProperties;
	TUniquePtr<TMap<FGuid, UAGX_TrackProperties*>> TrackProperties;
	TUniquePtr<TMap<FGuid, UAGX_TrackInternalMergeProperties*>> TrackMergeProperties;

	UAGX_ModelSourceComponent* ModelSourceComponent {nullptr};
	UAGX_ContactMaterialRegistrarComponent* ContactMaterialRegistrar {nullptr};
	UAGX_CollisionGroupDisablerComponent* CollisionGroupDisabler {nullptr};
	UOpenPLX_SignalHandlerComponent* SignalHandler {nullptr};

	FGuid SessionGuid;

	const FAGX_ImportSettings* Settings {nullptr};

	// TransientPackage for editor imports and UWorld for runtime imports.
	UObject* Outer {nullptr};

	/**
	 *The root model name, not always set.
	 * For OpenPLX imports, this is set to the root System name.
	 */
	FString RootModelName = "";
};
