// Copyright 2024, Algoryx Simulation AB.

#include "AGX_SimObjectsImporterHelper.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "Import/AGX_ModelSourceComponent.h"
#include "AGX_ObserverFrameComponent.h"
#include "AGX_RigidBodyComponent.h"
#include "AMOR/AGX_AmorEnums.h"
#include "AMOR/ConstraintMergeSplitThresholdsBarrier.h"
#include "AMOR/ShapeContactMergeSplitThresholdsBarrier.h"
#include "AMOR/WireMergeSplitThresholdsBarrier.h"
#include "CollisionGroups/AGX_CollisionGroupDisablerComponent.h"
#include "Constraints/AGX_BallConstraintComponent.h"
#include "Constraints/AGX_Constraint1DofComponent.h"
#include "Constraints/AGX_Constraint2DofComponent.h"
#include "Constraints/AGX_CylindricalConstraintComponent.h"
#include "Constraints/AGX_DistanceConstraintComponent.h"
#include "Constraints/AGX_HingeConstraintComponent.h"
#include "Constraints/AGX_LockConstraintComponent.h"
#include "Constraints/AGX_PrismaticConstraintComponent.h"
#include "Constraints/BallJointBarrier.h"
#include "Constraints/Constraint1DOFBarrier.h"
#include "Constraints/Constraint2DOFBarrier.h"
#include "Constraints/ConstraintBarrier.h"
#include "Constraints/CylindricalJointBarrier.h"
#include "Constraints/DistanceJointBarrier.h"
#include "Constraints/HingeBarrier.h"
#include "Constraints/LockJointBarrier.h"
#include "Constraints/PrismaticBarrier.h"
#include "Materials/AGX_ContactMaterial.h"
#include "Materials/AGX_ContactMaterialRegistrarComponent.h"
#include "Materials/AGX_ShapeMaterial.h"
#include "Materials/ContactMaterialBarrier.h"
#include "Materials/ShapeMaterialBarrier.h"
#include "OpenPLX/PLX_SignalHandlerComponent.h"
#include "RigidBodyBarrier.h"
#include "Shapes/AGX_BoxShapeComponent.h"
#include "Shapes/AGX_CapsuleShapeComponent.h"
#include "Shapes/AGX_CylinderShapeComponent.h"
#include "Shapes/AGX_SphereShapeComponent.h"
#include "Shapes/AGX_TrimeshShapeComponent.h"
#include "Shapes/RenderDataBarrier.h"
#include "Terrain/AGX_ShovelComponent.h"
#include "Terrain/AGX_ShovelProperties.h"
#include "Tires/AGX_TwoBodyTireComponent.h"
#include "Tires/TwoBodyTireBarrier.h"
#include "Utilities/AGX_BlueprintUtilities.h"
#include "Utilities/AGX_ConstraintUtilities.h"
#include "Utilities/AGX_EditorUtilities.h"
#include "Utilities/AGX_ObjectUtilities.h"
#include "Vehicle/AGX_TrackComponent.h"
#include "Vehicle/AGX_TrackInternalMergeProperties.h"
#include "Vehicle/AGX_TrackProperties.h"
#include "Vehicle/TrackPropertiesBarrier.h"
#include "Vehicle/TrackWheelBarrier.h"
#include "Wire/AGX_WireComponent.h"

// Unreal Engine includes.
#include "AssetToolsModule.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Factories/MaterialInstanceConstantFactoryNew.h"
#include "FileHelpers.h"
#include "GameFramework/Actor.h"
#include "IAssetTools.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Materials/MaterialInterface.h"
#include "Materials/Material.h"
#include "MeshDescription.h"
#include "Misc/Paths.h"
#include "UObject/UObjectGlobals.h"

namespace
{
	void WriteImportErrorMessage(
		const TCHAR* ObjectType, const FString& Name, const FString& FilePath, const TCHAR* Message)
	{
		UE_LOG(
			LogAGX, Error, TEXT("Could not import '%s' '%s' from file '%s': %s."), ObjectType,
			*Name, *FilePath, Message);
	}

	UAGX_TrackProperties* GetOrCreateTrackPropertiesAsset(
		const FTrackPropertiesBarrier& Barrier, const FString& Name,
		TMap<FGuid, UAGX_TrackProperties*>& ProcessedTrackProperties, const FString& DirectoryPath)
	{
		const FGuid Guid = Barrier.GetGuid();
		if (!Guid.IsValid())
		{
			// The GUID is invalid, but try to create the asset anyway but without adding it to
			// the ProcessedTrackProperties cache.
			return FAGX_ImportUtilities::SaveImportedTrackPropertiesAsset(
				Barrier, DirectoryPath, Name);
		}

		if (UAGX_TrackProperties* Asset = ProcessedTrackProperties.FindRef(Guid))
		{
			// We have seen this asset before, use the one in the cache.
			return Asset;
		}

		// This is a new Track Properties. Create the asset and add to the cache.
		UAGX_TrackProperties* Asset =
			FAGX_ImportUtilities::SaveImportedTrackPropertiesAsset(Barrier, DirectoryPath, Name);
		if (Asset != nullptr)
		{
			ProcessedTrackProperties.Add(Guid, Asset);
		}
		return Asset;
	}

	FString GetName(UAGX_ShapeMaterial* Material)
	{
		if (Material == nullptr)
		{
			return TEXT("Default");
		}
		return Material->GetName();
	}

	UMaterial* GetDefaultRenderMaterial(bool bIsSensor)
	{
		const TCHAR* AssetPath =
			bIsSensor
				? TEXT("Material'/AGXUnreal/Runtime/Materials/M_SensorMaterial.M_SensorMaterial'")
				: TEXT("Material'/AGXUnreal/Runtime/Materials/M_ImportedBase.M_ImportedBase'");
		UMaterial* Material = FAGX_ObjectUtilities::GetAssetFromPath<UMaterial>(AssetPath);
		if (Material == nullptr)
		{
			UE_LOG(
				LogAGX, Warning, TEXT("Could not load default%s render material from '%s'."),
				(bIsSensor ? TEXT(" sensor") : TEXT("")), AssetPath);
		}
		return Material;
	}

	/**
	 * Convert the given Trimesh to an Unreal Engine Static Mesh asset.
	 *
	 * The created meshes are cached on the Trimesh's Mesh Data GUID so asking for the same mesh
	 * again will return the previously created Static Mesh asset.
	 *
	 * @param Trimesh The Trimesh containing the mesh to store.
	 * @param FallbackName A name to give the asset in case the Trimesh doesn't have a valid name.
	 * @param ProcessedMeshes Static Mesh cache.
	 * @param ProcessedRenderMaterials Render Material cache.
	 * @param DirectoryPath The path of the folder where all assets for this imported model is
	 * stored.
	 * @return
	 */
	UStaticMesh* GetOrCreateStaticMeshAsset(
		const FTrimeshShapeBarrier& Trimesh, const FString& FallbackName,
		TMap<FGuid, UStaticMesh*>& ProcessedMeshes,
		const TMap<FGuid, UMaterialInstanceConstant*>& ProcessedRenderMaterials,
		const FString& DirectoryPath)
	{
		UMaterialInstanceConstant* RenderMaterial = [&]() -> UMaterialInstanceConstant*
		{
			if (!Trimesh.HasRenderMaterial())
			{
				return nullptr;
			}

			const FGuid RenderMaterialGuid = Trimesh.GetRenderMaterial().Guid;
			return ProcessedRenderMaterials.FindRef(RenderMaterialGuid);
		}();

		const FGuid Guid = Trimesh.GetMeshDataGuid();
		if (!Guid.IsValid())
		{
			// The GUID is invalid, but try to create the mesh asset anyway but without adding it to
			// the ProcessedMeshes cache.
			return FAGX_ImportUtilities::SaveImportedStaticMeshAsset(
				Trimesh, DirectoryPath, FallbackName, RenderMaterial);
		}

		{
			UStaticMesh* ProcessedMesh = ProcessedMeshes.FindRef(Guid);
			if (ProcessedMesh != nullptr)
			{
				// We have seen this mesh before, use the one in the cache.
				return ProcessedMesh;
			}
		}

		// This is a new mesh. Create the Static Mesh asset and add to the cache.
		UStaticMesh* Asset = FAGX_ImportUtilities::SaveImportedStaticMeshAsset(
			Trimesh, DirectoryPath, FallbackName, RenderMaterial);
		if (Asset != nullptr)
		{
			ProcessedMeshes.Add(Guid, Asset);
		}
		return Asset;
	}

	/**
	 * Convert the given Render Data to an Unreal Engine Static Mesh asset.
	 *
	 * The created meshes are cached on GUID so asking for the same Render Data mesh again will
	 * return the previously created Static Mesh asset.
	 *
	 * @param RenderData The Render Data Barrier containing the mesh to store.
	 * @param ProcessedMeshes Static Mesh cache.
	 * @param ProcessedRenderMaterials Render Material cache.
	 * @param DirectoryPath The path of the folder where all assets for the imported model is
	 * stored.
	 * @return The Static Mesh asset for the given Render Data.
	 */
	UStaticMesh* GetOrCreateStaticMeshAsset(
		const FRenderDataBarrier& RenderData, TMap<FGuid, UStaticMesh*>& ProcessedMeshes,
		const TMap<FGuid, UMaterialInstanceConstant*>& ProcessedRenderMaterials,
		const FString& DirectoryPath)
	{
		UMaterialInstanceConstant* RenderMaterial = [&]() -> UMaterialInstanceConstant*
		{
			if (!RenderData.HasMaterial())
			{
				return nullptr;
			}

			const FGuid RenderMaterialGuid = RenderData.GetMaterial().Guid;
			return ProcessedRenderMaterials.FindRef(RenderMaterialGuid);
		}();

		const FGuid Guid = RenderData.GetGuid();
		if (!Guid.IsValid())
		{
			// The GUID is invalid, but try to create the mesh asset anyway but without adding it to
			// the ProcessedMeshes cache.
			return FAGX_ImportUtilities::SaveImportedStaticMeshAsset(
				RenderData, DirectoryPath, RenderMaterial);
		}

		{
			UStaticMesh* ProcessedMesh = ProcessedMeshes.FindRef(Guid);
			if (ProcessedMesh != nullptr)
			{
				// We have seen this mesh before, use the one in the cache.
				return ProcessedMesh;
			}
		}

		// This is a new mesh. Create the Static Mesh asset and add to the cache.
		UStaticMesh* Asset = FAGX_ImportUtilities::SaveImportedStaticMeshAsset(
			RenderData, DirectoryPath, RenderMaterial);
		if (Asset != nullptr)
		{
			ProcessedMeshes.Add(Guid, Asset);
		}
		return Asset;
	}

	FString CreateMergeSplitThresholdsAssetName(EAGX_AmorOwningType OwningType, const FGuid& Guid)
	{
		switch (OwningType)
		{
			case EAGX_AmorOwningType::BodyOrShape:
				return "AGX_SMST_" + Guid.ToString();
			case EAGX_AmorOwningType::Constraint:
				return "AGX_CMST_" + Guid.ToString();
			case EAGX_AmorOwningType::Wire:
				return "AGX_WMST_" + Guid.ToString();
		}

		UE_LOG(LogAGX, Warning, TEXT("Unknown OwningType in CreateMergeSplitThresholdsAssetName."));
		return "AGX_MST_" + Guid.ToString();
	}

	void UpdateAndSaveMergeSplitThresholdsAsset(
		const FMergeSplitThresholdsBarrier& Barrier, UAGX_MergeSplitThresholdsBase& Asset,
		TMap<FGuid, UAGX_MergeSplitThresholdsBase*>& ProcessedThresholds,
		EAGX_AmorOwningType OwningType)
	{
		const FGuid Guid = Barrier.GetGuid();
		const FString AssetName = CreateMergeSplitThresholdsAssetName(OwningType, Guid);

		FAGX_EditorUtilities::RenameAsset(Asset, AssetName, "AGX_MST");
		Asset.CopyFrom(Barrier);
		FAGX_ObjectUtilities::SaveAsset(Asset);
		if (Guid.IsValid())
		{
			ProcessedThresholds.Add(Guid, &Asset);
		}
	}

	template <typename TBarrier, typename TThresholdsBarrier>
	UAGX_MergeSplitThresholdsBase* GetOrCreateMergeSplitThresholdsAsset(
		const TBarrier& Barrier, EAGX_AmorOwningType OwningType,
		TMap<FGuid, UAGX_MergeSplitThresholdsBase*>& ProcessedThresholds,
		const FString& DirectoryPath)
	{
		const TThresholdsBarrier ThresholdsBarrier = TThresholdsBarrier::CreateFrom(Barrier);
		if (!ThresholdsBarrier.HasNative())
		{
			// The native object did not have any MergeSplitThreshold associated with it.
			return nullptr;
		}

		const FGuid Guid = ThresholdsBarrier.GetGuid();
		const FString AssetName = CreateMergeSplitThresholdsAssetName(OwningType, Guid);
		auto CreateAsset = [&]() -> UAGX_MergeSplitThresholdsBase*
		{
			const FString MSTDir =
				FAGX_ImportUtilities::GetImportMergeSplitThresholdsDirectoryName();
			switch (OwningType)
			{
				case EAGX_AmorOwningType::BodyOrShape:
					return FAGX_ImportUtilities::CreateAsset<UAGX_ShapeContactMergeSplitThresholds>(
						DirectoryPath, AssetName, MSTDir);
				case EAGX_AmorOwningType::Constraint:
					return FAGX_ImportUtilities::CreateAsset<UAGX_ConstraintMergeSplitThresholds>(
						DirectoryPath, AssetName, MSTDir);
				case EAGX_AmorOwningType::Wire:
					return FAGX_ImportUtilities::CreateAsset<UAGX_WireMergeSplitThresholds>(
						DirectoryPath, AssetName, MSTDir);
				default:
					return nullptr;
			}
		};

		if (UAGX_MergeSplitThresholdsBase* Asset = ProcessedThresholds.FindRef(Guid))
		{
			// We have seen this before, use the one in the cache.
			return Asset;
		}

		// This is a new merge split thresholds. Create the asset and add to the cache.
		UAGX_MergeSplitThresholdsBase* Asset = CreateAsset();
		if (Asset == nullptr)
		{
			UE_LOG(
				LogAGX, Error, TEXT("Unable to create Merge Split Thresholds asset for '%s'."),
				*Barrier.GetName());
			return nullptr;
		}

		UpdateAndSaveMergeSplitThresholdsAsset(
			ThresholdsBarrier, *Asset, ProcessedThresholds, OwningType);

		return Asset;
	}

	UAGX_ContactMaterialRegistrarComponent* GetOrCreateContactMaterialRegistrar(AActor& Owner)
	{
		UAGX_ContactMaterialRegistrarComponent* Component =
			Owner.FindComponentByClass<UAGX_ContactMaterialRegistrarComponent>();

		if (Component != nullptr)
		{
			return Component;
		}

		// No UAGX_ContactMaterialRegistrarComponent exists in Owner. Create and add one.
		const FString CMRName = FAGX_ImportUtilities::GetContactMaterialRegistrarDefaultName();
		Component = NewObject<UAGX_ContactMaterialRegistrarComponent>(&Owner, *CMRName);

		Component->SetFlags(RF_Transactional);
		Owner.AddInstanceComponent(Component);
		Component->RegisterComponent();
		return Component;
	}

	bool IsMeshEquivalent(const FRenderDataBarrier& RenderDataBarrier, UStaticMesh* StaticMesh)
	{
		if (StaticMesh == nullptr)
		{
			return false;
		}

		// @todo: can we match the meshes in a fast way? The vertex count does not generally match
		// apparently, probably because UE does some optimizations when building the original
		// StaticMesh.
#if 0
		FMeshDescription* MeshDescr = StaticMesh->GetMeshDescription(0);
		const auto& Vertices = MeshDescr->Vertices();
		// etc...
#endif
		return false;
	}

	bool IsMeshEquivalent(const FTrimeshShapeBarrier& TrimeshBarrier, UStaticMesh* StaticMesh)
	{
		// @todo : see comment in IsMeshEquivalent(const FRenderDataBarrier& RenderDataBarrier,
		// UStaticMesh* StaticMesh) above.
		return false;
	}
}

void FAGX_SimObjectsImporterHelper::UpdateRigidBodyComponent(
	const FRigidBodyBarrier& Barrier, UAGX_RigidBodyComponent& Component,
	const TMap<FGuid, UAGX_MergeSplitThresholdsBase*>& MSTsOnDisk, bool ForceOverwriteInstances)
{
	FAGX_ImportUtilities::Rename(Component, Barrier.GetName());
	Component.CopyFrom(Barrier, ForceOverwriteInstances);

	const FShapeContactMergeSplitThresholdsBarrier ThresholdsBarrier =
		FShapeContactMergeSplitThresholdsBarrier::CreateFrom(Barrier);

	UAGX_MergeSplitThresholdsBase* MSThresholds = nullptr;
	if (ThresholdsBarrier.HasNative())
	{
		const FGuid MSTGuid = ThresholdsBarrier.GetGuid();
		MSThresholds = MSTsOnDisk.FindRef(MSTGuid);
		if (MSThresholds == nullptr)
		{
			MSThresholds = ::GetOrCreateMergeSplitThresholdsAsset<
				FRigidBodyBarrier, FShapeContactMergeSplitThresholdsBarrier>(
				Barrier, EAGX_AmorOwningType::BodyOrShape, ProcessedThresholds, RootDirectoryPath);
		}
		else
		{
			::UpdateAndSaveMergeSplitThresholdsAsset(
				ThresholdsBarrier, *MSThresholds, ProcessedThresholds,
				EAGX_AmorOwningType::BodyOrShape);
		}
	}

	if (FAGX_ObjectUtilities::IsTemplateComponent(Component))
	{
		for (auto Instance : FAGX_ObjectUtilities::GetArchetypeInstances(Component))
		{
			if (ForceOverwriteInstances || Instance->MergeSplitProperties.Thresholds ==
											   Component.MergeSplitProperties.Thresholds)
			{
				Instance->MergeSplitProperties.Thresholds =
					Cast<UAGX_ShapeContactMergeSplitThresholds>(MSThresholds);
			}
		}
	}

	Component.MergeSplitProperties.Thresholds =
		Cast<UAGX_ShapeContactMergeSplitThresholds>(MSThresholds);

	AGX_CHECK(!ProcessedBodies.Contains(Barrier.GetGuid()));
	ProcessedBodies.Add(Barrier.GetGuid(), &Component);
}

UAGX_RigidBodyComponent* FAGX_SimObjectsImporterHelper::InstantiateBody(
	const FRigidBodyBarrier& Barrier, AActor& Actor)
{
	// Only instantiate body if it has not already been instantiated. It might have been
	// instantiated already during import of e.g. Tire model.
	if (GetBody(Barrier, false) != nullptr)
	{
		return nullptr;
	}

	UAGX_RigidBodyComponent* Component = NewObject<UAGX_RigidBodyComponent>(&Actor);
	if (Component == nullptr)
	{
		WriteImportErrorMessage(
			TEXT("AGX Dynamics RigidBody"), Barrier.GetName(), SourceFilePath,
			TEXT("Could not create new AGX_RigidBodyComponent"));
		return nullptr;
	}

	TMap<FGuid, UAGX_MergeSplitThresholdsBase*> Unused;
	UpdateRigidBodyComponent(Barrier, *Component, Unused, false);
	Component->SetFlags(RF_Transactional);
	Actor.AddInstanceComponent(Component);

	/// @todo What does this do, really? Are we required to call it? A side effect of this is that
	/// BeginPlay is called, which in turn calls AllocateNative. Which means that an AGX Dynamics
	/// RigidBody is created. I'm not sure if this is consistent with AGX_RigidBodyComponents
	/// created with using the Editor's Add Component button for an Actor in the Level Viewport.
	/// <investigating>
	/// ActorComponent.cpp, RegisterComponentWithWorld, has the following code snippet, somewhat
	/// simplified:
	///
	/// if (!InWorld->IsGameWorld())
	/// {}
	/// else if (MyOwner == nullptr)
	/// {}
	/// else
	/// {
	///    if (MyOwner->HasActorBegunPlay() && !bHasBegunPlay)
	///    {
	///        BeginPlay();
	///     }
	/// }
	///
	/// So, BeginPlay is only called if we don't have a Game world (have Editor world, for example)
	/// and the owning Actor have had its BeginPlay called already.
	///
	/// This makes the Editor situation different from the Automation Test situation since the
	/// Editor has an Editor world and Automation Tests run with a Game world. So creating an
	/// AGX_RigidBodyComponent in the editor does not trigger BeginPlay, but creating an
	/// AGX_RigidBody while importing an AGX Dynamics archive during an Automation Test does trigger
	/// BeginPlay here. Not sure if this is a problem or not, but something to be aware of.
	Component->RegisterComponent();

	Component->PostEditChange();
	return Component;
}

UAGX_SphereShapeComponent* FAGX_SimObjectsImporterHelper::InstantiateSphere(
	const FSphereShapeBarrier& Barrier, AActor& Owner, const FRigidBodyBarrier* BodyBarrier)
{
	UAGX_RigidBodyComponent* Body = BodyBarrier != nullptr ? GetBody(*BodyBarrier) : nullptr;
	UAGX_SphereShapeComponent* Component = FAGX_EditorUtilities::CreateSphereShape(&Owner, Body);
	if (Component == nullptr)
	{
		WriteImportErrorMessage(
			TEXT("AGX Dynamics Sphere"), Barrier.GetName(), SourceFilePath,
			TEXT("Could not create new UAGX_SphereShapeComponent"));
		return nullptr;
	}

	Component->SetFlags(RF_Transactional);
	const TMap<FGuid, UAGX_MergeSplitThresholdsBase*> Unused;
	UpdateComponent(Barrier, *Component, Unused, false, false);

	if (Barrier.HasValidRenderData())
	{
		InstantiateRenderData(Barrier, Owner, *Component);
	}

	return Component;
}

void FAGX_SimObjectsImporterHelper::UpdateComponent(
	const FSphereShapeBarrier& Barrier, UAGX_SphereShapeComponent& Component,
	const TMap<FGuid, UAGX_MergeSplitThresholdsBase*>& MSTsOnDisk,
	bool ForceOverwritePropertiesInInstances, bool ForceReassignRenderMaterialInInstances)
{
	Component.CopyFrom(Barrier, ForceOverwritePropertiesInInstances);
	UpdateShapeComponent(
		Barrier, Component, MSTsOnDisk, ForceOverwritePropertiesInInstances,
		ForceReassignRenderMaterialInInstances);
}

UAGX_BoxShapeComponent* FAGX_SimObjectsImporterHelper::InstantiateBox(
	const FBoxShapeBarrier& Barrier, AActor& Owner, const FRigidBodyBarrier* BodyBarrier)
{
	UAGX_RigidBodyComponent* Body = BodyBarrier != nullptr ? GetBody(*BodyBarrier) : nullptr;
	UAGX_BoxShapeComponent* Component = FAGX_EditorUtilities::CreateBoxShape(&Owner, Body);
	if (Component == nullptr)
	{
		WriteImportErrorMessage(
			TEXT("AGX Dynamics Box"), Barrier.GetName(), SourceFilePath,
			TEXT("Could not create new UAGX_BoxShapeComponent"));
		return nullptr;
	}

	Component->SetFlags(RF_Transactional);
	const TMap<FGuid, UAGX_MergeSplitThresholdsBase*> Unused;
	UpdateComponent(Barrier, *Component, Unused, false, false);

	if (Barrier.HasValidRenderData())
	{
		InstantiateRenderData(Barrier, Owner, *Component);
	}

	return Component;
}

void FAGX_SimObjectsImporterHelper::UpdateComponent(
	const FBoxShapeBarrier& Barrier, UAGX_BoxShapeComponent& Component,
	const TMap<FGuid, UAGX_MergeSplitThresholdsBase*>& MSTsOnDisk,
	bool ForceOverwritePropertiesInInstances, bool ForceReassignRenderMaterialInInstances)
{
	Component.CopyFrom(Barrier, ForceOverwritePropertiesInInstances);
	UpdateShapeComponent(
		Barrier, Component, MSTsOnDisk, ForceOverwritePropertiesInInstances,
		ForceReassignRenderMaterialInInstances);
}

UAGX_CylinderShapeComponent* FAGX_SimObjectsImporterHelper::InstantiateCylinder(
	const FCylinderShapeBarrier& Barrier, AActor& Owner, const FRigidBodyBarrier* BodyBarrier)
{
	UAGX_RigidBodyComponent* Body = BodyBarrier != nullptr ? GetBody(*BodyBarrier) : nullptr;
	UAGX_CylinderShapeComponent* Component =
		FAGX_EditorUtilities::CreateCylinderShape(&Owner, Body);
	if (Component == nullptr)
	{
		WriteImportErrorMessage(
			TEXT("AGX Dynamics Cylinder"), Barrier.GetName(), SourceFilePath,
			TEXT("Could not create new UAGX_CylinderShapeComponent"));
		return nullptr;
	}

	Component->SetFlags(RF_Transactional);
	const TMap<FGuid, UAGX_MergeSplitThresholdsBase*> Unused;
	UpdateComponent(Barrier, *Component, Unused, false, false);

	if (Barrier.HasValidRenderData())
	{
		InstantiateRenderData(Barrier, Owner, *Component);
	}

	return Component;
}

void FAGX_SimObjectsImporterHelper::UpdateComponent(
	const FCylinderShapeBarrier& Barrier, UAGX_CylinderShapeComponent& Component,
	const TMap<FGuid, UAGX_MergeSplitThresholdsBase*>& MSTsOnDisk,
	bool ForceOverwritePropertiesInInstances, bool ForceReassignRenderMaterialInInstances)
{
	Component.CopyFrom(Barrier, ForceOverwritePropertiesInInstances);
	UpdateShapeComponent(
		Barrier, Component, MSTsOnDisk, ForceOverwritePropertiesInInstances,
		ForceReassignRenderMaterialInInstances);
}

UAGX_CapsuleShapeComponent* FAGX_SimObjectsImporterHelper::InstantiateCapsule(
	const FCapsuleShapeBarrier& Barrier, AActor& Owner, const FRigidBodyBarrier* BodyBarrier)
{
	UAGX_RigidBodyComponent* Body = BodyBarrier != nullptr ? GetBody(*BodyBarrier) : nullptr;
	UAGX_CapsuleShapeComponent* Component = FAGX_EditorUtilities::CreateCapsuleShape(&Owner, Body);
	if (Component == nullptr)
	{
		WriteImportErrorMessage(
			TEXT("AGX Dynamics Capsule"), Barrier.GetName(), SourceFilePath,
			TEXT("Could not create new UAGX_CapsuleShapeComponent"));
		return nullptr;
	}

	Component->SetFlags(RF_Transactional);
	const TMap<FGuid, UAGX_MergeSplitThresholdsBase*> Unused;
	UpdateComponent(Barrier, *Component, Unused, false, false);

	if (Barrier.HasValidRenderData())
	{
		InstantiateRenderData(Barrier, Owner, *Component);
	}

	return Component;
}

void FAGX_SimObjectsImporterHelper::UpdateComponent(
	const FCapsuleShapeBarrier& Barrier, UAGX_CapsuleShapeComponent& Component,
	const TMap<FGuid, UAGX_MergeSplitThresholdsBase*>& MSTsOnDisk,
	bool ForceOverwritePropertiesInInstances, bool ForceReassignRenderMaterialInInstances)
{
	Component.CopyFrom(Barrier, ForceOverwritePropertiesInInstances);
	UpdateShapeComponent(
		Barrier, Component, MSTsOnDisk, ForceOverwritePropertiesInInstances,
		ForceReassignRenderMaterialInInstances);
}

UAGX_TrimeshShapeComponent* FAGX_SimObjectsImporterHelper::InstantiateTrimesh(
	const FTrimeshShapeBarrier& Barrier, AActor& Owner, const FRigidBodyBarrier* BodyBarrier)
{
	UAGX_RigidBodyComponent* Body = BodyBarrier != nullptr ? GetBody(*BodyBarrier) : nullptr;
	UAGX_TrimeshShapeComponent* Component =
		FAGX_EditorUtilities::CreateTrimeshShape(&Owner, Body, false);
	if (Component == nullptr)
	{
		WriteImportErrorMessage(
			TEXT("AGX Dynamics Trimesh"), Barrier.GetName(), SourceFilePath,
			TEXT("Could not instantiate a new Trimesh Shape Component."));
		return nullptr;
	}

	Component->MeshSourceLocation = EAGX_StaticMeshSourceLocation::TSL_CHILD_STATIC_MESH_COMPONENT;
	Component->SetFlags(RF_Transactional);
	const TMap<FGuid, UAGX_MergeSplitThresholdsBase*> Unused;
	UpdateComponent(Barrier, *Component, Unused, false, false);

	// Add collision Static mesh.
	UStaticMeshComponent* CollisionMesh =
		FAGX_ImportUtilities::CreateComponent<UStaticMeshComponent>(Owner, *Component);
	UpdateTrimeshCollisionMeshComponent(Barrier, *CollisionMesh, false, false);

	if (Barrier.HasValidRenderData())
	{
		InstantiateRenderData(Barrier, Owner, *CollisionMesh);
	}

	Component->RegisterComponent();

	return Component;
}

void FAGX_SimObjectsImporterHelper::UpdateTrimeshCollisionMeshComponent(
	const FTrimeshShapeBarrier& ShapeBarrier, UStaticMeshComponent& Component,
	bool ForceOverwritePropertiesInInstances, bool ForceReassignRenderMaterialInInstances)
{
	UStaticMesh* NewMeshAsset = nullptr;
	UStaticMesh* OriginalMeshAsset = Component.GetStaticMesh();

	if (IsMeshEquivalent(ShapeBarrier, OriginalMeshAsset))
	{
		NewMeshAsset = OriginalMeshAsset;
	}
	else
	{
		const FString FallbackName =
			ShapeBarrier.GetName().IsEmpty()
				? "SM_CollisionMesh"
				: FString("SM_CollisionMesh_") + ShapeBarrier.GetShapeGuid().ToString();
		UStaticMesh* Asset = GetOrCreateStaticMeshAsset(
			ShapeBarrier, FallbackName, ProcessedMeshes, ProcessedRenderMaterials,
			RootDirectoryPath);
		NewMeshAsset = Asset;
	}

	FAGX_ImportUtilities::Rename(
		Component, FString("CollisionMesh_") + ShapeBarrier.GetShapeGuid().ToString());

	UMaterialInterface* RenderMaterial = nullptr;
	if (ShapeBarrier.HasRenderMaterial())
	{
		FAGX_RenderMaterial RMBarrier = ShapeBarrier.GetRenderMaterial();
		const FGuid RMGuid = RMBarrier.Guid;
		RenderMaterial = ProcessedRenderMaterials.FindRef(RMGuid);
		if (RenderMaterial == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Expected to find processed render material with GUID '%s' from Shape '%s' "
					 "but it was not avaiable. The render material will not be set."),
				*RMGuid.ToString(), *ShapeBarrier.GetName());
		}
	}
	else
	{
		RenderMaterial = GetDefaultRenderMaterial(ShapeBarrier.GetIsSensor());
	}

	// The reason we let GetEnableCollisions determine whether or not the Collision Static Mesh
	// should be visible or not has to do with the behavior of agxViewer which we want to mimic. If
	// a shape in a agxCollide::Geometry which has canCollide == false is written to a AGX
	// archive and then read by agxViewer, the shape will not be visible (unless it has RenderData).
	const bool Visible = ShapeBarrier.GetEnableCollisions() && ShapeBarrier.GetEnabled() &&
						 !ShapeBarrier.HasRenderData();
	if (FAGX_ObjectUtilities::IsTemplateComponent(Component))
	{
		// Sync all component instances.
		for (UStaticMeshComponent* Instance :
			 FAGX_ObjectUtilities::GetArchetypeInstances(Component))
		{
			// Update Mesh asset.
			if (ForceOverwritePropertiesInInstances ||
				Instance->GetStaticMesh() == Component.GetStaticMesh())
			{
				Instance->SetStaticMesh(NewMeshAsset);
			}

			// Update Render Materials.
			if (Visible && (ForceReassignRenderMaterialInInstances ||
							Instance->GetMaterial(0) == Component.GetMaterial(0)))
			{
				Instance->SetMaterial(0, RenderMaterial);
			}

			// Update visibility.
			if (ForceOverwritePropertiesInInstances ||
				Instance->GetVisibleFlag() == Component.GetVisibleFlag())
			{
				Instance->SetVisibility(Visible);
			}
		}
	}

	Component.SetStaticMesh(NewMeshAsset);
	Component.SetMaterial(0, RenderMaterial);
	Component.SetVisibility(Visible);

	const FGuid ShapeGuid = ShapeBarrier.GetShapeGuid();
	if (!ProcessedCollisionStaticMeshComponents.Contains(ShapeGuid))
	{
		ProcessedCollisionStaticMeshComponents.Add(ShapeGuid, &Component);
	}
}

void FAGX_SimObjectsImporterHelper::UpdateComponent(
	const FTrimeshShapeBarrier& Barrier, UAGX_TrimeshShapeComponent& Component,
	const TMap<FGuid, UAGX_MergeSplitThresholdsBase*>& MSTsOnDisk,
	bool ForceOverwritePropertiesInInstances, bool ForceReassignRenderMaterialInInstances)
{
	Component.CopyFrom(Barrier, ForceOverwritePropertiesInInstances);
	UpdateShapeComponent(
		Barrier, Component, MSTsOnDisk, ForceOverwritePropertiesInInstances,
		ForceReassignRenderMaterialInInstances);
}

void FAGX_SimObjectsImporterHelper::UpdateShapeComponent(
	const FShapeBarrier& Barrier, UAGX_ShapeComponent& Component,
	const TMap<FGuid, UAGX_MergeSplitThresholdsBase*>& MSTsOnDisk,
	bool ForceOverwritePropertiesInInstances, bool ForceReassignRenderMaterialInInstances)
{
	FAGX_ImportUtilities::Rename(Component, Barrier.GetName());

	FShapeMaterialBarrier NativeMaterial = Barrier.GetMaterial();
	UAGX_ShapeMaterial* NewShapeMaterial = nullptr;
	if (NativeMaterial.HasNative())
	{
		const FGuid Guid = NativeMaterial.GetGuid();
		NewShapeMaterial = ProcessedShapeMaterials.FindRef(Guid);
	}

	UMaterialInterface* RenderMaterial = nullptr;
	if (Barrier.HasRenderMaterial())
	{
		FAGX_RenderMaterial RMBarrier = Barrier.GetRenderMaterial();
		const FGuid RMGuid = RMBarrier.Guid;
		RenderMaterial = ProcessedRenderMaterials.FindRef(RMGuid);
		if (RenderMaterial == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Expected to find processed render material with GUID '%s' from Shape '%s' "
					 "but it was not avaiable. The render material will not be set."),
				*RMGuid.ToString(), *Barrier.GetName());
		}
	}
	else
	{
		RenderMaterial = GetDefaultRenderMaterial(Barrier.GetIsSensor());
	}

	const FShapeContactMergeSplitThresholdsBarrier ThresholdsBarrier =
		FShapeContactMergeSplitThresholdsBarrier::CreateFrom(Barrier);
	UAGX_MergeSplitThresholdsBase* MSThresholds = nullptr;

	if (ThresholdsBarrier.HasNative())
	{
		const FGuid MSTGuid = ThresholdsBarrier.GetGuid();
		MSThresholds = MSTsOnDisk.FindRef(MSTGuid);
		if (MSThresholds != nullptr)
		{
			::UpdateAndSaveMergeSplitThresholdsAsset(
				ThresholdsBarrier, *MSThresholds, ProcessedThresholds,
				EAGX_AmorOwningType::BodyOrShape);
		}
		else
		{
			MSThresholds = ::GetOrCreateMergeSplitThresholdsAsset<
				FShapeBarrier, FShapeContactMergeSplitThresholdsBarrier>(
				Barrier, EAGX_AmorOwningType::BodyOrShape, ProcessedThresholds, RootDirectoryPath);
		}
	}

	// The reason we let GetEnableCollisions and GetEnable determine whether or not this Shape
	// should be visible or not has to do with the behavior of agxViewer which we want to mimic. If
	// a shape in a agxCollide::Geometry which has canCollide == false is written to a AGX archive
	// and then read by agxViewer, the shape will not be visible (unless it has RenderData).
	const bool Visible =
		Barrier.GetEnableCollisions() && Barrier.GetEnabled() && !Barrier.HasRenderData();
	if (FAGX_ObjectUtilities::IsTemplateComponent(Component))
	{
		// Sync all component instances.
		for (UAGX_ShapeComponent* Instance : FAGX_ObjectUtilities::GetArchetypeInstances(Component))
		{
			if (ForceReassignRenderMaterialInInstances ||
				Instance->GetMaterial(0) == Component.GetMaterial(0))
			{
				Instance->SetMaterial(0, RenderMaterial);
			}

			Instance->UpdateVisualMesh();

			if (ForceOverwritePropertiesInInstances ||
				Instance->ShapeMaterial == Component.ShapeMaterial)
			{
				Instance->ShapeMaterial = NewShapeMaterial;
			}

			if (ForceOverwritePropertiesInInstances ||
				Instance->MergeSplitProperties.Thresholds ==
					Component.MergeSplitProperties.Thresholds)
			{
				Instance->MergeSplitProperties.Thresholds =
					Cast<UAGX_ShapeContactMergeSplitThresholds>(MSThresholds);
			}

			if (ForceOverwritePropertiesInInstances ||
				Instance->GetVisibleFlag() == Component.GetVisibleFlag())
			{
				Instance->SetVisibility(Visible);
			}
		}
	}

	Component.SetMaterial(0, RenderMaterial);
	Component.UpdateVisualMesh();
	Component.ShapeMaterial = NewShapeMaterial;
	Component.MergeSplitProperties.Thresholds =
		Cast<UAGX_ShapeContactMergeSplitThresholds>(MSThresholds);
	Component.SetVisibility(Visible);
}

UStaticMeshComponent* FAGX_SimObjectsImporterHelper::InstantiateRenderData(
	const FShapeBarrier& ShapeBarrier, AActor& Owner, USceneComponent& AttachParent,
	FTransform* RelTransformOverride)
{
	const FRenderDataBarrier RenderDataBarrier = ShapeBarrier.GetRenderData();
	if (!RenderDataBarrier.HasMesh())
		return nullptr;

	UStaticMeshComponent* RenderMeshComponent =
		FAGX_ImportUtilities::CreateComponent<UStaticMeshComponent>(Owner, AttachParent);

	UpdateRenderDataComponent(
		ShapeBarrier, RenderDataBarrier, *RenderMeshComponent, false, false, RelTransformOverride);

	return RenderMeshComponent;
}

UStaticMeshComponent* FAGX_SimObjectsImporterHelper::InstantiateRenderDataInBodyOrRoot(
	const FTrimeshShapeBarrier& TrimeshBarrier, AActor& Owner, const FRigidBodyBarrier* Body)
{
	USceneComponent* AttachParent = [&]() -> USceneComponent*
	{
		if (Body != nullptr)
		{
			if (auto BodyComponent = GetBody(*Body))
			{
				return BodyComponent;
			}
		}

		return Owner.GetRootComponent();
	}();

	FTransform RenderDataTransform = FTransform::Identity;
	{
		FVector TrimeshPosition;
		FQuat TrimeshRotation;
		std::tie(TrimeshPosition, TrimeshRotation) = TrimeshBarrier.GetLocalPositionAndRotation();
		const FTransform TrimeshTransform(TrimeshRotation, TrimeshPosition);
		const FTransform ShapeToGeometry = TrimeshBarrier.GetGeometryToShapeTransform().Inverse();
		FTransform::Multiply(&RenderDataTransform, &ShapeToGeometry, &TrimeshTransform);
	}

	return InstantiateRenderData(TrimeshBarrier, Owner, *AttachParent, &RenderDataTransform);
}

void FAGX_SimObjectsImporterHelper::UpdateRenderDataComponent(
	const FShapeBarrier& ShapeBarrier, const FRenderDataBarrier& RenderDataBarrier,
	UStaticMeshComponent& Component, bool ForceOverwritePropertiesInInstances,
	bool ForceReassignRenderMaterialInInstances, FTransform* RelTransformOverride)
{
	AGX_CHECK(RenderDataBarrier.HasMesh());

	// The triangles in the AGX Dynamics render data are relative to the Geometry, but the
	// Unreal Engine Component we create is placed at the position of the AGX Dynamics
	// Shape. There is no Component for the Geometry. To get the triangles in the right
	// place we need to offset the render data Component by the inverse of the
	// Geometry-to-Shape transformation in the source AGX Dynamics data.
	//
	// In AGX Dynamics:
	//
	//   Geometry           Shape       A
	//   origin             origin   triangle
	//     X                  O         |
	//     '-----------28-------------->'
	//           The vertex position
	//     '--------18------->'
	//      The Shape position
	//      relative to the
	//      geometry
	//
	//
	// In Unreal Engine:
	//
	//                      Shape       A
	//                      origin   triangle
	//                        O         |                  |
	//                        '-------------28------------>'
	//                         Where the triangle would end
	//                         up if the vertex position is
	//                         used as-is.
	//                                  '<------18---------'
	//                                   The inverse of the
	//                                   Geometry-to-Shape
	//                                   transformation.
	FTransform NewRelTransform = RelTransformOverride != nullptr
									 ? *RelTransformOverride
									 : ShapeBarrier.GetGeometryToShapeTransform().Inverse();

	UMaterialInterface* OriginalRenderMaterial = Component.GetMaterial(0);

	UMaterialInterface* NewRenderMaterial = nullptr;
	if (RenderDataBarrier.HasMaterial())
	{
		FAGX_RenderMaterial RMBarrier = RenderDataBarrier.GetMaterial();
		const FGuid RMGuid = RMBarrier.Guid;
		NewRenderMaterial = ProcessedRenderMaterials.FindRef(RMGuid);
		if (NewRenderMaterial == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Expected to find processed render material with GUID '%s' from Shape '%s' "
					 "but it was not avaiable. The render material will not be set."),
				*RMGuid.ToString(), *ShapeBarrier.GetName());
		}
	}
	else
	{
		NewRenderMaterial = GetDefaultRenderMaterial(ShapeBarrier.GetIsSensor());
	}

	UStaticMesh* OriginalMeshAsset = Component.GetStaticMesh();
	UStaticMesh* NewMeshAsset = nullptr;
	if (IsMeshEquivalent(RenderDataBarrier, OriginalMeshAsset))
	{
		NewMeshAsset = OriginalMeshAsset;
	}
	else
	{
		UStaticMesh* Asset = GetOrCreateStaticMeshAsset(
			RenderDataBarrier, ProcessedMeshes, ProcessedRenderMaterials, RootDirectoryPath);
		NewMeshAsset = Asset;
	}

	FAGX_ImportUtilities::Rename(
		Component, FString("RenderMesh_") + RenderDataBarrier.GetGuid().ToString());

	const bool Visible = RenderDataBarrier.GetShouldRender();
	FAGX_BlueprintUtilities::SetTemplateComponentRelativeTransform(Component, NewRelTransform);

	if (FAGX_ObjectUtilities::IsTemplateComponent(Component))
	{
		// Sync all component instances.
		for (UStaticMeshComponent* Instance :
			 FAGX_ObjectUtilities::GetArchetypeInstances(Component))
		{
			// Update Mesh asset.
			if (ForceOverwritePropertiesInInstances ||
				Instance->GetStaticMesh() == Component.GetStaticMesh())
			{
				Instance->SetStaticMesh(NewMeshAsset);
			}

			// Update Render Materials.
			if (ForceReassignRenderMaterialInInstances ||
				Instance->GetMaterial(0) == OriginalRenderMaterial)
			{
				Instance->SetMaterial(0, NewRenderMaterial);
			}

			// Update visibility.
			if (ForceOverwritePropertiesInInstances ||
				Instance->GetVisibleFlag() == Component.GetVisibleFlag())
			{
				Instance->SetVisibility(Visible);
			}
		}
	}

	Component.SetStaticMesh(NewMeshAsset);
	Component.SetMaterial(0, NewRenderMaterial);
	Component.SetVisibility(Visible);

	const FGuid ShapeGuid = ShapeBarrier.GetShapeGuid();
	if (!ProcessedRenderStaticMeshComponents.Contains(ShapeGuid))
	{
		ProcessedRenderStaticMeshComponents.Add(ShapeGuid, &Component);
	}
}

void FAGX_SimObjectsImporterHelper::UpdateAndSaveShapeMaterialAsset(
	const FShapeMaterialBarrier& Barrier, UAGX_ShapeMaterial& Asset)
{
	Asset.CopyFrom(Barrier);
	FAGX_EditorUtilities::RenameAsset(Asset, Barrier.GetName(), "ShapeMaterial");
	FAGX_ObjectUtilities::SaveAsset(Asset);

	ProcessedShapeMaterials.Add(Barrier.GetGuid(), &Asset);
}

UAGX_ShapeMaterial* FAGX_SimObjectsImporterHelper::InstantiateShapeMaterial(
	const FShapeMaterialBarrier& Barrier)
{
	UAGX_ShapeMaterial* Asset = FAGX_ImportUtilities::CreateAsset<UAGX_ShapeMaterial>(
		RootDirectoryPath, Barrier.GetName(),
		FAGX_ImportUtilities::GetImportShapeMaterialDirectoryName());
	if (Asset == nullptr)
	{
		WriteImportErrorMessage(
			TEXT("AGX Dynamics Shape Material"), Barrier.GetName(), SourceFilePath,
			TEXT("Could not create a Shape Material Asset from given ShapeMaterialBarrier."));
		return nullptr;
	}

	UpdateAndSaveShapeMaterialAsset(Barrier, *Asset);
	return Asset;
}

UMaterialInterface* FAGX_SimObjectsImporterHelper::InstantiateRenderMaterial(
	const FAGX_RenderMaterial& Barrier)
{
	if (UMaterialInstanceConstant* ProcessedMat = ProcessedRenderMaterials.FindRef(Barrier.Guid))
	{
		// This Render Material has already been processed before, simply return it.
		return ProcessedMat;
	}

	UMaterial* Base = LoadObject<UMaterial>(
		nullptr, TEXT("Material'/AGXUnreal/Runtime/Materials/M_ImportedBase.M_ImportedBase'"));
	if (Base == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Could not load parent material for imported AGX Dynamics render materials."));
		return nullptr;
	}
	UMaterialInstanceConstantFactoryNew* Factory = NewObject<UMaterialInstanceConstantFactoryNew>();
	Factory->InitialParent = Base;

	const FGuid Guid = Barrier.Guid;
	const FString MaterialName =
		Barrier.Name.IsNone() ? FString::Printf(TEXT("MI_RenderMaterial_%s"), *Guid.ToString())
							  : FString::Printf(TEXT("MI_%s"), *Barrier.Name.ToString());

	FString AssetName = FAGX_ImportUtilities::CreateAssetName(
		MaterialName, TEXT("ImportedAGXDynamicsMaterial"),
		FAGX_ImportUtilities::GetImportRenderMaterialDirectoryName());

	FString PackagePath = FAGX_ImportUtilities::CreatePackagePath(
		RootDirectoryPath, FAGX_ImportUtilities::GetImportRenderMaterialDirectoryName());

	IAssetTools& AssetTools =
		FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	AssetTools.CreateUniqueAssetName(PackagePath, AssetName, PackagePath, AssetName);
	UObject* Asset = AssetTools.CreateAsset(
		AssetName, FPackageName::GetLongPackagePath(PackagePath),
		UMaterialInstanceConstant::StaticClass(), Factory);
	if (Asset == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Could not create new Material asset for material '%s' imported from '%s'."),
			*MaterialName, *RootDirectoryPath);
		return nullptr;
	}

	UMaterialInstanceConstant* Material = Cast<UMaterialInstanceConstant>(Asset);
	if (Material == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Could not create new Material Instance Constant for material '%s' imported from "
				 "'%s'."),
			*MaterialName, *RootDirectoryPath)
		return nullptr;
	}

	UpdateAndSaveRenderMaterialAsset(Barrier, *Material);
	return Material;
}

void FAGX_SimObjectsImporterHelper::UpdateAndSaveRenderMaterialAsset(
	const FAGX_RenderMaterial& Material, UMaterialInstanceConstant& Asset)
{
	auto SetVector = [&Asset](const TCHAR* Name, const FVector4& Value)
	{
		Asset.SetVectorParameterValueEditorOnly(
			FName(Name), FAGX_RenderMaterial::ConvertToLinear(Value));
	};

	auto SetScalar = [&Asset](const TCHAR* Name, float Value)
	{ Asset.SetScalarParameterValueEditorOnly(FName(Name), Value); };

	Asset.ClearParameterValuesEditorOnly();
	if (Material.bHasDiffuse)
	{
		SetVector(TEXT("Diffuse"), Material.Diffuse);
	}
	if (Material.bHasAmbient)
	{
		SetVector(TEXT("Ambient"), Material.Ambient);
	}
	if (Material.bHasEmissive)
	{
		SetVector(TEXT("Emissive"), Material.Emissive);
	}
	if (Material.bHasShininess)
	{
		SetScalar(TEXT("Shininess"), Material.Shininess);
	}

	Asset.PostEditChange();
	FAGX_ObjectUtilities::SaveAsset(Asset);

	ProcessedRenderMaterials.Add(Material.Guid, &Asset);
}

void FAGX_SimObjectsImporterHelper::UpdateAndSaveContactMaterialAsset(
	const FContactMaterialBarrier& Barrier, UAGX_ContactMaterial& Asset,
	UAGX_ContactMaterialRegistrarComponent& CMRegistrar)
{
	FShapeMaterialPair Materials = GetShapeMaterials(Barrier);
	Asset.CopyFrom(Barrier);
	Asset.Material1 = Materials.first;
	Asset.Material2 = Materials.second;

	const FString Name =
		TEXT("CM_") + GetName(Materials.first) + TEXT("_") + GetName(Materials.second);
	FAGX_EditorUtilities::RenameAsset(Asset, Name, "ContactMaterial");
	FAGX_ObjectUtilities::SaveAsset(Asset);

	CMRegistrar.ContactMaterials.AddUnique(&Asset);
	if (FAGX_ObjectUtilities::IsTemplateComponent(CMRegistrar))
	{
		for (UAGX_ContactMaterialRegistrarComponent* Instance :
			 FAGX_ObjectUtilities::GetArchetypeInstances(CMRegistrar))
		{
			Instance->ContactMaterials.AddUnique(&Asset);
		}
	}
}

UAGX_ContactMaterial* FAGX_SimObjectsImporterHelper::InstantiateContactMaterial(
	const FContactMaterialBarrier& Barrier, UAGX_ContactMaterialRegistrarComponent& CMRegistrar)
{
	FShapeMaterialPair Materials = GetShapeMaterials(Barrier);
	const FString Name =
		TEXT("CM_") + GetName(Materials.first) + TEXT("_") + GetName(Materials.second);
	UAGX_ContactMaterial* Asset = FAGX_ImportUtilities::CreateAsset<UAGX_ContactMaterial>(
		RootDirectoryPath, Name, FAGX_ImportUtilities::GetImportContactMaterialDirectoryName());
	if (Asset == nullptr)
	{
		WriteImportErrorMessage(
			TEXT("AGX Dynamics Contact Material"), Name, SourceFilePath,
			TEXT("Could not create a Contact Material Asset from given ContactMaterialBarrier."));
		return nullptr;
	}

	UpdateAndSaveContactMaterialAsset(Barrier, *Asset, CMRegistrar);
	return Asset;
}

UAGX_ContactMaterialRegistrarComponent*
FAGX_SimObjectsImporterHelper::InstantiateContactMaterialRegistrar(AActor& Owner)
{
	const FString CMRName = FAGX_ImportUtilities::GetContactMaterialRegistrarDefaultName();
	UAGX_ContactMaterialRegistrarComponent* Component =
		NewObject<UAGX_ContactMaterialRegistrarComponent>(&Owner, *CMRName);

	Component->SetFlags(RF_Transactional);
	Owner.AddInstanceComponent(Component);
	Component->RegisterComponent();
	return Component;
}

namespace
{
	// This function does not update Constraint Controllers and should not be called in isolation
	// for 1 or 2 DOF Constraints. Instead, one of the UpdateConstraintNDofComponent functions
	// should be called to completely update those.
	void UpdateConstraintComponentNoControllers(
		UAGX_ConstraintComponent& Constraint, const FConstraintBarrier& Barrier,
		FAGX_SimObjectsImporterHelper& Helper,
		TMap<FGuid, UAGX_MergeSplitThresholdsBase*>& ProcessedThresholds,
		const TMap<FGuid, UAGX_MergeSplitThresholdsBase*>& MSTsOnDisk, bool ForceOverwriteInstances)
	{
		FAGX_SimObjectsImporterHelper::FBodyPair Bodies = Helper.GetBodies(Barrier);
		if (Bodies.first == nullptr)
		{
			// Not having a second body is fine, means that the first body is constrained to the
			// world. Not having a first body is unexpected.
			UE_LOG(
				LogAGX, Warning,
				TEXT("Constraint '%s' imported from '%s' does not have a first body."),
				*Barrier.GetName(), *Helper.SourceFilePath);
		}

		auto ToFName = [](const UAGX_RigidBodyComponent& Body)
		{
			if (FAGX_ObjectUtilities::IsTemplateComponent(Body))
			{
				return FName(FAGX_BlueprintUtilities::GetRegularNameFromTemplateComponentName(
					Body.GetName()));
			}
			return Body.GetFName();
		};

		const FConstraintMergeSplitThresholdsBarrier ThresholdsBarrier =
			FConstraintMergeSplitThresholdsBarrier::CreateFrom(Barrier);

		UAGX_MergeSplitThresholdsBase* MSThresholds = nullptr;
		if (ThresholdsBarrier.HasNative())
		{
			const FGuid MSTGuid = ThresholdsBarrier.GetGuid();
			MSThresholds = MSTsOnDisk.FindRef(MSTGuid);
			if (MSThresholds != nullptr)
			{
				::UpdateAndSaveMergeSplitThresholdsAsset(
					ThresholdsBarrier, *MSThresholds, ProcessedThresholds,
					EAGX_AmorOwningType::Constraint);
			}
			else
			{
				MSThresholds = ::GetOrCreateMergeSplitThresholdsAsset<
					FConstraintBarrier, FConstraintMergeSplitThresholdsBarrier>(
					Barrier, EAGX_AmorOwningType::Constraint, ProcessedThresholds,
					Helper.RootDirectoryPath);
			}
		}

		const TOptional<FName> BodyName1 =
			Bodies.first != nullptr ? ToFName(*Bodies.first) : TOptional<FName>();
		const TOptional<FName> BodyName2 =
			Bodies.second != nullptr ? ToFName(*Bodies.second) : TOptional<FName>();

		for (auto Instance : FAGX_ObjectUtilities::GetArchetypeInstances(Constraint))
		{
			if (BodyName1.IsSet() &&
				(ForceOverwriteInstances || Instance->BodyAttachment1.RigidBody.Name ==
												Constraint.BodyAttachment1.RigidBody.Name))
			{
				Instance->BodyAttachment1.RigidBody.Name = BodyName1.GetValue();
			}

			if (BodyName2.IsSet() &&
				(ForceOverwriteInstances || Instance->BodyAttachment2.RigidBody.Name ==
												Constraint.BodyAttachment2.RigidBody.Name))
			{
				Instance->BodyAttachment2.RigidBody.Name = BodyName2.GetValue();
			}

			if (ForceOverwriteInstances || Instance->MergeSplitProperties.Thresholds ==
											   Constraint.MergeSplitProperties.Thresholds)
			{
				Instance->MergeSplitProperties.Thresholds =
					Cast<UAGX_ConstraintMergeSplitThresholds>(MSThresholds);
			}
		}

		if (BodyName1.IsSet())
		{
			Constraint.BodyAttachment1.RigidBody.Name = BodyName1.GetValue();
		}
		if (BodyName2.IsSet())
		{
			Constraint.BodyAttachment2.RigidBody.Name = BodyName2.GetValue();
		}

		FAGX_ImportUtilities::Rename(Constraint, Barrier.GetName());
		Constraint.CopyFrom(Barrier, ForceOverwriteInstances);
		const FTransform NewWorldTransform =
			FAGX_ConstraintUtilities::SetupConstraintAsFrameDefiningSource(
				Barrier, Constraint, Bodies.first, Bodies.second);
		if (FAGX_ObjectUtilities::IsTemplateComponent(Constraint))
		{
			FAGX_BlueprintUtilities::SetTemplateComponentWorldTransform(
				&Constraint, NewWorldTransform, true, ForceOverwriteInstances);
		}

		Constraint.MergeSplitProperties.Thresholds =
			Cast<UAGX_ConstraintMergeSplitThresholds>(MSThresholds);
	}

	void UpdateConstraint1DofComponent(
		UAGX_Constraint1DofComponent& Constraint, const FConstraintBarrier& Barrier,
		FAGX_SimObjectsImporterHelper& Helper,
		TMap<FGuid, UAGX_MergeSplitThresholdsBase*>& ProcessedThresholds,
		const TMap<FGuid, UAGX_MergeSplitThresholdsBase*>& MSTsOnDisk, bool ForceOverwriteInstances)
	{
		UpdateConstraintComponentNoControllers(
			Constraint, Barrier, Helper, ProcessedThresholds, MSTsOnDisk, ForceOverwriteInstances);
		FAGX_ConstraintUtilities::CopyControllersFrom(
			Constraint, *static_cast<const FConstraint1DOFBarrier*>(&Barrier),
			ForceOverwriteInstances);
	}

	void UpdateConstraint2DofComponent(
		UAGX_Constraint2DofComponent& Constraint, const FConstraintBarrier& Barrier,
		FAGX_SimObjectsImporterHelper& Helper,
		TMap<FGuid, UAGX_MergeSplitThresholdsBase*>& ProcessedThresholds,
		const TMap<FGuid, UAGX_MergeSplitThresholdsBase*>& MSTsOnDisk, bool ForceOverwriteInstances)
	{
		UpdateConstraintComponentNoControllers(
			Constraint, Barrier, Helper, ProcessedThresholds, MSTsOnDisk, ForceOverwriteInstances);
		FAGX_ConstraintUtilities::CopyControllersFrom(
			Constraint, *static_cast<const FConstraint2DOFBarrier*>(&Barrier),
			ForceOverwriteInstances);
	}
}

UAGX_HingeConstraintComponent* FAGX_SimObjectsImporterHelper::InstantiateHinge(
	const FHingeBarrier& Barrier, AActor& Owner)
{
	UAGX_HingeConstraintComponent* Constraint = NewObject<UAGX_HingeConstraintComponent>(&Owner);
	const TMap<FGuid, UAGX_MergeSplitThresholdsBase*> Unused;
	UpdateConstraint1DofComponent(*Constraint, Barrier, *this, ProcessedThresholds, Unused, false);
	Constraint->SetFlags(RF_Transactional);
	Owner.AddInstanceComponent(Constraint);
	Constraint->RegisterComponent();
	return Constraint;
}

UAGX_PrismaticConstraintComponent* FAGX_SimObjectsImporterHelper::InstantiatePrismatic(
	const FPrismaticBarrier& Barrier, AActor& Owner)
{
	UAGX_PrismaticConstraintComponent* Constraint =
		NewObject<UAGX_PrismaticConstraintComponent>(&Owner);
	const TMap<FGuid, UAGX_MergeSplitThresholdsBase*> Unused;
	UpdateConstraint1DofComponent(*Constraint, Barrier, *this, ProcessedThresholds, Unused, false);
	Constraint->SetFlags(RF_Transactional);
	Owner.AddInstanceComponent(Constraint);
	Constraint->RegisterComponent();
	return Constraint;
}

UAGX_BallConstraintComponent* FAGX_SimObjectsImporterHelper::InstantiateBallConstraint(
	const FBallJointBarrier& Barrier, AActor& Owner)
{
	UAGX_BallConstraintComponent* Constraint = NewObject<UAGX_BallConstraintComponent>(&Owner);
	const TMap<FGuid, UAGX_MergeSplitThresholdsBase*> Unused;
	UpdateConstraintComponentNoControllers(
		*Constraint, Barrier, *this, ProcessedThresholds, Unused, false);
	FAGX_ConstraintUtilities::CopyControllersFrom(*Constraint, Barrier, false);
	Constraint->SetFlags(RF_Transactional);
	Owner.AddInstanceComponent(Constraint);
	Constraint->RegisterComponent();
	return Constraint;
}

UAGX_CylindricalConstraintComponent*
FAGX_SimObjectsImporterHelper::InstantiateCylindricalConstraint(
	const FCylindricalJointBarrier& Barrier, AActor& Owner)
{
	UAGX_CylindricalConstraintComponent* Constraint =
		NewObject<UAGX_CylindricalConstraintComponent>(&Owner);
	const TMap<FGuid, UAGX_MergeSplitThresholdsBase*> Unused;
	UpdateConstraint2DofComponent(*Constraint, Barrier, *this, ProcessedThresholds, Unused, false);
	Constraint->SetFlags(RF_Transactional);
	Owner.AddInstanceComponent(Constraint);
	Constraint->RegisterComponent();
	return Constraint;
}

UAGX_DistanceConstraintComponent* FAGX_SimObjectsImporterHelper::InstantiateDistanceConstraint(
	const FDistanceJointBarrier& Barrier, AActor& Owner)
{
	UAGX_DistanceConstraintComponent* Constraint =
		NewObject<UAGX_DistanceConstraintComponent>(&Owner);
	const TMap<FGuid, UAGX_MergeSplitThresholdsBase*> Unused;
	UpdateConstraint1DofComponent(*Constraint, Barrier, *this, ProcessedThresholds, Unused, false);
	Constraint->SetFlags(RF_Transactional);
	Owner.AddInstanceComponent(Constraint);
	Constraint->RegisterComponent();
	return Constraint;
}

UAGX_LockConstraintComponent* FAGX_SimObjectsImporterHelper::InstantiateLockConstraint(
	const FLockJointBarrier& Barrier, AActor& Owner)
{
	UAGX_LockConstraintComponent* Constraint = NewObject<UAGX_LockConstraintComponent>(&Owner);
	const TMap<FGuid, UAGX_MergeSplitThresholdsBase*> Unused;
	UpdateConstraintComponentNoControllers(
		*Constraint, Barrier, *this, ProcessedThresholds, Unused, false);
	Constraint->SetFlags(RF_Transactional);
	Owner.AddInstanceComponent(Constraint);
	Constraint->RegisterComponent();
	return Constraint;
}

// TODO Consider making this a virtual member function on UAGX_Constraint.
void FAGX_SimObjectsImporterHelper::UpdateConstraintComponent(
	const FConstraintBarrier& Barrier, UAGX_ConstraintComponent& Component,
	const TMap<FGuid, UAGX_MergeSplitThresholdsBase*>& MSTsOnDisk, bool ForceOverwriteInstances)
{
	if (UAGX_Constraint1DofComponent* Constraint1Dof =
			Cast<UAGX_Constraint1DofComponent>(&Component))
	{
		UpdateConstraint1DofComponent(
			*Constraint1Dof, Barrier, *this, ProcessedThresholds, MSTsOnDisk,
			ForceOverwriteInstances);
	}
	else if (
		UAGX_Constraint2DofComponent* Constraint2Dof =
			Cast<UAGX_Constraint2DofComponent>(&Component))
	{
		UpdateConstraint2DofComponent(
			*Constraint2Dof, Barrier, *this, ProcessedThresholds, MSTsOnDisk,
			ForceOverwriteInstances);
	}
	else
	{
		UpdateConstraintComponentNoControllers(
			Component, Barrier, *this, ProcessedThresholds, MSTsOnDisk, ForceOverwriteInstances);
	}
}

void FAGX_SimObjectsImporterHelper::UpdateConstraintComponent(
	const FBallJointBarrier& Barrier, UAGX_ConstraintComponent& Component,
	const TMap<FGuid, UAGX_MergeSplitThresholdsBase*>& MSTsOnDisk, bool bForceOverwriteInstances)
{
	UpdateConstraintComponentNoControllers(
		Component, Barrier, *this, ProcessedThresholds, MSTsOnDisk, bForceOverwriteInstances);
	UAGX_BallConstraintComponent* BallConstraint = Cast<UAGX_BallConstraintComponent>(&Component);
	AGX_CHECK(BallConstraint != nullptr);
	if (BallConstraint == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Cannot copy controllers during update of Ball Constraint '%s' because the Ball "
				 "Constraint is null."),
			*Component.GetName());
		return;
	}
	FAGX_ConstraintUtilities::CopyControllersFrom(
		*BallConstraint, Barrier, bForceOverwriteInstances);
}

UAGX_TwoBodyTireComponent* FAGX_SimObjectsImporterHelper::InstantiateTwoBodyTire(
	const FTwoBodyTireBarrier& Barrier, AActor& Owner)
{
	UAGX_TwoBodyTireComponent* Component = NewObject<UAGX_TwoBodyTireComponent>(&Owner);

	UpdateTwoBodyTire(Barrier, *Component, false);

	Component->SetFlags(RF_Transactional);
	Owner.AddInstanceComponent(Component);
	Component->RegisterComponent();
	return Component;
}

void FAGX_SimObjectsImporterHelper::UpdateTwoBodyTire(
	const FTwoBodyTireBarrier& Barrier, UAGX_TwoBodyTireComponent& Component,
	bool ForceOverwriteInstances)
{
	auto SetRigidBody = [&](UAGX_RigidBodyComponent* Body, FAGX_RigidBodyReference& BodyRef)
	{
		if (Body == nullptr)
		{
			WriteImportErrorMessage(
				TEXT("AGX Dynamics TwoBodyTire"), Barrier.GetName(), SourceFilePath,
				TEXT("Could not set Rigid Body"));
			return;
		}

		BodyRef.Name = Body->GetFName();
	};

	// Update any archetype instance.
	for (auto Instance : FAGX_ObjectUtilities::GetArchetypeInstances(Component))
	{
		if (ForceOverwriteInstances || Instance->TireRigidBody.Name == Component.TireRigidBody.Name)
		{
			SetRigidBody(GetBody(Barrier.GetTireRigidBody()), Instance->TireRigidBody);
		}

		if (ForceOverwriteInstances || Instance->HubRigidBody.Name == Component.HubRigidBody.Name)
		{
			SetRigidBody(GetBody(Barrier.GetHubRigidBody()), Instance->HubRigidBody);
		}
	}

	SetRigidBody(GetBody(Barrier.GetTireRigidBody()), Component.TireRigidBody);
	SetRigidBody(GetBody(Barrier.GetHubRigidBody()), Component.HubRigidBody);
	FAGX_ImportUtilities::Rename(Component, Barrier.GetName());
	Component.CopyFrom(Barrier, ForceOverwriteInstances);
}

UAGX_CollisionGroupDisablerComponent*
FAGX_SimObjectsImporterHelper::InstantiateCollisionGroupDisabler(
	AActor& Owner, const TArray<std::pair<FString, FString>>& DisabledPairs)
{
	UAGX_CollisionGroupDisablerComponent* Component =
		NewObject<UAGX_CollisionGroupDisablerComponent>(&Owner);

	UpdateCollisionGroupDisabler(DisabledPairs, *Component);

	Component->SetFlags(RF_Transactional);
	Owner.AddInstanceComponent(Component);
	Component->RegisterComponent();
	return Component;
}

void FAGX_SimObjectsImporterHelper::UpdateCollisionGroupDisabler(
	const TArray<std::pair<FString, FString>>& DisabledPairs,
	UAGX_CollisionGroupDisablerComponent& Component)
{
	const FString CGDName = FAGX_ImportUtilities::GetCollisionGroupDisablerDefaultName();
	FAGX_ImportUtilities::Rename(Component, *CGDName);

	TArray<FAGX_CollisionGroupPair> BarrierPairs;
	BarrierPairs.Reserve(DisabledPairs.Num());
	for (const auto& Pair : DisabledPairs)
	{
		BarrierPairs.Add({FName(std::get<0>(Pair)), FName(std::get<1>(Pair))});
	}

	// Collect pairs to remove (i.e. pairs not present in the DisabledPairs list).
	TArray<FAGX_CollisionGroupPair> CGPairsToRemove;
	for (const auto& Pair : Component.DisabledCollisionGroupPairs)
	{
		if (!Pair.IsIn(BarrierPairs))
		{
			CGPairsToRemove.Add(Pair);
		}
	}

	// Update any archetype instance with the new groups.
	for (UAGX_CollisionGroupDisablerComponent* Instance :
		 FAGX_ObjectUtilities::GetArchetypeInstances(Component))
	{
		for (const auto& PairToRemove : CGPairsToRemove)
		{
			if (Instance->IsCollisionGroupPairDisabled(PairToRemove.Group1, PairToRemove.Group2))
			{
				Instance->EnableCollisionGroupPair(PairToRemove.Group1, PairToRemove.Group2, true);
			}
		}

		for (const std::pair<FString, FString>& DisabledPair : DisabledPairs)
		{
			Instance->DisableCollisionGroupPair(
				FName(*DisabledPair.first), FName(*DisabledPair.second), true);
		}

		Instance->UpdateAvailableCollisionGroupsFromWorld();
	}

	for (const auto& PairToRemove : CGPairsToRemove)
	{
		if (Component.IsCollisionGroupPairDisabled(PairToRemove.Group1, PairToRemove.Group2))
		{
			Component.EnableCollisionGroupPair(PairToRemove.Group1, PairToRemove.Group2, true);
		}
	}

	for (const std::pair<FString, FString>& DisabledPair : DisabledPairs)
	{
		Component.DisableCollisionGroupPair(
			FName(*DisabledPair.first), FName(*DisabledPair.second), true);
	}

	Component.UpdateAvailableCollisionGroupsFromWorld();
}

UAGX_WireComponent* FAGX_SimObjectsImporterHelper::InstantiateWire(
	const FWireBarrier& Barrier, AActor& Owner)
{
	UAGX_WireComponent* Component = NewObject<UAGX_WireComponent>(&Owner);
	if (Component == nullptr)
	{
		WriteImportErrorMessage(
			TEXT("AGX Dynamics Wire"), Barrier.GetName(), SourceFilePath,
			TEXT("Could not create new AGX_WireComponent"));
		return nullptr;
	}

	FAGX_ImportUtilities::Rename(*Component, Barrier.GetName());

	// Copy simple properties such as radius and segment length. More complicated properties, such
	// as physical material, winches and route nodes, are handled below.
	Component->CopyFrom(Barrier);

	// Find and assign the physical material asset.
	FShapeMaterialBarrier NativeMaterial = Barrier.GetMaterial();
	if (NativeMaterial.HasNative())
	{
		const FGuid Guid = NativeMaterial.GetGuid();
		UAGX_ShapeMaterial* Material = ProcessedShapeMaterials.FindRef(Guid);
		Component->ShapeMaterial = Material;
	}

	// Configure winches.
	auto ConfigureWinch = [this, &Barrier](EWireSide Side, UAGX_WireComponent& Wire) -> bool
	{
		FWireWinchBarrier WinchBarrier = Barrier.GetWinch(Side);
		if (WinchBarrier.HasNative())
		{
			FAGX_WireWinch* Winch = Wire.GetOwnedWinch(Side);
			// Get Owned Winch can never return nullptr when a valid Side is passed.
			Winch->CopyFrom(WinchBarrier);
			FRigidBodyBarrier WinchBodyBarrier = WinchBarrier.GetRigidBody();
			UAGX_RigidBodyComponent* WinchBodyComponent = GetBody(WinchBodyBarrier);
			// Ok for WinchBodyComponent to be nullptr. Means attached to the world.
			Winch->SetBodyAttachment(WinchBodyComponent);
			Wire.SetWinchOwnerType(Side, EWireWinchOwnerType::Wire);
			return true;
		}
		else
		{
			Wire.SetWinchOwnerType(Side, EWireWinchOwnerType::None);
			return false;
		}
	};
	bool bHaveBeginWinch = ConfigureWinch(EWireSide::Begin, *Component);
	bool bHaveEndWinch = ConfigureWinch(EWireSide::End, *Component);

	TArray<FWireRoutingNode>& Route = Component->RouteNodes;
	Route.Empty();

	// Helper function to create Body Fixed nodes.
	auto TryCreateBodyFixedNode = [this, &Route](FWireNodeBarrier NodeBarrier)
	{
		if (NodeBarrier.GetType() != EWireNodeType::BodyFixed)
		{
			return;
		}
		FRigidBodyBarrier NodeBodyBarrier = NodeBarrier.GetRigidBody();
		if (!NodeBodyBarrier.HasNative())
		{
			/// @todo Is it OK to have a Body Fixed Node without a body?
			return;
		}
		UAGX_RigidBodyComponent* Body = GetBody(NodeBodyBarrier);
		if (Body == nullptr)
		{
			/// @todo Is it OK to have a Body Fixed Node without a body?
			return;
		}

		if (Route.Num() > 0 && Route[0].NodeType == EWireNodeType::Free)
		{
			// In an initialized wire there may be a Free node right on top of the Body Fixe node.
			// The Body Fixed node represents the body itself, while the Free node represents the
			// part of the wire that approaches the body. While routing we only need the Body Fixed
			// node, it represents both concepts.
			Route.Pop();
		}

		FWireRoutingNode RouteNode;
		RouteNode.NodeType = EWireNodeType::BodyFixed;
		RouteNode.Frame.SetParentComponent(Body);
		RouteNode.Frame.LocalLocation = NodeBarrier.GetTranslate();

		RouteNode.SetBody(Body);
		Route.Add(RouteNode);
	};

	if (!bHaveBeginWinch)
	{
		// Configure initial Body Fixe node. Some Body Fixed nodes are owned by the winch on that
		// side, do not create an explicit Body Fixed node in that case.
		TryCreateBodyFixedNode(Barrier.GetFirstNode());
	}

	// Configure "normal" route nodes.
	for (auto It = Barrier.GetRenderBeginIterator(), End = Barrier.GetRenderEndIterator();
		 It != End; It.Inc())
	{
		const FAGX_WireNode NodeAGX = It.Get();
		const EWireNodeType NodeType = [&NodeAGX, &Barrier, &It]() -> EWireNodeType
		{
			if (Barrier.IsLumpedNode(It))
			{
				// Lumped nodes are special somehow, and should be created as free nodes.
				return EWireNodeType::Free;
			}
			switch (NodeAGX.GetType())
			{
				case EWireNodeType::Free:
					// Free nodes can be routed as-is.
					return EWireNodeType::Free;
				case EWireNodeType::Eye:
					// Eye nodes can be routed as-is.
					return EWireNodeType::Eye;
				case EWireNodeType::BodyFixed:
					// A Body Fixed node found by the render iterator is an implicitly created node
					// and should be routed as a Free node. It should not be attached to any of the
					// Rigid Bodies in the Component list.
					return EWireNodeType::Free;
				case EWireNodeType::Stop:
					// Stop nodes are used by winches, which we detect with GetWinch instead.
					return EWireNodeType::Other;
				default:
					// Any other node type is routed as a Free node for now. Special handling may
					// be needed in the future, if routing with additional node types become
					// supported.
					return EWireNodeType::Free;
			}
		}();

		if (NodeType == EWireNodeType::Other)
		{
			// Other nodes are used to signal "skip this node".
			continue;
		}

		FWireRoutingNode RouteNode;
		RouteNode.NodeType = NodeType;
		if (NodeType == EWireNodeType::Eye)
		{
			// Eye nodes are attached to a Rigid Body, so make Local Location relative to that body.
			FRigidBodyBarrier BodyBarrier = NodeAGX.GetRigidBody();
			UAGX_RigidBodyComponent* BodyComponent = GetBody(BodyBarrier);
			if (BodyComponent != nullptr)
			{
				RouteNode.SetBody(BodyComponent);
				RouteNode.Frame.SetParentComponent(BodyComponent);
				RouteNode.Frame.LocalLocation = NodeAGX.GetLocalLocation();
			}
		}
		else
		{
			// All other node types are placed relative to the Wire Component.
			RouteNode.Frame.SetParentComponent(nullptr);
			RouteNode.Frame.LocalLocation = NodeAGX.GetWorldLocation();
		}

		Route.Add(RouteNode);
	}

	if (!bHaveEndWinch)
	{
		// Configure ending Body Fixed node. Some Body Fixed nodes are owned by the winch on that
		// side, do not create an explicit Body Fixed node in that case.
		TryCreateBodyFixedNode(Barrier.GetLastNode());
	}

	if (auto ThresholdsAsset =
			::GetOrCreateMergeSplitThresholdsAsset<FWireBarrier, FWireMergeSplitThresholdsBarrier>(
				Barrier, EAGX_AmorOwningType::Wire, ProcessedThresholds, RootDirectoryPath))
	{
		Component->MergeSplitProperties.Thresholds =
			Cast<UAGX_WireMergeSplitThresholds>(ThresholdsAsset);
	}

	Component->SetFlags(RF_Transactional);
	Owner.AddInstanceComponent(Component);
	Component->RegisterComponent();
	Component->PostEditChange();
	// May chose to store a table of all imported wires. If so, add this wire to the table here.
	return Component;
}

UAGX_ShovelComponent* FAGX_SimObjectsImporterHelper::InstantiateShovel(
	const FShovelBarrier& ShovelBarrier, AActor& Owner)
{
	UAGX_RigidBodyComponent* BodyComponent = GetBody(ShovelBarrier.GetRigidBody());

	// Shovels don't have names in AGX Dynamics so borrow the body's name. There can only be one
	// shovel per body, at least as of AGX Dynamics 2.36, so this is still unique.
	const FString BaseName =
		BodyComponent != nullptr ? BodyComponent->GetName() : FString("Bodiless");

	UAGX_ShovelComponent* ShovelComponent = NewObject<UAGX_ShovelComponent>(&Owner);
	if (ShovelComponent == nullptr)
	{
		WriteImportErrorMessage(
			TEXT("AGX Dynamics Shovel"), BaseName, SourceFilePath,
			TEXT("Could not create new AGX_ShovelComponent"));
		return nullptr;
	}

	UpdateShovel(ShovelBarrier, *ShovelComponent, BaseName, BodyComponent, false);

	ShovelComponent->SetFlags(RF_Transactional);
	Owner.AddInstanceComponent(ShovelComponent);
	ShovelComponent->RegisterComponent();
	// Component->PostEditChange(); // Some have PostEditChange here, some don't. What's the rule?

	if (BodyComponent != nullptr)
	{
		ShovelComponent->AttachToComponent(
			BodyComponent, FAttachmentTransformRules::KeepRelativeTransform);
	}

	return ShovelComponent;
}

void FAGX_SimObjectsImporterHelper::UpdateShovel(
	const FShovelBarrier& ShovelBarrier, UAGX_ShovelComponent& ShovelComponent,
	const FString& BaseName, const UAGX_RigidBodyComponent* BodyComponent,
	bool ForceOverwriteInstances)
{
	if (ShovelComponent.ShovelProperties == nullptr)
	{
		// We don't try to share equal Shovel Properties assets, every Shovel gets its own instance.
		// This matches AGX Dynamics since its Shovels also can't share properties since the
		// properties are members of the Shove class.
		const FString AssetName = FString::Printf(TEXT("AGX_SP_%s"), *BaseName);
		const FString AssetFolder = FAGX_ImportUtilities::GetImportShovelPropertiesDirectoryName();
		UAGX_ShovelProperties* ShovelProperties =
			FAGX_ImportUtilities::CreateAsset<UAGX_ShovelProperties>(
				RootDirectoryPath, AssetName, AssetFolder);
		if (ShovelProperties == nullptr)
		{
			UE_LOG(
				LogAGX, Warning, TEXT("Unable to create Shovel Properties asset %s."), *AssetName);
			// No return, must complete as much as we can of the setup without the asset.
		}
		else
		{
			ShovelProperties->ImportGuid = ShovelBarrier.GetGuid();
			ShovelComponent.ShovelProperties = ShovelProperties;
		}
	}

	// The edges and the direction in AGX Dynamics are relative to the body, so try to make all
	// frames relative to that. If we don't have a body then the frames will be relative to the
	// shovel instead, which is not correct but the best we can do.
	const FName BodyName = BodyComponent != nullptr ? BodyComponent->GetFName() : NAME_None;
	ShovelComponent.RigidBody.Name = BodyName;
	ShovelComponent.TopEdge.Start.Parent.Name = BodyName;
	ShovelComponent.TopEdge.End.Parent.Name = BodyName;
	ShovelComponent.CuttingEdge.Start.Parent.Name = BodyName;
	ShovelComponent.CuttingEdge.End.Parent.Name = BodyName;
	ShovelComponent.CuttingDirection.Parent.Name = BodyName;

	// Copy the values that have a 1:1 mapping from the Barrier to the Component.
	ShovelComponent.CopyFrom(ShovelBarrier, ForceOverwriteInstances);

	const FString Name = FString::Printf(TEXT("Shovel_%s"), *BaseName);
	FAGX_ImportUtilities::Rename(ShovelComponent, Name);

	if (ShovelComponent.ShovelProperties != nullptr)
	{
		FAGX_ObjectUtilities::SaveAsset(*ShovelComponent.ShovelProperties);
	}
}

UAGX_TrackComponent* FAGX_SimObjectsImporterHelper::InstantiateTrack(
	const FTrackBarrier& Barrier, AActor& Owner)
{
	UAGX_TrackComponent* Component = NewObject<UAGX_TrackComponent>(&Owner);
	if (Component == nullptr)
	{
		WriteImportErrorMessage(
			TEXT("AGX Dynamics Track"), Barrier.GetName(), SourceFilePath,
			TEXT("Could not create new AGX_TrackComponent"));
		return nullptr;
	}

	FAGX_ImportUtilities::Rename(*Component, Barrier.GetName());

	// Copy simple properties such as number of nodes and width. More complicated properties, such
	// as Wheels, TrackProperties asset etc, are handled below.
	Component->CopyFrom(Barrier);

	// Apply Shape Material.
	FShapeMaterialBarrier ShapeMaterial = Barrier.GetMaterial();
	if (ShapeMaterial.HasNative())
	{
		const FGuid Guid = ShapeMaterial.GetGuid();
		UAGX_ShapeMaterial* Material = ProcessedShapeMaterials.FindRef(Guid);
		Component->ShapeMaterial = Material;
	}

	const FString BarrierName = Barrier.GetName();

	// Apply Track Properties.
	{
		const FString AssetName =
			BarrierName.IsEmpty() ? FString("AGX_TP_Track") : FString("AGX_TP_") + BarrierName;

		UAGX_TrackProperties* TrackProperties = GetOrCreateTrackPropertiesAsset(
			Barrier.GetProperties(), AssetName, ProcessedTrackProperties, RootDirectoryPath);
		if (TrackProperties == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Unable to create an Asset for the Track Properties '%s' of Track '%s' during "
					 "import."),
				*AssetName, *Barrier.GetName());
		}
		else
		{
			Component->TrackProperties = TrackProperties;
		}
	}

	// Apply Internal Merge Properties.
	{
		const FString AssetName =
			BarrierName.IsEmpty() ? FString("AGX_TIMP_Track") : FString("AGX_TIMP_") + BarrierName;

		Component->InternalMergeProperties =
			FAGX_ImportUtilities::SaveImportedTrackInternalMergePropertiesAsset(
				Barrier, RootDirectoryPath, AssetName);
	}

	auto SetRigidBody = [&](UAGX_RigidBodyComponent* Body, FAGX_RigidBodyReference& BodyRef)
	{
		if (Body == nullptr)
		{
			WriteImportErrorMessage(
				TEXT("AGX Dynamics Track"), Barrier.GetName(), SourceFilePath,
				TEXT("Could not set Rigid Body"));
			return;
		}

		BodyRef.Name = Body->GetFName();
	};

	// Copy Wheels.
	for (const FTrackWheelBarrier& WheelBarrier : Barrier.GetWheels())
	{
		FAGX_TrackWheel Wheel;
		SetRigidBody(GetBody(WheelBarrier.GetRigidBody()), Wheel.RigidBody);
		Wheel.bUseFrameDefiningComponent = false;
		Wheel.RelativeLocation = WheelBarrier.GetRelativeLocation();
		Wheel.RelativeRotation = WheelBarrier.GetRelativeRotation();
		Wheel.Radius = static_cast<float>(WheelBarrier.GetRadius());
		Wheel.Model = WheelBarrier.GetModel();
		Wheel.bSplitSegments = WheelBarrier.GetSplitSegments();
		Wheel.bMoveNodesToRotationPlane = WheelBarrier.GetMoveNodesToRotationPlane();
		Wheel.bMoveNodesToWheel = WheelBarrier.GetMoveNodesToWheel();
		Component->Wheels.Add(Wheel);
	}

	Component->SetFlags(RF_Transactional);
	Owner.AddInstanceComponent(Component);
	Component->RegisterComponent();
	Component->PostEditChange();
	return Component;
}

void FAGX_SimObjectsImporterHelper::UpdateModelSourceComponent(UAGX_ModelSourceComponent& Component)
{
	// Note: we always overwrite any archetype instances of the Model Source Component. This is
	// because the user should or cannot edit its properties. It is a Component completely managed
	// by AGX Dynamics for Unreal.
	auto UpdateModelSourceComponent = [this](UAGX_ModelSourceComponent* C)
	{
		if (C == nullptr)
		{
			return;
		}

		C->FilePath = SourceFilePath;
		C->bIgnoreDisabledTrimeshes = bIgnoreDisabledTrimeshes;
		C->StaticMeshComponentToOwningTrimesh.Empty();
		for (const auto& ProcessedSMCTuple : ProcessedCollisionStaticMeshComponents)
		{
			const FString Name = ProcessedSMCTuple.Value->GetName();
			AGX_CHECK(!C->StaticMeshComponentToOwningTrimesh.Contains(Name));
			C->StaticMeshComponentToOwningTrimesh.Add(Name, ProcessedSMCTuple.Key);
		}

		C->StaticMeshComponentToOwningShape.Empty();
		for (const auto& [ShapeGuid, RenderComponent] : ProcessedRenderStaticMeshComponents)
		{
			const FString Name = RenderComponent->GetName();
			AGX_CHECK(!C->StaticMeshComponentToOwningShape.Contains(Name));
			C->StaticMeshComponentToOwningShape.Add(Name, ShapeGuid);
		}

		C->UnrealMaterialToImportGuid.Empty(ProcessedRenderMaterials.Num());
		for (const auto& GuidToMaterial : ProcessedRenderMaterials)
		{
			const FGuid ImportGuid = GuidToMaterial.Key;
			const UMaterialInstanceConstant* const Material = GuidToMaterial.Value;
			const FString RelativePath = FAGX_EditorUtilities::GetRelativePath(
				FPaths::GetPath(RootDirectoryPath), Material->GetPathName());

			C->UnrealMaterialToImportGuid.Add(RelativePath, ImportGuid);
		}
	};

	for (UAGX_ModelSourceComponent* Instance :
		 FAGX_ObjectUtilities::GetArchetypeInstances(Component))
	{
		UpdateModelSourceComponent(Instance);
	}

	UpdateModelSourceComponent(&Component);
	FAGX_ImportUtilities::Rename(Component, "AGX_ModelSource");
}

UAGX_ModelSourceComponent* FAGX_SimObjectsImporterHelper::InstantiateModelSourceComponent(
	AActor& Owner)
{
	UAGX_ModelSourceComponent* ModelSourceComponent = NewObject<UAGX_ModelSourceComponent>(&Owner);
	if (ModelSourceComponent == nullptr)
	{
		return nullptr;
	}

	UpdateModelSourceComponent(*ModelSourceComponent);
	ModelSourceComponent->SetFlags(RF_Transactional);
	Owner.AddInstanceComponent(ModelSourceComponent);
	ModelSourceComponent->RegisterComponent();
	ModelSourceComponent->PostEditChange();

	return ModelSourceComponent;
}

UAGX_ObserverFrameComponent* FAGX_SimObjectsImporterHelper::InstantiateObserverFrame(
	const FString& Name, const FGuid& BodyGuid, const FGuid& ObserverGuid,
	const FTransform& Transform, AActor& Owner)
{
	// Get the Rigid Body the imported Observer Frame should be attached to.
	UAGX_RigidBodyComponent* Body = ProcessedBodies.FindRef(BodyGuid);
	if (Body == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT(
				"While importing from '%s': Observer Frame %s is attached to a Rigid Body that "
				"has not been Processed. Cannot create Unreal Engine representation. Tried to find "
				"Rigid Body with GUID %s."),
			*SourceFilePath, *Name, *BodyGuid.ToString());
		return nullptr;
	}

	UAGX_ObserverFrameComponent* Component =
		FAGX_ImportUtilities::CreateComponent<UAGX_ObserverFrameComponent>(Owner, *Body);

	UpdateObserverFrameComponent(Name, ObserverGuid, Transform, *Component, false);

	Component->SetFlags(RF_Transactional);
	Owner.AddInstanceComponent(Component);
	Component->RegisterComponent();

	return Component;
}

void FAGX_SimObjectsImporterHelper::UpdateObserverFrameComponent(
	const FString& Name, const FGuid& ObserverGuid, const FTransform& Transform,
	UAGX_ObserverFrameComponent& Component, bool ForceOverwriteInstances)
{
	FAGX_ImportUtilities::Rename(Component, Name);
	FAGX_BlueprintUtilities::SetTemplateComponentRelativeTransform(
		Component, Transform, true, ForceOverwriteInstances);

	// Update any archetype instances.
	if (FAGX_ObjectUtilities::IsTemplateComponent(Component))
	{
		for (UAGX_ObserverFrameComponent* Instance :
			 FAGX_ObjectUtilities::GetArchetypeInstances(Component))
		{
			// Update Import Guid.
			Instance->ImportGuid = ObserverGuid;
		}
	}

	Component.ImportGuid = ObserverGuid;
}

UPLX_SignalHandlerComponent* FAGX_SimObjectsImporterHelper::InstantiateSignalHandlerComponent(
	AActor& Owner)
{
	UPLX_SignalHandlerComponent* Component = NewObject<UPLX_SignalHandlerComponent>(&Owner);
	if (Component == nullptr)
		return nullptr;

	UpdateSignalHandlerComponent(*Component);
	Component->SetFlags(RF_Transactional);
	Owner.AddInstanceComponent(Component);
	Component->RegisterComponent();
	Component->PostEditChange();

	return Component;
}

void FAGX_SimObjectsImporterHelper::UpdateSignalHandlerComponent(
	UPLX_SignalHandlerComponent& Component)
{
	FAGX_ImportUtilities::Rename(Component, "PLX_SignalHandler");
}

UAGX_RigidBodyComponent* FAGX_SimObjectsImporterHelper::GetBody(
	const FRigidBodyBarrier& Barrier, bool LogErrorIfNotFound)
{
	/// \todo Callers cannot differentiate between a nullptr return because the Barrier really
	/// represents a nullptr body, and a nullptr return because the AGXUnreal representation of an
	/// existing Barrier body couldn't be found. This may cause e.g. constraints that should be
	/// between two bodies to be between a body and the world instead. An error message will be
	/// printed, however, so the user will know what happened, if they read the log.

	if (!Barrier.HasNative())
	{
		// Not an error for constraints. Means that the other body is constrained to the world.
		return nullptr;
	}

	UAGX_RigidBodyComponent* Component = ProcessedBodies.FindRef(Barrier.GetGuid());
	if (Component == nullptr && LogErrorIfNotFound)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("While importing from '%s': A component references a body '%s', but that body "
				 "hasn't been Processed."),
			*SourceFilePath, *Barrier.GetName());
	}

	return Component;
}

FAGX_SimObjectsImporterHelper::FBodyPair FAGX_SimObjectsImporterHelper::GetBodies(
	const FConstraintBarrier& Barrier)
{
	return {GetBody(Barrier.GetFirstBody()), GetBody(Barrier.GetSecondBody())};
}

UAGX_ShapeMaterial* FAGX_SimObjectsImporterHelper::GetShapeMaterial(
	const FShapeMaterialBarrier& Barrier)
{
	return ProcessedShapeMaterials.FindRef(Barrier.GetGuid());
}

FAGX_SimObjectsImporterHelper::FShapeMaterialPair FAGX_SimObjectsImporterHelper::GetShapeMaterials(
	const FContactMaterialBarrier& ContactMaterial)
{
	return {
		GetShapeMaterial(ContactMaterial.GetMaterial1()),
		GetShapeMaterial(ContactMaterial.GetMaterial2())};
}

void FAGX_SimObjectsImporterHelper::FinalizeImport()
{
	// Build mesh assets.
	TArray<UStaticMesh*> StaticMeshes;
	ProcessedMeshes.GenerateValueArray(StaticMeshes);

	// Add a Unreal Simple Collision box to each Mesh. Needed to support Component Overlap Events.
	for (UStaticMesh* Mesh : StaticMeshes)
		if (Mesh != nullptr)
			FAGX_EditorUtilities::AddSimpleCollisionBox(*Mesh);

	FAGX_EditorUtilities::SaveStaticMeshAssetsInBulk(StaticMeshes);
}

namespace
{
	FString MakeModelName(FString SourceFilename)
	{
		return FAGX_EditorUtilities::SanitizeName(
			SourceFilename, FAGX_ImportUtilities::GetImportRootDirectoryName());
	}

	FString MakeDirectoryName(const FString ModelName)
	{
		const FString ImportDirPath =
			FString::Printf(TEXT("/Game/%s/"), *FAGX_ImportUtilities::GetImportRootDirectoryName());
		FString BasePath = FAGX_ImportUtilities::CreatePackagePath(ImportDirPath, ModelName, false);

		auto PackageExists = [&](const FString& DirPath)
		{
			/// @todo Is this check necessary? Can it be something less crashy? It was copied from
			/// somewhere, where?
			check(!FEditorFileUtils::IsMapPackageAsset(DirPath));

			FString DiskPath = FPackageName::LongPackageNameToFilename(DirPath);
			return FPackageName::DoesPackageExist(DirPath) ||
				   FindObject<UPackage>(nullptr, *DirPath) != nullptr ||
				   FPaths::DirectoryExists(DiskPath) || FPaths::FileExists(DiskPath);
		};

		int32 TryCount = 0;
		FString DirectoryPath = BasePath;
		FString DirectoryName = ModelName;
		while (PackageExists(DirectoryPath))
		{
			++TryCount;
			DirectoryPath = BasePath + TEXT("_") + FString::FromInt(TryCount);
			DirectoryName = ModelName + TEXT("_") + FString::FromInt(TryCount);
		}
		UE_LOG(LogAGX, Display, TEXT("Importing model '%s' to '%s'."), *ModelName, *DirectoryPath);
		return DirectoryName;
	}
}

FString GetDefaultImportDirectoryPath(const FString& ValidModelName)
{
	return FPaths::Combine(
		FString("/Game"), FAGX_ImportUtilities::GetImportRootDirectoryName(), ValidModelName);
}

FAGX_SimObjectsImporterHelper::FAGX_SimObjectsImporterHelper(
	const FString& InSourceFilePath, bool bInIgnoreDisabledTrimeshes)
	: SourceFilePath(InSourceFilePath)
	, SourceFileName(FPaths::GetBaseFilename(SourceFilePath))
	, RootDirectoryName(MakeDirectoryName(MakeModelName(SourceFileName)))
	, RootDirectoryPath(GetDefaultImportDirectoryPath(RootDirectoryName))
	, bIgnoreDisabledTrimeshes(bInIgnoreDisabledTrimeshes)
{
}

FAGX_SimObjectsImporterHelper::FAGX_SimObjectsImporterHelper(
	const FString& InSourceFilePath, bool bInIgnoreDisabledTrimeshes,
	const FString& InRootDirectoryPath)
	: SourceFilePath(InSourceFilePath)
	, SourceFileName(FPaths::GetBaseFilename(SourceFilePath))
	, RootDirectoryName(FPaths::GetBaseFilename(InRootDirectoryPath))
	, RootDirectoryPath(InRootDirectoryPath)
	, bIgnoreDisabledTrimeshes(bInIgnoreDisabledTrimeshes)
{
}
