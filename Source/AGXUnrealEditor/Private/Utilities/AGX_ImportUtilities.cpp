// Copyright 2024, Algoryx Simulation AB.

#include "Utilities/AGX_ImportUtilities.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "Materials/AGX_ContactMaterial.h"
#include "Materials/AGX_ShapeMaterial.h"
#include "Materials/ContactMaterialBarrier.h"
#include "Materials/ShapeMaterialBarrier.h"
#include "Shapes/TrimeshShapeBarrier.h"
#include "Shapes/RenderDataBarrier.h"
#include "Terrain/AGX_ShovelProperties.h"
#include "Utilities/AGX_BlueprintUtilities.h"
#include "Utilities/AGX_EditorUtilities.h"
#include "Utilities/AGX_ObjectUtilities.h"
#include "Vehicle/AGX_TrackInternalMergeProperties.h"
#include "Vehicle/AGX_TrackProperties.h"
#include "Vehicle/TrackBarrier.h"

// Unreal Engine includes.
#include "AssetToolsModule.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Misc/EngineVersionComparison.h"
#include "Misc/Paths.h"
#include "RawMesh.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/ComponentEditorUtils.h"
#if !UE_VERSION_OLDER_THAN(5, 0, 0)
#include "UObject/SavePackage.h"
#endif

#if PLATFORM_WINDOWS
#define AGXUNREALEDITOR_API_TEMPLATE AGXUNREALEDITOR_API
#else
#define AGXUNREALEDITOR_API_TEMPLATE
#endif

namespace
{
	template <typename UAsset, typename FInitAssetCallback>
	UAsset* PrepareWriteAssetToDisk(
		const FString& DirectoryPath, FString AssetName, const FString& FallbackName,
		const FString& AssetType, FInitAssetCallback InitAsset)
	{
		AssetName = FAGX_ImportUtilities::CreateAssetName(AssetName, FallbackName, AssetType);

		// If the asset name ends with lots of numbers then Unreal believes that
		// it is a counter starts looping trying to find the next available number,
		// which fails if the number is larger than the largest int32. This hack
		// twarts that by adding a useless character to the end of the name.
		int32 NumEndingNumerics = 0;
		for (int32 CharIndex = AssetName.Len() - 1; CharIndex >= 0; --CharIndex)
		{
			bool isNumeric = AssetName[CharIndex] >= TEXT('0') && AssetName[CharIndex] <= TEXT('9');
			if (!isNumeric)
				break;
			NumEndingNumerics++;
		}
		if (NumEndingNumerics >= 10)
		{
			AssetName = AssetName + "c";
			UE_LOG(
				LogAGX, Warning,
				TEXT("Asset '%s' was appended with a 'c' to avoid Unreal name processing bug."),
				*AssetName);
		}

		FString PackagePath = FAGX_ImportUtilities::CreatePackagePath(DirectoryPath, AssetType);
		FAGX_ImportUtilities::MakePackageAndAssetNameUnique(PackagePath, AssetName);
#if UE_VERSION_OLDER_THAN(4, 26, 0)
		UPackage* Package = CreatePackage(nullptr, *PackagePath);
#else
		UPackage* Package = CreatePackage(*PackagePath);
#endif

#if 0
		/// \todo Unclear if this is needed or not. Leaving it out for now but
		/// test with it restored if there are problems.
		Package->FullyLoad();
#endif
		UAsset* Asset = NewObject<UAsset>(Package, FName(*AssetName), RF_Public | RF_Standalone);
		if (Asset == nullptr)
		{
			UE_LOG(
				LogAGX, Error, TEXT("Could not create asset '%s' from '%s'."), *AssetName,
				*DirectoryPath);
			return nullptr;
		}
		InitAsset(*Asset);

		return Asset;
	}

	bool WriteAssetToDisk(UObject& Asset)
	{
		return FAGX_ObjectUtilities::SaveAsset(Asset);
	}
}

FString FAGX_ImportUtilities::CreatePackagePath(
	const FString& DirectoryPath, FString AssetType, bool AppendSeparator)
{
	AssetType = FAGX_EditorUtilities::SanitizeName(AssetType);

	if (AppendSeparator)
		return FPaths::Combine(DirectoryPath, AssetType, FString(""));
	else
		return FPaths::Combine(DirectoryPath, AssetType);
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
			LogAGX, Log, TEXT("Asset '%s' imported with name '%s' because of name conflict."),
			*WantedAssetName, *AssetName);
	}
}

namespace AGX_ImportUtilities_helpers
{
	template <typename FMeshFactory, typename FMeshDescription>
	void InitStaticMesh(
		FMeshFactory MeshFactory, const FMeshDescription& MeshDescription, UStaticMesh& Asset,
		bool bAllowCPUAccess)
	{
		FRawMesh RawMesh = MeshFactory(MeshDescription);
		FAGX_EditorUtilities::AddRawMeshToStaticMesh(RawMesh, &Asset);
		Asset.ImportVersion = EImportStaticMeshVersion::LastVersion;
		// Reading triangle data from a Static Mesh asset in a cooked build produces garbage on
		// Linux, which makes it impossible to create the corresponding AGX Dynamics Trimesh shape.
		// By setting this flag Unreal Engine will keep a copy of the triangle data in CPU memory
		// which we can read and create the Trimesh from.
		//
		// It comes with a memory cost, so once we have fixed the GPU copy problem the following
		// line should be removed.
		Asset.bAllowCPUAccess = bAllowCPUAccess;
	}
}

FLinearColor FAGX_ImportUtilities::SRGBToLinear(const FVector4& SRGB)
{
	FColor SRGBBytes(
		static_cast<uint8>(SRGB.X * 255.0f), static_cast<uint8>(SRGB.Y * 255.0f),
		static_cast<uint8>(SRGB.Z * 255.0f), static_cast<uint8>(SRGB.W * 255.0f));
	return {SRGBBytes};
}

FVector4 FAGX_ImportUtilities::LinearToSRGB(const FLinearColor& Linear)
{
	FColor SRGBBytes = Linear.ToFColor(true);
	return FVector4(
		static_cast<float>(SRGBBytes.R) / 255.0f, static_cast<float>(SRGBBytes.G) / 255.0f,
		static_cast<float>(SRGBBytes.B) / 255.0f, static_cast<float>(SRGBBytes.A) / 255.0f);
}

FString FAGX_ImportUtilities::GetImportRootDirectoryName()
{
	return FString("ImportedAGXModels");
}

FString FAGX_ImportUtilities::GetImportShapeMaterialDirectoryName()
{
	return FString("ShapeMaterial");
}

FString FAGX_ImportUtilities::GetImportContactMaterialDirectoryName()
{
	return FString("ContactMaterial");
}

FString FAGX_ImportUtilities::GetImportRenderMaterialDirectoryName()
{
	return FString("RenderMaterial");
}

FString FAGX_ImportUtilities::GetImportMergeSplitThresholdsDirectoryName()
{
	return FString("MergeSplitThresholds");
}

FString FAGX_ImportUtilities::GetImportCollisionStaticMeshDirectoryName()
{
	return FString("StaticMesh");
}

FString FAGX_ImportUtilities::GetImportRenderStaticMeshDirectoryName()
{
	return FString("RenderMesh");
}

FString FAGX_ImportUtilities::GetImportShovelPropertiesDirectoryName()
{
	return FString("ShovelProperties");
}

FString FAGX_ImportUtilities::GetImportTrackPropertiesDirectoryName()
{
	return FString("TrackProperties");
}

FString FAGX_ImportUtilities::GetImportTrackMergePropertiesDirectoryName()
{
	return FString("TrackInternalMergeProperties");
}

template <>
AGXUNREALEDITOR_API_TEMPLATE FString
FAGX_ImportUtilities::GetImportAssetDirectoryName<UAGX_ShapeMaterial>()
{
	return GetImportShapeMaterialDirectoryName();
}

template <>
AGXUNREALEDITOR_API_TEMPLATE FString
FAGX_ImportUtilities::GetImportAssetDirectoryName<UAGX_ContactMaterial>()
{
	return GetImportContactMaterialDirectoryName();
}

template <>
AGXUNREALEDITOR_API_TEMPLATE FString
FAGX_ImportUtilities::GetImportAssetDirectoryName<UMaterialInterface>()
{
	return GetImportRenderMaterialDirectoryName();
}

template <>
AGXUNREALEDITOR_API_TEMPLATE FString
FAGX_ImportUtilities::GetImportAssetDirectoryName<UAGX_MergeSplitThresholdsBase>()
{
	return GetImportMergeSplitThresholdsDirectoryName();
}

template <>
AGXUNREALEDITOR_API_TEMPLATE FString
FAGX_ImportUtilities::GetImportAssetDirectoryName<UAGX_ConstraintMergeSplitThresholds>()
{
	return GetImportMergeSplitThresholdsDirectoryName();
}

template <>
AGXUNREALEDITOR_API_TEMPLATE FString
FAGX_ImportUtilities::GetImportAssetDirectoryName<UAGX_ShapeContactMergeSplitThresholds>()
{
	return GetImportMergeSplitThresholdsDirectoryName();
}

template <>
AGXUNREALEDITOR_API_TEMPLATE FString
FAGX_ImportUtilities::GetImportAssetDirectoryName<UAGX_WireMergeSplitThresholds>()
{
	return GetImportMergeSplitThresholdsDirectoryName();
}

template <>
AGXUNREALEDITOR_API_TEMPLATE FString
FAGX_ImportUtilities::GetImportAssetDirectoryName<UAGX_ShovelProperties>()
{
	return GetImportShovelPropertiesDirectoryName();
}

FString FAGX_ImportUtilities::GetContactMaterialRegistrarDefaultName()
{
	return FString("AGX_ContactMaterialRegistrar");
}

FString FAGX_ImportUtilities::GetCollisionGroupDisablerDefaultName()
{
	return FString("AGX_CollisionGroupDisabler");
}

FString FAGX_ImportUtilities::GetUnsetUniqueImportName()
{
	return FString("AGX_Import_Unnamed_") + FGuid::NewGuid().ToString();
}

FString FAGX_ImportUtilities::GetDefaultModelImportDirectory(const FString& ModelName)
{
	const FString Name = FAGX_EditorUtilities::SanitizeName(ModelName);
	const FString Root = FPaths::ProjectContentDir();
	const FString ImportsLocal = FPaths::Combine(GetImportRootDirectoryName(), ModelName);
	const FString ImportsFull = FPaths::Combine(Root, ImportsLocal);
	const FString ImportsAbsolute = FPaths::ConvertRelativePathToFull(ImportsFull);
	return ImportsAbsolute;
}

EAGX_ImportType FAGX_ImportUtilities::GetFrom(const FString& FilePath)
{
	const FString FileExtension = FPaths::GetExtension(FilePath);
	if (FileExtension.Equals("agx"))
	{
		return EAGX_ImportType::Agx;
	}
	else if (FileExtension.Equals("urdf"))
	{
		return EAGX_ImportType::Urdf;
	}

	return EAGX_ImportType::Invalid;
}
