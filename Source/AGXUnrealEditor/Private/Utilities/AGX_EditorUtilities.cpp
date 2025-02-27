// Copyright 2024, Algoryx Simulation AB.

#include "Utilities/AGX_EditorUtilities.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"
#include "Import/AGX_ImportSettings.h"
#include "Import/AGX_ImporterToEditor.h"
#include "Shapes/AGX_ShapeComponent.h"
#include "Shapes/AGX_SphereShapeComponent.h"
#include "Shapes/AGX_CylinderShapeComponent.h"
#include "Shapes/AGX_CapsuleShapeComponent.h"
#include "Shapes/AGX_TrimeshShapeComponent.h"
#include "Shapes/AGX_BoxShapeComponent.h"
#include "Shapes/RenderDataBarrier.h"
#include "Constraints/AGX_ConstraintActor.h"
#include "Constraints/AGX_ConstraintComponent.h"
#include "Constraints/AGX_ConstraintFrameActor.h"
#include "Import/AGX_ImportSettings.h"
#include "Import/AGX_ModelSourceComponent.h"
#include "Utilities/AGX_BlueprintUtilities.h"
#include "Utilities/AGX_ImportUtilities.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_ObjectUtilities.h"
#include "Utilities/AGX_StringUtilities.h"
#include "Widgets/AGX_SynchronizeModelDialog.h"

// Unreal Engine includes.
#include "ActorEditorUtils.h"
#include "AssetDeleteModel.h"
#include "AssetToolsModule.h"
#include "Containers/Ticker.h"
#include "ContentBrowserModule.h"
#include "DesktopPlatformModule.h"
#include "Editor.h"
#include "Editor/EditorEngine.h"
#include "Engine/EngineTypes.h"
#include "Engine/Selection.h"
#include "Engine/StaticMesh.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/PlayerController.h"
#include "IContentBrowserSingleton.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "MeshDescription.h"
#include "Misc/Char.h"
#include "Misc/EngineVersionComparison.h"
#include "Misc/MessageDialog.h"
#include "PackageTools.h"
#include "PhysicsEngine/BodySetup.h"
#include "RawMesh.h"
#include "SSubobjectEditor.h"
#include "UObject/SavePackage.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UObjectIterator.h"

#define LOCTEXT_NAMESPACE "FAGX_EditorUtilities"

void FAGX_EditorUtilities::SynchronizeModel(UBlueprint& Blueprint, bool bOpenBlueprintEditorAfter)
{
	// The reason we use FTSTicker here is to ensure that this function returns before we do the
	// actual Model Synchronization. This is important because we close all asset editors before
	// doing the Model Synchronization, and if this function was called from the details panel of
	// the ModelSourceComponent, it may cause issues since it lives in the context of the blueprint
	// editor (which will be closed).
#if UE_VERSION_OLDER_THAN(5, 0, 0)
	FTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda(
#else
	FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda(
#endif
		[&Blueprint, bOpenBlueprintEditorAfter](float)
		{
			UBlueprint* OuterMostParent = FAGX_BlueprintUtilities::GetOutermostParent(&Blueprint);

			if (OuterMostParent == nullptr)
			{
				FAGX_NotificationUtilities::ShowDialogBoxWithErrorLog(
					"Could not get the original parent Blueprint. Model synchronization will not "
					"be performed.");
				return false;
			}

			// Ensure there exists a Model Source Component.
			UAGX_ModelSourceComponent* ModelSourceComponent =
				FAGX_BlueprintUtilities::GetFirstComponentOfType<UAGX_ModelSourceComponent>(
					OuterMostParent);
			if (ModelSourceComponent == nullptr)
			{
				FAGX_NotificationUtilities::ShowDialogBoxWithErrorLog(
					"Could not find an AGX Model Source Component in the selected Blueprint. The "
					"selected Blueprint is not valid for Model Synchronization.");
				return false;
			}

			// Open up the import settings Window to get user import settings.
			TSharedRef<SWindow> Window =
				SNew(SWindow)
					.SupportsMinimize(false)
					.SupportsMaximize(false)
					.SizingRule(ESizingRule::Autosized)
					.Title(NSLOCTEXT(
						"AGX", "AGXUnrealSynchronizeModel", "Synchronize model with source file"));

			const FString FilePath =
				ModelSourceComponent != nullptr ? ModelSourceComponent->FilePath : "";
			const bool IgnoreDisabledTrimeshes =
				ModelSourceComponent != nullptr ? ModelSourceComponent->bIgnoreDisabledTrimeshes
												: false;

			TSharedRef<SAGX_SynchronizeModelDialog> SynchronizeDialog =
				SNew(SAGX_SynchronizeModelDialog);
			SynchronizeDialog->SetFilePath(FilePath);
			SynchronizeDialog->SetIgnoreDisabledTrimeshes(IgnoreDisabledTrimeshes);
			SynchronizeDialog->RefreshGui();
			Window->SetContent(SynchronizeDialog);
			FSlateApplication::Get().AddModalWindow(Window, nullptr);

			if (auto Settings = SynchronizeDialog->ToReimportSettings())
			{
				const static FString Info =
					"Model synchronization may permanently remove or overwrite existing "
					"data.\nIt is recommended to always backup your imported models.\n\nAll asset "
					"editors will be closed.\nContinue?";
				if (FMessageDialog::Open(EAppMsgType::YesNo, FText::FromString(Info)) !=
					EAppReturnType::Yes)
				{
					return false;
				}

				FAGX_ImporterToEditor Importer;
				Importer.Reimport(*OuterMostParent, *Settings, &Blueprint);
			}

			return false; // This tells the FTSTicker to not call this lambda again.
		}));
}

bool FAGX_EditorUtilities::RenameAsset(
	UObject& Asset, const FString& WantedName, const FString& AssetType)
{
	if (WantedName.IsEmpty())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("RenameAsset called with Asset '%s' and WantedName was empty. The asset will not "
				 "be renamed."),
			*Asset.GetName());
		return false;
	}

	UPackage* Package = Asset.GetPackage();
	if (Package == nullptr || Package->GetPathName().IsEmpty())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT(
				"RenameAsset called with Asset '%s' without a valid Package. The asset will not be "
				"renamed."),
			*Asset.GetName());
		return false;
	}

	FString AssetNameNew = FAGX_ImportUtilities::CreateAssetName(WantedName, "", AssetType);
	if (Asset.GetName().Equals(AssetNameNew))
	{
		return true;
	}

	FString PackagePathNew = *Asset.GetPackage()->GetPathName();
	PackagePathNew.RemoveFromEnd(Asset.GetName(), ESearchCase::CaseSensitive);
	FAGX_ImportUtilities::MakePackageAndAssetNameUnique(PackagePathNew, AssetNameNew);

	const FString OldPath = Asset.GetPathName();

	FAssetToolsModule& AssetToolsModule =
		FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	TArray<FAssetRenameData> AssetRenameData;
	FAssetRenameData Ard;
	Ard.Asset = &Asset;
	Ard.NewPackagePath = FPackageName::GetLongPackagePath(PackagePathNew);
	Ard.NewName = AssetNameNew;
	AssetRenameData.Add(Ard);
	AssetToolsModule.Get().RenameAssets(AssetRenameData);
	FAssetRegistryModule::AssetRenamed(&Asset, OldPath);
	Asset.MarkPackageDirty();
	Asset.GetOuter()->MarkPackageDirty();

	return true;
}

int32 FAGX_EditorUtilities::DeleteImportedAssets(const TArray<UObject*>& InAssets)
{
	/*
	 * This function is a subset/variant of Unreal Engine's ObjectTools::DeleteObjects. We can't use
	 * that directly because it doesn't handle references to the deleted assets properly. In
	 * particular, it always calls FAssetDeleteModel::DoDelete even though in some cases it is
	 * necessary to call DoForceDelete.
	 *
	 * Not sure how regularly ObjectTools::DeleteObjects is updated by Epic Games, and how closely
	 * we need to track that implementation over time. The initial implementation of this function
	 * is based on Unreal Engine 4.27.
	 *
	 * There is also ObjectTools::DeleteAssets which sounds like exactly what we want to do, but
	 * that is basically just a call to ObjectTools::DeleteObjects followed by
	 * CleanupAfterSuccessfulDelete. The latter is called by the former, which makes me believe that
	 * the call from ObjectTools::DeleteAssets is redundant, and the former is what we are doing a
	 * variant of here since it is incomplete.
	 */

	TArray<UObject*> ObjectsToDelete = InAssets;

	// Here the engine implementation creates an FScopedBusyCursor. Should we too?

	// Here the engine implementation calls ObjectTools::AddExtraObjectsToDelete. As of Unreal
	// Engine 4.27 that only involves UWorld objects, which we don't create during model import and
	// thus don't currently need to handle here.

	// There the engine implementation does stuff for sounds. We don't do anything with sound assets
	// so skipping that for now.

	FCanDeleteAssetResult CanDeleteResult;
	FEditorDelegates::OnAssetsCanDelete.Broadcast(ObjectsToDelete, CanDeleteResult);
	if (!CanDeleteResult.Get())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT(
				"Cannot currently delete imported assets, rejected by OnAssetsCanDelete. See prior "
				"log messagesfor details."));
		return 0;
	}

	// Only packages that are fully loaded can be deleted, so ask for them to be loaded.
	//
	// Here the engine implementation calls a non-exported helper function in ObjectTools,
	// HandleFullyLoadedPackages. We can't call that from our module, so doing something similar
	// in-line instead.
	{
		TArray<UPackage*> Packages;
		for (UObject* Object : ObjectsToDelete)
		{
			Packages.AddUnique(Object->GetOutermost());
		}
		if (!UPackageTools::HandleFullyLoadingPackages(
				Packages, LOCTEXT("DeleteImportedAssets", "Delete imported assets.")))
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Cannot delete imported assets because at least one asset could not be fully "
					 "loaded."));
			return 0;
		}
	}

	// We cannot delete assets if the asset registry is currently busy loading assets.
	if (IAssetRegistry::GetChecked().IsLoadingAssets())
	{
		// Is there a better way to handle this case? Can we wait a bit? Is asset loading
		// asynchronous, i.e. can we spin here for a second or two?
		UE_LOG(
			LogAGX, Warning,
			TEXT("Cannot delete assets because the asset registry is currently loading assets."));
		return 0;
	}

	// Here the engine implementation checks if the list of assets to delete include an active
	// world, including all editor world contexts and streaming levels, by calling
	// ObjectTools::ContainsWorldInUse. That function isn't available to us. We currently don't
	// create any worlds during import so don't need to handle that case yet.

	// Let everyone know that these assets are about to disappear, so they can clear any references
	// they may have to the assets.
	FEditorDelegates::OnAssetsPreDelete.Broadcast(ObjectsToDelete);

	// The delete model helps us find references to the deleted assets.
	FAssetDeleteModel DeleteModel(ObjectsToDelete);

	// Here the engine implementation uses GWarn to begin a slow task. The model
	// synchronize code already have a progress bar created with FScopedSlowTask, not sure how
	// those would interact.

	while (DeleteModel.GetState() != FAssetDeleteModel::Finished)
	{
		DeleteModel.Tick(0);

		// Here the engine implementation does stuff with GWarn status update and user canceling.
		// The model synchronize code already have a progress bar created with FScopedSlowTask, not
		// sure how those would interact.
	}

	if (DeleteModel.CanDelete())
	{
		DeleteModel.DoDelete();
		return DeleteModel.GetDeletedObjectCount();
	}
	else if (DeleteModel.CanForceDelete())
	{
		DeleteModel.DoForceDelete();
		return DeleteModel.GetDeletedObjectCount();
	}
	else
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Coult not verify safe asset deletion, old model assets remain in the Content "
				 "Browser and on drive."));
		return 0;
	}
}

std::tuple<AActor*, USceneComponent*> FAGX_EditorUtilities::CreateEmptyActor(
	const FTransform& Transform, UWorld* World)
{
	/// \todo The intention is to mimmic dragging in an "Empty Actor" from the
	/// Place mode. Investigate if we can use ActorFactoryEmptyActor instead.

	AActor* NewActor = World->SpawnActor<AActor>(AActor::StaticClass());
	if (NewActor == nullptr)
	{
		UE_LOG(LogAGX, Warning, TEXT("Failed to create empty actor."));
		/// \todo Do we need to destroy the Actor here?
		return {nullptr, nullptr};
	}

	/// \todo I don't know what RF_Transactional means. Taken from UActorFactoryEmptyActor.
	/// Related to undo/redo, I think.
	USceneComponent* Root = NewObject<USceneComponent>(
		NewActor, USceneComponent::GetDefaultSceneRootVariableName() /*, RF_Transactional*/);
	NewActor->SetRootComponent(Root);
	NewActor->AddInstanceComponent(Root);
	Root->RegisterComponent();
	NewActor->SetActorTransform(Transform, false);

	return {NewActor, Root};
}

namespace
{
	template <typename TComponent>
	TComponent* CreateComponent(AActor* Owner)
	{
		UClass* Class = TComponent::StaticClass();
		TComponent* Component = NewObject<TComponent>(Owner, Class);
		if (Component == nullptr)
		{
			UE_LOG(LogAGX, Log, TEXT("Could not create component %s."), *Class->GetName());
			return nullptr;
		}
		Owner->AddInstanceComponent(Component);
		Component->RegisterComponent();
		return Component;
	}

	template <typename TShapeComponent>
	TShapeComponent* CreateShapeComponent(AActor* Owner, USceneComponent* Outer, bool bRegister)
	{
		/// \todo Is the Owner pointless here since we do `AttachToComponent`
		/// immediately afterwards?
		UClass* Class = TShapeComponent::StaticClass();
		TShapeComponent* Shape = NewObject<TShapeComponent>(
			Owner, FName(FAGX_ImportUtilities::GetUnsetUniqueImportName()));
		Owner->AddInstanceComponent(Shape);
		if (bRegister)
		{
			Shape->RegisterComponent();
		}
		if (Outer)
		{
			const bool Attached = Shape->AttachToComponent(
				Outer, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			check(Attached);
		}
		return Shape;
	}

	template <typename T>
	static T* GetAssetByPath(const FString& AssetPath)
	{
		FAssetRegistryModule& AssetRegistryModule =
			FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		TArray<FAssetData> AssetData;
		FARFilter Filter;
		Filter.PackageNames.Add(FName(*AssetPath));
		AssetRegistryModule.Get().GetAssets(Filter, AssetData);
		T* Asset = FAssetData::GetFirstAsset<T>(AssetData);

		if (Asset == nullptr)
		{
			UE_LOG(LogAGX, Error, TEXT("Could not find asset with path %s."), *AssetPath);
		}

		return Asset;
	}

	// Based on Engine code: GenerateBoxAsSimpleCollision in GeomFitUtils.h.
	void AddBoxSimpleCollision(UStaticMesh* StaticMesh)
	{
		if (StaticMesh == nullptr)
			return;

		UBodySetup* bs = StaticMesh->GetBodySetup();
		if (bs == nullptr)
			return;

		// Calculate bounding Box.
		FVector Center, Extents;
		StaticMesh->GetMeshDescription(0)->ComputeBoundingBox().GetCenterAndExtents(
			Center, Extents);
		Extents *= (FVector) bs->BuildScale3D;

		// Create new GUID.
		bs->InvalidatePhysicsData();

		FKBoxElem BoxElem;
		BoxElem.Center = Center;
		BoxElem.X = Extents.X * 2.0f;
		BoxElem.Y = Extents.Y * 2.0f;
		BoxElem.Z = Extents.Z * 2.0f;
		bs->AggGeom.BoxElems.Add(BoxElem);

		// Mark the static mesh for collision customization
		StaticMesh->bCustomizedCollision = true;
	}
}

UAGX_RigidBodyComponent* FAGX_EditorUtilities::CreateRigidBody(AActor* Owner)
{
	UAGX_RigidBodyComponent* Body = ::CreateComponent<UAGX_RigidBodyComponent>(Owner);
	Body->AttachToComponent(
		Owner->GetRootComponent(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, false));
#if UE_VERSION_OLDER_THAN(4, 24, 0)
	Body->RelativeLocation = FVector(0.0f, 0.0f, 0.0f);
	Body->RelativeRotation = FRotator(0.0f, 0.0f, 0.0f);
#else
	Body->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	Body->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
#endif
	return Body;
}

/// \todo Can the Owner parameter be removed, and instead use Outer->GetOwner()?
/// When would we want to attach the sphere to a component that is in another Actor.
UAGX_SphereShapeComponent* FAGX_EditorUtilities::CreateSphereShape(
	AActor* Owner, USceneComponent* Outer)
{
	return ::CreateShapeComponent<UAGX_SphereShapeComponent>(Owner, Outer, true);
}

UAGX_BoxShapeComponent* FAGX_EditorUtilities::CreateBoxShape(AActor* Owner, USceneComponent* Outer)
{
	return ::CreateShapeComponent<UAGX_BoxShapeComponent>(Owner, Outer, true);
}

UAGX_CylinderShapeComponent* FAGX_EditorUtilities::CreateCylinderShape(
	AActor* Owner, USceneComponent* Outer)
{
	return ::CreateShapeComponent<UAGX_CylinderShapeComponent>(Owner, Outer, true);
}

UAGX_CapsuleShapeComponent* FAGX_EditorUtilities::CreateCapsuleShape(
	AActor* Owner, USceneComponent* Outer)
{
	return ::CreateShapeComponent<UAGX_CapsuleShapeComponent>(Owner, Outer, true);
}

UAGX_TrimeshShapeComponent* FAGX_EditorUtilities::CreateTrimeshShape(
	AActor* Owner, USceneComponent* Outer, bool bRegister)
{
	return ::CreateShapeComponent<UAGX_TrimeshShapeComponent>(Owner, Outer, bRegister);
}

/// \todo There is probably a name sanitizer already in Unreal. Find it.
/// \todo The sanitizers are called multiple times on the same string. Find a root function, or
/// a suitable helper function, and sanitize once. Assume already sanitizied in all other helper
/// functions.

FString FAGX_EditorUtilities::SanitizeName(const FString& Name)
{
	FString Sanitized;
	Sanitized.Reserve(Name.Len());
	for (TCHAR C : Name)
	{
		/// \todo Will this accept non-english characters? Should it?
		if (TChar<TCHAR>::IsAlnum(C) || C == TCHAR('_') || C == TCHAR('-'))
		{
			Sanitized.AppendChar(C);
		}
	}
	return Sanitized;
}

FString FAGX_EditorUtilities::SanitizeName(const FString& Name, const FString& Fallback)
{
	FString Sanitized = SanitizeName(Name);
	if (Sanitized.IsEmpty())
	{
		return Fallback;
	}
	return Sanitized;
}

FString FAGX_EditorUtilities::SanitizeName(const FString& Name, const TCHAR* Fallback)
{
	FString Sanitized = SanitizeName(Name);
	if (Sanitized.IsEmpty())
	{
		return FString(Fallback);
	}
	return Sanitized;
}

FString FAGX_EditorUtilities::CreateAssetName(
	FString SourceName, FString ActorName, FString DefaultName)
{
	SourceName = FAGX_EditorUtilities::SanitizeName(SourceName);
	if (!SourceName.IsEmpty())
	{
		return SourceName;
	}

	ActorName = FAGX_EditorUtilities::SanitizeName(ActorName);
	if (!ActorName.IsEmpty())
	{
		return ActorName;
	}

	DefaultName = FAGX_EditorUtilities::SanitizeName(DefaultName);
	if (!DefaultName.IsEmpty())
	{
		return DefaultName;
	}

	return TEXT("ImportedAGXObject");
}

void FAGX_EditorUtilities::MakePackageAndAssetNameUnique(FString& PackageName, FString& AssetName)
{
	IAssetTools& AssetTools =
		FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	AssetTools.CreateUniqueAssetName(PackageName, AssetName, PackageName, AssetName);
}

void FAGX_EditorUtilities::AddSimpleCollisionBox(UStaticMesh& StaticMesh)
{
	AddBoxSimpleCollision(&StaticMesh);
}

bool FAGX_EditorUtilities::SaveStaticMeshAssetsInBulk(const TArray<UStaticMesh*>& Meshes)
{
	bool EncounteredIssue = false;
	for (auto Mesh : Meshes)
	{
		if (Mesh == nullptr)
			continue;

		FAssetRegistryModule::AssetCreated(Mesh);
		Mesh->MarkPackageDirty();
	}

	// Do the costly Build as a batch Build.
	UStaticMesh::BatchBuild(Meshes);

	for (auto Mesh : Meshes)
	{
		if (Mesh == nullptr)
			continue;

		UPackage* Package = Mesh->GetPackage();
		if (Package == nullptr || Package->GetPathName().IsEmpty())
		{
			EncounteredIssue = true;
			UE_LOG(
				LogAGX, Warning,
				TEXT("Got invalid package from asset '%s' in SaveStaticMeshAssetInBulk. The asset "
					 "will not be saved."),
				*Mesh->GetName());
		}
		// The below PostEditChange call is what normally takes a lot of time, since it internally
		// calls Build() each time. But since we have already done the Build (in batch) above, it
		// will not actually Build the asset again. So the PostEditChange call below will execute
		// really fast and we get the benefit of still getting everything else done in
		// PostEditChange.
		Mesh->PostEditChange();
		Mesh->AddToRoot();
		Package->SetDirtyFlag(true);

		// Store our new package to disk.
		const FString PackageFilename = FPackageName::LongPackageNameToFilename(
			Package->GetPathName(), FPackageName::GetAssetPackageExtension());
		if (PackageFilename.IsEmpty())
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Unreal Engine unable to provide a package filename for package path '%s'."),
				*Package->GetPathName());
			EncounteredIssue = true;
			continue;
		}

		Package->GetMetaData();
#if UE_VERSION_OLDER_THAN(5, 0, 0)
		bool bSaved = UPackage::SavePackage(Package, Mesh, RF_NoFlags, *PackageFilename);
#else
		FSavePackageArgs SaveArgs;
		SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
		bool bSaved = UPackage::SavePackage(Package, Mesh, *PackageFilename, SaveArgs);
#endif
		if (!bSaved)
		{
			UE_LOG(
				LogAGX, Error, TEXT("Unreal Engine unable to save package '%s' to file '%s'."),
				*Package->GetPathName(), *PackageFilename);
			EncounteredIssue = true;
		}
	}

	return !EncounteredIssue;
}

FRawMesh FAGX_EditorUtilities::CreateRawMeshFromTrimesh(const FTrimeshShapeBarrier& Trimesh)
{
	if (Trimesh.GetNumPositions() <= 0 || Trimesh.GetNumIndices() <= 0)
	{
		// No collision mesh data available, this trimesh is invalid.
		UE_LOG(
			LogAGX, Error,
			TEXT("Did not find any triangle data in imported trimesh '%s'. Cannot create "
				 "StaticMesh asset."),
			*Trimesh.GetSourceName())
		return FRawMesh();
	}

	if (Trimesh.GetNumIndices() != Trimesh.GetNumTriangles() * 3)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("The trimesh '%s' does not have three vertex indices per triangle. The mesh "
				 "may be be imported incorrectly. Found %d triangles and %d indices."),
			*Trimesh.GetSourceName(), Trimesh.GetNumTriangles(), Trimesh.GetNumIndices());
	}

	// Data we have:
	//
	// The collision data consists of three arrays: positions, indices, and normals. The
	// positions is a list of vertex positions. The indices comes in triplets for each triangle
	// and the triplet defines which vertex positions form that triangle. There is one normal
	// per triangle and they are stored in the same order as the position index triplets.
	//
	// Data shared among triangles:
	//   positions: [Vec3, Vec3, Vec3 Vec3, Vec3, ... ]
	//
	// Data owned by each triangle:
	//               |  Triangle 0   |  Triangle 1   | ... |
	//   indices:    | int, int, int | int, int, int | ... | Index into positions.
	//   normals:    |     Vec3      |      Vec3     | ... |
	//
	//
	// Data we want:
	//
	// Unreal Engine store its meshes in a similar format but with additional vertex data stored per
	// triangle. The Unreal Engine format consists of six arrays: positions, indices, tangent X,
	// tangent Y, tangent Z, and texture coordinates. Tangent Z has the same meaning as the normal
	// in the AGX Dynamics data. I don't know how to compute the X and Y tangents, but there are
	// flags to auto-compute them. See AddRawMeshToStaticMesh.
	//
	// Data shared among triangles:
	//   positions: [Vec3, Vec3, Vec3, Vec3, Vec3, ... ]
	//
	// Data owned by each triangle:
	//               |    Triangle 0    |    Triangle 1    | ... |
	//    indices:   | int,  int,  int  | int,  int,  int  | ... | Index into positions.
	//    tangent x: | Vec3, Vec3, Vec3 | Vec3, Vec3, Vec3 | ... | Not written, generated.
	//    tangent y: | Vec3, Vec3, Vec3 | Vec3, Vec3, Vec3 | ... | Not written, generated.
	//    tangent z: | Vec3, Vec3, Vec3 | Vec3, Vec3, Vec3 | ... | 3x collision normal per triangle.
	//    tex coord: | Vec2, Vec2, Vec2 | Vec2, Vec2, Vec2 | ... | (0, 0) everywhere.
	//    colors:    | Fcol, FCol, FCol | Fcol, FCol, FCol | ... | White everywhere.
	//    material:  |       int        |       int        | ... | 0 everywhere.
	//    smoothing: |      uint        |      uint        | ... | 0 everywhere, i.e., no smoothing.
	//
	// The positions and indices can simply be copied over. The normals must be triplicated over
	// all three vertices of each triangle. The other tangents are left unset and
	// bRecomputeTangents enabled in the StaticMesh SourceModel settings, see
	// AddRawMeshToStaticMesh. Texture coordinates are set to (0, 0) since the Trimesh doesn't have
	// any texture coordinates.

	FRawMesh RawMesh;

#if UE_VERSION_OLDER_THAN(5, 0, 0)
	RawMesh.VertexPositions = Trimesh.GetVertexPositions();
#else
	const TArray<FVector>& TrimeshVertexPositions = Trimesh.GetVertexPositions();
	RawMesh.VertexPositions.SetNum(TrimeshVertexPositions.Num());
	for (int32 I = 0; I < TrimeshVertexPositions.Num(); ++I)
	{
		// May do a double -> float conversion, depending on the UE_LARGE_WORLD_COORDINATES_DISABLED
		// preprocessor macro.
		RawMesh.VertexPositions[I] = ToMeshVector(TrimeshVertexPositions[I]);
	}
#endif
	RawMesh.WedgeIndices = Trimesh.GetVertexIndices();

	const int32 NumTriangles = Trimesh.GetNumTriangles();
	const int32 NumIndices = Trimesh.GetNumIndices();

	// Buffers with three elements per triangle, i.e., one per wedge.
	RawMesh.WedgeTangentZ.Reserve(NumIndices);
	RawMesh.WedgeColors.Reserve(NumIndices);
	RawMesh.WedgeTexCoords[0].Reserve(NumIndices);

	// Buffers with one element per triangle.
	RawMesh.FaceMaterialIndices.Reserve(NumTriangles);
	RawMesh.FaceSmoothingMasks.Reserve(NumTriangles);

	const TArray<FVector> TriangleNormals = Trimesh.GetTriangleNormals();

	for (int32 TIdx = 0; TIdx < NumTriangles; ++TIdx)
	{
		// I don't know how to compute the first two tangents, but bRecomputeTangents (not
		// bRecomputeNormals) has been enabled on the StaticMesh SourceModel in
		// AddRawMeshToStaticMesh. Perhaps that's enough.

		// Since we replicate the same normal for all vertices of a triangle we will get a
		// flat-shaded mesh, should this mesh ever be used for rendering.
		const FVector3f Normal = ToMeshVector(TriangleNormals[TIdx]);
		RawMesh.WedgeTangentZ.Add(Normal);
		RawMesh.WedgeTangentZ.Add(Normal);
		RawMesh.WedgeTangentZ.Add(Normal);

		// The collision mesh doesn't have color information, so just write white.
		const FColor Color(255, 255, 255);
		RawMesh.WedgeColors.Add(Color);
		RawMesh.WedgeColors.Add(Color);
		RawMesh.WedgeColors.Add(Color);

		// We must write something to the texture coordinates or else Unreal Engine crashes when
		// processing this mesh later. We could try to do something clever here, but I think just
		// writing zero everywhere is safest.
		RawMesh.WedgeTexCoords[0].Add({0.0f, 0.0f});
		RawMesh.WedgeTexCoords[0].Add({0.0f, 0.0f});
		RawMesh.WedgeTexCoords[0].Add({0.0f, 0.0f});

		// The collision mesh doesn't have material slots, so the best we can do is to provide a
		// single material and apply it to every triangle.
		RawMesh.FaceMaterialIndices.Add(0);

		// Not entirely sure on the FaceSmoothingMasks, the documentation is a little vague:
		//     Smoothing mask. Array[FaceId] = uint32
		// But I believe the process is that Unreal Engine does bitwise-and between two neighboring
		// faces and if the result comes out as non-zero then smoothing will happen along that
		// edge. Not sure what is being smoothed though. Perhaps the vertex normals are merged
		// if smoothing is on, and kept separate if smoothing is off. Also not sure how this
		// relates to the bRecompute.* settings on the StaticMesh's SourceModel.
		RawMesh.FaceSmoothingMasks.Add(0x00000000);
	}

	return RawMesh;
}

FRawMesh FAGX_EditorUtilities::CreateRawMeshFromRenderData(const FRenderDataBarrier& RenderData)
{
	if (RenderData.GetNumTriangles() <= 0)
	{
		// No render mesh data available, this render data is invalid.
		UE_LOG(
			LogAGX, Error,
			TEXT("Did not find any triangle data in imported render data '%s'. Cannot create "
				 "StaticMesh asset."),
			*RenderData.GetGuid().ToString());
		return FRawMesh();
	}

	// What we have:
	//
	// The render data consists of four arrays: positions, normals, texture coordinates, and
	// indices. The function is similar to the collision data, but this time everything is
	// indexed and all arrays except for the index array create a single conceptual Vertex
	// struct.
	//
	// Data shared among triangles:
	//    positions:  [Vec3, Vec3, Vec3, Vec3, Vec3, ... ]
	//    normals:    [Vec3, Vec3, Vec3, Vec3, Vec3, ... ]
	//    tex coords: [Vec2, Vec2, Vec2, Vec2, Vec2, ... ]
	//
	// Data owned by each triangle:
	//                |  Triangle 0   |  Triangle 1   | ... |
	//    indices:    | int, int, int | int, int, int | ... |
	//
	//
	// What we want:
	//
	// Unreal Engine store its meshes in a format similar to the render format in AGX Dynamics, but
	// more data is owned per triangle instead of shared between multiple triangles. The Unreal
	// Engine format consists of six arrays: positions, indices, tangent X, tangent Y, tangent Z,
	// and texture coordinates. Tangent Z has the same meaning as the normal in the AGX Dynamics
	// data.
	//
	// Data shared among triangles:
	//   positions: [Vec3, Vec3, Vec3, Vec3, Vec3, ... ]
	//
	// Data owned by each triangle:
	//               |    Triangle 0    |    Triangle 1    | ... |
	//    indices:   | int,  int,  int  | int,  int,  int  | ... | Index into positions.
	//    tangent x: | Vec3, Vec3, Vec3 | Vec3, Vec3, Vec3 | ... | Not written, generated.
	//    tangent y: | Vec3, Vec3, Vec3 | Vec3, Vec3, Vec3 | ... | Not written, generated.
	//    tangent z: | Vec3, Vec3, Vec3 | Vec3, Vec3, Vec3 | ... | Copied from Render Data.
	//    tex coord: | Vec2, Vec2, Vec2 | Vec2, Vec2, Vec2 | ... | Copied from Render Data.
	//    colors:    | FCol, FCol, FCol | FCol, FCol, FCol | ... | While everywhere.
	//    material:  |       int        |       int        | ... | 0 everywhere.
	//    smoothing: |      uint        |      uint        | ... | All-1 everywhere.

	FRawMesh RawMesh;

	// A straight up copy of the vertex positions may be wasteful since the render data may
	// contain duplicated positions with different normals or texture coordinates. Sine Unreal
	// Engine decouples the normals and the texture coordinates from the positions we may end up
	// with useless position duplicates. If this becomes a serious concern, then find a way to
	// remove duplicates and patch the WedgeIndies to point to the correct merged vertex position.
	// Must use the render vertex indices in the per-index conversion loop below.
#if UE_VERSION_OLDER_THAN(5, 0, 0)
	RawMesh.VertexPositions = RenderData.GetPositions();
#else
	const TArray<FVector>& TrimeshVertexPositions = RenderData.GetPositions();
	RawMesh.VertexPositions.SetNum(TrimeshVertexPositions.Num());
	for (int32 I = 0; I < TrimeshVertexPositions.Num(); ++I)
	{
		// May do a double -> float conversion, depending on the UE_LARGE_WORLD_COORDINATES_DISABLED
		// preprocessor macro.
		RawMesh.VertexPositions[I] = ToMeshVector(TrimeshVertexPositions[I]);
	}
#endif
	RawMesh.WedgeIndices = RenderData.GetIndices();

	const int32 NumTriangles = RenderData.GetNumTriangles();
	const int32 NumIndices = RenderData.GetNumIndices();

	RawMesh.WedgeTangentZ.Reserve(NumIndices);
	RawMesh.WedgeColors.Reserve(NumIndices);
	RawMesh.WedgeTexCoords[0].Reserve(NumIndices);

	const TArray<FVector> RenderNormals = RenderData.GetNormals();
	const auto RenderTexCoords = RenderData.GetTextureCoordinates();

	for (int32 I = 0; I < NumIndices; ++I)
	{
		const int32 RenderI = RawMesh.WedgeIndices[I];
		RawMesh.WedgeTangentZ.Add(ToMeshVector(RenderNormals[RenderI]));
		// Not all Render Data has texture coordinates.
		if (RenderTexCoords.Num() > I)
		{
#if UE_VERSION_OLDER_THAN(5, 0, 0)
			RawMesh.WedgeTexCoords[0].Add(RenderTexCoords[RenderI]);
#else
			RawMesh.WedgeTexCoords[0].Add(
				FVector2f {(float) RenderTexCoords[RenderI].X, (float) RenderTexCoords[RenderI].Y});
#endif
		}
		else
		{
			RawMesh.WedgeTexCoords[0].Add({0.0f, 0.0f});
		}
		RawMesh.WedgeColors.Add(FColor(255, 255, 255));
	}

	RawMesh.FaceMaterialIndices.Reserve(NumTriangles);
	RawMesh.FaceSmoothingMasks.Reserve(NumTriangles);
	for (int32 I = 0; I < NumTriangles; ++I)
	{
		RawMesh.FaceMaterialIndices.Add(0);
		RawMesh.FaceSmoothingMasks.Add(0xFFFFFFFF);
	}

	return RawMesh;
}

void FAGX_EditorUtilities::AddRawMeshToStaticMesh(FRawMesh& RawMesh, UStaticMesh* StaticMesh)
{
#if UE_VERSION_OLDER_THAN(4, 27, 0)
	StaticMesh->StaticMaterials.Add(FStaticMaterial());
#else
	StaticMesh->GetStaticMaterials().Add(FStaticMaterial());
#endif

#if UE_VERSION_OLDER_THAN(4, 23, 0)
	StaticMesh->SourceModels.Emplace();
	FStaticMeshSourceModel& SourceModel = StaticMesh->SourceModels.Last();
#elif UE_VERSION_OLDER_THAN(5, 0, 0)
	StaticMesh->GetSourceModels().Emplace();
	FStaticMeshSourceModel& SourceModel = StaticMesh->GetSourceModels().Last();
#else
	FStaticMeshSourceModel& SourceModel = StaticMesh->AddSourceModel();
#endif

#if UE_VERSION_OLDER_THAN(5, 0, 0)
	// There is a SaveRawMesh on the source model as well, but calling that causes a failed assert.
	// Is that a sign that we're doing something we shouldn't and the engine doesn't detect it
	// because we're sidestepping the safety checks? Or is it OK to do it this way?
	SourceModel.RawMeshBulkData->SaveRawMesh(RawMesh);
#else
	SourceModel.SaveRawMesh(RawMesh);
#endif

	FMeshBuildSettings& BuildSettings = SourceModel.BuildSettings;

	// Somewhat unclear what all these should be. Setting everything I don't understand to false.
	BuildSettings.bRecomputeNormals = false;
	BuildSettings.bRecomputeTangents = true;
	BuildSettings.bUseMikkTSpace = true;
	BuildSettings.bGenerateLightmapUVs = true;
#if UE_VERSION_OLDER_THAN(5, 0, 0)
	BuildSettings.bBuildAdjacencyBuffer = false;
#endif
	BuildSettings.bBuildReversedIndexBuffer = false;
	BuildSettings.bUseFullPrecisionUVs = false;
	BuildSettings.bUseHighPrecisionTangentBasis = false;
}

AAGX_ConstraintActor* FAGX_EditorUtilities::CreateConstraintActor(
	UClass* ConstraintType, UAGX_RigidBodyComponent* RigidBody1,
	UAGX_RigidBodyComponent* RigidBody2, bool bInPlayingWorldIfAvailable, bool bSelect,
	bool bShowNotification)
{
	UWorld* World = bInPlayingWorldIfAvailable ? GetCurrentWorld() : GetEditorWorld();

	check(World);
	check(ConstraintType->IsChildOf<AAGX_ConstraintActor>());

	// Create the new Constraint Actor.
	AAGX_ConstraintActor* NewActor =
		World->SpawnActorDeferred<AAGX_ConstraintActor>(ConstraintType, FTransform::Identity);
	check(NewActor);
	UAGX_ConstraintComponent* Constraint = NewActor->GetConstraintComponent();
	Constraint->BodyAttachment1.RigidBody.SetComponent(RigidBody1);
	Constraint->BodyAttachment2.RigidBody.SetComponent(RigidBody2);
	NewActor->FinishSpawning(FTransform::Identity, true);

	if (bSelect)
	{
		SelectActor(NewActor);
	}

	if (bShowNotification)
	{
		FAGX_NotificationUtilities::ShowNotification(
			"AGX Constraint Created", SNotificationItem::CS_Success);
	}

	return NewActor;
}

AAGX_ConstraintFrameActor* FAGX_EditorUtilities::CreateConstraintFrameActor(
	AActor* ParentActor, bool bSelect, bool bShowNotification, bool bInPlayingWorldIfAvailable)
{
	UWorld* World = bInPlayingWorldIfAvailable ? GetCurrentWorld() : GetEditorWorld();
	check(World);

	// Create the new Constraint Frame Actor.
	AAGX_ConstraintFrameActor* NewActor = World->SpawnActor<AAGX_ConstraintFrameActor>();
	check(NewActor);

	// Set the new actor as child to the Rigid Body.
	if (ParentActor)
	{
		if (ParentActor->GetWorld() == World)
		{
			NewActor->AttachToActor(ParentActor, FAttachmentTransformRules::KeepRelativeTransform);
		}
		else
		{
			UE_LOG(
				LogAGX, Log,
				TEXT("Failed to attach the new AGX Constraint Frame Actor to the specified "
					 "Parent Rigid Body Actor, because it is in another World."));
		}
	}

	if (bSelect)
	{
		SelectActor(NewActor);
	}

	if (bShowNotification)
	{
		FAGX_NotificationUtilities::ShowNotification(
			"AGX Constraint Frame Actor Created", SNotificationItem::CS_Success);
	}

	return NewActor;
}

void FAGX_EditorUtilities::SelectActor(AActor* Actor, bool bDeselectPrevious)
{
	if (bDeselectPrevious)
	{
		GEditor->SelectNone(
			/*bNoteSelectionChange*/ false,
			/*bDeselectBSPSurfs*/ true,
			/*WarnAboutManyActors*/ false);
	}

	if (Actor)
	{
		GEditor->SelectActor(
			Actor,
			/*bInSelected*/ true,
			/*bNotify*/ false);
	}

	GEditor->NoteSelectionChange();
}

bool FAGX_EditorUtilities::IsSelected(const UActorComponent& Component)
{
	// No idea why, but IsSelected seems to always be true when in the Blueprint Editor, regardless
	// of if the Component actually is selected in the Components panel or not.
	if (!FActorEditorUtils::IsAPreviewOrInactiveActor(Component.GetOwner()) &&
		Component.IsSelected())
	{
		// The shovel is directly selected in the level editor.
		return true;
	}

	// Check if the shovel is owned by a Blueprint editor and if so if the shovel is currently
	// selected in that Blueprint editor.
	TSharedPtr<IBlueprintEditor> BlueprintEditor =
		FKismetEditorUtilities::GetIBlueprintEditorForObject(&Component, false);
	if (BlueprintEditor.IsValid())
	{
		TArray<TSharedPtr<FSubobjectEditorTreeNode>> Selection =
			BlueprintEditor->GetSelectedSubobjectEditorTreeNodes();
		for (TSharedPtr<FSubobjectEditorTreeNode>& Selected : Selection)
		{
			const UActorComponent* SelectedComponent = Selected->GetComponentTemplate();
			if (SelectedComponent == &Component)
			{
				return true;
			}
		}
	}

	// If the Shovel is part of a preview that we also want to consider it selected if the
	// Shovel it is a preview of is selected. This happens when we are looking at the Shovel
	// that exists within the viewport of a Blueprint editor. Then the shovel we see isn't
	// selected anywhere, but the CSC node template shovel the preview shovel was created from
	// might be.
	if (Component.GetOwner() != nullptr &&
		FActorEditorUtils::IsAPreviewOrInactiveActor(Component.GetOwner()))
	{
		// The archetype of a preview component is the template instance in the Blueprint.
		// If the template instance is selected then we consider the preview instance to be
		// selected as well.
		UActorComponent* Archetype = Cast<UActorComponent>(Component.GetArchetype());
		if (Archetype != nullptr && IsSelected(*Archetype))
		{
			return true;
		}
	}

	// Did not find anything that has selected the Component. Did we check everything? Who knows.
	return false;
}

UWorld* FAGX_EditorUtilities::GetEditorWorld()
{
	return GEditor->GetEditorWorldContext().World();
}

UWorld* FAGX_EditorUtilities::GetPlayingWorld()
{
	// Without starting from an Actor, the world needs to be found
	// in another way:

	TArray<APlayerController*> PlayerControllers;
	GEngine->GetAllLocalPlayerControllers(PlayerControllers);

	if (PlayerControllers.Num() > 0)
	{
		return PlayerControllers[0]->GetWorld();
	}
	else
	{
		return nullptr;
	}
}

UWorld* FAGX_EditorUtilities::GetCurrentWorld()
{
	if (UWorld* PlayingWorld = GetPlayingWorld())
	{
		return PlayingWorld;
	}
	else
	{
		return GetEditorWorld();
	}
}

void FAGX_EditorUtilities::GetRigidBodyActorsFromSelection(
	AActor** OutActor1, AActor** OutActor2, bool bSearchSubtrees, bool bSearchAncestors)
{
	USelection* SelectedActors = GEditor->GetSelectedActors();

	if (!SelectedActors)
		return;

	if (OutActor1)
		*OutActor1 = nullptr;

	if (OutActor2)
		*OutActor2 = nullptr;

	// Assigns to first available of OutActor1 and OutActor2, and returns whether
	// at least one of them is afterwards still available for assignment.
	auto AssignOutActors = [OutActor1, OutActor2](AActor* RigidBodyActor)
	{
		if (OutActor1 && *OutActor1 == nullptr)
		{
			*OutActor1 = RigidBodyActor;
		}
		// Making sure same actor is not used for both OutActors.
		else if (OutActor2 && *OutActor2 == nullptr && (!OutActor1 || *OutActor1 != RigidBodyActor))
		{
			*OutActor2 = RigidBodyActor;
		}

		return (OutActor1 && *OutActor1 == nullptr) || (OutActor2 && *OutActor2 == nullptr);
	};

	// Search the selected actors fpr matching actors. Doing this step completely before
	// start searching in subtrees, in case selected actors are in each others subtrees.
	for (int32 i = 0; i < SelectedActors->Num(); ++i)
	{
		if (AActor* SelectedActor = Cast<AActor>(SelectedActors->GetSelectedObject(i)))
		{
			if (UAGX_RigidBodyComponent::GetFirstFromActor(SelectedActor))
			{
				// Found one. Assign it to next available OutActor!
				if (!AssignOutActors(SelectedActor))
				{
					return; // return if no more available OutActors
				}
			}
		}
	}

	// Search each selected actor's subtree for matching actors. Only one matching actor
	// allowed per selected actor subtree.
	if (bSearchSubtrees)
	{
		for (int32 i = 0; i < SelectedActors->Num(); ++i)
		{
			if (AActor* SelectedActor = Cast<AActor>(SelectedActors->GetSelectedObject(i)))
			{
				AActor* RigidBodyActor =
					GetRigidBodyActorFromSubtree(SelectedActor, (OutActor1 ? *OutActor1 : nullptr));

				// Found one. Assign it to next available OutActor!
				if (!AssignOutActors(RigidBodyActor))
				{
					return; // return if no more available OutActors
				}
			}
		}
	}

	// Search each selected actor's ancestor chain for matching actors. Only one matching actor
	// allowed per selected actor ancestor chain.
	if (bSearchAncestors)
	{
		for (int32 i = 0; i < SelectedActors->Num(); ++i)
		{
			if (AActor* SelectedActor = Cast<AActor>(SelectedActors->GetSelectedObject(i)))
			{
				AActor* RigidBodyActor = GetRigidBodyActorFromAncestors(
					SelectedActor, (OutActor1 ? *OutActor1 : nullptr));

				// Found one. Assign it to next available OutActor!
				if (!AssignOutActors(RigidBodyActor))
				{
					return; // return if no more available OutActors
				}
			}
		}
	}
}

AActor* FAGX_EditorUtilities::GetRigidBodyActorFromSubtree(
	AActor* SubtreeRoot, const AActor* IgnoreActor)
{
	AActor* RigidBodyActor = nullptr;

	if (SubtreeRoot)
	{
		if (SubtreeRoot != IgnoreActor && UAGX_RigidBodyComponent::GetFirstFromActor(SubtreeRoot))
		{
			RigidBodyActor = SubtreeRoot; // found it
		}
		else
		{
			TArray<AActor*> AttachedActors;
			SubtreeRoot->GetAttachedActors(AttachedActors);

			for (AActor* AttachedActor : AttachedActors)
			{
				RigidBodyActor = GetRigidBodyActorFromSubtree(AttachedActor, IgnoreActor);

				if (RigidBodyActor)
				{
					break; // found it
				}
			}
		}
	}

	return RigidBodyActor;
}

AActor* FAGX_EditorUtilities::GetRigidBodyActorFromAncestors(
	AActor* Actor, const AActor* IgnoreActor)
{
	AActor* RigidBodyActor = nullptr;

	if (Actor)
	{
		if (Actor != IgnoreActor && UAGX_RigidBodyComponent::GetFirstFromActor(Actor))
		{
			RigidBodyActor = Actor;
		}
		else
		{
			RigidBodyActor =
				GetRigidBodyActorFromAncestors(Actor->GetAttachParentActor(), IgnoreActor);
		}
	}

	return RigidBodyActor;
}

void FAGX_EditorUtilities::GetAllClassesOfType(
	TArray<UClass*>& OutMatches, UClass* BaseClass, bool bIncludeAbstract)
{
	for (TObjectIterator<UClass> ClassItr; ClassItr; ++ClassItr)
	{
		UClass* Class = *ClassItr;

		if (Class && Class->IsChildOf(BaseClass))
		{
			if (bIncludeAbstract || !Class->HasAnyClassFlags(CLASS_Abstract))
			{
				OutMatches.Add(Class);
			}
		}
	}
}

EVisibility FAGX_EditorUtilities::VisibleIf(bool bVisible)
{
	return bVisible ? EVisibility::Visible : EVisibility::Collapsed;
}

FString FAGX_EditorUtilities::SelectExistingFileDialog(
	const FString& FileDescription, const FString& FileExtension, const FString& InStartDir)
{
	const FString DialogTitle = FString("Select a ") + FileDescription;
	const FString FileTypes = FileDescription + FString("|*") + FileExtension;
	const FString StartDir = InStartDir.IsEmpty() ? FString("DefaultPath") : InStartDir;
	// For a discussion on window handles see
	// https://answers.unrealengine.com/questions/395516/opening-a-file-dialog-from-a-plugin.html
	TArray<FString> Filenames;
	bool FileSelected = FDesktopPlatformModule::Get()->OpenFileDialog(
		nullptr, DialogTitle, StartDir, TEXT("DefaultFile"), FileTypes, EFileDialogFlags::None,
		Filenames);
	if (!FileSelected || Filenames.Num() == 0)
	{
		UE_LOG(LogAGX, Log, TEXT("No %s file selected. Doing nothing."), *FileExtension);
		return "";
	}
	if (Filenames.Num() > 1)
	{
		FAGX_NotificationUtilities::ShowNotification(
			"Multiple files selected but but only single file selection supported. Doing "
			"nothing.",
			SNotificationItem::CS_None);
		return "";
	}
	return ToReadablePath(*Filenames[0]);
}

FString FAGX_EditorUtilities::SelectExistingDirectoryDialog(
	const FString& DialogTitle, const FString& InStartDir, bool AllowNoneSelected)
{
	const FString StartDir = InStartDir.IsEmpty() ? FString("DefaultPath") : InStartDir;
	FString DirectoryPath("");
	bool DirectorySelected = FDesktopPlatformModule::Get()->OpenDirectoryDialog(
		nullptr, DialogTitle, StartDir, DirectoryPath);

	if (!AllowNoneSelected && (!DirectorySelected || DirectoryPath.IsEmpty()))
	{
		UE_LOG(LogAGX, Log, TEXT("No directory selected. Doing nothing."));
		return "";
	}

	return DirectoryPath;
}

FString FAGX_EditorUtilities::SelectNewFileDialog(
	const FString& DialogTitle, const FString& FileTypes, const FString& DefaultFile,
	const FString& InStartDir)
{
	TArray<FString> Filenames;
	bool FileSelected = FDesktopPlatformModule::Get()->SaveFileDialog(
		nullptr, DialogTitle, InStartDir, DefaultFile, FileTypes, EFileDialogFlags::None,
		Filenames);
	if (!FileSelected || Filenames.Num() == 0)
	{
		UE_LOG(LogAGX, Warning, TEXT("No file selected, doing nothing."));
		return "";
	}

	if (Filenames.Num() > 1)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Multiple files selected but we only support selecting one. Doing nothing."));
		FAGX_NotificationUtilities::ShowNotification(
			"Multiple files selected but we only support single files for now. Doing "
			"nothing.",
			SNotificationItem::CS_None);
		return "";
	}

	const FString Filename = Filenames[0];
	if (Filename.IsEmpty())
	{
		UE_LOG(LogAGX, Warning, TEXT("Selected file has empty file name. Doing nothing."));
		return "";
	}

	return FPaths::ConvertRelativePathToFull(Filename);
}

FString FAGX_EditorUtilities::SelectNewAssetDialog(
	UClass* SavedClass, const FString& InDefaultPath, const FString& AssetNameSuggestion,
	const FString& DialogTitle)
{
	if (SavedClass == nullptr)
		return "";

	const FString DefaultPath = InDefaultPath.IsEmpty() ? FString("/Game") : InDefaultPath;

	const FSaveAssetDialogConfig SaveAssetDialogConfig = [&]()
	{
		FSaveAssetDialogConfig Config;
		Config.DefaultPath = DefaultPath;
		Config.DefaultAssetName = AssetNameSuggestion;
		Config.AssetClassNames.Add(SavedClass->GetClassPathName());
		Config.ExistingAssetPolicy = ESaveAssetDialogExistingAssetPolicy::Disallow;
		Config.DialogTitleOverride = FText::FromString(DialogTitle);
		return Config;
	}();

	FContentBrowserModule& ContentBrowserModule =
		FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	FString SaveObjectPath =
		ContentBrowserModule.Get().CreateModalSaveAssetDialog(SaveAssetDialogConfig);

	if (SaveObjectPath.IsEmpty())
		return "";

	return FPackageName::ObjectPathToPackageName(SaveObjectPath);
}

void FAGX_EditorUtilities::SaveAndCompile(UBlueprint& Blueprint)
{
	FKismetEditorUtilities::CompileBlueprint(&Blueprint);
	FAGX_ObjectUtilities::SaveAsset(Blueprint);
}

FString FAGX_EditorUtilities::GetRelativePath(const FString& BasePath, FString FullPath)
{
	FullPath.RemoveFromStart(BasePath);
	FullPath.RemoveFromStart("/");
	return FullPath;
}

#undef LOCTEXT_NAMESPACE
