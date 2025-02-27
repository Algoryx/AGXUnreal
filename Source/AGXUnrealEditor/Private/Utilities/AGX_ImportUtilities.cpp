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

UStaticMesh* FAGX_ImportUtilities::SaveImportedStaticMeshAsset(
	const FTrimeshShapeBarrier& Trimesh, const FString& DirectoryPath, const FString& FallbackName,
	UMaterialInstanceConstant* Material)
{
	auto InitAsset = [&](UStaticMesh& Asset)
	{
		AGX_ImportUtilities_helpers::InitStaticMesh(
			&FAGX_EditorUtilities::CreateRawMeshFromTrimesh, Trimesh, Asset, true);

		if (Material != nullptr)
			Asset.SetMaterial(0, Material);
	};

	FString TrimeshSourceName = Trimesh.GetSourceName();
	if (TrimeshSourceName.Contains("\\") || TrimeshSourceName.Contains("/"))
	{
		TrimeshSourceName = FPaths::GetBaseFilename(TrimeshSourceName);
	}

	if (!TrimeshSourceName.StartsWith(TEXT("SM_")) && !TrimeshSourceName.IsEmpty())
	{
		TrimeshSourceName = FString::Printf(TEXT("SM_%s"), *TrimeshSourceName);
	}

	return PrepareWriteAssetToDisk<UStaticMesh>(
		DirectoryPath, TrimeshSourceName, FallbackName, TEXT("StaticMesh"), InitAsset);
}

UStaticMesh* FAGX_ImportUtilities::SaveImportedStaticMeshAsset(
	const FRenderDataBarrier& RenderData, const FString& DirectoryPath,
	UMaterialInstanceConstant* Material)
{
	auto InitAsset = [&](UStaticMesh& Asset)
	{
		AGX_ImportUtilities_helpers::InitStaticMesh(
			&FAGX_EditorUtilities::CreateRawMeshFromRenderData, RenderData, Asset, true);

		if (Material != nullptr)
			Asset.SetMaterial(0, Material);
	};

	return PrepareWriteAssetToDisk<UStaticMesh>(
		DirectoryPath, FString::Printf(TEXT("SM_RenderMesh_%s"), *RenderData.GetGuid().ToString()),
		TEXT("SM_RenderMesh"), TEXT("RenderMesh"), InitAsset);
}

namespace
{
	bool IsUniqueTemplateComponentName(UActorComponent& Component, const FString& Name)
	{
		UObject* ExistingObject =
			StaticFindObject(/*Class=*/nullptr, Component.GetOuter(), *Name, true);

		if (ExistingObject != nullptr)
		{
			return false;
		}

		for (auto Instance : FAGX_ObjectUtilities::GetArchetypeInstances(Component))
		{
			if (Instance != nullptr &&
				Instance->HasAllFlags(RF_ArchetypeObject | RF_InheritableComponentTemplate))
			{
				return IsUniqueTemplateComponentName(*Instance, Name);
			}
		}
		return true;
	}

	FString GetUniqueNameForComponentTemplate(UActorComponent& Component, const FString& WantedName)
	{
		AGX_CHECK(FAGX_ObjectUtilities::IsTemplateComponent(Component));
		FString Name = WantedName;
		int suffix = 1;
		UBlueprint* Bp = FAGX_BlueprintUtilities::GetBlueprintFrom(Component);
		if (Bp == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Unable to get Bluprint from Component '%s'. The final name may not be "
					 "correct."),
				*Component.GetName());
			return Component.GetName() + FGuid::NewGuid().ToString();
		}

		auto IsUniqueName =
			[](const FString& InName, UBlueprint& Blueprint, UActorComponent& InComponent)
		{
			if (Blueprint.SimpleConstructionScript->FindSCSNode(FName(InName)) != nullptr)
			{
				return false;
			}

			// This is a similar check as in UObject::Rename(). Normally, we expect to never get a
			// match below since no SCS Node has this name (according to above check), but in some
			// cases there was an unexpected crash caused by an object being found with the
			// wanted name. The reason is not clear, perhaps some lingering, removed object not yet
			// destroyed?
			// See the comment in AGX_ImporterToBlueprint.cpp - RemoveDeletedComponents() which is
			// related to this.
			const FString TemplateName = InName + UActorComponent::ComponentTemplateNameSuffix;
			return IsUniqueTemplateComponentName(InComponent, TemplateName);
		};

		while (!IsUniqueName(Name, *Bp, Component))
		{
			Name = FString::Printf(TEXT("%s%d"), *WantedName, suffix++);
		}
		return Name;
	}

	// This is to some extent mimicking the behavior of
	// FComponentEditorUtils::GenerateValidVariableName but works for TemplateComponents which have
	// no owner Actor.
	FString GenerateValidVariableNameTemplateComponent(const UActorComponent& Component)
	{
		AGX_CHECK(FAGX_ObjectUtilities::IsTemplateComponent(Component));
		FString ComponentName =
			FBlueprintEditorUtils::GetClassNameWithoutSuffix(Component.GetClass());

		const FString SuffixToStrip(TEXT("Component"));
		if (ComponentName.EndsWith(SuffixToStrip))
		{
#if UE_VERSION_OLDER_THAN(5, 5, 0)
			ComponentName.LeftInline(ComponentName.Len() - SuffixToStrip.Len(), false);
#else
			ComponentName.LeftInline(
				ComponentName.Len() - SuffixToStrip.Len(), EAllowShrinking::No);
#endif
		}

		UBlueprint* Blueprint = FAGX_BlueprintUtilities::GetBlueprintFrom(Component);
		if (Blueprint == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Unable to get Bluprint from Component '%s'. The final name may not be "
					 "correct."),
				*Component.GetName());
			return Component.GetName() + FGuid::NewGuid().ToString();
		}

		// Find a suffix number that makes the name unique within the Blueprint. Start at 1 because
		// that is what FComponentEditorUtils::GenerateValidVariableName does, and that function is
		// called when the model is first imported, as opposed to synchronized which is what we're
		// doing here.
		for (int i = 1; FAGX_BlueprintUtilities::NameExists(*Blueprint, ComponentName); i++)
		{
			ComponentName = FString::Printf(TEXT("%s%d"), *ComponentName, i);
		}
		return ComponentName;
	}

	/**
	 * Checks whether the component name is valid, and if not, generates a valid name and sets it to
	 * the component.
	 */
	FString GetFinalizedComponentName(UActorComponent& Component, const FString& WantedName)
	{
		if (Component.GetOwner() == nullptr &&
			!FAGX_ObjectUtilities::IsTemplateComponent(Component))
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Could not find the owning actor of Actor Component: %s during name "
					 "finalization. The Component might not get the wanted name."),
				*Component.GetName());
			return FGuid::NewGuid().ToString();
		}

		if (!FComponentEditorUtils::IsValidVariableNameString(&Component, WantedName))
		{
			if (FAGX_ObjectUtilities::IsTemplateComponent(Component))
			{
				return GenerateValidVariableNameTemplateComponent(Component);
			}
			else
			{
				return FComponentEditorUtils::GenerateValidVariableName(
					Component.GetClass(), Component.GetOwner());
			}
		}

		return WantedName;
	}
}

UAGX_TrackInternalMergeProperties*
FAGX_ImportUtilities::SaveImportedTrackInternalMergePropertiesAsset(
	const FTrackBarrier& Barrier, const FString& DirectoryPath, const FString& Name)
{
	auto InitAsset = [&](UAGX_TrackInternalMergeProperties& Asset) { Asset.CopyFrom(Barrier); };

	UAGX_TrackInternalMergeProperties* Asset =
		PrepareWriteAssetToDisk<UAGX_TrackInternalMergeProperties>(
			DirectoryPath, Name, TEXT(""), TEXT("TrackInternalMergeProperties"), InitAsset);
	if (Asset == nullptr || !WriteAssetToDisk(*Asset))
	{
		return nullptr;
	}
	return Asset;
}

UAGX_TrackProperties* FAGX_ImportUtilities::SaveImportedTrackPropertiesAsset(
	const FTrackPropertiesBarrier& Barrier, const FString& DirectoryPath, const FString& Name)
{
	auto InitAsset = [&](UAGX_TrackProperties& Asset) { Asset.CopyFrom(Barrier); };

	UAGX_TrackProperties* Asset = PrepareWriteAssetToDisk<UAGX_TrackProperties>(
		DirectoryPath, Name, TEXT(""), TEXT("TrackProperties"), InitAsset);
	if (Asset == nullptr || !WriteAssetToDisk(*Asset))
	{
		return nullptr;
	}

	return Asset;
}

FString FAGX_ImportUtilities::CreateName(UObject& Object, const FString& Name)
{
	if (Name.IsEmpty())
	{
		// Not having an imported name means use whatever default name Unreal decided.
		return Name;
	}
	if (Object.Rename(*Name, nullptr, REN_Test))
	{
		return Name;
	}
	else
	{
		FName NewName = MakeUniqueObjectName(Object.GetOuter(), Object.GetClass(), FName(*Name));
		UE_LOG(
			LogAGX, Log, TEXT("%s '%s' imported with name '%s' because of name conflict."),
			*Object.GetClass()->GetName(), *Name, *NewName.ToString());
		return NewName.ToString();
	}
}

void FAGX_ImportUtilities::Rename(UActorComponent& Component, const FString& Name)
{
	if (FAGX_ObjectUtilities::IsTemplateComponent(Component))
	{
		if (FAGX_BlueprintUtilities::GetRegularNameFromTemplateComponentName(Component.GetName()) ==
			Name)
			return; // Wanted name is already set, we are done.
	}
	else
	{
		if (Component.GetName() == Name)
			return; // Wanted name is already set, we are done.
	}

	const FString ValidName = [&]()
	{
		if (FAGX_ObjectUtilities::IsTemplateComponent(Component))
		{
			return GetUniqueNameForComponentTemplate(Component, Name);
		}
		else
		{
			return CreateName(static_cast<UObject&>(Component), Name);
		}
	}();

	const FString FinalName = GetFinalizedComponentName(Component, ValidName);

	if (FAGX_ObjectUtilities::IsTemplateComponent(Component))
	{
		UBlueprint* Bp = FAGX_BlueprintUtilities::GetBlueprintFrom(Component);

		USCS_Node* Node =
			FAGX_BlueprintUtilities::GetSCSNodeFromComponent(*Bp, &Component, false).FoundNode;
		if (Node == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Unable to set name '%s' for Component '%s' because the owning SCS Node could "
					 "not be retrieved. The Component will not be renamed."),
				*FinalName, *Component.GetName());
			return;
		}

		// For future reference: earlier we sometimes got crashes inside this function. This was
		// when model synchronization was made with the Blueprint Editor opened. Now we always close
		// all asset editors prior to doing model synchronization, and the issue seems resolved.
		// However, if this function for some reason starts crashing on Existing object found, then
		// look in the internal gitlab history for feature/model-synchronization, commit 50ea329b.
		// It has a work-around for this. Basically, the issue was that on rare occasions, an
		// archetype instance was found for a Component Template that did not belong to that
		// Component. So the solution was to check if any archetype instances had non matching
		// names, and if so, calling SetVariableName with bRenameTemplate == false, and then
		// updating matched archetype instances manually in that case.
		Node->SetVariableName(FName(FinalName));
	}
	else
	{
		Component.Rename(*FinalName);
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
