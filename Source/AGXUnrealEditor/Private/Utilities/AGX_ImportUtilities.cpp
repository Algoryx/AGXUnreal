#include "Utilities/AGX_ImportUtilities.h"

// AGXUnreal includes.
#include "AGX_LogCategory.h"
#include "Utilities/AGX_EditorUtilities.h"
#include "Shapes/TrimeshShapeBarrier.h"
#include "Materials/AGX_ContactMaterialAsset.h"
#include "Materials/AGX_ShapeMaterialAsset.h"
#include "Materials/ContactMaterialBarrier.h"
#include "Materials/ShapeMaterialBarrier.h"

// Unreal Engine includes.
#include "AssetToolsModule.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMesh.h"
#include "RawMesh.h"

namespace
{
	/// \todo Determine if it's enough to return the created asset, or if we must pack it in a
	/// struct together with the package path and/or asset name.
	template <typename UAsset, typename FInitAssetCallback>
	UAsset* SaveImportedAsset(
		const FString& DirectoryName, FString AssetName, const FString& FallbackName,
		const FString& AssetType, FInitAssetCallback InitAsset)
	{
		AssetName = FAGX_ImportUtilities::CreateAssetName(AssetName, FallbackName, AssetType);
		FString PackagePath =
			FAGX_ImportUtilities::CreateArchivePackagePath(DirectoryName, AssetType);
		FAGX_ImportUtilities::MakePackageAndAssetNameUnique(PackagePath, AssetName);
		UPackage* Package = CreatePackage(nullptr, *PackagePath);
#if 0
		/// \todo Unclear if this is needed or not. Leaving it out for now but
		/// test with it restored if there are problems.
		Package->FullyLoad();
#endif
		UAsset* Asset = NewObject<UAsset>(Package, FName(*AssetName), RF_Public | RF_Standalone);
		if (!Asset)
		{
			UE_LOG(
				LogAGX, Error, TEXT("Could not create asset '%s' from archive '%s'."), *AssetName,
				*DirectoryName);
			return nullptr;
		}
		InitAsset(*Asset);
		if (!FAGX_EditorUtilities::FinalizeAndSavePackage(Package, Asset, PackagePath, AssetName))
		{
			return nullptr;
		}
		return Asset;
	}
}

FString FAGX_ImportUtilities::CreateArchivePackagePath(FString ArchiveName, FString AssetType)
{
	ArchiveName = FAGX_EditorUtilities::SanitizeName(ArchiveName);
	AssetType = FAGX_EditorUtilities::SanitizeName(AssetType);
	if (ArchiveName.IsEmpty() || AssetType.IsEmpty())
	{
		return FString();
	}
	return FString::Printf(TEXT("/Game/ImportedAgxArchives/%s/%ss/"), *ArchiveName, *AssetType);
}

FString FAGX_ImportUtilities::CreateArchivePackagePath(FString ArchiveName)
{
	ArchiveName = FAGX_EditorUtilities::SanitizeName(ArchiveName);
	if (ArchiveName.IsEmpty())
	{
		return FString();
	}
	return FString::Printf(TEXT("/Game/ImportedAgxArchives/%s"), *ArchiveName);
}

FString FAGX_ImportUtilities::CreateAssetName(
	const FString& NativeName, const FString& FallbackName, const FString& AssetType)
{
	FString Name = FAGX_EditorUtilities::SanitizeName(NativeName);
	if (!Name.IsEmpty())
	{
		return Name;
	}
	Name = FAGX_EditorUtilities::SanitizeName(FallbackName);
	if (!Name.IsEmpty())
	{
		return Name;
	}
	return AssetType;
}

void FAGX_ImportUtilities::MakePackageAndAssetNameUnique(FString& PackageName, FString& AssetName)
{
	FString WantedPackageName = PackageName;
	FString WantedAssetName = AssetName;
	IAssetTools& AssetTools =
		FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	AssetTools.CreateUniqueAssetName(PackageName, AssetName, PackageName, AssetName);
	if (AssetName != WantedAssetName)
	{
		UE_LOG(
			LogAGX, Warning, TEXT("Asset '%s' imported with name '%s' because of name conflict."),
			*WantedAssetName, *AssetName);
	}
}

UStaticMesh* FAGX_ImportUtilities::SaveImportedStaticMeshAsset(
	const FTrimeshShapeBarrier& Trimesh, const FString& DirectoryName, const FString& FallbackName)
{
	auto InitAsset = [&](UStaticMesh& Asset) {
		FRawMesh RawMesh = FAGX_EditorUtilities::CreateRawMeshFromTrimesh(Trimesh);
		FAGX_EditorUtilities::AddRawMeshToStaticMesh(RawMesh, &Asset);
		Asset.ImportVersion = EImportStaticMeshVersion::LastVersion;
	};
	UStaticMesh* CreatedAsset = SaveImportedAsset<UStaticMesh>(
		DirectoryName, Trimesh.GetSourceName(), FallbackName, TEXT("StaticMesh"), InitAsset);
	return CreatedAsset;
}

UAGX_ShapeMaterialAsset* FAGX_ImportUtilities::SaveImportedShapeMaterialAsset(
	const FShapeMaterialBarrier& Material, const FString& DirectoryName)
{
	auto InitAsset = [&](UAGX_ShapeMaterialAsset& Asset) { Asset.CopyFrom(&Material); };
	UAGX_ShapeMaterialAsset* CreatedAsset = SaveImportedAsset<UAGX_ShapeMaterialAsset>(
		DirectoryName, Material.GetName(), TEXT(""), TEXT("ShapeMaterial"), InitAsset);
	return CreatedAsset;
}

namespace
{
	FString GetName(UAGX_ShapeMaterialAsset* Material)
	{
		if (Material == nullptr)
		{
			return TEXT("Default");
		}
		return Material->GetName();
	}
}

UAGX_ContactMaterialAsset* FAGX_ImportUtilities::SaveImportedContactMaterialAsset(
	const FContactMaterialBarrier& ContactMaterial, UAGX_ShapeMaterialAsset* Material1,
	UAGX_ShapeMaterialAsset* Material2, const FString& DirectoryName)
{
	const FString Name = TEXT("CM") + GetName(Material1) + GetName(Material2);

	auto InitAsset = [&](UAGX_ContactMaterialAsset& Asset) {
		Asset.CopyFrom(&ContactMaterial);
		Asset.Material1 = Material1;
		Asset.Material2 = Material2;
	};

	UAGX_ContactMaterialAsset* Asset = SaveImportedAsset<UAGX_ContactMaterialAsset>(
		DirectoryName, Name, TEXT(""), TEXT("ContactMaterial"), InitAsset);

	return Asset;
}

void FAGX_ImportUtilities::Rename(UObject& Object, const FString& Name)
{
	if (Name.IsEmpty())
	{
		// Not having an imported name means use whatever default name Unreal decided.
		return;
	}
	if (Object.Rename(*Name, nullptr, REN_Test))
	{
		Object.Rename(*Name, nullptr, REN_DontCreateRedirectors);
	}
	else
	{
		FName NewName = MakeUniqueObjectName(Object.GetOuter(), Object.GetClass(), FName(*Name));
		UE_LOG(
			LogAGX, Warning, TEXT("%s '%s' imported with name '%s' because of name conflict."),
			*Object.GetClass()->GetName(), *Name, *NewName.ToString());
		Object.Rename(*NewName.ToString(), nullptr, REN_DontCreateRedirectors);
	}
}