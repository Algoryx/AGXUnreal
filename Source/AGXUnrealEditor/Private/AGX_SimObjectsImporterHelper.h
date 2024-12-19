// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Import/AGX_ImportEnums.h"
#include "Utilities/AGX_ImportUtilities.h"

// Unreal Engine includes.
#include "Containers/Map.h"
#include "Misc/Guid.h"

// AGX Dynamics for Unreal structs.
struct FAGX_ImportSettings;

// AGX Dynamics for Unreal classes.
class FBallJointBarrier;
class FBoxShapeBarrier;
class FCapsuleShapeBarrier;
class FConstraintBarrier;
class FContactMaterialBarrier;
class FCylinderShapeBarrier;
class FCylindricalJointBarrier;
class FDistanceJointBarrier;
class FHingeBarrier;
class FLockJointBarrier;
class FPrismaticBarrier;
class FRigidBodyBarrier;
class FShapeMaterialBarrier;
class FShovelBarrier;
class FSphereShapeBarrier;
class FTwoBodyTireBarrier;
class FWireBarrier;
class UAGX_BallConstraintComponent;
class UAGX_BoxShapeComponent;
class UAGX_CapsuleShapeComponent;
class UAGX_CollisionGroupDisablerComponent;
class UAGX_ConstraintComponent;
class UAGX_ContactMaterial;
class UAGX_ContactMaterialRegistrarComponent;
class UAGX_CylinderShapeComponent;
class UAGX_CylindricalConstraintComponent;
class UAGX_DistanceConstraintComponent;
class UAGX_HingeConstraintComponent;
class UAGX_LockConstraintComponent;
class UAGX_MergeSplitThresholdsBase;
class UAGX_ModelSourceComponent;
class UAGX_ObserverFrameComponent;
class UAGX_PrismaticConstraintComponent;
class UAGX_RigidBodyComponent;
class UAGX_ShapeComponent;
class UAGX_ShapeMaterial;
class UAGX_ShovelComponent;
class UAGX_SphereShapeComponent;
class UAGX_TrackComponent;
class UAGX_TrackProperties;
class UAGX_TrimeshShapeComponent;
class UAGX_TwoBodyTireComponent;
class UAGX_WireComponent;

// Unreal Engine classes.
class AActor;
class UMaterialInstanceConstant;
class USceneComponent;
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

	void UpdateRigidBodyComponent(
		const FRigidBodyBarrier& Barrier, UAGX_RigidBodyComponent& Component,
		const TMap<FGuid, UAGX_MergeSplitThresholdsBase*>& MSTsOnDisk,
		bool ForceOverwriteInstances);

	UAGX_SphereShapeComponent* InstantiateSphere(
		const FSphereShapeBarrier& Sphere, AActor& Owner, const FRigidBodyBarrier* Body = nullptr);

	void UpdateComponent(
		const FSphereShapeBarrier& Barrier, UAGX_SphereShapeComponent& Component,
		const TMap<FGuid, UAGX_MergeSplitThresholdsBase*>& MSTsOnDisk,
		bool ForceOverwritePropertiesInInstances, bool ForceReassignRenderMaterialInInstances);

	UAGX_BoxShapeComponent* InstantiateBox(
		const FBoxShapeBarrier& Barrier, AActor& Owner, const FRigidBodyBarrier* Body = nullptr);

	void UpdateComponent(
		const FBoxShapeBarrier& Barrier, UAGX_BoxShapeComponent& Component,
		const TMap<FGuid, UAGX_MergeSplitThresholdsBase*>& MSTsOnDisk,
		bool ForceOverwritePropertiesInInstances, bool ForceReassignRenderMaterialInInstances);

	UAGX_CylinderShapeComponent* InstantiateCylinder(
		const FCylinderShapeBarrier& Barrier, AActor& Owner,
		const FRigidBodyBarrier* Body = nullptr);

	void UpdateComponent(
		const FCylinderShapeBarrier& Barrier, UAGX_CylinderShapeComponent& Component,
		const TMap<FGuid, UAGX_MergeSplitThresholdsBase*>& MSTsOnDisk,
		bool ForceOverwritePropertiesInInstances, bool ForceReassignRenderMaterialInInstances);

	UAGX_CapsuleShapeComponent* InstantiateCapsule(
		const FCapsuleShapeBarrier& Barrier, AActor& Owner,
		const FRigidBodyBarrier* Body = nullptr);

	void UpdateComponent(
		const FCapsuleShapeBarrier& Barrier, UAGX_CapsuleShapeComponent& Component,
		const TMap<FGuid, UAGX_MergeSplitThresholdsBase*>& MSTsOnDisk,
		bool ForceOverwritePropertiesInInstances, bool ForceReassignRenderMaterialInInstances);

	UAGX_TrimeshShapeComponent* InstantiateTrimesh(
		const FTrimeshShapeBarrier& Barrier, AActor& Owner,
		const FRigidBodyBarrier* Body = nullptr);

	// Updates only the Trimesh Component itself, does not create any static meshes.
	void UpdateComponent(
		const FTrimeshShapeBarrier& Barrier, UAGX_TrimeshShapeComponent& Component,
		const TMap<FGuid, UAGX_MergeSplitThresholdsBase*>& MSTsOnDisk,
		bool ForceOverwritePropertiesInInstances, bool ForceReassignRenderMaterialInInstances);

	// This also updates/creates the corresponding static mesh asset and material.
	void UpdateTrimeshCollisionMeshComponent(
		const FTrimeshShapeBarrier& Barrier, UStaticMeshComponent& Component,
		bool ForceOverwritePropertiesInInstances, bool ForceReassignRenderMaterialInInstances);

	UStaticMeshComponent* InstantiateRenderData(
		const FShapeBarrier& ShapeBarrier, AActor& Owner, USceneComponent& AttachParent,
		FTransform* RelTransformOverride = nullptr);

	UStaticMeshComponent* InstantiateRenderDataInBodyOrRoot(
		const FTrimeshShapeBarrier& Barrier, AActor& Owner,
		const FRigidBodyBarrier* Body = nullptr);

	// This also updates/creates the corresponding static mesh asset and material.
	void UpdateRenderDataComponent(
		const FShapeBarrier& ShapeBarrier, const FRenderDataBarrier& RenderDataBarrier,
		UStaticMeshComponent& Component, bool ForceOverwritePropertiesInInstances,
		bool ForceReassignRenderMaterialInInstances, FTransform* RelTransformOverride = nullptr);

	void UpdateAndSaveShapeMaterialAsset(
		const FShapeMaterialBarrier& Barrier, UAGX_ShapeMaterial& Asset);

	UAGX_ShapeMaterial* InstantiateShapeMaterial(const FShapeMaterialBarrier& Barrier);

	UMaterialInterface* InstantiateRenderMaterial(const FAGX_RenderMaterial& Material);

	void UpdateAndSaveRenderMaterialAsset(
		const FAGX_RenderMaterial& Barrier, UMaterialInstanceConstant& Asset);

	// This function also adds the Asset to the passed Contact Material Registrar, if not yet added.
	void UpdateAndSaveContactMaterialAsset(
		const FContactMaterialBarrier& Barrier, UAGX_ContactMaterial& Asset,
		UAGX_ContactMaterialRegistrarComponent& CMRegistrar);

	// This function also adds the new Contact Material to the Contact Material Registrar.
	UAGX_ContactMaterial* InstantiateContactMaterial(
		const FContactMaterialBarrier& Barrier,
		UAGX_ContactMaterialRegistrarComponent& CMRegistrar);

	UAGX_ContactMaterialRegistrarComponent* InstantiateContactMaterialRegistrar(AActor& Owner);

	UAGX_HingeConstraintComponent* InstantiateHinge(const FHingeBarrier& Barrier, AActor& Owner);

	UAGX_PrismaticConstraintComponent* InstantiatePrismatic(
		const FPrismaticBarrier& Barrier, AActor& Owner);

	UAGX_BallConstraintComponent* InstantiateBallConstraint(
		const FBallJointBarrier& Barrier, AActor& Owner);

	UAGX_CylindricalConstraintComponent* InstantiateCylindricalConstraint(
		const FCylindricalJointBarrier& Barrier, AActor& Owner);

	UAGX_DistanceConstraintComponent* InstantiateDistanceConstraint(
		const FDistanceJointBarrier& Barrier, AActor& Owner);

	UAGX_LockConstraintComponent* InstantiateLockConstraint(
		const FLockJointBarrier& Barrier, AActor& Owner);

	void UpdateConstraintComponent(
		const FConstraintBarrier& Barrier, UAGX_ConstraintComponent& Component,
		const TMap<FGuid, UAGX_MergeSplitThresholdsBase*>& MSTsOnDisk,
		bool ForceOverwriteInstances);

	void UpdateConstraintComponent(
		const FBallJointBarrier& Barrier, UAGX_ConstraintComponent& Component,
		const TMap<FGuid, UAGX_MergeSplitThresholdsBase*>& MSTsOnDisk,
		bool bForceOverwriteInstances);

	UAGX_TwoBodyTireComponent* InstantiateTwoBodyTire(
		const FTwoBodyTireBarrier& Barrier, AActor& Owner);

	void UpdateTwoBodyTire(
		const FTwoBodyTireBarrier& Barrier, UAGX_TwoBodyTireComponent& Component,
		bool ForceOverwriteInstances);

	UAGX_CollisionGroupDisablerComponent* InstantiateCollisionGroupDisabler(
		AActor& Owner, const TArray<std::pair<FString, FString>>& DisabledPairs);

	void UpdateCollisionGroupDisabler(
		const TArray<std::pair<FString, FString>>& DisabledPairs,
		UAGX_CollisionGroupDisablerComponent& Component);

	UAGX_WireComponent* InstantiateWire(const FWireBarrier& Barrier, AActor& Owner);

	UAGX_ShovelComponent* InstantiateShovel(const FShovelBarrier& ShovelBarrier, AActor& Owner);
	void UpdateShovel(
		const FShovelBarrier& ShovelBarrier, UAGX_ShovelComponent& ShovelComponent,
		const FString& BodyName, const UAGX_RigidBodyComponent* BodyComponent,
		bool ForceOverwriteInstances);

	UAGX_TrackComponent* InstantiateTrack(const FTrackBarrier& Barrier, AActor& Owner);

	void UpdateModelSourceComponent(UAGX_ModelSourceComponent& Component);

	UAGX_ModelSourceComponent* InstantiateModelSourceComponent(AActor& Owner);

	UAGX_ObserverFrameComponent* InstantiateObserverFrame(
		const FString& Name, const FGuid& BodyGuid, const FGuid& ObserverGuid,
		const FTransform& Transform, AActor& Owner);

	void UpdateObserverFrameComponent(
		const FString& Name, const FGuid& ObserverGuid, const FTransform& Transform,
		UAGX_ObserverFrameComponent& Component, bool ForceOverwriteInstances);

	UAGX_RigidBodyComponent* GetBody(
		const FRigidBodyBarrier& Barrier, bool LogErrorIfNotFound = true);

	using FBodyPair = std::pair<UAGX_RigidBodyComponent*, UAGX_RigidBodyComponent*>;
	FBodyPair GetBodies(const FConstraintBarrier& Barrier);

	UAGX_ShapeMaterial* GetShapeMaterial(const FShapeMaterialBarrier& Barrier);

	using FShapeMaterialPair = std::pair<UAGX_ShapeMaterial*, UAGX_ShapeMaterial*>;
	FShapeMaterialPair GetShapeMaterials(const FContactMaterialBarrier& ContactMaterial);

	/*
	 * Must be called at the end of an import.
	 */
	void FinalizeImport();

	// The root model directory will be set to the default import directory.
	explicit FAGX_SimObjectsImporterHelper(
		const FString& InSourceFilePath, bool bInIgnoreDisabledTrimeshes);

	// The root model directory specified by caller.
	explicit FAGX_SimObjectsImporterHelper(
		const FString& InSourceFilePath, bool bInIgnoreDisabledTrimeshes,
		const FString& InRootDirectoryPath);

	const FString SourceFilePath;
	const FString SourceFileName;
	const FString RootDirectoryName;
	const FString RootDirectoryPath;
	const bool bIgnoreDisabledTrimeshes;

private:
	void UpdateShapeComponent(
		const FShapeBarrier& Barrier, UAGX_ShapeComponent& Component,
		const TMap<FGuid, UAGX_MergeSplitThresholdsBase*>& MSTsOnDisk,
		bool ForceOverwritePropertiesInInstances, bool ForceReassignRenderMaterialInInstances);

	TMap<FGuid, UStaticMesh*> ProcessedMeshes; // Static Mesh assets.
	TMap<FGuid, UAGX_MergeSplitThresholdsBase*> ProcessedThresholds;
	TMap<FGuid, UAGX_RigidBodyComponent*> ProcessedBodies;
	TMap<FGuid, UAGX_ShapeMaterial*> ProcessedShapeMaterials;
	TMap<FGuid, UMaterialInstanceConstant*> ProcessedRenderMaterials;
	TMap<FGuid, UAGX_TrackProperties*> ProcessedTrackProperties;

	// The key is the GUID of the Shape Component that the Static Mesh Component is rendering the
	// render data for.
	TMap<FGuid, UStaticMeshComponent*> ProcessedRenderStaticMeshComponents;

	// The key is the Guid of the corresponding AGX Dynamics Trimesh.
	TMap<FGuid, UStaticMeshComponent*> ProcessedCollisionStaticMeshComponents;
};
