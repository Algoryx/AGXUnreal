#pragma once

// AGX Dynamics for Unreal includes.
#include "AGXSimObjectsReader.h"
#include "AGX_ImportEnums.h"

// Unreal Engine includes.
#include "Containers/Map.h"
#include "Misc/Guid.h"

// AGXUnreal classes.
class FConstraintBarrier;
class UAGX_RigidBodyComponent;
class UAGX_SphereShapeComponent;
class UAGX_BoxShapeComponent;
class UAGX_CylinderShapeComponent;
class UAGX_CapsuleShapeComponent;
class UAGX_TrimeshShapeComponent;
class UAGX_ShapeMaterialAsset;
class UAGX_ContactMaterialAsset;
class UAGX_HingeConstraintComponent;
class UAGX_PrismaticConstraintComponent;
class UAGX_BallConstraintComponent;
class UAGX_CylindricalConstraintComponent;
class UAGX_DistanceConstraintComponent;
class UAGX_LockConstraintComponent;
class FTwoBodyTireBarrier;
class UAGX_TwoBodyTireComponent;
class UAGX_CollisionGroupDisablerComponent;
class UAGX_ContactMaterialRegistrarComponent;
class UAGX_WireComponent;

// Unreal Engine classes.
class AActor;
class UMaterialInstanceConstant;
class UStaticMesh;

/**
 * An Unreal Engine side helper that creates `[UA]AGX_.*` objects from Barrier objects read from an
 * AGX Dynamics archive or URDF file.
 */
struct FAGX_SimObjectsImporterHelper
{
public:
	/** Create a new UAGX_RigidBodyComponent in the given actor. */
	UAGX_RigidBodyComponent* InstantiateBody(const FRigidBodyBarrier& Barrier, AActor& Owner);

	UAGX_SphereShapeComponent* InstantiateSphere(
		const FSphereShapeBarrier& Sphere, AActor& Owner, UAGX_RigidBodyComponent* Body = nullptr);

	UAGX_BoxShapeComponent* InstantiateBox(
		const FBoxShapeBarrier& Barrier, AActor& Owner, UAGX_RigidBodyComponent* Body = nullptr);

	UAGX_CylinderShapeComponent* InstantiateCylinder(
		const FCylinderShapeBarrier& Barrier, AActor& Owner,
		UAGX_RigidBodyComponent* Body = nullptr);

	UAGX_CapsuleShapeComponent* InstantiateCapsule(
		const FCapsuleShapeBarrier& Barrier, AActor& Owner,
		UAGX_RigidBodyComponent* Body = nullptr);

	UAGX_TrimeshShapeComponent* InstantiateTrimesh(
		const FTrimeshShapeBarrier& Barrier, AActor& Owner,
		UAGX_RigidBodyComponent* Body = nullptr);

	UAGX_ShapeMaterialAsset* InstantiateShapeMaterial(const FShapeMaterialBarrier& Barrier);

	UAGX_ContactMaterialAsset* InstantiateContactMaterial(
		const FContactMaterialBarrier& Barrier, AActor& Owner);

	UAGX_HingeConstraintComponent* InstantiateHinge(const FHingeBarrier& Barrier, AActor& Owner);

	UAGX_PrismaticConstraintComponent* InstantiatePrismatic(
		const FPrismaticBarrier& Barrier, AActor& Owner);

	UAGX_BallConstraintComponent* InstantiateBallJoint(
		const FBallJointBarrier& Barrier, AActor& Owner);

	UAGX_CylindricalConstraintComponent* InstantiateCylindricalJoint(
		const FCylindricalJointBarrier& Barrier, AActor& Owner);

	UAGX_DistanceConstraintComponent* InstantiateDistanceJoint(
		const FDistanceJointBarrier& Barrier, AActor& Owner);

	UAGX_LockConstraintComponent* InstantiateLockJoint(
		const FLockJointBarrier& Barrier, AActor& Owner);

	UAGX_TwoBodyTireComponent* InstantiateTwoBodyTire(
		const FTwoBodyTireBarrier& Barrier, AActor& Owner, bool IsBlueprintOwner = false);

	UAGX_CollisionGroupDisablerComponent* InstantiateCollisionGroupDisabler(
		AActor& Owner, const TArray<std::pair<FString, FString>>& DisabledPairs);

	UAGX_WireComponent* InstantiateWire(const FWireBarrier& Barrier, AActor& Owner);

	UAGX_RigidBodyComponent* GetBody(
		const FRigidBodyBarrier& Barrier, bool LogErrorIfNotFound = true);

	using FBodyPair = std::pair<UAGX_RigidBodyComponent*, UAGX_RigidBodyComponent*>;
	FBodyPair GetBodies(const FConstraintBarrier& Barrier);

	UAGX_ShapeMaterialAsset* GetShapeMaterial(const FShapeMaterialBarrier& Barrier);

	using FShapeMaterialPair = std::pair<UAGX_ShapeMaterialAsset*, UAGX_ShapeMaterialAsset*>;
	FShapeMaterialPair GetShapeMaterials(const FContactMaterialBarrier& ContactMaterial);

	explicit FAGX_SimObjectsImporterHelper(const FString& InSourceFilePath);

	const FString SourceFilePath;
	const FString SourceFileName;
	const FString ModelName;
	const FString DirectoryName;

private:
	TMap<FGuid, UStaticMesh*> RestoredMeshes;
	TMap<FGuid, UAGX_RigidBodyComponent*> RestoredBodies;
	TMap<FGuid, UAGX_ShapeMaterialAsset*> RestoredShapeMaterials;
	TMap<FGuid, UMaterialInstanceConstant*> RestoredRenderMaterials;

	// List of Constraints that should not be imported the usual way, i.e. through the
	// Instantiate<Constraint-type>() functions. These may be owned by higher level models such as
	// e.g. TwoBodyTire and it is up to those models to handle import of their own Constraints.
	TArray<FGuid> ConstraintIgnoreList;
};

/// \todo Consider creating a FEditorBody inheriting from FAGXSimObjectBody that has a Body and a
/// Helper and simply forwards each call to the helper.

/**
 * A SimObjectBody that creates nothing. Used when the Unreal object couldn't be created.
 */
class NopEditorBody final : public FAGXSimObjectBody
{
	virtual void InstantiateSphere(const FSphereShapeBarrier& Barrier) override
	{
	}

	virtual void InstantiateBox(const FBoxShapeBarrier& Barrier) override
	{
	}

	virtual void InstantiateCylinder(const FCylinderShapeBarrier& Barrier) override
	{
	}

	virtual void InstantiateCapsule(const FCapsuleShapeBarrier& Barrier) override
	{
	}

	virtual void InstantiateTrimesh(const FTrimeshShapeBarrier& Barrier) override
	{
	}
};