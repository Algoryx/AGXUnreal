#include "AGX_SimObjectsImporterHelper.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"
#include "RigidBodyBarrier.h"
#include "Constraints/AGX_Constraint1DofComponent.h"
#include "Constraints/AGX_Constraint2DofComponent.h"
#include "Constraints/AGX_BallConstraintComponent.h"
#include "Constraints/AGX_CylindricalConstraintComponent.h"
#include "Constraints/AGX_DistanceConstraintComponent.h"
#include "Constraints/AGX_HingeConstraintComponent.h"
#include "Constraints/AGX_LockConstraintComponent.h"
#include "Constraints/AGX_PrismaticConstraintComponent.h"
#include "Constraints/ConstraintBarrier.h"
#include "Constraints/Constraint1DOFBarrier.h"
#include "Constraints/Constraint2DOFBarrier.h"
#include "Constraints/BallJointBarrier.h"
#include "Constraints/CylindricalJointBarrier.h"
#include "Constraints/DistanceJointBarrier.h"
#include "Constraints/HingeBarrier.h"
#include "Constraints/LockJointBarrier.h"
#include "Constraints/PrismaticBarrier.h"
#include "Shapes/AGX_SphereShapeComponent.h"
#include "Shapes/AGX_BoxShapeComponent.h"
#include "Shapes/AGX_CylinderShapeComponent.h"
#include "Shapes/AGX_CapsuleShapeComponent.h"
#include "Shapes/AGX_TrimeshShapeComponent.h"
#include "Shapes/RenderDataBarrier.h"
#include "Materials/AGX_ContactMaterialAsset.h"
#include "Materials/AGX_ContactMaterialRegistrarComponent.h"
#include "Materials/AGX_ShapeMaterialAsset.h"
#include "Materials/ContactMaterialBarrier.h"
#include "Materials/ShapeMaterialBarrier.h"
#include "Tires/TwoBodyTireBarrier.h"
#include "Tires/AGX_TwoBodyTireComponent.h"
#include "CollisionGroups/AGX_CollisionGroupDisablerComponent.h"
#include "Utilities/AGX_ImportUtilities.h"
#include "Utilities/AGX_ConstraintUtilities.h"
#include "Utilities/AGX_TextureUtilities.h"
#include "Wire/AGX_WireComponent.h"

// Unreal Engine includes.
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "FileHelpers.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Misc/Paths.h"
#include "UObject/UObjectGlobals.h"

namespace
{
	void WriteImportErrorMessage(
		const TCHAR* ObjectType, const FString& Name, const FString& FilePath,
		const TCHAR* Message)
	{
		UE_LOG(
			LogAGX, Error, TEXT("Could not import '%s' '%s' from file '%s': %s."), ObjectType, *Name,
			*FilePath, Message);
	}
};

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
	FAGX_ImportUtilities::Rename(*Component, Barrier.GetName());
	Component->CopyFrom(Barrier);
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
	RestoredBodies.Add(Barrier.GetGuid(), Component);
	return Component;
}

namespace
{
	/**
	 * Convert an AGX Dynamics Render Material to an Unreal Engine Render Material and store it
	 * as an asset in the given directory. Will cache and reuse Render Materials if the same one
	 * is passed multiple times. Will fall back to the base import material if asset creation
	 * fails. Will return nullptr if the base import material can't be loaded.
	 *
	 * If a new Render Material is created then it is created as a Material Instance Constant
	 * from the base import material.
	 *
	 * @param RenderMaterial The AGX Dynamics Material to convert to an Unreal Engine Material.
	 * @param DirectoryName The name of the directory where this imported model's assets are stored.
	 * @param RestoredMaterials Cache of restored Render Materials.
	 * @return The Unreal Engine material for the AGX Dynamics material, or the base material, or
	 * nullptr.
	 */
	UMaterialInterface* GetOrCreateRenderMaterialInstance(
		const FAGX_RenderMaterial& RenderMaterial, const FString& DirectoryName,
		TMap<FGuid, UMaterialInstanceConstant*>& RestoredMaterials)
	{
		const FGuid Guid = RenderMaterial.Guid;

		// Have we seen this render material before?
		if (UMaterialInstanceConstant** It = RestoredMaterials.Find(Guid))
		{
			// Yes, used the cached Material Instance.
			return *It;
		}

		// This is a new material. Save it as an asset and in the cache.
		const FString MaterialName =
			RenderMaterial.Name.IsNone()
				? FString::Printf(TEXT("RenderMaterial_%s"), *Guid.ToString())
				: RenderMaterial.Name.ToString();
		UMaterialInterface* Material = FAGX_ImportUtilities::SaveImportedRenderMaterialAsset(
			RenderMaterial, DirectoryName, MaterialName);
		if (Material == nullptr)
		{
			// Both asset creation and default material load failed. That's bad.
			return nullptr;
		}

		// Check if we got a new Material Instance, or if we fell back to the default material.
		if (UMaterialInstanceConstant* Instance = Cast<UMaterialInstanceConstant>(Material))
		{
			// This is a new Material Instance, store it in the cache.
			RestoredMaterials.Add(Guid, Instance);
		}

		return Material;
	}

	UMaterial* GetDefaultRenderMaterial(bool bIsSensor)
	{
		const TCHAR* AssetPath =
			bIsSensor
				? TEXT("Material'/AGXUnreal/Runtime/Materials/M_SensorMaterial.M_SensorMaterial'")
				: TEXT("Material'/AGXUnreal/Runtime/Materials/M_ImportedBase.M_ImportedBase'");
		UMaterial* Material = FAGX_TextureUtilities::GetMaterialFromAssetPath(AssetPath);
		if (Material == nullptr)
		{
			UE_LOG(
				LogAGX, Warning, TEXT("Could not load default%s render material from '%s'."),
				(bIsSensor ? TEXT(" sensor") : TEXT("")), AssetPath);
		}
		return Material;
	}

	void SetDefaultRenderMaterial(UMeshComponent& Component, bool bIsSensor)
	{
		UMaterial* Material = GetDefaultRenderMaterial(bIsSensor);
		if (Material == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Could not set render material on imported shape '%s'. Could not load the "
					 "default render material"),
				*Component.GetName());
			return;
		}
		Component.SetMaterial(0, Material);
	}

	/**
	 * Convert the given Trimesh to an Unreal Engine Static Mesh asset stored in the StaticMeshes
	 * folder in the imported model's folder in the ImportedAGXModels folder.
	 *
	 * The created meshes are cached on the Trimesh's Mesh Data GUID so asking for the same mesh
	 * again will return the previously created Static Mesh asset.
	 *
	 * @param Trimesh The Trimesh containing the mesh to store.
	 * @param FallbackName A name to give the asset in case the Trimesh doesn't have a valid name.
	 * @param RestoredMeshes Static Mesh cache.
	 * @param DirectoryName The name of the folder where all assets for this imported model is stored.
	 * @return
	 */
	UStaticMesh* GetOrCreateStaticMeshAsset(
		const FTrimeshShapeBarrier& Trimesh, const FString& FallbackName,
		TMap<FGuid, UStaticMesh*>& RestoredMeshes, const FString& DirectoryName)
	{
		const FGuid Guid = Trimesh.GetMeshDataGuid();
		if (!Guid.IsValid())
		{
			// The GUID is invalid, but try to create the mesh asset anyway but without adding it to
			// the RestoredMeshes cache.
			return FAGX_ImportUtilities::SaveImportedStaticMeshAsset(
				Trimesh, DirectoryName, FallbackName);
		}

		if (UStaticMesh* Asset = RestoredMeshes.FindRef(Guid))
		{
			// We have seen this mesh before, use the one in the cache.
			return Asset;
		}

		// This is a new mesh. Create the Static Mesh asset and add to the cache.
		UStaticMesh* Asset =
			FAGX_ImportUtilities::SaveImportedStaticMeshAsset(Trimesh, DirectoryName, FallbackName);
		if (Asset != nullptr)
		{
			RestoredMeshes.Add(Guid, Asset);
		}
		return Asset;
	}

	/**
	 * Convert the given Render Data to an Unreal Engine Static Mesh asset stored in the
	 * RenderMeshes folder in the imported model's folder in the ImportedAGXModels folder.
	 *
	 * The created meshes are cached on GUID so asking for the same Render Data mesh again will
	 * return the previously created Static Mesh asset.
	 *
	 * @param RenderData The Render Data Barrier containing the mesh to store.
	 * @param RestoredMeshes Static Mesh cache.
	 * @param DirectoryName The name of the folder where all assets for the imported model is stored.
	 * @return The Static Mesh asset for the given Render Data.
	 */
	UStaticMesh* GetOrCreateStaticMeshAsset(
		const FRenderDataBarrier& RenderData, TMap<FGuid, UStaticMesh*>& RestoredMeshes,
		const FString& DirectoryName)
	{
		const FGuid Guid = RenderData.GetGuid();
		if (!Guid.IsValid())
		{
			// The GUID is invalid, but try to create the mesh asset anyway but without adding it to
			// the RestoredMeshes cache.
			return FAGX_ImportUtilities::SaveImportedStaticMeshAsset(RenderData, DirectoryName);
		}

		if (UStaticMesh* Asset = RestoredMeshes.FindRef(Guid))
		{
			// We have seen this mesh before, use the one in the cache.
			return Asset;
		}

		// This is a new mesh. Create the Static Mesh asset and add to the cache.
		UStaticMesh* Asset =
			FAGX_ImportUtilities::SaveImportedStaticMeshAsset(RenderData, DirectoryName);
		if (Asset != nullptr)
		{
			RestoredMeshes.Add(Guid, Asset);
		}
		return Asset;
	}

	/**
	 * Apply the Barrier's Render Data. This will disable visibility for VisualMesh and instead
	 * create a Static Mesh Component from the triangles, if any, in the Render Data. If there are
	 * no triangles then no Static Mesh Component will be created and this Shape will be invisible.
	 *
	 * If the Render Data has a Render Material then that will be converted to an Unreal Engine
	 * Render Material and applied to both VisualMesh and the newly created Static Mesh Component,
	 * if any. This makes it possible to hide the Render Data mesh and instead use the collision
	 * data also for rendering.
	 *
	 * @param RenderData The AGX Dynamics representation of the Render Data.
	 * @param RenderMeshTransform The local transform to give the render mesh Static Mesh Component.
	 * @param Component The Shape Component that is being created.
	 * @param VisualMesh The default Component used for visualization. Often the Shape itself.
	 */
	void ApplyRenderingData(
		const FRenderDataBarrier& RenderData, const FTransform& RenderMeshTransform,
		UAGX_ShapeComponent& Component, UMeshComponent& VisualMesh,
		TMap<FGuid, UStaticMesh*>& RestoredMeshes,
		TMap<FGuid, UMaterialInstanceConstant*>& RestoredMaterials, const FString& DirectoryName)
	{
		VisualMesh.SetVisibility(false);

		// Convert Render Data Mesh, if there is one.
		UStaticMeshComponent* RenderDataComponent = nullptr;
		if (RenderData.HasMesh())
		{
			UStaticMesh* RenderDataMeshAsset =
				GetOrCreateStaticMeshAsset(RenderData, RestoredMeshes, DirectoryName);
			if (RenderDataMeshAsset != nullptr)
			{
				// The new Static Mesh Component must be a child of the Visual Mesh and not the
				// Shape Component because the Trimesh Shape Component assume that it only has a
				// single child Static Mesh Component and will use the first one it finds to read
				// collision triangles from. We do not want it to find the rendering mesh.
				RenderDataComponent = FAGX_EditorUtilities::CreateStaticMeshComponent(
					*Component.GetOwner(), VisualMesh, *RenderDataMeshAsset, true);
				if (RenderDataComponent != nullptr)
				{
					RenderDataComponent->SetVisibility(RenderData.GetShouldRender());
					RenderDataComponent->SetRelativeTransform(RenderMeshTransform);
				}
			}
		}

		// Convert Render Data Material, if there is one. May fall back to the base import Material,
		// and may also fail completely, leaving RenderDataMaterial being nullptr.
		UMaterialInterface* RenderDataMaterial = nullptr;
		if (RenderData.HasMaterial() && GIsEditor)
		{
			RenderDataMaterial = GetOrCreateRenderMaterialInstance(
				RenderData.GetMaterial(), DirectoryName, RestoredMaterials);
		}
		else
		{
			// Use base import material if the Render Data didn't have one. Also use the
			// base when not in the Editor since creating new Materials is an Editor only
			// operation.
			//
			// We are only allowed to create new assets, such as a MaterialInstance, when running
			// within the Unreal Editor.
			/// @todo This is not true. It seems we are allowed to create StaticMeshes. What's the
			/// difference between a StaticMesh and a MaterialInstance? Is it because we create
			/// Constant Material Instances?
			RenderDataMaterial = GetDefaultRenderMaterial(Component.bIsSensor);
		}

		// Apply the Material we got, either from the Render Data or the base one, to all the
		// rendering meshes we have.
		if (RenderDataMaterial != nullptr)
		{
			VisualMesh.SetMaterial(0, RenderDataMaterial);
			if (RenderDataComponent != nullptr)
			{
				RenderDataComponent->SetMaterial(0, RenderDataMaterial);
			}
		}
	}

	/**
	 * Do the configuration that is common for all shape types. This includes setting object flags,
	 * Component renaming, and applying Physics and Rendering Materials.
	 *
	 * @param Component The Unreal Engine representation of the imported Shape.
	 * @param Barrier The AGX Dynamics representation of the imported Shape.
	 * @param RestoredShapeMaterials Cache of imported Shape Materials.
	 * @param RestoredRenderMaterials Cache of imported Render Materials.
	 * @param RestoredMeshes Cache of imported Meshes, both for collision detection and rendering.
	 * @param DirectoryName The name of the directory where all the imported assets are stored.
	 * @param VisualMesh The Mesh Component that contains the default rendering of the Shape
	 */
	void FinalizeShape(
		UAGX_ShapeComponent& Component, const FShapeBarrier& Barrier,
		const TMap<FGuid, UAGX_ShapeMaterialAsset*>& RestoredShapeMaterials,
		TMap<FGuid, UMaterialInstanceConstant*>& RestoredRenderMaterials,
		TMap<FGuid, UStaticMesh*>& RestoredMeshes, const FString& DirectoryName,
		UMeshComponent& VisualMesh)
	{
		Component.UpdateVisualMesh();
		Component.SetFlags(RF_Transactional);
		FAGX_ImportUtilities::Rename(Component, Barrier.GetName());

		FShapeMaterialBarrier NativeMaterial = Barrier.GetMaterial();
		if (NativeMaterial.HasNative())
		{
			const FGuid Guid = NativeMaterial.GetGuid();
			UAGX_ShapeMaterialAsset* Material = RestoredShapeMaterials.FindRef(Guid);
			Component.ShapeMaterial = Material;
		}

		if (Barrier.HasRenderData())
		{
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
			const FTransform ShapeTransform = Barrier.GetGeometryToShapeTransform();
			const FTransform ShapeInvTransform = ShapeTransform.Inverse();
			ApplyRenderingData(
				Barrier.GetRenderData(), ShapeInvTransform, Component, VisualMesh, RestoredMeshes,
				RestoredRenderMaterials, DirectoryName);
		}
		else
		{
			SetDefaultRenderMaterial(VisualMesh, Component.bIsSensor);
		}
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
		Component = NewObject<UAGX_ContactMaterialRegistrarComponent>(
			&Owner, TEXT("AGX_ContactMaterialRegistrar"));

		Component->SetFlags(RF_Transactional);
		Owner.AddInstanceComponent(Component);
		Component->RegisterComponent();
		return Component;
	}
}

UAGX_SphereShapeComponent* FAGX_SimObjectsImporterHelper::InstantiateSphere(
	const FSphereShapeBarrier& Barrier, AActor& Owner, UAGX_RigidBodyComponent* Body)
{
	UAGX_SphereShapeComponent* Component = FAGX_EditorUtilities::CreateSphereShape(&Owner, Body);
	if (Component == nullptr)
	{
		WriteImportErrorMessage(
			TEXT("AGX Dynamics Sphere"), Barrier.GetName(), SourceFilePath,
			TEXT("Could not create new UAGX_SphereShapeComponent"));
		return nullptr;
	}
	Component->CopyFrom(Barrier);
	::FinalizeShape(
		*Component, Barrier, RestoredShapeMaterials, RestoredRenderMaterials, RestoredMeshes,
		DirectoryName, *Component);
	return Component;
}

UAGX_BoxShapeComponent* FAGX_SimObjectsImporterHelper::InstantiateBox(
	const FBoxShapeBarrier& Barrier, AActor& Owner, UAGX_RigidBodyComponent* Body)
{
	UAGX_BoxShapeComponent* Component = FAGX_EditorUtilities::CreateBoxShape(&Owner, Body);
	if (Component == nullptr)
	{
		WriteImportErrorMessage(
			TEXT("AGX Dynamics Box"), Barrier.GetName(), SourceFilePath,
			TEXT("Could not create new UAGX_BoxShapeComponent"));
		return nullptr;
	}
	Component->CopyFrom(Barrier);
	::FinalizeShape(
		*Component, Barrier, RestoredShapeMaterials, RestoredRenderMaterials, RestoredMeshes,
		DirectoryName, *Component);
	return Component;
}

UAGX_CylinderShapeComponent* FAGX_SimObjectsImporterHelper::InstantiateCylinder(
	const FCylinderShapeBarrier& Barrier, AActor& Owner, UAGX_RigidBodyComponent* Body)
{
	UAGX_CylinderShapeComponent* Component =
		FAGX_EditorUtilities::CreateCylinderShape(&Owner, Body);
	if (Component == nullptr)
	{
		WriteImportErrorMessage(
			TEXT("AGX Dynamics Cylinder"), Barrier.GetName(), SourceFilePath,
			TEXT("Could not create new UAGX_CylinderShapeComponent"));
		return nullptr;
	}
	Component->CopyFrom(Barrier);
	::FinalizeShape(
		*Component, Barrier, RestoredShapeMaterials, RestoredRenderMaterials, RestoredMeshes,
		DirectoryName, *Component);
	return Component;
}

UAGX_CapsuleShapeComponent* FAGX_SimObjectsImporterHelper::InstantiateCapsule(
	const FCapsuleShapeBarrier& Barrier, AActor& Owner, UAGX_RigidBodyComponent* Body)
{
	UAGX_CapsuleShapeComponent* Component = FAGX_EditorUtilities::CreateCapsuleShape(&Owner, Body);
	if (Component == nullptr)
	{
		WriteImportErrorMessage(
			TEXT("AGX Dynamics Capsule"), Barrier.GetName(), SourceFilePath,
			TEXT("Could not create new UAGX_CapsuleShapeComponent"));
		return nullptr;
	}
	Component->CopyFrom(Barrier);
	::FinalizeShape(
		*Component, Barrier, RestoredShapeMaterials, RestoredRenderMaterials, RestoredMeshes,
		DirectoryName, *Component);
	return Component;
}

UAGX_TrimeshShapeComponent* FAGX_SimObjectsImporterHelper::InstantiateTrimesh(
	const FTrimeshShapeBarrier& Barrier, AActor& Owner, UAGX_RigidBodyComponent* Body)
{
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
	const FString FallbackName = Body != nullptr ? Body->GetName() : Owner.GetName();
	UStaticMesh* MeshAsset =
		GetOrCreateStaticMeshAsset(Barrier, FallbackName, RestoredMeshes, DirectoryName);
	if (MeshAsset == nullptr)
	{
		// No point in continuing further. Logging handled in GetOrCreateStaticMeshAsset.
		/// \todo Consider moving logging in here, using WriteImportErrorMessage.
		return nullptr;
	}

	UStaticMeshComponent* MeshComponent =
		FAGX_EditorUtilities::CreateStaticMeshComponent(Owner, *Component, *MeshAsset, false);
	FString SourceName = Barrier.GetSourceName();
	if (SourceName.Contains("\\") || SourceName.Contains("/"))
	{
		SourceName = FPaths::GetBaseFilename(SourceName);
	}

	FString MeshName = !SourceName.IsEmpty() ? SourceName : (Barrier.GetName() + TEXT("Mesh"));
	if (MeshComponent == nullptr)
	{
		UE_LOG(
			LogAGX, Warning, TEXT("Could not create MeshComponent for imported trimesh '%s'."),
			*MeshName);
	}
	if (MeshComponent != nullptr)
	{
		FAGX_ImportUtilities::Rename(*MeshComponent, *MeshName);
	}

	// Both components must be created and attached before they are registered because BeginPlay
	// may be called by RegisterComponent and the TrimeshMeshComponent must know of the
	// StaticMeshComponent before that happens.
	/// @todo In which order should these be? Does it matter?
	if (MeshComponent != nullptr)
	{
		MeshComponent->RegisterComponent();
	}
	Component->RegisterComponent();

	Component->CopyFrom(Barrier);
	::FinalizeShape(
		*Component, Barrier, RestoredShapeMaterials, RestoredRenderMaterials, RestoredMeshes,
		DirectoryName, *MeshComponent);
	return Component;
}

UAGX_ShapeMaterialAsset* FAGX_SimObjectsImporterHelper::InstantiateShapeMaterial(
	const FShapeMaterialBarrier& Barrier)
{
	/// \todo Do we need any special handling of the default material?
	UAGX_ShapeMaterialAsset* Asset =
		FAGX_ImportUtilities::SaveImportedShapeMaterialAsset(Barrier, DirectoryName);
	RestoredShapeMaterials.Add(Barrier.GetGuid(), Asset);
	return Asset;
}

UAGX_ContactMaterialAsset* FAGX_SimObjectsImporterHelper::InstantiateContactMaterial(
	const FContactMaterialBarrier& Barrier, AActor& Owner)
{
	FShapeMaterialPair Materials = GetShapeMaterials(Barrier);
	UAGX_ContactMaterialAsset* Asset = FAGX_ImportUtilities::SaveImportedContactMaterialAsset(
		Barrier, Materials.first, Materials.second, DirectoryName);

	UAGX_ContactMaterialRegistrarComponent* CMRegistrar =
		GetOrCreateContactMaterialRegistrar(Owner);
	if (CMRegistrar == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Contact Material Registrar could not be added to '%s' during import."),
			*Owner.GetName());
		return Asset;
	}

	CMRegistrar->ContactMaterials.Add(Asset);

	return Asset;
}

namespace
{
	template <typename UComponent, typename FBarrier>
	UComponent* InstantiateConstraint(
		const FBarrier& Barrier, AActor& Owner, FAGX_SimObjectsImporterHelper& Helper,
		const TArray<FGuid>& IgnoreList)
	{
		if (IgnoreList.Contains(Barrier.GetGuid()))
		{
			// Don't instantiate the Constraint if it is in the ignore list. This might be the case
			// for e.g. the Hinge constraint owned by a native TwoBodyTire object.
			return nullptr;
		}

		FAGX_SimObjectsImporterHelper::FBodyPair Bodies = Helper.GetBodies(Barrier);
		if (Bodies.first == nullptr)
		{
			// Not having a second body is fine, means that the first body is constrained to the
			// world. Not having a first body is bad.
			UE_LOG(
				LogAGX, Warning,
				TEXT("Constraint '%s' imported from '%s' does not have a first body. Ignoring."),
				*Barrier.GetName(), *Helper.SourceFilePath);
			return nullptr;
		}

		UComponent* Component = FAGX_EditorUtilities::CreateConstraintComponent<UComponent>(
			&Owner, Bodies.first, Bodies.second);
		if (Component == nullptr)
		{
			return nullptr;
		}

		Component->CopyFrom(Barrier);
		FAGX_ConstraintUtilities::SetupConstraintAsFrameDefiningSource(
			Barrier, *Component, Bodies.first, Bodies.second);
		FAGX_ConstraintUtilities::CopyControllersFrom(*Component, Barrier);
		FAGX_ImportUtilities::Rename(*Component, Barrier.GetName());
		return Component;
	}

	/// \todo Consider removing the 1Dof and 2Dof instantiatior functions. Does not seem to be
	/// needed, just call the generic InstantiateConstraint immediately.

	template <typename UComponent>
	UComponent* InstantiateConstraint1Dof(
		const FConstraint1DOFBarrier& Barrier, AActor& Owner, FAGX_SimObjectsImporterHelper& Helper,
		const TArray<FGuid>& IgnoreList)
	{
		return InstantiateConstraint<UComponent>(Barrier, Owner, Helper, IgnoreList);
	}

	template <typename UConstraint>
	UConstraint* InstantiateConstraint2Dof(
		const FConstraint2DOFBarrier& Barrier, AActor& Owner, FAGX_SimObjectsImporterHelper& Helper,
		const TArray<FGuid>& IgnoreList)
	{
		return InstantiateConstraint<UConstraint>(Barrier, Owner, Helper, IgnoreList);
	}
}

UAGX_HingeConstraintComponent* FAGX_SimObjectsImporterHelper::InstantiateHinge(
	const FHingeBarrier& Barrier, AActor& Owner)
{
	return ::InstantiateConstraint1Dof<UAGX_HingeConstraintComponent>(
		Barrier, Owner, *this, ConstraintIgnoreList);
}

UAGX_PrismaticConstraintComponent* FAGX_SimObjectsImporterHelper::InstantiatePrismatic(
	const FPrismaticBarrier& Barrier, AActor& Owner)
{
	return ::InstantiateConstraint1Dof<UAGX_PrismaticConstraintComponent>(
		Barrier, Owner, *this, ConstraintIgnoreList);
}

UAGX_BallConstraintComponent* FAGX_SimObjectsImporterHelper::InstantiateBallJoint(
	const FBallJointBarrier& Barrier, AActor& Owner)
{
	return InstantiateConstraint<UAGX_BallConstraintComponent>(
		Barrier, Owner, *this, ConstraintIgnoreList);
}

UAGX_CylindricalConstraintComponent* FAGX_SimObjectsImporterHelper::InstantiateCylindricalJoint(
	const FCylindricalJointBarrier& Barrier, AActor& Owner)
{
	return ::InstantiateConstraint2Dof<UAGX_CylindricalConstraintComponent>(
		Barrier, Owner, *this, ConstraintIgnoreList);
}

UAGX_DistanceConstraintComponent* FAGX_SimObjectsImporterHelper::InstantiateDistanceJoint(
	const FDistanceJointBarrier& Barrier, AActor& Owner)
{
	return ::InstantiateConstraint1Dof<UAGX_DistanceConstraintComponent>(
		Barrier, Owner, *this, ConstraintIgnoreList);
}

UAGX_LockConstraintComponent* FAGX_SimObjectsImporterHelper::InstantiateLockJoint(
	const FLockJointBarrier& Barrier, AActor& Owner)
{
	return ::InstantiateConstraint<UAGX_LockConstraintComponent>(
		Barrier, Owner, *this, ConstraintIgnoreList);
}

UAGX_TwoBodyTireComponent* FAGX_SimObjectsImporterHelper::InstantiateTwoBodyTire(
	const FTwoBodyTireBarrier& Barrier, AActor& Owner, bool IsBlueprintOwner)
{
	UAGX_TwoBodyTireComponent* Component = NewObject<UAGX_TwoBodyTireComponent>(&Owner);
	if (Component == nullptr)
	{
		WriteImportErrorMessage(
			TEXT("AGX Dynamics TwoBodyTire"), Barrier.GetName(), SourceFilePath,
			TEXT("Could not create new AGX_TwoBodyTireComponent"));
		return nullptr;
	}

	auto SetRigidBody = [&](UAGX_RigidBodyComponent* Body, FAGX_RigidBodyReference& BodyRef)
	{
		if (Body == nullptr)
		{
			WriteImportErrorMessage(
				TEXT("AGX Dynamics TwoBodyTire"), Barrier.GetName(), SourceFilePath,
				TEXT("Could not set Rigid Body"));
			return;
		}

		BodyRef.BodyName = Body->GetFName();
		if (!IsBlueprintOwner)
		{
			BodyRef.OwningActor = Body->GetOwner();
		}
	};

	SetRigidBody(GetBody(Barrier.GetTireRigidBody()), Component->TireRigidBody);
	SetRigidBody(GetBody(Barrier.GetHubRigidBody()), Component->HubRigidBody);
	FAGX_ImportUtilities::Rename(*Component, Barrier.GetName());
	Component->CopyFrom(Barrier);
	Component->SetFlags(RF_Transactional);
	Owner.AddInstanceComponent(Component);
	Component->RegisterComponent();

	// The internal constraint owned by the TwoBodyTire should not be imported, but is created after
	// BeginPlay by the native TwoBodyTire.
	ConstraintIgnoreList.Add(Barrier.GetHingeGuid());

	return Component;
}

UAGX_CollisionGroupDisablerComponent* FAGX_SimObjectsImporterHelper::InstantiateCollisionGroupDisabler(
	AActor& Owner, const TArray<std::pair<FString, FString>>& DisabledPairs)
{
	UAGX_CollisionGroupDisablerComponent* Component =
		NewObject<UAGX_CollisionGroupDisablerComponent>(&Owner, TEXT("AGX_CollisionGroupDisabler"));

	Component->SetFlags(RF_Transactional);
	Owner.AddInstanceComponent(Component);
	Component->RegisterComponent();
	for (const std::pair<FString, FString>& DisabledPair : DisabledPairs)
	{
		Component->DisableCollisionGroupPair(
			FName(*DisabledPair.first), FName(*DisabledPair.second));
	}

	return Component;
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
		UAGX_ShapeMaterialAsset* Material = RestoredShapeMaterials.FindRef(Guid);
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
		/// @todo This should be changed to GetLocation once route nodes with a body have their
		/// location relative to the body instead of the wire.
		RouteNode.Location = NodeBarrier.GetWorldLocation();
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
		/// @todo This should be changed to GetLocation for nodes with a body once route nodes with
		/// a body have their location relative to the body instead of the wire.
		RouteNode.Location = NodeAGX.GetWorldLocation();

		if (NodeType == EWireNodeType::Eye)
		{
			FRigidBodyBarrier BodyBarrier = NodeAGX.GetRigidBody();
			UAGX_RigidBodyComponent* BodyComponent = GetBody(BodyBarrier);
			if (BodyComponent != nullptr)
			{
				RouteNode.SetBody(BodyComponent);
			}
		}

		Route.Add(RouteNode);
	}

	if (!bHaveEndWinch)
	{
		// Configure ending Body Fixed node. Some Body Fixed nodes are owned by the winch on that
		//	side, do not create an explicit Body Fixed node in that case.
		TryCreateBodyFixedNode(Barrier.GetLastNode());
	}

	Component->SetFlags(RF_Transactional);
	Owner.AddInstanceComponent(Component);
	Component->RegisterComponent();
	Component->PostEditChange();
	// May chose to store a table of all imported wires. If so, add this wire to the table here.
	return Component;
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

	UAGX_RigidBodyComponent* Component = RestoredBodies.FindRef(Barrier.GetGuid());
	if (Component == nullptr && LogErrorIfNotFound)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("While importing from '%s': A component references a body '%s', but that body "
				 "hasn't been restored."),
			*SourceFilePath, *Barrier.GetName());
	}

	return Component;
}

FAGX_SimObjectsImporterHelper::FBodyPair FAGX_SimObjectsImporterHelper::GetBodies(
	const FConstraintBarrier& Barrier)
{
	return {GetBody(Barrier.GetFirstBody()), GetBody(Barrier.GetSecondBody())};
}

UAGX_ShapeMaterialAsset* FAGX_SimObjectsImporterHelper::GetShapeMaterial(
	const FShapeMaterialBarrier& Barrier)
{
	return RestoredShapeMaterials.FindRef(Barrier.GetGuid());
}

FAGX_SimObjectsImporterHelper::FShapeMaterialPair FAGX_SimObjectsImporterHelper::GetShapeMaterials(
	const FContactMaterialBarrier& ContactMaterial)
{
	return {
		GetShapeMaterial(ContactMaterial.GetMaterial1()),
		GetShapeMaterial(ContactMaterial.GetMaterial2())};
}

namespace
{
	FString MakeModelName(FString SourceFilename)
	{
		return FAGX_EditorUtilities::SanitizeName(SourceFilename, TEXT("ImportedAgxModel"));
	}

	FString MakeDirectoryName(const FString ModelName)
	{
		FString BasePath = FAGX_ImportUtilities::CreatePackagePath(ModelName);

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
		UE_LOG(
			LogAGX, Display, TEXT("Importing model '%s' to '%s'."), *ModelName,
			*DirectoryPath);
		return DirectoryName;
	}
}

FAGX_SimObjectsImporterHelper::FAGX_SimObjectsImporterHelper(const FString& InSourceFilePath)
	: SourceFilePath(InSourceFilePath)
	, SourceFileName(FPaths::GetBaseFilename(InSourceFilePath))
	, ModelName(MakeModelName(SourceFileName))
	, DirectoryName(MakeDirectoryName(ModelName))
{
}