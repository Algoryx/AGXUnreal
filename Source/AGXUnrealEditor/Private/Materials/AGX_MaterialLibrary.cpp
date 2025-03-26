// Copyright 2024, Algoryx Simulation AB.

#include "Materials/AGX_MaterialLibrary.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Materials/AGX_ContactMaterial.h"
#include "Materials/AGX_ShapeMaterial.h"
#include "Materials/AGX_TerrainMaterial.h"
#include "Materials/MaterialLibraryBarrier.h"
#include "Sensors/AGX_LidarAmbientMaterial.h"
#include "Sensors/RtAmbientMaterialBarrier.h"
#include "Sensors/SensorEnvironmentBarrier.h"
#include "Utilities/AGX_ImportUtilities.h"
#include "Utilities/AGX_ObjectUtilities.h"

// Unreal Engine includes.
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "Modules/ModuleManager.h"
#include "PackageTools.h"
#include "Misc/EngineVersionComparison.h"
#include "UObject/SavePackage.h"

// Naming convention:
//   Name: dirt_1
//   AssetName: AGX_TM_dirt_1.
//   PackagePath: /AGXUnreal/Terrain/TerrainMaterials/
//   AssetPath: /AGXUnreal/Terrain/TerrainMaterials/AGX_TM_dirt_1.AGX_TM_dirt_1

namespace AGX_MaterialLibrary_helpers
{
	enum class LibraryMaterialType
	{
		ContactMaterial,
		ShapeMaterial,
		TerrainMaterial
	};

	FString ToAssetName(const FString& NameAGX, LibraryMaterialType Type)
	{
		switch (Type)
		{
			case LibraryMaterialType::ContactMaterial:
				return FString::Printf(TEXT("AGX_CM_%s"), *NameAGX);
			case LibraryMaterialType::ShapeMaterial:
				return FString::Printf(TEXT("AGX_SM_%s"), *NameAGX);
			case LibraryMaterialType::TerrainMaterial:
				return FString::Printf(TEXT("AGX_TM_%s"), *NameAGX);
		}

		UE_LOG(
			LogAGX, Warning,
			TEXT("Usupported LibraryMaterialType was given to ToAssetName for material '%s'."),
			*NameAGX);
		return FString();
	}

	FString ToMaterialDirectoryContentBrowser(LibraryMaterialType Type)
	{
		switch (Type)
		{
			case LibraryMaterialType::ContactMaterial:
				return FString(TEXT("/AGXUnreal/Shape/ContactMaterialLibrary"));
			case LibraryMaterialType::ShapeMaterial:
				return FString(TEXT("/AGXUnreal/Shape/ShapeMaterialLibrary"));
			case LibraryMaterialType::TerrainMaterial:
				return FString(TEXT("/AGXUnreal/Terrain/TerrainMaterialLibrary"));
		}

		UE_LOG(
			LogAGX, Warning,
			TEXT("Usupported LibraryMaterialType was given to ToMaterialDirectoryContentBrowser."));
		return FString();
	}

	template <typename MaterialType, typename BarrierType, typename MaterialLoadFunc>
	MaterialType* EnsureMaterialImported(
		const FString& NameAGX, LibraryMaterialType Type, MaterialLoadFunc LoadFunc,
		bool ForceOverwrite)
	{
		// Create a package for our asset.
		const FString Name = UPackageTools::SanitizePackageName(NameAGX);
		const FString AssetName = ToAssetName(NameAGX, Type);
		const FString AssetDir = ToMaterialDirectoryContentBrowser(Type);
		const FString AssetPath = FString::Printf(TEXT("%s/%s"), *AssetDir, *AssetName);
		const bool OldAssetExists = FPackageName::DoesPackageExist(AssetPath);

		// Create the asset itself, reading data from the AGX Dynamics material library.
		auto OptionalBarrier = LoadFunc(Name);
		if (!OptionalBarrier.IsSet())
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Unable to import Material '%s' from AGX Material Library to '%s'. The "
					 "material may not be avaiable in the AGX Dynamics for Unreal Contents."),
				*NameAGX, *AssetPath);
			return nullptr;
		}

		const BarrierType& Material = OptionalBarrier.GetValue();
		MaterialType* Asset = nullptr;
		if (OldAssetExists)
		{
			Asset = LoadObject<MaterialType>(nullptr, *AssetPath);
			if (!ForceOverwrite)
				return Asset;
		}
		else
		{
			Asset = FAGX_ImportUtilities::CreateAsset<MaterialType>(AssetDir, AssetName, "");
		}

		Asset->CopyFrom(Material, nullptr);

		// Must fully load the package or else project packaging will fail with:
		//
		//    Package /AGXUnreal/Terrain/TerrainMaterialLibrary/AGX_TM_gravel_1 supposed
		//    to be fully loaded but isn't. RF_WasLoaded is set
		//
		//    Unable to cook package for platform because it is unable to be loaded:
		//    <PATH>/AGXUnreal/Content/Terrain/TerrainMaterialLibrary/AGX_TM_gravel_1.uasset
		//
		// I'm not entirely sure where the FullyLoad call should be for it to
		// take effect in all cases, so there are a few of them. Remove the
		// unnecessary ones once we know which can safely be removed.
		FAGX_ObjectUtilities::SaveAsset(*Asset, true);
		return Asset;
	}

	template <typename BarrierFunc>
	bool UpdateLidarAmbientMaterialAssetLibrary(
		const FString& Name, BarrierFunc Func, bool ForceOverwrite)
	{
		const FString AssetDir(TEXT("/AGXUnreal/Sensor/Lidar/AmbientMaterialLibrary"));

		// Create a package for our asset.
		const FString AssetName = UPackageTools::SanitizePackageName(Name);
		const FString AssetPath = FString::Printf(TEXT("%s/%s"), *AssetDir, *AssetName);
		const bool OldAssetExists = FPackageName::DoesPackageExist(AssetPath);

		UAGX_LidarAmbientMaterial* Asset = nullptr;
		if (OldAssetExists)
		{
			Asset = LoadObject<UAGX_LidarAmbientMaterial>(nullptr, *AssetPath);
			if (!ForceOverwrite)
				return Asset != nullptr;
		}
		else
		{
			Asset = FAGX_ImportUtilities::CreateAsset<UAGX_LidarAmbientMaterial>(
				AssetDir, AssetName, "");
		}

		if (Asset == nullptr)
		{
			UE_LOG(
				LogAGX, Error, TEXT("Unable to initialize Lidar Ambient Material Asset '%s'"),
				*AssetPath);
			return false;
		}

		FRtAmbientMaterialBarrier Barrier;
		Barrier.AllocateNative();
		Func(Barrier);
		Asset->CopyFrom(Barrier);
		Barrier.ReleaseNative();

		FAGX_ObjectUtilities::SaveAsset(*Asset, true);
		return true;
	}

	bool AssignLibraryShapeMaterialsToContactMaterial(
		UAGX_ContactMaterial& OutCm, const FString& NameAGX)
	{
		if (NameAGX.IsEmpty())
			return false;

		// Contact Materials in the AGX Dynamics Materials Library does not have agx::Materials
		// assigned to them. The only conenction between those Contact Materials and the
		// corresponding agx::Materials is the name which has the form "materialName-materialName".
		// So here we use that fact to extract the agx::Material names and look for the
		// corresponding Shape Material Asset by name and assign that to the Contact Material. This
		// approach is not pretty but is the best mechanism known at the time of writing this.
		TArray<FString> NameAGXSplit;
		NameAGX.ParseIntoArray(NameAGXSplit, TEXT("-"));
		if (NameAGXSplit.Num() != 2)
			return false;

		auto GetShapeMaterialByName = [](const FString& Name)
		{
			const FString ShapeMaterialPath = FString::Printf(
				TEXT("%s/%s"),
				*ToMaterialDirectoryContentBrowser(LibraryMaterialType::ShapeMaterial),
				*ToAssetName(Name, LibraryMaterialType::ShapeMaterial));
			return LoadObject<UAGX_ShapeMaterial>(nullptr, *ShapeMaterialPath);
		};

		UAGX_ShapeMaterial* Mat1 = GetShapeMaterialByName(NameAGXSplit[0]);
		if (Mat1 == nullptr)
			return false;

		UAGX_ShapeMaterial* Mat2 = GetShapeMaterialByName(NameAGXSplit[1]);
		if (Mat2 == nullptr)
			return false;

		OutCm.Material1 = Mat1;
		OutCm.Material2 = Mat2;
		return true;
	}
}

bool AGX_MaterialLibrary::InitializeContactMaterialAssetLibrary(bool ForceOverwrite)
{
	using namespace AGX_MaterialLibrary_helpers;
	using namespace AGX_MaterialLibraryBarrier;

	bool IssuesEncountered = false;
	const TArray<FString> Names = AGX_MaterialLibraryBarrier::GetAvailableLibraryContactMaterials();
	for (const FString& NameAGX : Names)
	{
		UAGX_ContactMaterial* Cm =
			EnsureMaterialImported<UAGX_ContactMaterial, FContactMaterialBarrier>(
				NameAGX, LibraryMaterialType::ContactMaterial, LoadContactMaterialProfile,
				ForceOverwrite);
		if (Cm == nullptr)
		{
			IssuesEncountered = true;
			continue; // Logging done in EnsureMaterialImported.
		}

		bool bNeedSave {false};

		// The AGX Dynamics Material Library imports Contact Materials without a Friction Model.
		// This causes the value to be set to Not Defined, which in turn causes no AGX Dynamics
		// Friction Model to be created at Begin Play.
		if (Cm->FrictionModel == EAGX_FrictionModel::NotDefined)
		{
			Cm->FrictionModel = EAGX_FrictionModel::IterativeProjectedConeFriction;
			bNeedSave = true;
		}
		if (Cm->ContactSolver == EAGX_ContactSolver::NotDefined)
		{
			Cm->ContactSolver = EAGX_ContactSolver::Split;
			bNeedSave = true;
		}

		if (AssignLibraryShapeMaterialsToContactMaterial(*Cm, NameAGX))
		{
			bNeedSave = true;
		}
		else
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Unable to assign Shape Materials for Contact Material '%s' in the Material "
					 "Library. This Contact Material will not have the correct Shape Materials set "
					 "up."),
				*Cm->GetName());
			IssuesEncountered = true;
		}

		if (bNeedSave)
		{
			FAGX_ObjectUtilities::SaveAsset(*Cm, true);
		}
	}

	return !IssuesEncountered;
}

bool AGX_MaterialLibrary::InitializeShapeMaterialAssetLibrary(bool ForceOverwrite)
{
	using namespace AGX_MaterialLibrary_helpers;
	using namespace AGX_MaterialLibraryBarrier;

	bool IssuesEncountered = false;
	const TArray<FString> Names = AGX_MaterialLibraryBarrier::GetAvailableLibraryShapeMaterials();
	for (const FString& NameAGX : Names)
	{
		auto mat = EnsureMaterialImported<UAGX_ShapeMaterial, FShapeMaterialBarrier>(
			NameAGX, LibraryMaterialType::ShapeMaterial, LoadShapeMaterialProfile, ForceOverwrite);
		if (mat == nullptr)
			IssuesEncountered = true;
	}

	return !IssuesEncountered;
}

bool AGX_MaterialLibrary::InitializeTerrainMaterialAssetLibrary(bool ForceOverwrite)
{
	using namespace AGX_MaterialLibrary_helpers;
	using namespace AGX_MaterialLibraryBarrier;

	bool IssuesEncountered = false;
	const TArray<FString> Names = GetAvailableLibraryTerrainMaterials();
	for (const FString& NameAGX : Names)
	{
		auto mat = EnsureMaterialImported<UAGX_TerrainMaterial, FTerrainMaterialBarrier>(
			NameAGX, LibraryMaterialType::TerrainMaterial, LoadTerrainMaterialProfile,
			ForceOverwrite);
		if (mat == nullptr)
			IssuesEncountered = true;
	}

	return !IssuesEncountered;
}

bool AGX_MaterialLibrary::InitializeLidarAmbientMaterialAssetLibrary(bool ForceOverwrite)
{
	using namespace AGX_MaterialLibrary_helpers;

	if (!FSensorEnvironmentBarrier::IsRaytraceSupported())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT(
				"Lidar raytracing (RTX) not supported on this computer, unable to initialize Lidar "
				"Ambient Material Library. To enable Lidar raytracing (RTX) support, use an RTX "
				"Graphical Processing Unit (GPU)."));
		return false;
	}
	else
	{
		UE_LOG(LogAGX, Log, TEXT("Lidar raytracing (RTX) is supported by the hardware."));
	}

	bool Result = true;

	Result &= UpdateLidarAmbientMaterialAssetLibrary(
		"AGX_LAM_Air_1000m_Visibility",
		[](FRtAmbientMaterialBarrier& Barrier) { Barrier.ConfigureAsAir(1.0f); }, ForceOverwrite);

	Result &= UpdateLidarAmbientMaterialAssetLibrary(
		"AGX_LAM_Air_2500m_Visibility",
		[](FRtAmbientMaterialBarrier& Barrier) { Barrier.ConfigureAsAir(2.5f); }, ForceOverwrite);

	Result &= UpdateLidarAmbientMaterialAssetLibrary(
		"AGX_LAM_Air_5000m_Visibility",
		[](FRtAmbientMaterialBarrier& Barrier) { Barrier.ConfigureAsAir(5.0f); }, ForceOverwrite);

	Result &= UpdateLidarAmbientMaterialAssetLibrary(
		"AGX_LAM_Air_50000m_Visibility",
		[](FRtAmbientMaterialBarrier& Barrier) { Barrier.ConfigureAsAir(50.0f); }, ForceOverwrite);

	Result &= UpdateLidarAmbientMaterialAssetLibrary(
		"AGX_LAM_Fog_3000m_Visibility",
		[](FRtAmbientMaterialBarrier& Barrier) { Barrier.ConfigureAsFog(3.0f, 900.f); },
		ForceOverwrite);

	Result &= UpdateLidarAmbientMaterialAssetLibrary(
		"AGX_LAM_Fog_2000m_Visibility",
		[](FRtAmbientMaterialBarrier& Barrier) { Barrier.ConfigureAsFog(2.0f, 900.f); },
		ForceOverwrite);

	Result &= UpdateLidarAmbientMaterialAssetLibrary(
		"AGX_LAM_Fog_1000m_Visibility",
		[](FRtAmbientMaterialBarrier& Barrier) { Barrier.ConfigureAsFog(1.0f, 900.f); },
		ForceOverwrite);

	Result &= UpdateLidarAmbientMaterialAssetLibrary(
		"AGX_LAM_Fog_200m_Visibility",
		[](FRtAmbientMaterialBarrier& Barrier) { Barrier.ConfigureAsFog(0.2f, 900.f); },
		ForceOverwrite);

	Result &= UpdateLidarAmbientMaterialAssetLibrary(
		"AGX_LAM_Rainfall_1mm_Per_Hour",
		[](FRtAmbientMaterialBarrier& Barrier) { Barrier.ConfigureAsRainfall(1.f); },
		ForceOverwrite);

	Result &= UpdateLidarAmbientMaterialAssetLibrary(
		"AGX_LAM_Rainfall_4mm_Per_Hour",
		[](FRtAmbientMaterialBarrier& Barrier) { Barrier.ConfigureAsRainfall(4.f); },
		ForceOverwrite);

	Result &= UpdateLidarAmbientMaterialAssetLibrary(
		"AGX_LAM_Rainfall_8mm_Per_Hour",
		[](FRtAmbientMaterialBarrier& Barrier) { Barrier.ConfigureAsRainfall(8.f); },
		ForceOverwrite);

	Result &= UpdateLidarAmbientMaterialAssetLibrary(
		"AGX_LAM_Snowfall_1mm_Per_Hour",
		[](FRtAmbientMaterialBarrier& Barrier) { Barrier.ConfigureAsSnowfall(1.f, 900.f); },
		ForceOverwrite);

	Result &= UpdateLidarAmbientMaterialAssetLibrary(
		"AGX_LAM_Snowfall_4mm_Per_Hour",
		[](FRtAmbientMaterialBarrier& Barrier) { Barrier.ConfigureAsSnowfall(4.f, 900.f); },
		ForceOverwrite);

	Result &= UpdateLidarAmbientMaterialAssetLibrary(
		"AGX_LAM_Snowfall_8mm_Per_Hour",
		[](FRtAmbientMaterialBarrier& Barrier) { Barrier.ConfigureAsSnowfall(8.f, 900.f); },
		ForceOverwrite);

	return Result;
}
