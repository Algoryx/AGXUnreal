// Copyright 2026, Algoryx Simulation AB.

#if WITH_DEV_AUTOMATION_TESTS

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AgxAutomationCommon.h"
#include "Import/AGX_ImporterToEditor.h"
#include "Import/AGX_ImportSettings.h"
#include "Import/AGX_ModelSourceComponent.h"
#include "Utilities/AGX_BlueprintUtilities.h"
#include "Utilities/AGX_ImportUtilities.h"
#include "Utilities/OpenPLXUtilities.h"

// Unreal Engine includes.
#include "Components/StaticMeshComponent.h"
#include "Engine/Blueprint.h"
#include "Engine/Texture.h"
#include "Engine/Texture2D.h"
#include "Engine/TextureDefines.h"
#include "HAL/FileManager.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Materials/MaterialInterface.h"
#include "Misc/AutomationTest.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Tests/AutomationCommon.h"
#include "UObject/Package.h"

/*
 * This file contains tests for importing OpenPLX files to Blueprints.
 *
 * Common import, basic validation, and cleanup logic lives at the top. Each concrete test below
 * specifies one OpenPLX file, its expected copied OpenPLX files, its expected imported assets, and
 * any Blueprint state checks specific to that model.
 */

namespace OpenPLX_ImporterToBlueprintTest_helpers
{
	struct FOpenPLXImportState
	{
		FString OpenPLXFile;
		TArray<const TCHAR*> ExpectedCopiedOpenPLXFiles;
		// The generated base Blueprint asset is added by the common cleanup command.
		TArray<const TCHAR*> ExpectedImportedAssetsExcludingBaseBP;

		UBlueprint* Blueprint = nullptr;
		FString SourceFilePath;
		FString CopiedFilePath;
		FString ImportRootPackagePath;
	};

	FString GetImportRootPackagePath(const UBlueprint& Blueprint)
	{
		const FString BlueprintPackagePath = Blueprint.GetOutermost()->GetName();
		const FString BlueprintDirectory = FPaths::GetPath(BlueprintPackagePath);
		if (!FPaths::GetBaseFilename(BlueprintDirectory).Equals(TEXT("Blueprint")))
			return FString();

		return FPaths::GetPath(BlueprintDirectory);
	}

	FString GetImportRootDirectory(const UBlueprint& Blueprint)
	{
		const FString RootPackagePath = GetImportRootPackagePath(Blueprint);
		if (RootPackagePath.IsEmpty())
			return FString();

		return FPaths::ConvertRelativePathToFull(
			FPackageName::LongPackageNameToFilename(RootPackagePath));
	}

	FString NormalizeDirectory(FString Directory)
	{
		Directory = FPaths::ConvertRelativePathToFull(Directory);
		FPaths::NormalizeDirectoryName(Directory);
		return Directory;
	}

	bool IsSafeImportDirectory(const FString& Directory)
	{
		if (Directory.IsEmpty())
			return false;

		const FString ContentDirectory = NormalizeDirectory(FPaths::ProjectContentDir());
		const FString ImportDirectory = NormalizeDirectory(Directory);
		const FString ExpectedParent = NormalizeDirectory(
			FPaths::Combine(ContentDirectory, FAGX_ImportUtilities::GetImportRootDirectoryName()));

		return ImportDirectory.StartsWith(ExpectedParent + TEXT("/")) &&
			   ImportDirectory != ExpectedParent && ImportDirectory != ContentDirectory;
	}

	bool IsSafeOpenPLXModelDirectory(const FString& Directory)
	{
		if (Directory.IsEmpty())
			return false;

		const FString ModelsDirectory = NormalizeDirectory(FOpenPLXUtilities::GetModelsDirectory());
		const FString OpenPLXDirectory = NormalizeDirectory(Directory);

		return OpenPLXDirectory.StartsWith(ModelsDirectory + TEXT("/")) &&
			   OpenPLXDirectory != ModelsDirectory;
	}

	bool DeleteOpenPLXDirectoryWithExpectedContents(
		const FString& Directory, const TArray<const TCHAR*>& ExpectedFileAndDirectoryNames,
		FAutomationTestBase& Test)
	{
		if (!FPaths::DirectoryExists(Directory))
		{
			Test.AddError(FString::Printf(
				TEXT("Unable to delete directory '%s' because it does not exist."), *Directory));
			return false;
		}

		TArray<FString> DirectoryContents;
		IFileManager::Get().FindFilesRecursive(
			DirectoryContents, *Directory, TEXT("*"), true, true);
		if (DirectoryContents.Num() != ExpectedFileAndDirectoryNames.Num())
		{
			Test.AddError(FString::Printf(
				TEXT("Refusing to delete directory '%s': expected %d files and directories but "
					 "found %d."),
				*Directory, ExpectedFileAndDirectoryNames.Num(), DirectoryContents.Num()));
			return false;
		}

		for (const FString& Entry : DirectoryContents)
		{
			const FString Name = FPaths::GetCleanFilename(Entry);
			if (!ExpectedFileAndDirectoryNames.ContainsByPredicate([&Name](const TCHAR* Expected)
																   { return Name == Expected; }))
			{
				Test.AddError(FString::Printf(
					TEXT("Refusing to delete directory '%s': found unexpected entry '%s'."),
					*Directory, *Name));
				return false;
			}
		}

		const bool bDeleted =
			IFileManager::Get().DeleteDirectory(*Directory, /*RequireExists=*/true, /*Tree=*/true);
		Test.TestTrue(
			FString::Printf(TEXT("Removed directory '%s'."), *Directory),
			bDeleted && !FPaths::DirectoryExists(Directory));
		return bDeleted;
	}

	void AddCommonOpenPLXImportCommands(FOpenPLXImportState& State, FAutomationTestBase& Test);
	void AddCommonOpenPLXCleanupCommands(FOpenPLXImportState& State, FAutomationTestBase& Test);
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(
	FImportOpenPLXBlueprintCommand, OpenPLX_ImporterToBlueprintTest_helpers::FOpenPLXImportState&,
	State, FAutomationTestBase&, Test);

bool FImportOpenPLXBlueprintCommand::Update()
{
	using namespace OpenPLX_ImporterToBlueprintTest_helpers;

	const FString OpenPLXFilePath = AgxAutomationCommon::GetTestScenePath(State.OpenPLXFile);
	if (OpenPLXFilePath.IsEmpty())
	{
		Test.AddError(FString::Printf(TEXT("Did not find OpenPLX file '%s'."), *State.OpenPLXFile));
		return true;
	}

	FAGX_ImportSettings Settings;
	Settings.FilePath = OpenPLXFilePath;
	Settings.SourceFilePath = OpenPLXFilePath;
	Settings.bIgnoreDisabledTrimeshes = false;
	Settings.ImportType = EAGX_ImportType::Plx;
	Settings.bOpenBlueprintEditorAfterImport = false;

	FAGX_ImporterToEditor Importer;
	UBlueprint* ChildBlueprint = Importer.Import(Settings);
	State.Blueprint = FAGX_BlueprintUtilities::GetOutermostParent(ChildBlueprint);
	State.SourceFilePath = OpenPLXFilePath;

	Test.TestNotNull(TEXT("Imported OpenPLX Blueprint"), State.Blueprint);
	if (State.Blueprint != nullptr)
		State.ImportRootPackagePath = GetImportRootPackagePath(*State.Blueprint);

	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(
	FCheckOpenPLXBlueprintImportedCommand,
	OpenPLX_ImporterToBlueprintTest_helpers::FOpenPLXImportState&, State, FAutomationTestBase&,
	Test);

bool FCheckOpenPLXBlueprintImportedCommand::Update()
{
	using namespace OpenPLX_ImporterToBlueprintTest_helpers;

	if (State.Blueprint == nullptr)
		return true;

	TArray<UActorComponent*> Components =
		FAGX_BlueprintUtilities::GetTemplateComponents(*State.Blueprint, EAGX_Inherited::Include);
	Test.TestTrue(TEXT("OpenPLX import created component templates"), Components.Num() > 1);

	UAGX_ModelSourceComponent* ModelSource =
		AgxAutomationCommon::GetByName<UAGX_ModelSourceComponent>(
			Components, *FAGX_BlueprintUtilities::ToTemplateComponentName(TEXT("AGX_ModelSource")));
	Test.TestNotNull(TEXT("AGX_ModelSource"), ModelSource);
	if (ModelSource == nullptr)
		return true;

	State.CopiedFilePath = ModelSource->FilePath;
	Test.TestEqual(
		TEXT("OpenPLX source file path"), ModelSource->SourceFilePath, State.SourceFilePath);
	Test.TestEqual(
		TEXT("OpenPLX model source extension"), FPaths::GetExtension(ModelSource->FilePath),
		TEXT("openplx"));
	Test.TestTrue(
		TEXT("OpenPLX file was copied to OpenPLXModels"),
		ModelSource->FilePath.StartsWith(FOpenPLXUtilities::GetModelsDirectory()));
	Test.TestTrue(TEXT("Copied OpenPLX file exists"), FPaths::FileExists(ModelSource->FilePath));

	const int32 NumStaticMeshComponents =
		Components
			.FilterByPredicate([](UActorComponent* Component)
							   { return Cast<UStaticMeshComponent>(Component) != nullptr; })
			.Num();
	UE_LOG(
		LogAGX, Display,
		TEXT("OpenPLX Blueprint import state: Blueprint='%s', RootPackage='%s', Components=%d, "
			 "StaticMeshComponents=%d, CopiedFile='%s'."),
		*State.Blueprint->GetName(), *State.ImportRootPackagePath, Components.Num(),
		NumStaticMeshComponents, *State.CopiedFilePath);

	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(
	FClearOpenPLXBlueprintImportCommand,
	OpenPLX_ImporterToBlueprintTest_helpers::FOpenPLXImportState&, State, FAutomationTestBase&,
	Test);

bool FClearOpenPLXBlueprintImportCommand::Update()
{
	using namespace OpenPLX_ImporterToBlueprintTest_helpers;

	if (State.Blueprint == nullptr)
		return true;

	if (State.CopiedFilePath.IsEmpty())
	{
		TArray<UActorComponent*> Components = FAGX_BlueprintUtilities::GetTemplateComponents(
			*State.Blueprint, EAGX_Inherited::Include);
		if (UAGX_ModelSourceComponent* ModelSource =
				AgxAutomationCommon::GetByName<UAGX_ModelSourceComponent>(
					Components,
					*FAGX_BlueprintUtilities::ToTemplateComponentName(TEXT("AGX_ModelSource"))))
		{
			State.CopiedFilePath = ModelSource->FilePath;
		}
	}

	if (!State.CopiedFilePath.IsEmpty())
	{
		const FString OpenPLXDirectory = FPaths::GetPath(State.CopiedFilePath);
		if (!IsSafeOpenPLXModelDirectory(OpenPLXDirectory))
		{
			Test.AddError(FString::Printf(
				TEXT("Refusing to delete unsafe copied OpenPLX directory '%s'."),
				*OpenPLXDirectory));
		}
		else
		{
			DeleteOpenPLXDirectoryWithExpectedContents(
				OpenPLXDirectory, State.ExpectedCopiedOpenPLXFiles, Test);
		}
	}

	const FString ImportDirectory = GetImportRootDirectory(*State.Blueprint);
	if (!IsSafeImportDirectory(ImportDirectory))
	{
		Test.AddError(FString::Printf(
			TEXT("Refusing to delete unsafe OpenPLX import directory '%s'."), *ImportDirectory));
		State.Blueprint = nullptr;
		return true;
	}

	const FString BaseBlueprintName = State.Blueprint->GetName() + FString(".uasset");
	const FString ImportDirectoryName = FPaths::GetCleanFilename(ImportDirectory);
	State.Blueprint = nullptr;

	TArray<const TCHAR*> ExpectedImportFiles = State.ExpectedImportedAssetsExcludingBaseBP;
	ExpectedImportFiles.Add(*BaseBlueprintName);
	AgxAutomationCommon::DeleteImportDirectory(*ImportDirectoryName, ExpectedImportFiles);

	return false;
}

namespace OpenPLX_ImporterToBlueprintTest_helpers
{
	void AddCommonOpenPLXImportCommands(FOpenPLXImportState& State, FAutomationTestBase& Test)
	{
		ADD_LATENT_AUTOMATION_COMMAND(FImportOpenPLXBlueprintCommand(State, Test));
		ADD_LATENT_AUTOMATION_COMMAND(FCheckOpenPLXBlueprintImportedCommand(State, Test));
	}

	void AddCommonOpenPLXCleanupCommands(FOpenPLXImportState& State, FAutomationTestBase& Test)
	{
		ADD_LATENT_AUTOMATION_COMMAND(FClearOpenPLXBlueprintImportCommand(State, Test));
		ADD_LATENT_AUTOMATION_COMMAND(AgxAutomationCommon::FWaitNTicksCommand(1));
	}
}

//
// box_w_texture test starts here.
//

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(
	FCheckBoxWithTextureImportedCommand,
	OpenPLX_ImporterToBlueprintTest_helpers::FOpenPLXImportState&, State, FAutomationTestBase&,
	Test);

bool FCheckBoxWithTextureImportedCommand::Update()
{
	using namespace OpenPLX_ImporterToBlueprintTest_helpers;

	Test.TestNotNull(TEXT("Blueprint"), State.Blueprint);
	if (State.Blueprint == nullptr)
		return true;

	TArray<UActorComponent*> Components =
		FAGX_BlueprintUtilities::GetTemplateComponents(*State.Blueprint, EAGX_Inherited::Include);

	// SceneRoot, 1 Rigid Body, 1 Trimesh, 1 RenderMesh, 1 CollisionMesh, 1 OpenPLXSignalHandler, 1
	// ModelSource.
	Test.TestEqual(TEXT("box_w_texture num Components"), Components.Num(), 7);

	UStaticMeshComponent* RenderMesh = AgxAutomationCommon::GetByName<UStaticMeshComponent>(
		Components, *FAGX_BlueprintUtilities::ToTemplateComponentName(
						TEXT("RenderMesh_6CC37468D0C257748FD4F097E39E36EE")));
	Test.TestNotNull(TEXT("box_w_texture RenderMesh"), RenderMesh);
	if (RenderMesh == nullptr)
		return true;

	UMaterialInstanceConstant* Material =
		Cast<UMaterialInstanceConstant>(RenderMesh->GetMaterial(0));
	Test.TestNotNull(TEXT("MI_BoxMaterial"), Material);
	if (Material == nullptr)
		return true;

	Test.TestEqual(
		TEXT("MI_BoxMaterial name"), Material->GetName(), FString(TEXT("MI_BoxMaterial")));
	Test.TestNotNull(TEXT("MI_BoxMaterial parent"), Material->Parent.Get());
	if (Material->Parent != nullptr)
	{
		Test.TestEqual(
			TEXT("MI_BoxMaterial parent"), Material->Parent->GetName(),
			FString(TEXT("M_PLXImportedBase")));
	}

	Test.TestEqual(
		TEXT("MI_BoxMaterial vector overrides"), Material->VectorParameterValues.Num(), 1);
	Test.TestEqual(
		TEXT("MI_BoxMaterial scalar overrides"), Material->ScalarParameterValues.Num(), 0);
	Test.TestEqual(
		TEXT("MI_BoxMaterial texture overrides"), Material->TextureParameterValues.Num(), 1);
	Test.TestEqual(
		TEXT("MI_BoxMaterial overrides BlendMode"),
		Material->BasePropertyOverrides.bOverride_BlendMode, 0u);

	if (Material->TextureParameterValues.Num() != 1)
		return true;

	const FTextureParameterValue& BaseColorTextureParameter = Material->TextureParameterValues[0];
	Test.TestEqual(
		TEXT("MI_BoxMaterial texture parameter name"), BaseColorTextureParameter.ParameterInfo.Name,
		FName(TEXT("BaseColorTexture")));

	UTexture2D* BaseColorTexture = Cast<UTexture2D>(BaseColorTextureParameter.ParameterValue);
	Test.TestNotNull(TEXT("MI_BoxMaterial BaseColorTexture"), BaseColorTexture);
	if (BaseColorTexture == nullptr)
		return true;

	Test.TestEqual(
		TEXT("BaseColorTexture name"), BaseColorTexture->GetName(), FString(TEXT("T_BoxTexture")));
	Test.TestTrue(TEXT("BaseColorTexture SRGB"), BaseColorTexture->SRGB);
	Test.TestEqual(
		TEXT("BaseColorTexture compression"),
		static_cast<int32>(BaseColorTexture->CompressionSettings.GetValue()),
		static_cast<int32>(TC_Default));
	Test.TestEqual(TEXT("BaseColorTexture width"), BaseColorTexture->GetSizeX(), 1024);
	Test.TestEqual(TEXT("BaseColorTexture height"), BaseColorTexture->GetSizeY(), 1024);

	return true;
}

class FOpenPLX_ImporterToBlueprint_BoxWithTextureTest final
	: public AgxAutomationCommon::FAgxAutomationTest
{
public:
	FOpenPLX_ImporterToBlueprint_BoxWithTextureTest()
		: AgxAutomationCommon::FAgxAutomationTest(
			  TEXT("FOpenPLX_ImporterToBlueprint_BoxWithTextureTest"),
			  TEXT("AGXUnreal.Editor.OpenPLX.ImporterToBlueprint.BoxWithTexture"))
	{
		State.OpenPLXFile = TEXT("OpenPLX/box_w_texture/box_with_texture.openplx");
		State.ExpectedCopiedOpenPLXFiles = {
			TEXT("box_with_texture.openplx"), TEXT("box.obj"), TEXT("box_texture.png")};
		State.ExpectedImportedAssetsExcludingBaseBP = {
			TEXT("Blueprint"),
			TEXT("RenderMaterial"),
			TEXT("RenderMesh"),
			TEXT("StaticMesh"),
			TEXT("Textures"),
			TEXT("BP_box_with_texture.uasset"),
			TEXT("MI_BoxMaterial.uasset"),
			TEXT("SM_CollisionMesh_6CC37468D0C257748FD4F097E39E36EE.uasset"),
			TEXT("SM_RenderMesh_05D95A98FE515109B3C2926FD1D97DDA.uasset"),
			TEXT("T_BoxTexture.uasset")};
	}

protected:
	bool RunTest(const FString& Parameters) override
	{
		using namespace OpenPLX_ImporterToBlueprintTest_helpers;

		BAIL_TEST_IF_NOT_EDITOR(false)

		AddCommonOpenPLXImportCommands(State, *this);
		ADD_LATENT_AUTOMATION_COMMAND(FCheckBoxWithTextureImportedCommand(State, *this));
		AddCommonOpenPLXCleanupCommands(State, *this);

		return true;
	}

private:
	OpenPLX_ImporterToBlueprintTest_helpers::FOpenPLXImportState State;
};

namespace
{
	FOpenPLX_ImporterToBlueprint_BoxWithTextureTest OpenPLX_ImporterToBlueprint_BoxWithTextureTest;
}

namespace OpenPLX_ImporterToBlueprintTest_AgxIcon_helpers
{
	void CheckTextureParameter(
		UMaterialInstanceConstant& Material, const TCHAR* ParameterName, const TCHAR* TextureName,
		bool bSRGB, TextureCompressionSettings CompressionSettings, FAutomationTestBase& Test)
	{
		UTexture* Texture = nullptr;
		const bool bHasTexture = Material.GetTextureParameterValue(
			FMaterialParameterInfo(FName(ParameterName)), Texture);
		Test.TestTrue(
			FString::Printf(TEXT("%s has %s"), *Material.GetName(), ParameterName), bHasTexture);

		UTexture2D* Texture2D = Cast<UTexture2D>(Texture);
		Test.TestNotNull(
			FString::Printf(TEXT("%s %s texture"), *Material.GetName(), ParameterName), Texture2D);
		if (Texture2D == nullptr)
			return;

		Test.TestEqual(
			FString::Printf(TEXT("%s texture name"), ParameterName), Texture2D->GetName(),
			FString(TextureName));
		Test.TestEqual(FString::Printf(TEXT("%s SRGB"), ParameterName), Texture2D->SRGB, bSRGB);
		Test.TestEqual(
			FString::Printf(TEXT("%s compression"), ParameterName),
			static_cast<int32>(Texture2D->CompressionSettings.GetValue()),
			static_cast<int32>(CompressionSettings));
	}

	void CheckScalarParameter(
		UMaterialInstanceConstant& Material, const TCHAR* ParameterName, float Expected,
		FAutomationTestBase& Test)
	{
		float Actual = 0.0f;
		const bool bHasScalar =
			Material.GetScalarParameterValue(FMaterialParameterInfo(FName(ParameterName)), Actual);
		Test.TestTrue(
			FString::Printf(TEXT("%s has %s"), *Material.GetName(), ParameterName), bHasScalar);
		if (!bHasScalar)
			return;

		Test.TestTrue(
			FString::Printf(TEXT("%s scalar"), ParameterName),
			FMath::IsNearlyEqual(Actual, Expected, UE_KINDA_SMALL_NUMBER));
	}

	void CheckVectorParameter(
		UMaterialInstanceConstant& Material, const TCHAR* ParameterName,
		const FLinearColor& Expected, FAutomationTestBase& Test)
	{
		FLinearColor Actual;
		const bool bHasVector =
			Material.GetVectorParameterValue(FMaterialParameterInfo(FName(ParameterName)), Actual);
		Test.TestTrue(
			FString::Printf(TEXT("%s has %s"), *Material.GetName(), ParameterName), bHasVector);
		if (!bHasVector)
			return;

		Test.TestTrue(
			FString::Printf(TEXT("%s vector"), ParameterName),
			Actual.Equals(Expected, UE_KINDA_SMALL_NUMBER));
	}

	void CheckCommonTextures(UMaterialInstanceConstant& Material, FAutomationTestBase& Test)
	{
		CheckTextureParameter(
			Material, TEXT("BaseColorTexture"), TEXT("T_DiffuseTexture"), true, TC_Default, Test);
		CheckTextureParameter(
			Material, TEXT("RoughnessTexture"), TEXT("T_RoughnessTexture"), false, TC_Grayscale,
			Test);
		CheckTextureParameter(
			Material, TEXT("MetallicTexture"), TEXT("T_MetallicTexture"), false, TC_Grayscale,
			Test);
		CheckTextureParameter(
			Material, TEXT("NormalTexture"), TEXT("T_NormalTexture"), false, TC_Normalmap, Test);
		CheckTextureParameter(
			Material, TEXT("AmbientOcclusionTexture"), TEXT("T_AOTexture"), false, TC_Grayscale,
			Test);
	}
}

//
// agx_icon_masked test starts here.
//

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(
	FCheckAgxIconMaskedImportedCommand,
	OpenPLX_ImporterToBlueprintTest_helpers::FOpenPLXImportState&, State, FAutomationTestBase&,
	Test);

bool FCheckAgxIconMaskedImportedCommand::Update()
{
	Test.TestNotNull(TEXT("Blueprint"), State.Blueprint);
	if (State.Blueprint == nullptr)
		return true;

	TArray<UActorComponent*> Components =
		FAGX_BlueprintUtilities::GetTemplateComponents(*State.Blueprint, EAGX_Inherited::Include);

	// SceneRoot, 1 RenderMesh, 1 Trimesh, 1 CollisionMesh, 1 OpenPLXSignalHandler, 1 ModelSource.
	Test.TestEqual(TEXT("agx_icon_masked num Components"), Components.Num(), 6);

	UStaticMeshComponent* RenderMesh = AgxAutomationCommon::GetByName<UStaticMeshComponent>(
		Components, *FAGX_BlueprintUtilities::ToTemplateComponentName(
						TEXT("RenderMesh_044BF93AA4FA5652BD713DC07DA5C3B6")));
	Test.TestNotNull(TEXT("agx_icon_masked RenderMesh"), RenderMesh);
	if (RenderMesh == nullptr)
		return true;

	UMaterialInstanceConstant* Material =
		Cast<UMaterialInstanceConstant>(RenderMesh->GetMaterial(0));
	Test.TestNotNull(TEXT("MI_AgxIconMaterial masked"), Material);
	if (Material == nullptr)
		return true;

	Test.TestEqual(
		TEXT("MI_AgxIconMaterial masked name"), Material->GetName(),
		FString(TEXT("MI_AgxIconMaterial")));
	Test.TestNotNull(TEXT("MI_AgxIconMaterial masked parent"), Material->Parent.Get());
	if (Material->Parent != nullptr)
	{
		Test.TestEqual(
			TEXT("MI_AgxIconMaterial masked parent"), Material->Parent->GetName(),
			FString(TEXT("M_PLXImportedBase")));
	}

	Test.TestEqual(
		TEXT("MI_AgxIconMaterial masked vector overrides"), Material->VectorParameterValues.Num(),
		1);
	Test.TestEqual(
		TEXT("MI_AgxIconMaterial masked scalar overrides"), Material->ScalarParameterValues.Num(),
		4);
	Test.TestEqual(
		TEXT("MI_AgxIconMaterial masked texture overrides"), Material->TextureParameterValues.Num(),
		6);
	Test.TestEqual(
		TEXT("MI_AgxIconMaterial masked overrides BlendMode"),
		Material->BasePropertyOverrides.bOverride_BlendMode, 1u);
	Test.TestEqual(
		TEXT("MI_AgxIconMaterial masked BlendMode"), Material->BasePropertyOverrides.BlendMode,
		BLEND_Masked);

	using namespace OpenPLX_ImporterToBlueprintTest_AgxIcon_helpers;
	CheckVectorParameter(
		*Material, TEXT("BaseColor"), FLinearColor(0.99f, 0.98f, 0.97f, 1.0f), Test);
	CheckScalarParameter(*Material, TEXT("Roughness"), 0.98f, Test);
	CheckScalarParameter(*Material, TEXT("Metallic"), 0.97f, Test);
	CheckScalarParameter(*Material, TEXT("NormalScale"), 0.96f, Test);
	CheckScalarParameter(*Material, TEXT("Opacity"), 0.99f, Test);
	CheckCommonTextures(*Material, Test);
	CheckTextureParameter(
		*Material, TEXT("OpacityTexture"), TEXT("T_MaskTexture"), false, TC_Grayscale, Test);

	return true;
}

class FOpenPLX_ImporterToBlueprint_AgxIconMaskedTest final
	: public AgxAutomationCommon::FAgxAutomationTest
{
public:
	FOpenPLX_ImporterToBlueprint_AgxIconMaskedTest()
		: AgxAutomationCommon::FAgxAutomationTest(
			  TEXT("FOpenPLX_ImporterToBlueprint_AgxIconMaskedTest"),
			  TEXT("AGXUnreal.Editor.OpenPLX.ImporterToBlueprint.AgxIconMasked"))
	{
		State.OpenPLXFile = TEXT("OpenPLX/agx_icon_masked/agx_icon_masked.openplx");
		State.ExpectedCopiedOpenPLXFiles = {
			TEXT("agx_icon_masked.openplx"), TEXT("agx_icon.obj"),
			TEXT("T_agx_icon_D.png"),		 TEXT("T_agx_icon_N.png"), TEXT("T_agx_icon_ORM.png"),
			TEXT("T_agx_icon_Mask.png")};
		State.ExpectedImportedAssetsExcludingBaseBP = {
			TEXT("Blueprint"),
			TEXT("RenderMaterial"),
			TEXT("RenderMesh"),
			TEXT("StaticMesh"),
			TEXT("Textures"),
			TEXT("BP_agx_icon_masked.uasset"),
			TEXT("MI_AgxIconMaterial.uasset"),
			TEXT("SM_RenderMesh_DDF194887D5353ADB3EA51F31E951A38.uasset"),
			TEXT("SM_CollisionMesh_044BF93AA4FA5652BD713DC07DA5C3B6.uasset"),
			TEXT("T_AOTexture.uasset"),
			TEXT("T_DiffuseTexture.uasset"),
			TEXT("T_MaskTexture.uasset"),
			TEXT("T_MetallicTexture.uasset"),
			TEXT("T_NormalTexture.uasset"),
			TEXT("T_RoughnessTexture.uasset")};
	}

protected:
	bool RunTest(const FString& Parameters) override
	{
		using namespace OpenPLX_ImporterToBlueprintTest_helpers;

		BAIL_TEST_IF_NOT_EDITOR(false)

		AddCommonOpenPLXImportCommands(State, *this);
		ADD_LATENT_AUTOMATION_COMMAND(FCheckAgxIconMaskedImportedCommand(State, *this));
		AddCommonOpenPLXCleanupCommands(State, *this);

		return true;
	}

private:
	OpenPLX_ImporterToBlueprintTest_helpers::FOpenPLXImportState State;
};

namespace
{
	FOpenPLX_ImporterToBlueprint_AgxIconMaskedTest OpenPLX_ImporterToBlueprint_AgxIconMaskedTest;
}

#endif // WITH_DEV_AUTOMATION_TESTS
