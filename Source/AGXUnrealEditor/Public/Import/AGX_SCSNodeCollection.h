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
	TMap<FGuid, USCS_Node*> SphereShapes;
	TMap<FGuid, USCS_Node*> BoxShapes;
	TMap<FGuid, USCS_Node*> CylinderShapes;
	TMap<FGuid, USCS_Node*> CapsuleShapes;
	TMap<FGuid, USCS_Node*> TrimeshShapes;

	TMap<FGuid, USCS_Node*> HingeConstraints;
	TMap<FGuid, USCS_Node*> PrismaticConstraints;
	TMap<FGuid, USCS_Node*> BallConstraints;
	TMap<FGuid, USCS_Node*> CylindricalConstraints;
	TMap<FGuid, USCS_Node*> DistanceConstraints;
	TMap<FGuid, USCS_Node*> LockConstraints;
	TMap<FGuid, USCS_Node*> TwoBodyTires;
	TMap<FGuid, USCS_Node*> ObserverFrames;
	TMap<FGuid, USCS_Node*> Shovels;

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
