// Copyright 2025, Algoryx Simulation AB.

#include "AGXUnreal.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Environment.h"
#include "AGX_LogCategory.h"
#include "AGX_RuntimeStyle.h"
#include "Materials/AGX_ShapeMaterial.h"
#include "Materials/AGX_TerrainMaterial.h"
#include "Utilities/AGX_ObjectUtilities.h"

// Unreal Engine includes.
#include "UObject/CoreRedirects.h"
#include "RenderGraphResources.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include "ShaderCore.h"

#define LOCTEXT_NAMESPACE "FAGXUnrealModule"

namespace AGXUnrealModule_helpers
{
	void PrintVersion()
	{
		const FString Version = FAGX_Environment::GetPluginVersion();
		FString Revision = FAGX_Environment::GetPluginRevision();
		if (!Revision.IsEmpty())
		{
			Revision = FString::Printf(TEXT(", revision %s"), *Revision);
		}
		UE_LOG(
			LogAGX, Log, TEXT("AGX Dynamics for Unreal (AGXUnreal) version %s%s."), *Version,
			*Revision);
	}
}

static struct FShaderDirectoryMapping
{
	// Where to properly do this? Currently ugly hack.
	FShaderDirectoryMapping()
	{
		// Map the existing folder containing particle upsampling shader files to virtual shader directory.
		FString PluginShaderDir = FPaths::Combine(
			IPluginManager::Get().FindPlugin(TEXT("AGXUnreal"))->GetBaseDir(),
			TEXT("Source/AGXUnreal/Private/Terrain/ParticleRendering/Shaders"));
		AddShaderSourceDirectoryMapping(TEXT("/ParticleRenderingShaders"), PluginShaderDir);
	}
} GShaderDirectoryMapping;

void FAGXUnrealModule::StartupModule()
{
	using namespace AGXUnrealModule_helpers;
	PrintVersion();
	RegisterCoreRedirects();
	FAGX_RuntimeStyle::Initialize();
	FAGX_RuntimeStyle::ReloadTextures();
	LoadRuntimeAssets();
}

void FAGXUnrealModule::ShutdownModule()
{
	FAGX_RuntimeStyle::Shutdown();
}

void FAGXUnrealModule::RegisterCoreRedirects()
{
	// This is used to handle name changes of UFUNCTIONs, UPROPERTYs and UObjects (backward
	// compatibility). See
	// https://docs.unrealengine.com/en-US/core-redirects-in-unreal-engine/

	TArray<FCoreRedirect> Redirects;

	Redirects.Emplace(
		ECoreRedirectFlags::Type_Class, TEXT("AGX_ContactMaterialBase"),
		TEXT("AGX_ContactMaterial"));
	Redirects.Emplace(
		ECoreRedirectFlags::Type_Class, TEXT("AGX_ContactMaterialAsset"),
		TEXT("AGX_ContactMaterial"));
	Redirects.Emplace(
		ECoreRedirectFlags::Type_Class, TEXT("AGX_ContactMaterialInstance"),
		TEXT("AGX_ContactMaterial"));
	Redirects.Emplace(
		ECoreRedirectFlags::Type_Class, TEXT("AGX_ShapeMaterialBase"), TEXT("AGX_ShapeMaterial"));
	Redirects.Emplace(
		ECoreRedirectFlags::Type_Class, TEXT("AGX_ShapeMaterialAsset"), TEXT("AGX_ShapeMaterial"));
	Redirects.Emplace(
		ECoreRedirectFlags::Type_Class, TEXT("AGX_ShapeMaterialInstance"),
		TEXT("AGX_ShapeMaterial"));
	Redirects.Emplace(
		ECoreRedirectFlags::Type_Class, TEXT("AGX_TerrainMaterialBase"),
		TEXT("AGX_TerrainMaterial"));
	Redirects.Emplace(
		ECoreRedirectFlags::Type_Class, TEXT("AGX_TerrainMaterialAsset"),
		TEXT("AGX_TerrainMaterial"));
	Redirects.Emplace(
		ECoreRedirectFlags::Type_Class, TEXT("AGX_TerrainMaterialInstance"),
		TEXT("AGX_TerrainMaterial"));
	Redirects.Emplace(
		ECoreRedirectFlags::Type_Class, TEXT("AGX_MaterialBase"), TEXT("AGX_ShapeMaterial"));

	// The Shovel Refactor effort, the addition of Shovel Component, also introduced
	// FAGX_ComponentReference and replaced the FAGX_RigidBodyComponentReference and
	// FAGX_SceneComponentReference implementations with ones based on the new reference class. For
	// a while we had both FAGX_BodyReference and FAGX_RigidBodyReference used in different parts of
	// the code. Eventually the old one was removed and the new took the old ones' name. This
	// redirect makes it so that scenes created during that interim time can find the new name even
	// though the asset was saved with the old. I think this was only ever used by the Shovel
	// Component and never released to users.
	//
	// In the switch the name of the property naming the referenced Component was changed from
	// BodyName to just Name. Add a redirect for that as well, so that assets using the old
	// FAGX_RigidBodyReference can be restored into the the new FAGX_ComponentReference based
	// FAGX_RigidBodyReference. Also the same for SceneComponentName used by the old
	// FAGX_SceneComponentReference.
	Redirects.Emplace(
		ECoreRedirectFlags::Type_Struct, TEXT("AGX_BodyReference"), TEXT("AGX_RigidBodyReference"));
	Redirects.Emplace(
		ECoreRedirectFlags::Type_Property,
		TEXT("/Script/AGXUnreal.AGX_RigidBodyReference.BodyName"),
		TEXT("/Script/AGXUnreal.AGX_RigidBodyReference.Name"));
	Redirects.Emplace(
		ECoreRedirectFlags::Type_Property,
		TEXT("/Script/AGXUnreal.AGX_SceneComponentReference.SceneComponentName"),
		TEXT("/Script/AGXUnreal.AGX_SceneComponentReference.Name"));

	// Properties renamed in AGX Dynamics with the introduction of AGX Dynamics 2.37.
	Redirects.Emplace(
		ECoreRedirectFlags::Type_Property, TEXT("AGX_Shovel.BottomContactThreshold"),
		TEXT("ContactRegionThreshold"));
	Redirects.Emplace(
		ECoreRedirectFlags::Type_Property, TEXT("AGX_Shovel.bOverrideBottomContactThreshold"),
		TEXT("bOverrideContactRegionThreshold"));

	// ShapeMaterial UFUNCTIONs with _BP postfix removed.
	// This should be feasable to removed in AGXUnreal 1.14 or later.
	for (TFieldIterator<UFunction> FuncIt(UAGX_ShapeMaterial::StaticClass()); FuncIt; ++FuncIt)
	{
		UFunction* Function = *FuncIt;
		const FString BpName =
			FString::Printf(TEXT("AGX_ShapeMaterial.%s_BP"), *Function->GetName());
		Redirects.Emplace(ECoreRedirectFlags::Type_Function, *BpName, *Function->GetName());
	}

	// TerrainMaterial UFUNCTIONs with _BP postfix removed.
	// This should be feasable to removed in AGXUnreal 1.14 or later.
	for (TFieldIterator<UFunction> FuncIt(UAGX_TerrainMaterial::StaticClass()); FuncIt; ++FuncIt)
	{
		UFunction* Function = *FuncIt;
		const FString BpName =
			FString::Printf(TEXT("AGX_TerrainMaterial.%s_BP"), *Function->GetName());
		Redirects.Emplace(ECoreRedirectFlags::Type_Function, *BpName, *Function->GetName());
	}

	FCoreRedirects::AddRedirectList(Redirects, TEXT("AGXUnreal"));
}

void FAGXUnrealModule::LoadRuntimeAssets()
{
	// Explicitly loading these assets here ensures they are included in cooked builds.
	// This is critical for e.g. runtime import since some assets may be needed and there is no way
	// for Unreal Engine to know at cook-time which assets might be needed since only a path to a
	// model file may be what is available at that time, for example.
	// Assets that are referenced directly or indirectly by something in the Level itself will be
	// included in the cooked build automatically, so there is no need to list all AGXUnreal assets
	// here, only those that may be needed for runtime creation.
	// Also note that using a node in the Blueprint Event Graph to select an asset to assign to some
	// object in runtime is enough to make it included in the cooked build automatically as well.

	// Example to make this more clear: the UAGX_WireComponent uses some Static Meshes to render
	// itself. If a user adds a WireComponent in the Level, and builds a standalone executable from
	// the project, the Static Meshes will be included automatically in the Cooked Build since it is
	// referenced at cook time. HOWEVER, say a user has an empty Level and wishes to runtime import
	// an .agx file containing a Wire. This will not work automatically since there was no way for
	// Unreal to know at cook time that the Wire's Static Meshes would be needed during runtime.
	// Therefore, the Static Meshes used by the UAGX_WireComponent must be listed below.

	auto LoadWCheck = [](const TCHAR* Path)
	{
		auto A = FAGX_ObjectUtilities::GetAssetFromPath<UObject>(Path);
		if (A == nullptr)
		{
			// Log Error to catch this in unit tests.
			UE_LOG(
				LogAGX, Error,
				TEXT("Could not load Asset given path '%s'. Ensure it has not beed deleted."),
				Path);
		}
	};

	// Render Materials.
	LoadWCheck(TEXT("Material'/AGXUnreal/Runtime/Materials/M_SensorMaterial.M_SensorMaterial'"));
	LoadWCheck(TEXT("Material'/AGXUnreal/Runtime/Materials/M_ImportedBase.M_ImportedBase'"));
	LoadWCheck(TEXT("Material'/AGXUnreal/Track/Materials/MI_TrackDefault.MI_TrackDefault'"));
	LoadWCheck(TEXT("Material'/AGXUnreal/Wire/MI_GrayWire.MI_GrayWire'"));

	// Static Meshes.
	LoadWCheck(TEXT("StaticMesh'/AGXUnreal/Wire/SM_WireVisualCylinder.SM_WireVisualCylinder'"));
	LoadWCheck(TEXT("StaticMesh'/AGXUnreal/Wire/SM_WireVisualSphere.SM_WireVisualSphere'"));
	LoadWCheck(TEXT("StaticMesh'/AGXUnreal/Track/StaticMeshes/SM_TrackShoeCube.SM_TrackShoeCube'"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAGXUnrealModule, AGXUnreal)
