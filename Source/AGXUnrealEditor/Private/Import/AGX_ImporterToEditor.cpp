// Copyright 2024, Algoryx Simulation AB.

#include "Import/AGX_ImporterToEditor.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_ObserverFrameComponent.h"
#include "AGX_RigidBodyComponent.h"
#include "AMOR/AGX_ShapeContactMergeSplitThresholds.h"
#include "CollisionGroups/AGX_CollisionGroupDisablerComponent.h"
#include "Constraints/AGX_ConstraintComponent.h"
#include "Import/AGX_ImportContext.h"
#include "Import/AGX_Importer.h"
#include "Import/AGX_ImportSettings.h"
#include "Import/AGX_ModelSourceComponent.h"
#include "Import/AGX_SCSNodeCollection.h"
#include "Materials/AGX_ContactMaterial.h"
#include "Materials/AGX_ContactMaterialRegistrarComponent.h"
#include "Materials/AGX_ShapeMaterial.h"
#include "OpenPLX/PLX_SignalHandlerComponent.h"
#include "Shapes/AGX_ShapeComponent.h"
#include "Terrain/AGX_ShovelComponent.h"
#include "Terrain/AGX_ShovelProperties.h"
#include "Terrain/ShovelBarrier.h"
#include "Tires/AGX_TwoBodyTireComponent.h"
#include "Utilities/AGX_BlueprintUtilities.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_EditorUtilities.h"
#include "Utilities/AGX_ImportRuntimeUtilities.h"
#include "Utilities/AGX_ImportUtilities.h"
#include "Utilities/AGX_MeshUtilities.h"
#include "Utilities/AGX_ObjectUtilities.h"
#include "Utilities/PLXUtilities.h"
#include "Vehicle/AGX_TrackComponent.h"
#include "Vehicle/AGX_TrackInternalMergeProperties.h"
#include "Vehicle/AGX_TrackProperties.h"
#include "Vehicle/TrackBarrier.h"
#include "Wire/AGX_WireComponent.h"

// Unreal Engine includes.
#include "Components/StaticMeshComponent.h"
#include "Editor.h"
#include "Engine/SCS_Node.h"
#include "FileHelpers.h"
#include "Materials/MaterialInterface.h"
#include "Misc/ScopedSlowTask.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "PackageTools.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "UObject/MetaData.h"

// Standard library includes.
#include <type_traits>

namespace AGX_ImporterToEditor_helpers
{
	void PreCreateBlueprintSetup()
	{
		GEditor->SelectNone(false, false);
	}

	void CloseAssetEditors()
	{
		// During Model reimport, old assets are deleted and references to these assets are
		// automatically cleared. Having the Blueprint Editor opened while doing this causes
		// crashing during this process and the exact reason why is not clear. So we solve this
		// by closing all asset editors here first.
		if (GEditor != nullptr)
			GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->CloseAllAssetEditors();
	}

	FString MakeModelName(FString SourceFilename)
	{
		return FAGX_EditorUtilities::SanitizeName(SourceFilename, "ImportedModel");
	}

	FString MakeRootDirectoryPath(const FString ModelName)
	{
		const FString ImportDirPath =
			FString::Printf(TEXT("/Game/%s/"), *FAGX_ImportUtilities::GetImportRootDirectoryName());
		FString BasePath = FAGX_ImportUtilities::CreatePackagePath(ImportDirPath, ModelName, false);

		auto PackageExists = [&](const FString& DirPath)
		{
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
		return DirectoryPath;
	}

	// When saving an asset to disk, Unreal sometimes creates a redirector object with
	// TransientPackage as owner. This object may cause name collisions on multiple imports done in
	// a row because it may linger between imports.
	void DestroyRedirectorAfterSave(UObject* SavedAsset, const FAGX_ImportContext& Context)
	{
		if (SavedAsset == nullptr)
			return;

		auto RedirectorObj =
			StaticFindObjectFast(UObject::StaticClass(), Context.Outer, *SavedAsset->GetName());
		if (RedirectorObj != nullptr && RedirectorObj != SavedAsset)
			RedirectorObj->ConditionalBeginDestroy();
	}

	template <typename T>
	FString GetAssetTypeFromType(const UObject& Asset)
	{
		if constexpr (std::is_same_v<T, UAGX_MergeSplitThresholdsBase>)
			return FAGX_ImportUtilities::GetImportMergeSplitThresholdsDirectoryName();

		if constexpr (std::is_same_v<T, UStaticMesh>)
		{
			if (Asset.GetName().StartsWith("SM_RenderMesh_"))
				return FAGX_ImportUtilities::GetImportRenderStaticMeshDirectoryName();
			else
				return FAGX_ImportUtilities::GetImportCollisionStaticMeshDirectoryName();
		}

		if constexpr (std::is_same_v<T, UMaterialInterface>)
			return FAGX_ImportUtilities::GetImportRenderMaterialDirectoryName();

		if constexpr (std::is_same_v<T, UMaterialInstanceDynamic>)
			return FAGX_ImportUtilities::GetImportRenderMaterialDirectoryName();

		if constexpr (std::is_same_v<T, UAGX_ShapeMaterial>)
			return FAGX_ImportUtilities::GetImportShapeMaterialDirectoryName();

		if constexpr (std::is_same_v<T, UAGX_ContactMaterial>)
			return FAGX_ImportUtilities::GetImportContactMaterialDirectoryName();

		if constexpr (std::is_same_v<T, UAGX_ShovelProperties>)
			return FAGX_ImportUtilities::GetImportShovelPropertiesDirectoryName();

		if constexpr (std::is_same_v<T, UAGX_TrackProperties>)
			return FAGX_ImportUtilities::GetImportTrackPropertiesDirectoryName();

		if constexpr (std::is_same_v<T, UAGX_TrackInternalMergeProperties>)
			return FAGX_ImportUtilities::GetImportTrackMergePropertiesDirectoryName();
	}

	FString GetModelDirectoryPathFromBaseBlueprint(UBlueprint& BaseBP)
	{
		const FString ParentDir = FPaths::GetPath(BaseBP.GetPathName());
		if (!FPaths::GetBaseFilename(ParentDir).Equals("Blueprint"))
		{
			return "";
		}

		return FPaths::GetPath(ParentDir);
	}

	FString CreateBlueprintPackagePath(
		const FString& RootDir, const FString& ModelName, bool IsBase)
	{
		const FString SubDirectory = IsBase ? "Blueprint" : "";
		FString ParentPackagePath = FAGX_ImportUtilities::CreatePackagePath(RootDir, SubDirectory);
		UPackage* ParentPackage = CreatePackage(*ParentPackagePath);
		const FString Path = FPaths::GetPath(ParentPackage->GetName());

		// Create a known unique name for the Blueprint package, but don't create the actual
		// package yet.
		const FString BlueprintName =
			IsBase ? FAGX_ImportUtilities::GetImportBaseBlueprintNamePrefix() +
						 FGuid::NewGuid().ToString()
				   : TEXT("BP_") + ModelName;
		FString BasePackagePath = UPackageTools::SanitizePackageName(Path + "/" + BlueprintName);
		FString PackagePath = BasePackagePath;

		return PackagePath;
	}

	UPackage* GetPackage(const FString& BlueprintPackagePath)
	{
		UPackage* Package = CreatePackage(*BlueprintPackagePath);
		check(Package != nullptr);
		Package->FullyLoad();
		return Package;
	}

	UBlueprint* CreateBaseBlueprint(
		const FString& RootDir, const FString& ModelName, AActor& Template)
	{
		PreCreateBlueprintSetup();
		FString BlueprintPackagePath = CreateBlueprintPackagePath(RootDir, ModelName, true);
		UPackage* Package = GetPackage(BlueprintPackagePath);
		static constexpr bool ReplaceInWorld = false;
		static constexpr bool KeepMobility = true;
		FKismetEditorUtilities::FCreateBlueprintFromActorParams Params;
		Params.bReplaceActor = ReplaceInWorld;
		Params.bKeepMobility = KeepMobility;
		Params.bOpenBlueprint = false;

		UBlueprint* Blueprint =
			FKismetEditorUtilities::CreateBlueprintFromActor(Package->GetName(), &Template, Params);
		check(Blueprint);

		FAGX_ObjectUtilities::SaveAsset(*Blueprint);
		return Blueprint;
	}

	UBlueprint* CreateChildBlueprint(
		const FString& RootDir, const FString& ModelName, UBlueprint& BaseBlueprint)
	{
		PreCreateBlueprintSetup();
		FString BlueprintPackagePath = CreateBlueprintPackagePath(RootDir, ModelName, false);
		UPackage* Package = GetPackage(BlueprintPackagePath);
		const FString AssetName = FPaths::GetBaseFilename(Package->GetName());

		UBlueprint* BlueprintChild = FKismetEditorUtilities::CreateBlueprint(
			BaseBlueprint.GeneratedClass, Package, FName(AssetName), EBlueprintType::BPTYPE_Normal,
			UBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass(),
			FName("AGXUnrealImport"));

		check(BlueprintChild);
		FAGX_ObjectUtilities::SaveAsset(*BlueprintChild);
		return BlueprintChild;
	}

	/**
	 * Important, the TransientToAsset lookup only works for properties not inside arrays.
	 */
	template <typename TOverwriteRuleFunc>
	void CopyPropertyRecursive(
		const void* Archetype, const void* Source, void* OutDest, const FProperty* Property,
		const TMap<UObject*, UObject*>& TransientToAsset, TOverwriteRuleFunc OverwriteRule)
	{
		if (Property == nullptr || !Property->HasAnyPropertyFlags(CPF_Edit))
			return;

		const void* ArchetypeVal = Property->ContainerPtrToValuePtr<void>(Archetype);
		const void* SourceVal = Property->ContainerPtrToValuePtr<void>(Source);
		void* DestVal = Property->ContainerPtrToValuePtr<void>(OutDest);

		if (Property->Identical(SourceVal, DestVal))
			return; // Nothing to do, already equal.

		if (const FObjectProperty* ObjectProperty = CastField<FObjectProperty>(Property))
		{
			UObject* SourceObject = ObjectProperty->GetObjectPropertyValue(SourceVal);
			if (UObject* Asset = TransientToAsset.FindRef(SourceObject))
			{
				const bool ShouldCopy =
					Archetype == OutDest || OverwriteRule(Property, ArchetypeVal, DestVal);

				if (ShouldCopy)
					ObjectProperty->SetObjectPropertyValue(DestVal, Asset);

				return;
			}
		}
		else if (const FStructProperty* StructProperty = CastField<FStructProperty>(Property))
		{
			const void* StructArchetype = StructProperty->ContainerPtrToValuePtr<void>(Archetype);
			const void* StructSource = StructProperty->ContainerPtrToValuePtr<void>(Source);
			void* StructDest = StructProperty->ContainerPtrToValuePtr<void>(OutDest);

			// Recursively copy properties within the struct.
			for (TFieldIterator<FProperty> StructPropIt(StructProperty->Struct); StructPropIt;
				 ++StructPropIt)
			{
				FProperty* StructPropertyField = *StructPropIt;
				CopyPropertyRecursive(
					StructArchetype, StructSource, StructDest, StructPropertyField,
					TransientToAsset, OverwriteRule);
			}
			return;
		}

		const bool ShouldCopy =
			Archetype == OutDest || OverwriteRule(Property, ArchetypeVal, DestVal);
		if (ShouldCopy)
			Property->CopyCompleteValue(DestVal, SourceVal);
	}

	// Similar to FAGX_ObjectUtilities::CopyProperties, but with some special handling for the
	// TransientToAsset map and overwrite rule.
	template <typename TOverwriteRuleFunc>
	bool CopyProperties(
		const UObject& Source, UObject& OutDest, const TMap<UObject*, UObject*>& TransientToAsset,
		TOverwriteRuleFunc OverwriteRule)
	{
		UClass* Class = Source.GetClass();
		if (OutDest.GetClass() != Class)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Tried to copy properties from object '%s' of type '%s' to object '%s' of "
					 "type '%s'. Types must match."),
				*Source.GetName(), *Source.GetClass()->GetName(), *OutDest.GetName(),
				*OutDest.GetClass()->GetName());
			return false;
		}

		TArray<UObject*> ArchetypeInstances;
		OutDest.GetArchetypeInstances(ArchetypeInstances);
		for (UObject* Instance : ArchetypeInstances)
		{
			for (TFieldIterator<FProperty> PropIt(Class); PropIt; ++PropIt)
			{
				const FProperty* Property = *PropIt;
				CopyPropertyRecursive(
					&OutDest, &Source, Instance, Property, TransientToAsset, OverwriteRule);
			}
		}

		for (TFieldIterator<FProperty> PropIt(Class); PropIt; ++PropIt)
		{
			const FProperty* Property = *PropIt;
			CopyPropertyRecursive(
				&OutDest, &Source, &OutDest, Property, TransientToAsset, OverwriteRule);
		}

		return true;
	}

	bool DefaultOverwriteRule(
		const FProperty* Property, const void* ArchetypeVal, const void* DestinationVal)
	{
		return Property->Identical(ArchetypeVal, DestinationVal);
	}

	bool ForceOverwriteRule(
		const FProperty* Property, const void* ArchetypeVal, const void* DestinationVal)
	{
		return true;
	}

	bool RenderMaterialOverwriteRule(
		const FProperty* Property, const void* ArchetypeVal, const void* DestinationVal)
	{
		if (const FObjectProperty* ObjectProperty = CastField<FObjectProperty>(Property))
		{
			if (ObjectProperty->PropertyClass->IsChildOf(UMaterialInterface::StaticClass()))
				return true; // Always overwrite material properties
		}
		else if (const FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property))
		{
			if (const FObjectProperty* InnerObjectProperty =
					CastField<FObjectProperty>(ArrayProperty->Inner))
			{
				if (InnerObjectProperty->PropertyClass->IsChildOf(
						UMaterialInterface::StaticClass()))
					return true; // Always overwrite arrays of material properties.
			}
		}

		// Default behavior for non-material properties.
		return DefaultOverwriteRule(Property, ArchetypeVal, DestinationVal);
	}

	template <typename T>
	void FixupRenderMaterialImpl(const TMap<UObject*, UObject*>& TransientToAsset, T& OutMesh)
	{
		auto CurrMat = OutMesh.GetMaterial(0);
		UMaterialInterface* Asset = Cast<UMaterialInterface>(TransientToAsset.FindRef(CurrMat));
		if (Asset != nullptr && CurrMat != Asset)
			OutMesh.SetMaterial(0, Asset);
	}

	// Because CopyProperties does not handle TransientToAsset objects inside arrays.
	template <typename T>
	void FixupRenderMaterial(const TMap<UObject*, UObject*>& TransientToAsset, T& OutMesh)
	{
		for (auto& Instance : FAGX_ObjectUtilities::GetArchetypeInstances(OutMesh))
			FixupRenderMaterialImpl(TransientToAsset, *Instance);

		FixupRenderMaterialImpl(TransientToAsset, OutMesh);
	}

	void FixupContactMaterial(
		const TMap<UObject*, UObject*>& TransientToAsset, UAGX_ContactMaterial& Cm)
	{
		auto CurrMat1 = Cm.Material1;
		auto CurrMat2 = Cm.Material2;
		auto AssetMat1 = Cast<UAGX_ShapeMaterial>(TransientToAsset.FindRef(CurrMat1));
		auto AssetMat2 = Cast<UAGX_ShapeMaterial>(TransientToAsset.FindRef(CurrMat2));
		if (AssetMat1 != nullptr && CurrMat1 != AssetMat1)
			Cm.Material1 = AssetMat1;

		if (AssetMat2 != nullptr && CurrMat2 != AssetMat2)
			Cm.Material2 = AssetMat2;
	}

	void FixupContactMaterialsImpl(
		const TMap<UObject*, UObject*>& TransientToAsset,
		UAGX_ContactMaterialRegistrarComponent& OutComp)
	{
		for (int32 i = 0; i < OutComp.ContactMaterials.Num(); i++)
		{
			if (auto Cm = TransientToAsset.FindRef(OutComp.ContactMaterials[i]))
				OutComp.ContactMaterials[i] = Cast<UAGX_ContactMaterial>(Cm);
		}
	}

	void FixupContactMaterials(
		const TMap<UObject*, UObject*>& TransientToAsset,
		UAGX_ContactMaterialRegistrarComponent& OutComp)
	{
		for (auto& Instance : FAGX_ObjectUtilities::GetArchetypeInstances(OutComp))
			FixupContactMaterialsImpl(TransientToAsset, *Instance);

		FixupContactMaterialsImpl(TransientToAsset, OutComp);
	}

	bool ValidateImportEnum(EAGX_ImportResult Result)
	{
		if (IsUnrecoverableError(Result))
		{
			const FString Text =
				FString::Printf(TEXT("Errors occurred during import and the result may not be the "
									 "expected result. Log category LogAGX in the Output Log may "
									 "contain more information."));
			FAGX_NotificationUtilities::ShowNotification(Text, SNotificationItem::CS_Fail);
			return false;
		}

		if (Result != EAGX_ImportResult::Success)
		{
			const FString Text = FString::Printf(
				TEXT("Some issues occurred during import and the result may not be the "
					 "expected result. Log category LogAGX in the Output Log may "
					 "contain more information."));
			FAGX_NotificationUtilities::ShowNotification(Text, SNotificationItem::CS_Fail);
			return false;
		}

		return true;
	}

	bool ValidateImportResult(const FAGX_ImportResult& Result, const FAGX_ImportSettings& Settings)
	{
		if (Result.Actor == nullptr)
		{
			const FString Text = FString::Printf(
				TEXT("Errors occurred during import. The file '%s' could not be imported. Log "
					 "category LogAGX in the Output Log may contain more information."),
				*Settings.FilePath);
			FAGX_NotificationUtilities::ShowDialogBoxWithError(
				Text, "Import Model to Blueprint");
			return false;
		}

		return ValidateImportEnum(Result.Result);
	}

	bool ValidateReimportSettings(const FAGX_ReimportSettings& Settings)
	{
		auto IsReimportSupported = [&]()
		{
			return Settings.ImportType == EAGX_ImportType::Agx ||
				   Settings.ImportType == EAGX_ImportType::Plx;
		};

		if (!IsReimportSupported())
		{
			const FString Text =
				FString::Printf(TEXT("Reimport is only supported for AGX Archives (.agx) and "
									 "OpenPLX (.openplx) files."));
			FAGX_NotificationUtilities::ShowDialogBoxWithError(Text, "Reimport model");
			return false;
		}

		return true;
	}

	bool ValidateBlueprintForReimport(const UBlueprint& Bp)
	{
		if (Bp.SimpleConstructionScript->FindSCSNode(TEXT("AGX_ModelSource")) == nullptr)
		{
			const FString Text = FString::Printf(TEXT(
				"Unable to find a valid AGX Mode Source Component in the selected Blueprint."));
			FAGX_NotificationUtilities::ShowNotification(Text, SNotificationItem::CS_Fail);
			return false;
		}

		return true;
	}

	void RemoveDeletedComponents(UBlueprint& Blueprint, const FGuid& SessionGuid)
	{
		auto HasMatchingTag = [&SessionGuid](const UActorComponent& Component)
		{
			if (Component.ComponentTags.Num() == 0)
				return false;

			return FGuid(Component.ComponentTags[0].ToString()) == SessionGuid;
		};

		// Any Component that does not have the current SessionGuid tag, we know is not part of the
		// newly imported model and should be removed.
		const TArray<USCS_Node*> Nodes = Blueprint.SimpleConstructionScript->GetAllNodes();
		for (USCS_Node* Node : Nodes)
		{
			if (Node == nullptr || Node->ComponentTemplate == nullptr)
			{
				AGX_CHECK(false);
				continue;
			}

			if (!HasMatchingTag(*Node->ComponentTemplate))
				Blueprint.SimpleConstructionScript->RemoveNodeAndPromoteChildren(Node);
		}
	}

	bool HasMatchingSessionGuid(const UObject& Object, const FGuid& SessionGuid)
	{
		UMetaData* MetaData = Object.GetOutermost()->GetMetaData();
		const FString GuidStr = MetaData->GetValue(&Object, TEXT("AGX_ImportSessionGuid"));
		return FGuid(GuidStr) == SessionGuid;
	}

	void RemoveDeletedAssets(const FString& RootDirectory, const FGuid& SessionGuid)
	{
		TArray<UObject*> AssetsToDelete;

		auto CollectForRemoval = [&](auto Assets)
		{
			for (auto Asset : Assets)
			{
				if (!HasMatchingSessionGuid(*Asset, SessionGuid))
					AssetsToDelete.Add(Asset);
			}
		};

		CollectForRemoval(FAGX_EditorUtilities::FindAssets<UAGX_ShapeMaterial>(FPaths::Combine(
			RootDirectory, FAGX_ImportUtilities::GetImportShapeMaterialDirectoryName())));

		CollectForRemoval(FAGX_EditorUtilities::FindAssets<UAGX_ContactMaterial>(FPaths::Combine(
			RootDirectory, FAGX_ImportUtilities::GetImportContactMaterialDirectoryName())));

		CollectForRemoval(FAGX_EditorUtilities::FindAssets<UMaterialInterface>(FPaths::Combine(
			RootDirectory, FAGX_ImportUtilities::GetImportRenderMaterialDirectoryName())));

		CollectForRemoval(FAGX_EditorUtilities::FindAssets<UStaticMesh>(FPaths::Combine(
			RootDirectory, FAGX_ImportUtilities::GetImportRenderStaticMeshDirectoryName())));

		CollectForRemoval(FAGX_EditorUtilities::FindAssets<UStaticMesh>(FPaths::Combine(
			RootDirectory, FAGX_ImportUtilities::GetImportCollisionStaticMeshDirectoryName())));

		CollectForRemoval(
			FAGX_EditorUtilities::FindAssets<UAGX_MergeSplitThresholdsBase>(FPaths::Combine(
				RootDirectory,
				FAGX_ImportUtilities::GetImportMergeSplitThresholdsDirectoryName())));

		CollectForRemoval(FAGX_EditorUtilities::FindAssets<UAGX_ShovelProperties>(FPaths::Combine(
			RootDirectory, FAGX_ImportUtilities::GetImportShovelPropertiesDirectoryName())));

		CollectForRemoval(FAGX_EditorUtilities::FindAssets<UAGX_TrackProperties>(FPaths::Combine(
			RootDirectory, FAGX_ImportUtilities::GetImportTrackPropertiesDirectoryName())));

		CollectForRemoval(
			FAGX_EditorUtilities::FindAssets<UAGX_TrackInternalMergeProperties>(FPaths::Combine(
				RootDirectory,
				FAGX_ImportUtilities::GetImportTrackMergePropertiesDirectoryName())));

		FAGX_EditorUtilities::DeleteImportedAssets(AssetsToDelete);
	}

	void WriteAssetToDisk(
		const FString& RootDir, const FString& AssetType, UObject& Object,
		const FAGX_ImportContext& Context)
	{
		const FString AssetName = Object.GetName();
		const FString PackagePath =
			FPaths::Combine(FAGX_ImportUtilities::CreatePackagePath(RootDir, AssetType), AssetName);

		// Handle asset with the name we want to use that is not part of this import session.
		// If existing, rename it to an unset name. It will likely be removed at the end of the
		// import/reimport process.
		UObject* ExistingAsset = FAGX_ObjectUtilities::GetAssetFromPath<UObject>(*PackagePath);
		if (ExistingAsset != nullptr &&
			!HasMatchingSessionGuid(*ExistingAsset, Context.SessionGuid))
		{
			auto ExistingOuter = ExistingAsset->GetOuter();
			ExistingAsset->Rename(*FAGX_ImportUtilities::GetUnsetUniqueImportName());
			auto RedirectorObj =
				StaticFindObjectFast(UObject::StaticClass(), ExistingOuter, *AssetName);
			if (RedirectorObj != nullptr)
				RedirectorObj->ConditionalBeginDestroy();
		}

		UPackage* Package = CreatePackage(*PackagePath);
		Object.Rename(*AssetName, Package);
		Package->MarkPackageDirty();
		Object.SetFlags(RF_Public | RF_Standalone);

		FAGX_ObjectUtilities::SaveAsset(Object);
		DestroyRedirectorAfterSave(&Object, Context);
	}

	void WriteAssetsToDisk(const FString& RootDir, const FAGX_ImportContext* Context)
	{
		if (Context == nullptr)
			return;

		if (Context->MSThresholds != nullptr)
		{
			const FString AssetType =
				FAGX_ImportUtilities::GetImportMergeSplitThresholdsDirectoryName();
			for (const auto& [Guid, MST] : *Context->MSThresholds)
			{
				WriteAssetToDisk(RootDir, AssetType, *MST, *Context);
			}
		}

		if (Context->RenderMaterials != nullptr)
		{
			const FString AssetType = FAGX_ImportUtilities::GetImportRenderMaterialDirectoryName();
			for (const auto& [Guid, Rm] : *Context->RenderMaterials)
			{
				WriteAssetToDisk(RootDir, AssetType, *Rm, *Context);
			}
		}

		if (Context->RenderStaticMeshes != nullptr)
		{
			const FString AssetType =
				FAGX_ImportUtilities::GetImportRenderStaticMeshDirectoryName();
			for (const auto& [Guid, Sm] : *Context->RenderStaticMeshes)
			{
				WriteAssetToDisk(RootDir, AssetType, *Sm, *Context);
			}
		}

		if (Context->CollisionStaticMeshes != nullptr)
		{
			const FString AssetType =
				FAGX_ImportUtilities::GetImportCollisionStaticMeshDirectoryName();
			for (const auto& [Guid, Sm] : *Context->CollisionStaticMeshes)
			{
				WriteAssetToDisk(RootDir, AssetType, *Sm, *Context);
			}
		}

		if (Context->ShapeMaterials != nullptr)
		{
			const FString AssetType = FAGX_ImportUtilities::GetImportShapeMaterialDirectoryName();
			for (const auto& [Guid, Sm] : *Context->ShapeMaterials)
			{
				WriteAssetToDisk(RootDir, AssetType, *Sm, *Context);
			}
		}

		if (Context->ContactMaterials != nullptr)
		{
			const FString AssetType = FAGX_ImportUtilities::GetImportContactMaterialDirectoryName();
			for (const auto& [Guid, Cm] : *Context->ContactMaterials)
			{
				WriteAssetToDisk(RootDir, AssetType, *Cm, *Context);
			}
		}

		if (Context->ShovelProperties != nullptr)
		{
			const FString AssetType =
				FAGX_ImportUtilities::GetImportShovelPropertiesDirectoryName();
			for (const auto& [Guid, Sp] : *Context->ShovelProperties)
			{
				WriteAssetToDisk(RootDir, AssetType, *Sp, *Context);
			}
		}

		if (Context->TrackProperties != nullptr)
		{
			const FString AssetType = FAGX_ImportUtilities::GetImportTrackPropertiesDirectoryName();
			for (const auto& [Guid, Tp] : *Context->TrackProperties)
			{
				WriteAssetToDisk(RootDir, AssetType, *Tp, *Context);
			}
		}

		if (Context->TrackMergeProperties != nullptr)
		{
			const FString AssetType =
				FAGX_ImportUtilities::GetImportTrackMergePropertiesDirectoryName();
			for (const auto& [Guid, Tp] : *Context->TrackMergeProperties)
			{
				WriteAssetToDisk(RootDir, AssetType, *Tp, *Context);
			}
		}
	}

	template <typename T, typename = void>
	struct HasImportGuid : std::false_type
	{
	};

	template <typename T>
	struct HasImportGuid<T, std::void_t<decltype(std::declval<T>().ImportGuid)>> : std::true_type
	{
	};

	/**
	 * Given a Component that has been reimported, but is not part of the Blueprint that the
	 * SCSNodeCollection represents, this function tries to find the USCS_Node corresponding to the
	 * reimported Component's attach parent.
	 * Some Components will always be attached to root, and we can use that fact to simplify this
	 * function.
	 */
	template <typename TComponent>
	USCS_Node* GetCorrespondingAttachParent(
		const UBlueprint& Bp, const FAGX_SCSNodeCollection& Nodes,
		const TComponent& ReimportedComponent)
	{
		USceneComponent* Parent = ReimportedComponent.GetAttachParent();
		AGX_CHECK(Parent != nullptr);
		if (Parent == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Found no attach parent to reimported Component '%s', returning root."),
				*ReimportedComponent.GetName());
			return Nodes.RootComponent;
		}

		if constexpr (std::is_same_v<TComponent, UAGX_ShapeComponent>)
		{
			// A Shape can have a RigidBody or Root as parent.
			if (auto Body = Cast<UAGX_RigidBodyComponent>(Parent))
				return Nodes.RigidBodies.FindRef(Body->ImportGuid);
		}

		if constexpr (std::is_same_v<TComponent, UAGX_ObserverFrameComponent>)
		{
			// An observer frame always have a RigidBody as parent.
			auto Body = Cast<UAGX_RigidBodyComponent>(Parent);
			AGX_CHECK(Body != nullptr);
			if (Body != nullptr)
				return Nodes.RigidBodies.FindRef(Body->ImportGuid);
		}

		if constexpr (std::is_same_v<TComponent, UStaticMeshComponent>)
		{
			// A StaticMesh can have a Shape, RigidBody, StaticMesh or Root as parent.
			if (auto Shape = Cast<UAGX_ShapeComponent>(Parent))
				return Nodes.Shapes.FindRef(Shape->ImportGuid);
			if (auto Body = Cast<UAGX_RigidBodyComponent>(Parent))
				return Nodes.RigidBodies.FindRef(Body->ImportGuid);
			if (auto Smc = Cast<UStaticMeshComponent>(Parent))
			{
				if (auto S = Bp.SimpleConstructionScript->FindSCSNode(*Parent->GetName()))
					return S;
			}
		}

		return Nodes.RootComponent; // Default.
	}

	void DestroyContextOuterOwnedAssets(FAGX_ImportContext& Context)
	{
		auto DestroyIfOwnedByContextOuter = [&Context](UObject* Obj)
		{
			if (!IsValid(Obj) || Obj->HasAnyFlags(RF_BeginDestroyed | RF_FinishDestroyed))
				return;

			if (Obj->GetOuter() == Context.Outer)
				Obj->ConditionalBeginDestroy();
		};

		if (Context.RenderMaterials != nullptr)
		{
			for (auto& [Unused, Obj] : *Context.RenderMaterials)
				DestroyIfOwnedByContextOuter(Obj);
		}

		if (Context.RenderStaticMeshes != nullptr)
		{
			for (auto& [Unused, Obj] : *Context.RenderStaticMeshes)
				DestroyIfOwnedByContextOuter(Obj);
		}

		if (Context.CollisionStaticMeshes != nullptr)
		{
			for (auto& [Unused, Obj] : *Context.CollisionStaticMeshes)
				DestroyIfOwnedByContextOuter(Obj);
		}

		if (Context.MSThresholds != nullptr)
		{
			for (auto& [Unused, Obj] : *Context.MSThresholds)
				DestroyIfOwnedByContextOuter(Obj);
		}

		if (Context.ShapeMaterials != nullptr)
		{
			for (auto& [Unused, Obj] : *Context.ShapeMaterials)
				DestroyIfOwnedByContextOuter(Obj);
		}

		if (Context.ContactMaterials != nullptr)
		{
			for (auto& [Unused, Obj] : *Context.ContactMaterials)
				DestroyIfOwnedByContextOuter(Obj);
		}

		if (Context.ShovelProperties != nullptr)
		{
			for (auto& [Unused, Obj] : *Context.ShovelProperties)
				DestroyIfOwnedByContextOuter(Obj);
		}

		if (Context.TrackProperties != nullptr)
		{
			for (auto& [Unused, Obj] : *Context.TrackProperties)
				DestroyIfOwnedByContextOuter(Obj);
		}

		if (Context.TrackMergeProperties != nullptr)
		{
			for (auto& [Unused, Obj] : *Context.TrackMergeProperties)
				DestroyIfOwnedByContextOuter(Obj);
		}
	}

	template <typename TComponent>
	USCS_Node* FindNodeAndResolveConflicts(
		const FGuid& Guid, const TComponent& ReimportedComponent,
		TMap<FGuid, USCS_Node*>& OutGuidToNode, UBlueprint& OutBlueprint)
	{
		const FName Name(*ReimportedComponent.GetName());
		USCS_Node* Node = OutGuidToNode.FindRef(Guid);

		// Resolve name collisions.
		USCS_Node* NameCollNode = OutBlueprint.SimpleConstructionScript->FindSCSNode(Name);
		if (Node != nullptr && NameCollNode != nullptr && NameCollNode != Node)
			NameCollNode->SetVariableName(*FAGX_ImportUtilities::GetUnsetUniqueImportName());

		return Node;
	}

	template <>
	USCS_Node* FindNodeAndResolveConflicts<UStaticMeshComponent>(
		const FGuid& Guid, const UStaticMeshComponent& ReimportedComponent,
		TMap<FGuid, USCS_Node*>& OutGuidToNode, UBlueprint& OutBlueprint)
	{
		// StaticMeshComponents we look up by using the name which includes the guid.
		// We expect no conflicts.
		const FName Name(*ReimportedComponent.GetName());
		return OutBlueprint.SimpleConstructionScript->FindSCSNode(Name);
	}

	template <>
	USCS_Node* FindNodeAndResolveConflicts<UAGX_ContactMaterialRegistrarComponent>(
		const FGuid& Guid, const UAGX_ContactMaterialRegistrarComponent& ReimportedComponent,
		TMap<FGuid, USCS_Node*>& OutGuidToNode, UBlueprint& OutBlueprint)
	{
		// UAGX_ContactMaterialRegistrarComponent we look up by using the name.
		const FName Name(*ReimportedComponent.GetName());
		return OutBlueprint.SimpleConstructionScript->FindSCSNode(Name);
	}

	template <>
	USCS_Node* FindNodeAndResolveConflicts<UAGX_CollisionGroupDisablerComponent>(
		const FGuid& Guid, const UAGX_CollisionGroupDisablerComponent& ReimportedComponent,
		TMap<FGuid, USCS_Node*>& OutGuidToNode, UBlueprint& OutBlueprint)
	{
		// UAGX_CollisionGroupDisablerComponent we look up by using the name.
		const FName Name(*ReimportedComponent.GetName());
		return OutBlueprint.SimpleConstructionScript->FindSCSNode(Name);
	}

	template <typename TComponent>
	USCS_Node* GetOrCreateNode(
		const FGuid& Guid, const TComponent& ReimportedComponent,
		const FAGX_SCSNodeCollection& Nodes, TMap<FGuid, USCS_Node*>& OutGuidToNode,
		UBlueprint& OutBlueprint)
	{
		const FName Name(*ReimportedComponent.GetName());
		USCS_Node* Node =
			FindNodeAndResolveConflicts(Guid, ReimportedComponent, OutGuidToNode, OutBlueprint);
		if (Node != nullptr &&
			ReimportedComponent.GetClass() != Node->ComponentTemplate->GetClass())
		{
			// The type of the object have changed. Ideally, this should never happen (an object
			// with a GUID has a new type during reimport), but Momentum sometimes does this for
			// shape primitives that may be converted to a Trimesh but keep it's GUID.
			// Setting nullptr here, we say we don't have a valid match, and we will create a
			// completely new object. The old object will be removed automatically during the
			// RemoveDeletedComponents stage.
			OutGuidToNode.Remove(Guid);
			Node = nullptr;
		}

		USCS_Node* Parent = nullptr;
		if constexpr (std::is_base_of_v<USceneComponent, TComponent>)
		{
			Parent = GetCorrespondingAttachParent(OutBlueprint, Nodes, ReimportedComponent);
			AGX_CHECK(Parent != nullptr);
			if (Parent == nullptr)
			{
				UE_LOG(
					LogAGX, Error,
					TEXT("Could not find corresponding attach parent for component '%s'."),
					*ReimportedComponent.GetName());
				return nullptr;
			}
		}

		if (Node == nullptr)
		{
			Node = OutBlueprint.SimpleConstructionScript->CreateNode(
				ReimportedComponent.GetClass(), Name);
			if constexpr (std::is_base_of_v<USceneComponent, TComponent>)
				Parent->AddChildNode(Node);

			AGX_CHECK(OutGuidToNode.FindRef(Guid) == nullptr);
			OutGuidToNode.Add(Guid, Node);
		}
		else // Node existed.
		{
			// We don't need to handle transform explicitly here, it is copied over later to the
			// component template owned by this node.
			if constexpr (std::is_base_of_v<USceneComponent, TComponent>)
				FAGX_BlueprintUtilities::ReParentNode(OutBlueprint, *Node, *Parent, false);

			if (!Node->GetVariableName().IsEqual(Name))
				Node->SetVariableName(Name);
		}

		return Node;
	}

	/**
	 * The FAGX_Importer only creates the Model Source Component but does not populate
	 * its contents since it cannot know the editor only information that is needed by
	 * e.g. render materials.
	 */
	EAGX_ImportResult FinalizeModelSourceComponent(
		const FAGX_ImportContext& Context, const FString& RootDir)
	{
		UAGX_ModelSourceComponent* Component = Context.ModelSourceComponent;

		if (Component == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("FinalizeModelSourceComponent called, but a ModelSourceComponent could not be "
					 "found in the given FAGX_ImportContext."));
			return EAGX_ImportResult::FatalError;
		}

		for (const auto& [Guid, CollisionComponent] : *Context.CollisionStaticMeshCom)
		{
			const FString Name = CollisionComponent->GetName();
			Component->StaticMeshComponentToOwningTrimesh.Add(Name, Guid);
		}

		for (const auto& [Guid, RenderComponent] : *Context.RenderStaticMeshCom)
		{
			const FString Name = RenderComponent->GetName();
			Component->StaticMeshComponentToOwningShape.Add(Name, Guid);
		}

		for (const auto& [Guid, Material] : *Context.RenderMaterials)
		{
			const FString RelativePath = FAGX_EditorUtilities::GetRelativePath(
				FPaths::GetPath(RootDir), Material->GetPathName());

			Component->UnrealMaterialToImportGuid.Add(RelativePath, Guid);
		}

		return EAGX_ImportResult::Success;
	}
}

UBlueprint* FAGX_ImporterToEditor::Import(FAGX_ImportSettings Settings)
{
	using namespace AGX_ImporterToEditor_helpers;

	FScopedSlowTask ImportTask(100.f, FText::FromString("Importing model"), true);
	ImportTask.MakeDialog();

	ImportTask.EnterProgressFrame(10.f, FText::FromString("Pre-import setup"));

	PreImport(Settings);

	ImportTask.EnterProgressFrame(10.f, FText::FromString("Importing from source file"));

	FAGX_Importer Importer;
	FAGX_ImportResult Result = Importer.Import(Settings, *GetTransientPackage());
	if (!ValidateImportResult(Result, Settings))
		return nullptr;

	ImportTask.EnterProgressFrame(40.f, FText::FromString("Validating import"));

	ModelName = AGX_ImporterToEditor_helpers::MakeModelName(Result.Actor->GetName());
	RootDirectory = MakeRootDirectoryPath(ModelName);

	if (!ValidateImportEnum(FinalizeModelSourceComponent(*Result.Context, RootDirectory)))
		return nullptr;

	ImportTask.EnterProgressFrame(10.f, FText::FromString("Saving Assets"));
	WriteAssetsToDisk(RootDirectory, Result.Context);

	ImportTask.EnterProgressFrame(10.f, FText::FromString("Creating Blueprint"));
	UBlueprint* BaseBlueprint = CreateBaseBlueprint(RootDirectory, ModelName, *Result.Actor);
	UBlueprint* ChildBlueprint = CreateChildBlueprint(RootDirectory, ModelName, *BaseBlueprint);

	ImportTask.EnterProgressFrame(20.f, FText::FromString("Finishing"));

	if (Settings.bOpenBlueprintEditorAfterImport)
		GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(ChildBlueprint);

	PostImport(Settings);

	return ChildBlueprint;
}

bool FAGX_ImporterToEditor::Reimport(
	UBlueprint& BaseBP, FAGX_ReimportSettings Settings, UBlueprint* OpenBlueprint)
{
	using namespace AGX_ImporterToEditor_helpers;

	FScopedSlowTask ImportTask(100.f, FText::FromString("Reimport model"), true);
	ImportTask.MakeDialog();

	ImportTask.EnterProgressFrame(3.f, FText::FromString("Pre-reimport setup"));

	PreReimport(BaseBP, Settings);

	ImportTask.EnterProgressFrame(2.f, FText::FromString("Initializing"));

	if (!ValidateReimportSettings(Settings))
		return false;

	if (!ValidateBlueprintForReimport(BaseBP))
		return false;

	if (GEditor != nullptr)
		GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->CloseAllAssetEditors();

	// As a safety measure, we save and compile the OpenBlueprint (now closed) so that all
	// references according to the state on disk are up to date.
	if (OpenBlueprint != nullptr)
		FAGX_EditorUtilities::SaveAndCompile(*OpenBlueprint);

	CloseAssetEditors();

	ImportTask.EnterProgressFrame(15.f, FText::FromString("Reading objects from source file"));

	FAGX_Importer Importer;
	FAGX_ImportResult Result = Importer.Import(Settings, *GetTransientPackage());

	ImportTask.EnterProgressFrame(20.f, FText::FromString("Validating result"));

	if (!ValidateImportResult(Result, Settings))
		return false;

	RootDirectory = GetModelDirectoryPathFromBaseBlueprint(BaseBP);

	if (!ValidateImportEnum(FinalizeModelSourceComponent(*Result.Context, RootDirectory)))
		return false;

	ImportTask.EnterProgressFrame(40.f, FText::FromString("Updating Blueprint"));
	const auto UpdateResult = UpdateBlueprint(BaseBP, Settings, Importer.GetContext());
	if (!ValidateImportEnum(UpdateResult))
		return false;

	ImportTask.EnterProgressFrame(20.f, FText::FromString("Finishing"));
	DestroyContextOuterOwnedAssets(*Result.Context);
	FAGX_EditorUtilities::SaveAndCompile(BaseBP);

	if (OpenBlueprint != nullptr)
	{
		FAGX_EditorUtilities::SaveAndCompile(*OpenBlueprint);
		if (Settings.bOpenBlueprintEditorAfterImport)
			GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(OpenBlueprint);
	}

	return true;
}

template <typename T>
T* FAGX_ImporterToEditor::UpdateOrCreateAsset(T& Source, const FAGX_ImportContext& Context)
{
	using namespace AGX_ImporterToEditor_helpers;
	const FString AssetType = GetAssetTypeFromType<T>(Source);
	const FString DirPath = FPaths::Combine(RootDirectory, AssetType);
	T* Asset = nullptr;
	if constexpr (HasImportGuid<T>::value)
		Asset = FAGX_EditorUtilities::FindAsset<T>(Source.ImportGuid, DirPath);
	else
		Asset = FAGX_EditorUtilities::FindAsset<T>(Source.GetName(), DirPath);

	if (Asset == nullptr)
	{
		// We got a new Asset type object during reimport and it will be written to disk.
		// Before we do that we need to ensure the object does not reference any transient objects.
		// This is the case only for some Asset types, and is specially handled here.
		if constexpr (std::is_same_v<T, UStaticMesh>)
			FixupRenderMaterial(TransientToAsset, Source);

		if constexpr (std::is_same_v<T, UAGX_ContactMaterial>)
			FixupContactMaterial(TransientToAsset, Source);

		WriteAssetToDisk(RootDirectory, AssetType, Source, Context);
		return &Source; // We are done.
	}

	AGX_CHECK(Asset != &Source);

	// At this point we have identified an existing Asset that we need to update using Source.

	if constexpr (std::is_same_v<T, UStaticMesh>)
	{
		// UStaticMesh needs some special handling, because it is a complicated object which also
		// can take a lot of time creating/building. If the new Static Mesh and the old one is
		// "equal", then we do nothing except add an entry to the TransientToAsset map and update
		// the session guid. If they are not equal, we update the old one, just like for any other
		// assets during reimport.
		if (!AGX_MeshUtilities::AreStaticMeshesEqual(&Source, Asset))
		{
			bool Result = AGX_MeshUtilities::CopyStaticMesh(&Source, Asset);
			AGX_CHECK(Result);
			Result = CopyProperties(Source, *Asset, TransientToAsset, DefaultOverwriteRule);
			FixupRenderMaterial(TransientToAsset, *Asset); // CopyProperties does not handle arrays.
			AGX_CHECK(Result);
		}
	}
	else // All other Asset types.
	{
		// For shared assets, we might be copying and saving multiple times here, but we assume
		// these operations are relatively cheap, and keep the code simple here.
		bool Result = CopyProperties(Source, *Asset, TransientToAsset, DefaultOverwriteRule);
		AGX_CHECK(Result);
	}

	FAGX_ImportRuntimeUtilities::WriteSessionGuidToAssetType(*Asset, Context.SessionGuid);
	if (!Asset->GetName().Equals(Source.GetName()))
		Asset->Rename(*Source.GetName());

	bool Result = FAGX_ObjectUtilities::SaveAsset(*Asset);
	AGX_CHECK(Result);
	DestroyRedirectorAfterSave(Asset, Context);
	TransientToAsset.Add(&Source, Asset);
	return Asset;
}

EAGX_ImportResult FAGX_ImporterToEditor::UpdateBlueprint(
	UBlueprint& Blueprint, const FAGX_ReimportSettings& Settings, const FAGX_ImportContext& Context)
{
	using namespace AGX_ImporterToEditor_helpers;
	FScopedSlowTask ImportTask(100.f, FText::FromString("Update Blueprint"));
	ImportTask.MakeDialog();
	ImportTask.EnterProgressFrame(30.f, FText::FromString("Updating Assets"));

	EAGX_ImportResult Result = UpdateAssets(Blueprint, Context);
	if (IsUnrecoverableError(Result))
		return Result;

	ImportTask.EnterProgressFrame(30.f, FText::FromString("Updating Components"));
	Result |= UpdateComponents(Blueprint, Settings, Context);
	if (IsUnrecoverableError(Result))
		return Result;

	ImportTask.EnterProgressFrame(20.f, FText::FromString("Removing old Components"));
	RemoveDeletedComponents(Blueprint, Context.SessionGuid);

	ImportTask.EnterProgressFrame(20.f, FText::FromString("Removing old Assets"));
	RemoveDeletedAssets(RootDirectory, Context.SessionGuid);

	return Result;
}

EAGX_ImportResult FAGX_ImporterToEditor::UpdateAssets(
	UBlueprint& Blueprint, const FAGX_ImportContext& Context)
{
	using namespace AGX_ImporterToEditor_helpers;
	EAGX_ImportResult Result = EAGX_ImportResult::Success;

	if (Context.MSThresholds != nullptr)
	{
		for (const auto& [Guid, MST] : *Context.MSThresholds)
		{
			const auto A = UpdateOrCreateAsset(*MST, Context);
			AGX_CHECK(A != nullptr);
			if (A == nullptr)
				Result |= EAGX_ImportResult::RecoverableErrorsOccured;
		}
	}

	if (Context.RenderMaterials != nullptr)
	{
		for (const auto& [Guid, Rm] : *Context.RenderMaterials)
		{
			UMaterialInstanceDynamic* Mid = Cast<UMaterialInstanceDynamic>(Rm);
			UMaterialInterface* A = nullptr;
			if (Mid != nullptr)
			{
				// This is somewhat of a work-around.
				// Before the AGX_ImporterToEditor class, we used an old import pipeline where
				// Render Materials were written to disk as UMaterialInstanceConstant types.
				// Now, we write them as UMaterialInstanceDynamic types.
				// This means that if we do Reimport of an old imported model we will get a type
				// mismatch in the CopyProperties function in UpdateOrCreateAsset (the new Render
				// Material is of type UMaterialInstanceDynamic, and the asset is of type
				// UMaterialInstanceConstant).
				// To make our lives simple, we actually want to upgrade the asset so that it's type
				// is of the new UMaterialInstanceDynamic type.
				// We do this by forcing the type matching done by UpdateOrCreateAsset to be
				// UMaterialInstanceDynamic, which means it will not find the old asset of type
				// UMaterialInstanceConstant, and a new asset will be created (and the old asset
				// will be removed).
				// The next time the same model is Reimported, the asset type will be the correct
				// one and we will get a match in UpdateOrCreateAsset and update the asset without
				// removing it which is what we want.
				A = UpdateOrCreateAsset<UMaterialInstanceDynamic>(*Mid, Context);
			}
			else
				A = UpdateOrCreateAsset(*Rm, Context);

			AGX_CHECK(A != nullptr);
			if (A == nullptr)
				Result |= EAGX_ImportResult::RecoverableErrorsOccured;
		}
	}

	if (Context.CollisionStaticMeshes != nullptr)
	{
		for (const auto& [Guid, Sm] : *Context.CollisionStaticMeshes)
		{
			const auto A = UpdateOrCreateAsset(*Sm, Context);
			AGX_CHECK(A != nullptr);
			if (A == nullptr)
				Result |= EAGX_ImportResult::RecoverableErrorsOccured;
		}
	}

	if (Context.RenderStaticMeshes != nullptr)
	{
		for (const auto& [Guid, Sm] : *Context.RenderStaticMeshes)
		{
			const auto A = UpdateOrCreateAsset(*Sm, Context);
			AGX_CHECK(A != nullptr);
			if (A == nullptr)
				Result |= EAGX_ImportResult::RecoverableErrorsOccured;
		}
	}

	if (Context.ShapeMaterials != nullptr)
	{
		for (const auto& [Guid, Sm] : *Context.ShapeMaterials)
		{
			const auto A = UpdateOrCreateAsset(*Sm, Context);
			AGX_CHECK(A != nullptr);
			if (A == nullptr)
				Result |= EAGX_ImportResult::RecoverableErrorsOccured;
		}
	}

	if (Context.ContactMaterials != nullptr)
	{
		for (const auto& [Guid, Cm] : *Context.ContactMaterials)
		{
			const auto A = UpdateOrCreateAsset(*Cm, Context);
			AGX_CHECK(A != nullptr);
			if (A == nullptr)
				Result |= EAGX_ImportResult::RecoverableErrorsOccured;
		}
	}

	if (Context.ShovelProperties != nullptr)
	{
		for (const auto& [Guid, Sp] : *Context.ShovelProperties)
		{
			const auto A = UpdateOrCreateAsset(*Sp, Context);
			AGX_CHECK(A != nullptr);
			if (A == nullptr)
				Result |= EAGX_ImportResult::RecoverableErrorsOccured;
		}
	}

	if (Context.TrackProperties != nullptr)
	{
		for (const auto& [Guid, Tp] : *Context.TrackProperties)
		{
			const auto A = UpdateOrCreateAsset(*Tp, Context);
			AGX_CHECK(A != nullptr);
			if (A == nullptr)
				Result |= EAGX_ImportResult::RecoverableErrorsOccured;
		}
	}

	if (Context.TrackMergeProperties != nullptr)
	{
		for (const auto& [Guid, Tp] : *Context.TrackMergeProperties)
		{
			const auto A = UpdateOrCreateAsset(*Tp, Context);
			AGX_CHECK(A != nullptr);
			if (A == nullptr)
				Result |= EAGX_ImportResult::RecoverableErrorsOccured;
		}
	}

	return Result;
}

EAGX_ImportResult FAGX_ImporterToEditor::UpdateComponents(
	UBlueprint& Blueprint, const FAGX_ReimportSettings& Settings, const FAGX_ImportContext& Context)
{
	using namespace AGX_ImporterToEditor_helpers;

	// The order of the Components below must be such that parent Components are updated first, and
	// potential child Components last. This is because the children Components may be assigned a
	// new parent during reimport and to avoid errors, the parent should then be up to date so that
	// it can be correctly identified (for example in GetCorrespondingAttachParent above).

	FAGX_SCSNodeCollection Nodes(Blueprint);

	FAGX_ImportRuntimeUtilities::WriteSessionGuid(
		*Nodes.RootComponent->ComponentTemplate, Context.SessionGuid);

	EAGX_ImportResult Result = EAGX_ImportResult::Success;
	auto OverwriteRule =
		Settings.bForceOverwriteProperties ? ForceOverwriteRule : DefaultOverwriteRule;

	if (Context.RigidBodies != nullptr)
	{
		for (const auto& [Guid, Component] : *Context.RigidBodies)
		{
			USCS_Node* N = GetOrCreateNode(Guid, *Component, Nodes, Nodes.RigidBodies, Blueprint);
			if (N == nullptr)
				Result |= EAGX_ImportResult::RecoverableErrorsOccured;
			else
				CopyProperties(*Component, *N->ComponentTemplate, TransientToAsset, OverwriteRule);
		}
	}

	if (Context.Constraints != nullptr)
	{
		for (const auto& [Guid, Component] : *Context.Constraints)
		{
			USCS_Node* N = GetOrCreateNode(Guid, *Component, Nodes, Nodes.Constraints, Blueprint);
			if (N == nullptr)
				Result |= EAGX_ImportResult::RecoverableErrorsOccured;
			else
				CopyProperties(*Component, *N->ComponentTemplate, TransientToAsset, OverwriteRule);
		}
	}

	if (Context.Tires != nullptr)
	{
		for (const auto& [Guid, Component] : *Context.Tires)
		{
			USCS_Node* N = GetOrCreateNode(Guid, *Component, Nodes, Nodes.TwoBodyTires, Blueprint);
			if (N == nullptr)
				Result |= EAGX_ImportResult::RecoverableErrorsOccured;
			else
				CopyProperties(*Component, *N->ComponentTemplate, TransientToAsset, OverwriteRule);
		}
	}

	if (Context.Shovels != nullptr)
	{
		for (const auto& [Guid, Component] : *Context.Shovels)
		{
			USCS_Node* N = GetOrCreateNode(Guid, *Component, Nodes, Nodes.Shovels, Blueprint);
			if (N == nullptr)
				Result |= EAGX_ImportResult::RecoverableErrorsOccured;
			else
				CopyProperties(*Component, *N->ComponentTemplate, TransientToAsset, OverwriteRule);
		}
	}

	if (Context.Wires != nullptr)
	{
		for (const auto& [Guid, Component] : *Context.Wires)
		{
			USCS_Node* N = GetOrCreateNode(Guid, *Component, Nodes, Nodes.Wires, Blueprint);
			if (N == nullptr)
				Result |= EAGX_ImportResult::RecoverableErrorsOccured;
			else
				CopyProperties(*Component, *N->ComponentTemplate, TransientToAsset, OverwriteRule);
		}
	}

	if (Context.Tracks != nullptr)
	{
		for (const auto& [Guid, Component] : *Context.Tracks)
		{
			USCS_Node* N = GetOrCreateNode(Guid, *Component, Nodes, Nodes.Tracks, Blueprint);
			if (N == nullptr)
				Result |= EAGX_ImportResult::RecoverableErrorsOccured;
			else
				CopyProperties(*Component, *N->ComponentTemplate, TransientToAsset, OverwriteRule);
		}
	}

	if (Context.ObserverFrames != nullptr)
	{
		for (const auto& [Guid, Component] : *Context.ObserverFrames)
		{
			USCS_Node* N =
				GetOrCreateNode(Guid, *Component, Nodes, Nodes.ObserverFrames, Blueprint);
			if (N == nullptr)
				Result |= EAGX_ImportResult::RecoverableErrorsOccured;
			else
				CopyProperties(*Component, *N->ComponentTemplate, TransientToAsset, OverwriteRule);
		}
	}

	if (auto Component = Context.ModelSourceComponent)
	{
		AGX_CHECK(Nodes.ModelSourceComponent != nullptr);
		CopyProperties(
			*Component, *Nodes.ModelSourceComponent->ComponentTemplate, TransientToAsset,
			OverwriteRule);
	}

	if (Context.Shapes != nullptr)
	{
		auto RenderMaterialRule =
			Settings.bForceReassignRenderMaterials ? RenderMaterialOverwriteRule : OverwriteRule;

		for (const auto& [Guid, Component] : *Context.Shapes)
		{
			USCS_Node* N = GetOrCreateNode(Guid, *Component, Nodes, Nodes.Shapes, Blueprint);
			if (N == nullptr)
				Result |= EAGX_ImportResult::RecoverableErrorsOccured;
			else
			{
				CopyProperties(
					*Component, *N->ComponentTemplate, TransientToAsset, RenderMaterialRule);

				// CopyProperties does not handle TransientToAsset mappings in arrays such as render
				// materials.
				FixupRenderMaterial(
					TransientToAsset, *Cast<UAGX_ShapeComponent>(N->ComponentTemplate));
			}
		}
	}

	if (Context.CollisionStaticMeshCom != nullptr)
	{
		auto RenderMaterialRule =
			Settings.bForceReassignRenderMaterials ? RenderMaterialOverwriteRule : OverwriteRule;

		for (const auto& [Guid, Component] : *Context.CollisionStaticMeshCom)
		{
			TMap<FGuid, USCS_Node*> Unused;
			USCS_Node* N = GetOrCreateNode(Guid, *Component, Nodes, Unused, Blueprint);
			if (N == nullptr)
				Result |= EAGX_ImportResult::RecoverableErrorsOccured;
			else
			{
				CopyProperties(
					*Component, *N->ComponentTemplate, TransientToAsset, RenderMaterialRule);

				auto BPStaticMeshComp = Cast<UStaticMeshComponent>(N->ComponentTemplate);

				// This fixes an issue where the Static Mesh Component of the Blueprint has nullptr
				// "KnownStaticMesh" property after the CopyProperties call (for some reason) which
				// causes a crash. The underlying reason and what KnownStaticMesh is used for is not
				// clear, but this seems to reset the property and fix the crash.
				for (auto Instance : FAGX_ObjectUtilities::GetArchetypeInstances(*BPStaticMeshComp))
					Instance->PostApplyToComponent();

				BPStaticMeshComp->PostApplyToComponent();

				// CopyProperties does not handle TransientToAsset mappings in arrays such as render
				// materials.
				FixupRenderMaterial(TransientToAsset, *BPStaticMeshComp);
			}
		}
	}

	if (Context.RenderStaticMeshCom != nullptr)
	{
		auto RenderMaterialRule =
			Settings.bForceReassignRenderMaterials ? RenderMaterialOverwriteRule : OverwriteRule;

		for (const auto& [Guid, Component] : *Context.RenderStaticMeshCom)
		{
			TMap<FGuid, USCS_Node*> Unused;
			USCS_Node* N = GetOrCreateNode(Guid, *Component, Nodes, Unused, Blueprint);
			if (N == nullptr)
				Result |= EAGX_ImportResult::RecoverableErrorsOccured;
			else
			{
				CopyProperties(
					*Component, *N->ComponentTemplate, TransientToAsset, RenderMaterialRule);

				auto BPStaticMeshComp = Cast<UStaticMeshComponent>(N->ComponentTemplate);

				// This fixes an issue where the Static Mesh Component of the Blueprint has nullptr
				// "KnownStaticMesh" property after the CopyProperties call (for some reason) which
				// causes a crash. The underlying reason and what KnownStaticMesh is used for is not
				// clear, but this seems to reset the property and fix the crash.
				for (auto Instance : FAGX_ObjectUtilities::GetArchetypeInstances(*BPStaticMeshComp))
					Instance->PostApplyToComponent();

				BPStaticMeshComp->PostApplyToComponent();

				// CopyProperties does not handle TransientToAsset mappings in arrays such as render
				// materials.
				FixupRenderMaterial(TransientToAsset, *BPStaticMeshComp);
			}
		}
	}

	if (auto Component = Context.ContactMaterialRegistrar)
	{
		FGuid UnusedGuid = FGuid::NewGuid();
		TMap<FGuid, USCS_Node*> Unused;
		USCS_Node* N = GetOrCreateNode(UnusedGuid, *Component, Nodes, Unused, Blueprint);
		if (N == nullptr)
			Result |= EAGX_ImportResult::RecoverableErrorsOccured;
		else
		{
			CopyProperties(
				*Component, *Nodes.ContactMaterialRegistrarComponent->ComponentTemplate,
				TransientToAsset, OverwriteRule);

			// We need to update CM pointers of the re-imported Contact Material Registrar since
			// CopyProperties does not support TransientToAsset mapping of arrays.
			FixupContactMaterials(
				TransientToAsset,
				*Cast<UAGX_ContactMaterialRegistrarComponent>(N->ComponentTemplate));
		}
	}

	if (auto Component = Context.CollisionGroupDisabler)
	{
		FGuid UnusedGuid = FGuid::NewGuid();
		TMap<FGuid, USCS_Node*> Unused;
		USCS_Node* N = GetOrCreateNode(UnusedGuid, *Component, Nodes, Unused, Blueprint);
		if (N == nullptr)
			Result |= EAGX_ImportResult::RecoverableErrorsOccured;
		else
		{
			CopyProperties(
				*Component, *Nodes.CollisionGroupDisablerComponent->ComponentTemplate,
				TransientToAsset, OverwriteRule);
		}
	}

	if (auto Component = Context.SignalHandler)
	{
		FGuid UnusedGuid = FGuid::NewGuid();
		TMap<FGuid, USCS_Node*> Unused;
		USCS_Node* N = GetOrCreateNode(UnusedGuid, *Component, Nodes, Unused, Blueprint);
		if (N == nullptr)
			Result |= EAGX_ImportResult::RecoverableErrorsOccured;
		else
		{
			CopyProperties(
				*Component, *Nodes.SignalHandler->ComponentTemplate, TransientToAsset,
				OverwriteRule);
		}
	}

	return Result;
}

void FAGX_ImporterToEditor::PreImport(FAGX_ImportSettings& OutSettings)
{
	if (OutSettings.ImportType != EAGX_ImportType::Plx)
		return;

	if (OutSettings.FilePath.StartsWith(FPLXUtilities::GetModelsDirectory()))
		return;

	// We need to copy the OpenPLX file (and any dependency) to the OpenPLX ModelsDirectory.
	// We also update the filepath in the ImportSettings to point to the new, copied OpenPLX file.
	const FString DestinationDir = FPLXUtilities::CreateUniqueModelDirectory(OutSettings.FilePath);
	const FString NewLocation =
		FPLXUtilities::CopyAllDependenciesToProject(OutSettings.FilePath, DestinationDir);
	OutSettings.FilePath = NewLocation;
}

void FAGX_ImporterToEditor::PreReimport(
	const UBlueprint& Blueprint, FAGX_ImportSettings& OutSettings)
{
	if (OutSettings.ImportType != EAGX_ImportType::Plx)
		return;

	if (OutSettings.FilePath.StartsWith(FPLXUtilities::GetModelsDirectory()))
		return;

	USCS_Node* MsNode = Blueprint.SimpleConstructionScript->FindSCSNode(TEXT("AGX_ModelSource"));
	if (MsNode == nullptr)
		return;

	UAGX_ModelSourceComponent* Ms = Cast<UAGX_ModelSourceComponent>(MsNode->ComponentTemplate);
	if (Ms == nullptr)
		return;

	const FString TargetDir = FPaths::GetPath(Ms->FilePath);
	if (!TargetDir.StartsWith(FPLXUtilities::GetModelsDirectory()))
		return;

	const FString NewLocation =
		FPLXUtilities::CopyAllDependenciesToProject(OutSettings.FilePath, TargetDir);
	OutSettings.FilePath = NewLocation;
}

void FAGX_ImporterToEditor::PostImport(const FAGX_ImportSettings& Settings)
{
	if (Settings.ImportType == EAGX_ImportType::Plx)
	{
		FAGX_NotificationUtilities::ShowDialogBoxWithSuccess(FString::Printf(
			TEXT("OpenPLX model files were copied to '%s'. These files are needed during runtime "
				 "and should not be removed as long as the imported model is used."),
			*FPaths::GetPath(Settings.FilePath)));
	}
}
