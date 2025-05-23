// Copyright 2024, Algoryx Simulation AB.

#include "Utilities/AGX_ImportRuntimeUtilities.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "Import/AGX_ImportContext.h"
#include "Materials/AGX_ShapeMaterial.h"
#include "Materials/ShapeMaterialBarrier.h"
#include "Utilities/PLXUtilities.h"

// Unreal Engine includes.
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "UObject/MetaData.h"

void FAGX_ImportRuntimeUtilities::WriteSessionGuid(
	UActorComponent& Component, const FGuid& SessionGuid)
{
	Component.ComponentTags.Empty();
	Component.ComponentTags.Add(*SessionGuid.ToString());
}

void FAGX_ImportRuntimeUtilities::WriteSessionGuidToAssetType(
	UObject& Object, const FGuid& SessionGuid)
{
#if WITH_EDITOR
	if (auto MetaData = Object.GetOutermost()->GetMetaData())
		MetaData->SetValue(&Object, TEXT("AGX_ImportSessionGuid"), *SessionGuid.ToString());
#endif
}

void FAGX_ImportRuntimeUtilities::OnComponentCreated(
	UActorComponent& Component, AActor& Owner, const FGuid& SessionGuid)
{
	WriteSessionGuid(Component, SessionGuid);
	Component.SetFlags(RF_Transactional);
	Owner.AddInstanceComponent(&Component);
}

void FAGX_ImportRuntimeUtilities::OnAssetTypeCreated(UObject& Object, const FGuid& SessionGuid)
{
	WriteSessionGuidToAssetType(Object, SessionGuid);
}

UAGX_ShapeMaterial* FAGX_ImportRuntimeUtilities::GetOrCreateShapeMaterial(
	const FShapeMaterialBarrier& Barrier, FAGX_ImportContext* Context)
{
	if (!Barrier.HasNative())
		return nullptr;

	if (Context != nullptr && Context->ShapeMaterials != nullptr)
	{
		if (auto Existing = Context->ShapeMaterials->FindRef(Barrier.GetGuid()))
			return Existing;
	}

	UObject* Outer = GetTransientPackage();
	if (Context != nullptr)
		Outer = Context->Outer;

	auto Sm = NewObject<UAGX_ShapeMaterial>(Outer, NAME_None, RF_Public | RF_Standalone);
	Sm->CopyFrom(Barrier, Context);

	if (Context != nullptr && Context->ShapeMaterials != nullptr)
	{
		OnAssetTypeCreated(*Sm, Context->SessionGuid);
		Context->ShapeMaterials->Add(Barrier.GetGuid(), Sm);
	}

	return Sm;
}

EAGX_ImportType FAGX_ImportRuntimeUtilities::GetFrom(const FString& FilePath)
{
	const FString FileExtension = FPaths::GetExtension(FilePath);
	if (FileExtension.Equals("agx"))
	{
		return EAGX_ImportType::Agx;
	}
	else if (FileExtension.Equals("openplx"))
	{
		return EAGX_ImportType::Plx;
	}
	else if (FileExtension.Equals("urdf"))
	{
		return EAGX_ImportType::Urdf;
	}

	return EAGX_ImportType::Invalid;
}

FString FAGX_ImportRuntimeUtilities::RemoveImportedOpenPLXFiles(const FString& FilePath)
{
	FString ModelsDir = FPLXUtilities::GetModelsDirectory();
	FPaths::NormalizeDirectoryName(ModelsDir);

	if (!FilePath.StartsWith(ModelsDir))
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("FilePath '%s' does not start with expected base directory '%s'."), *FilePath,
			*ModelsDir);
		return "";
	}

	// Strip off base dir and get the next subfolder.
	FString Remainder = FilePath.RightChop(ModelsDir.Len());
	Remainder.RemoveFromStart(TEXT("/")); // In case there's a leading slash.

	FString FirstFolder;
	if (!Remainder.Split(TEXT("/"), &FirstFolder, nullptr))
		FirstFolder = Remainder;

	FString FolderToDelete = FPaths::Combine(ModelsDir, FirstFolder);
	FPaths::NormalizeDirectoryName(FolderToDelete);

	if (FPaths::DirectoryExists(FolderToDelete))
	{
		if (IFileManager::Get().DeleteDirectory(
				*FolderToDelete, /*RequireExists=*/true, /*Tree=*/true))
			return FolderToDelete;
	}

	return "";
}
