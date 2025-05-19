// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/Class.h"

#include "AGX_SCSNodeCollection.generated.h"

class UBlueprint;
class USCS_Node;

USTRUCT()
struct AGXUNREALEDITOR_API FAGX_SCSNodeCollection
{
	GENERATED_BODY()

	FAGX_SCSNodeCollection() = default;
	explicit FAGX_SCSNodeCollection(const UBlueprint& Bp);

	// The key is the AGX Dynamics object's GUID at the time of the previous import.
	TMap<FGuid, USCS_Node*> RigidBodies;

	// Shapes are all Shapes, including Shapes owned by Rigid Bodies.
	TMap<FGuid, USCS_Node*> Shapes;

	TMap<FGuid, USCS_Node*> Constraints;
	TMap<FGuid, USCS_Node*> TwoBodyTires;
	TMap<FGuid, USCS_Node*> ObserverFrames;
	TMap<FGuid, USCS_Node*> Shovels;
	TMap<FGuid, USCS_Node*> Wires;
	TMap<FGuid, USCS_Node*> Tracks;

	// Guid is the AGX Dynamics shape (Trimesh) guid.
	TMap<FGuid, USCS_Node*> CollisionStaticMeshComponents;

	// The key is the GUID of the Shape Component for which the render data Static Mesh
	// Component has been created.
	TMap<FGuid, USCS_Node*> RenderStaticMeshComponents;

	USCS_Node* CollisionGroupDisablerComponent = nullptr;
	USCS_Node* ContactMaterialRegistrarComponent = nullptr;
	USCS_Node* ModelSourceComponent = nullptr;
	USCS_Node* RootComponent = nullptr;
};
