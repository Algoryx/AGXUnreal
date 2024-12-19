// Copyright 2024, Algoryx Simulation AB.

#include "Import/AGX_ImporterToEditor.h"

// AGX Dynamics for Unreal includes.
#include "AGX_AGXToUeContext.h"
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"
#include "AMOR/AGX_ShapeContactMergeSplitThresholds.h"
#include "Import/AGX_Importer.h"
#include "Import/AGX_ImporterSettings.h"
#include "Import/AGX_SCSNodeCollection.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_EditorUtilities.h"
#include "Utilities/AGX_ImportUtilities.h"
#include "Utilities/AGX_ObjectUtilities.h"

// Unreal Engine includes.
#include "Engine/SCS_Node.h"
#include "FileHelpers.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "PackageTools.h"

namespace AGX_ImporterToEditor_helpers
{
	void PreCreateBlueprintSetup()
	{
		GEditor->SelectNone(false, false);
	}

	void PreReimportSetup()
	{
		// During Model Synchronization, old assets are deleted and references to these assets are
		// automatically cleared. Having the Blueprint Editor opened while doing this causes
		// crashing during this process and the exact reason why is not clear. So we solve this
		// by closing all asset editors here first.
		if (GEditor != nullptr)
			GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->CloseAllAssetEditors();
	}

	FString MakeModelName(FString SourceFilename)
	{
		return FAGX_EditorUtilities::SanitizeName(
			SourceFilename, FAGX_ImportUtilities::GetImportRootDirectoryName());
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
			IsBase ? "BP_Base_" + FGuid::NewGuid().ToString() : TEXT("BP_") + ModelName;
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

	void CopyPropertyRecursive(
		const void* Archetype, const void* Source, void* OutDest, const FProperty* Property,
		const TMap<UObject*, UObject*>& TransientToAsset)
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
					Archetype == OutDest || Property->Identical(ArchetypeVal, DestVal);

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
					TransientToAsset);
			}
			return;
		}

		const bool ShouldCopy = Archetype == OutDest || Property->Identical(ArchetypeVal, DestVal);
		if (ShouldCopy)
			Property->CopyCompleteValue(DestVal, SourceVal);
	}

	// Similar to FAGX_ObjectUtilities::CopyProperties, but with some special handling for the
	// TransientToAsset map.
	bool CopyProperties(
		const UObject& Source, UObject& OutDest, const TMap<UObject*, UObject*>& TransientToAsset)
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
				CopyPropertyRecursive(&OutDest, &Source, Instance, Property, TransientToAsset);
			}
		}

		for (TFieldIterator<FProperty> PropIt(Class); PropIt; ++PropIt)
		{
			const FProperty* Property = *PropIt;
			CopyPropertyRecursive(&OutDest, &Source, &OutDest, Property, TransientToAsset);
		}

		return true;
	}

	bool ValidateImportResult(
		const FAGX_ImportResult& Result, const FAGX_ImporterSettings& Settings)
	{
		if (Result.Actor == nullptr)
		{
			const FString Text = FString::Printf(
				TEXT("Errors occurred during import. The file '%s' could not be imported. Log "
					 "category LogAGX in the Output Log may contain more information."),
				*Settings.FilePath);
			FAGX_NotificationUtilities::ShowDialogBoxWithErrorLog(
				Text, "Import model to Blueprint");
			return false;
		}

		if (Result.Result != EAGX_ImportResult::Success)
		{
			const FString Text = FString::Printf(
				TEXT("Some issues occurred during import and the result may not be the "
					 "expected result. Log category LogAGX in the Output Log may "
					 "contain more information."));
			FAGX_NotificationUtilities::ShowNotification(Text, SNotificationItem::CS_Fail);
		}

		return true;
	}

	void WriteAssetToDisk(const FString& RootDir, const FString& AssetType, UObject& Object)
	{
		const FString AssetName = Object.GetName();
		const FString PackagePath =
			FPaths::Combine(FAGX_ImportUtilities::CreatePackagePath(RootDir, AssetType), AssetName);
		UPackage* Package = CreatePackage(*PackagePath);
		Object.Rename(*AssetName, Package);
		Package->MarkPackageDirty();
		Object.SetFlags(RF_Public | RF_Standalone);
		FAGX_ObjectUtilities::SaveAsset(Object);
	}

	void WriteAssetsToDisk(const FString& RootDir, const FAGX_AGXToUeContext* Context)
	{
		if (Context == nullptr)
			return;

		if (Context->MSThresholds != nullptr)
		{
			const FString AssetType =
				FAGX_ImportUtilities::GetImportMergeSplitThresholdsDirectoryName();
			for (const auto& [Guid, MST] : *Context->MSThresholds)
			{
				if (auto SCMST = Cast<UAGX_ShapeContactMergeSplitThresholds>(MST))
					WriteAssetToDisk(RootDir, AssetType, *SCMST);
			}
		}
	}

	template <typename T>
	T* UpdateOrCreateAsset(
		T& Source, const FString& RootDir, const FString& AssetType,
		TMap<UObject*, UObject*>& OutTransientToAsset)
	{
		const FString DirPath = FPaths::Combine(RootDir, AssetType);
		T* Asset = FAGX_EditorUtilities::FindAsset<T>(Source.ImportGuid, DirPath);
		if (Asset == nullptr)
		{
			WriteAssetToDisk(RootDir, AssetType, Source);
			return &Source;
		}

		// For shared assets, we might be copying and saving multiple times here, but we assume
		// these operations are relatively cheap, and keep the code simple here.
		AGX_CHECK(FAGX_ObjectUtilities::CopyProperties(Source, *Asset, false));
		FAGX_ObjectUtilities::SaveAsset(*Asset);
		OutTransientToAsset.Add(&Source, Asset);
		return Asset;
	}

	// The Template Component must come from a context where it has a unique name.
	USCS_Node* GetOrCreateNode(
		const FGuid& Guid, const UActorComponent* Template, UBlueprint& OutBlueprint,
		TMap<FGuid, USCS_Node*>& OutGuidToNode)
	{
		const FName Name(*Template->GetName());
		USCS_Node* Node = OutGuidToNode.FindRef(Guid);

		// Resolve name collisions.
		USCS_Node* NameCollNode = OutBlueprint.SimpleConstructionScript->FindSCSNode(Name);
		if (NameCollNode != nullptr && NameCollNode != Node)
			NameCollNode->SetVariableName(*FAGX_ImportUtilities::GetUnsetUniqueImportName());

		if (Node == nullptr)
		{
			Node = OutBlueprint.SimpleConstructionScript->CreateNode(Template->GetClass(), Name);
			OutBlueprint.SimpleConstructionScript->GetDefaultSceneRootNode()->AddChildNode(Node);
			OutGuidToNode.Add(Guid, Node);
		}
		else if (!Node->GetVariableName().IsEqual(Name))
		{
			Node->SetVariableName(Name);
		}

		return Node;
	}

	template <typename T>
	FString GetAssetTypeFromType()
	{
		if constexpr (std::is_same_v<T, UAGX_MergeSplitThresholdsBase>)
			return FAGX_ImportUtilities::GetImportMergeSplitThresholdsDirectoryName();

		// Unsupported types will yield compile errors.
	}
}

UBlueprint* FAGX_ImporterToEditor::Import(const FAGX_ImporterSettings& Settings)
{
	using namespace AGX_ImporterToEditor_helpers;
	FAGX_Importer Importer;
	FAGX_ImportResult Result = Importer.Import(Settings);
	if (!ValidateImportResult(Result, Settings))
		return nullptr;

	ModelName = AGX_ImporterToEditor_helpers::MakeModelName(Result.Actor->GetName());
	RootDirectory = MakeRootDirectoryPath(ModelName);

	WriteAssetsToDisk(RootDirectory, Result.Context);

	UBlueprint* BaseBlueprint = CreateBaseBlueprint(RootDirectory, ModelName, *Result.Actor);
	UBlueprint* ChildBlueprint = CreateChildBlueprint(RootDirectory, ModelName, *BaseBlueprint);

	if (Settings.bOpenBlueprintEditorAfterImport)
		GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(ChildBlueprint);

	return ChildBlueprint;
}

bool FAGX_ImporterToEditor::Reimport(
	UBlueprint& BaseBP, const FAGX_ImporterSettings& Settings, UBlueprint* OpenBlueprint)
{
	using namespace AGX_ImporterToEditor_helpers;

	if (GEditor != nullptr)
		GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->CloseAllAssetEditors();

	// As a safety measure, we save and compile the OpenBlueprint (now closed) so that all
	// references according to the state on disk are up to date.
	if (OpenBlueprint != nullptr)
		FAGX_EditorUtilities::SaveAndCompile(*OpenBlueprint);

	PreReimportSetup();
	FAGX_Importer Importer;
	FAGX_ImportResult Result = Importer.Import(Settings);
	if (!ValidateImportResult(Result, Settings))
		return false;

	RootDirectory = GetModelDirectoryPathFromBaseBlueprint(BaseBP);
	UpdateBlueprint(BaseBP, Importer.GetContext());

	if (Settings.bOpenBlueprintEditorAfterImport && OpenBlueprint != nullptr)
		GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(OpenBlueprint);

	return true;
}

template <typename T>
T* FAGX_ImporterToEditor::UpdateOrCreateAsset(T& Source)
{
	using namespace AGX_ImporterToEditor_helpers;
	const FString AssetType = GetAssetTypeFromType<T>();
	const FString DirPath = FPaths::Combine(RootDirectory, AssetType);
	T* Asset = FAGX_EditorUtilities::FindAsset<T>(Source.ImportGuid, DirPath);
	if (Asset == nullptr)
	{
		WriteAssetToDisk(RootDirectory, AssetType, Source);
		return &Source;
	}

	// For shared assets, we might be copying and saving multiple times here, but we assume
	// these operations are relatively cheap, and keep the code simple here.
	AGX_CHECK(FAGX_ObjectUtilities::CopyProperties(Source, *Asset, false));
	FAGX_ObjectUtilities::SaveAsset(*Asset);
	TransientToAsset.Add(&Source, Asset);
	return Asset;
}

void FAGX_ImporterToEditor::UpdateBlueprint(
	UBlueprint& Blueprint, const FAGX_AGXToUeContext& Context)
{
	using namespace AGX_ImporterToEditor_helpers;
	FAGX_SCSNodeCollection Nodes(Blueprint);

	if (Context.MSThresholds != nullptr)
	{
		for (const auto& [Guid, MST] : *Context.MSThresholds)
		{
			const auto A = UpdateOrCreateAsset(*MST);
			AGX_CHECK(A != nullptr);
		}
	}

	if (Context.RigidBodies != nullptr)
	{
		for (const auto& [Guid, Component] : *Context.RigidBodies)
		{
			USCS_Node* N = GetOrCreateNode(Guid, Component, Blueprint, Nodes.RigidBodies);
			CopyProperties(*Component, *N->ComponentTemplate, TransientToAsset);
		}
	}
}
