// Copyright 2024, Algoryx Simulation AB.

#include "AGX_ModelSourceComponentCustomization.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_ImporterToBlueprint.h"
#include "AGX_ImportSettings.h"
#include "Import/AGX_ModelSourceComponent.h"
#include "Utilities/AGX_BlueprintUtilities.h"
#include "Utilities/AGX_EditorUtilities.h"
#include "Utilities/AGX_MaterialReplacer.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Widgets/AGX_ImportDialog.h"

// Unreal Engine includes.
#include "Components/MeshComponent.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Input/Reply.h"
#include "PropertyCustomizationHelpers.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "FAGX_ModelSourceComponentCustomization"

TSharedRef<IDetailCustomization> FAGX_ModelSourceComponentCustomization::MakeInstance()
{
	return MakeShareable(new FAGX_ModelSourceComponentCustomization);
}

void FAGX_ModelSourceComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& InDetailBuilder)
{
	DetailBuilder = &InDetailBuilder;

	UAGX_ModelSourceComponent* ModelSourceComponent =
		FAGX_EditorUtilities::GetSingleObjectBeingCustomized<UAGX_ModelSourceComponent>(
			InDetailBuilder);
	if (!ModelSourceComponent)
	{
		return;
	}

	IDetailCategoryBuilder& CategoryBuilder = InDetailBuilder.EditCategory("AGX Synchronize Model");

	// clang-format off
	CategoryBuilder.AddCustomRow(FText::GetEmpty())
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("SynchronizeModelButtonText", "Synchronize Model"))
			.ToolTipText(LOCTEXT(
				"SynchronizeModelTooltip",
				"Synchronizes the model against the source file of the original "
				"import and updates the Components and Assets to match said source file."))
			.OnClicked(this, &FAGX_ModelSourceComponentCustomization::OnSynchronizeModelButtonClicked)
		]
	];
	// clang-format on

	CustomizeMaterialReplacer(ModelSourceComponent);

	InDetailBuilder.HideCategory(FName("AGX Synchronize Model Info"));
	InDetailBuilder.HideCategory(FName("Activation"));
	InDetailBuilder.HideCategory(FName("AssetUserData"));
	InDetailBuilder.HideCategory(FName("Collision"));
	InDetailBuilder.HideCategory(FName("ComponentReplication"));
	InDetailBuilder.HideCategory(FName("ComponentTick"));
	InDetailBuilder.HideCategory(FName("Cooking"));
	InDetailBuilder.HideCategory(FName("Events"));
	InDetailBuilder.HideCategory(FName("Navigation"));
	InDetailBuilder.HideCategory(FName("Replication"));
	InDetailBuilder.HideCategory(FName("Sockets"));
	InDetailBuilder.HideCategory(FName("Tags"));
	InDetailBuilder.HideCategory(FName("Variable"));
}

namespace AGX_ModelSourceComponentCustomization_helpers
{
	UBlueprint* GetBlueprint(IDetailLayoutBuilder& DetailBuilder)
	{
		UAGX_ModelSourceComponent* ModelSourceComponent =
			FAGX_EditorUtilities::GetSingleObjectBeingCustomized<UAGX_ModelSourceComponent>(
				DetailBuilder);
		if (ModelSourceComponent == nullptr)
		{
			return nullptr;
		}

		if (!ModelSourceComponent->IsInBlueprint())
		{
			FAGX_NotificationUtilities::ShowDialogBoxWithErrorLog(
				"Model synchronization is only supported when in a Blueprint.");
			return nullptr;
		}

		if (auto Bp = Cast<UBlueprintGeneratedClass>(ModelSourceComponent->GetOuter()))
		{
			if (Bp->SimpleConstructionScript != nullptr)
			{
				return Bp->SimpleConstructionScript->GetBlueprint();
			}
		}

		FAGX_NotificationUtilities::ShowDialogBoxWithErrorLog(
			"Unable to get the Blueprint from the AGX Model Source Component. Model "
			"synchronization will not be possible.");
		return nullptr;
	}
}

FReply FAGX_ModelSourceComponentCustomization::OnSynchronizeModelButtonClicked()
{
	AGX_CHECK(DetailBuilder);
	using namespace AGX_ModelSourceComponentCustomization_helpers;
	UBlueprint* Blueprint = GetBlueprint(*DetailBuilder);
	if (Blueprint == nullptr)
	{
		// Logging done in GetBlueprint.
		return FReply::Handled();
	}

	FAGX_EditorUtilities::SynchronizeModel(*Blueprint);

	// Any logging is done in SynchronizeModel.
	return FReply::Handled();
}

namespace AGX_ModelSourceComponentCustomization_helpers
{
	// Callback for letting the UI widgets communicate back to the customization class.
	using FGetMaterial = FString (FAGX_ModelSourceComponentCustomization::*)() const;
	using FSetMaterial = void (FAGX_ModelSourceComponentCustomization::*)(const FAssetData&);
	using FShouldFilterMaterial =
		bool (FAGX_ModelSourceComponentCustomization::*)(const FAssetData&);

	static void CreateMaterialWidget(
		FAGX_ModelSourceComponentCustomization& Customizer, IDetailCategoryBuilder& CategoryBuilder,
		TSharedPtr<FAssetThumbnailPool>& ThumbnailPool, const FText& Label, const FText& ToolTip,
		FGetMaterial GetMaterial, FSetMaterial SetMaterial,
		FShouldFilterMaterial ShouldFilterMaterial)
	{
		// clang-format off
		CategoryBuilder.AddCustomRow(LOCTEXT("ReplaceMaterial", "Replace Material"))
		.NameContent()
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.Text(Label)
			.ToolTipText(ToolTip)
		]
		.ValueContent()
		[
			SNew(SObjectPropertyEntryBox)
			.AllowedClass(UMaterialInterface::StaticClass())
			.ObjectPath(&Customizer, GetMaterial)
			.ThumbnailPool(ThumbnailPool)
			.OnShouldFilterAsset(&Customizer, ShouldFilterMaterial)
			.OnObjectChanged(&Customizer, SetMaterial)
		];
		// clang-format on
	}

	// Instead of explicitly relying on Mesh Component we would like to use the Unreal Engine
	// type FMaterialIterator. Unfortunately, it is not public and thus not accessible here.

	void CollectRenderMaterials(const UMeshComponent& Mesh, TSet<UMaterialInterface*>& Materials)
	{
		for (int32 Index = 0; Index < Mesh.GetNumMaterials(); ++Index)
		{
			Materials.Add(Mesh.GetMaterial(Index));
		}
	}

	static TSet<UMaterialInterface*> CollectRenderMaterials(UBlueprint& Blueprint)
	{
		TSet<UMaterialInterface*> Materials;
		for (UMeshComponent* MeshTemplate :
			 FAGX_BlueprintUtilities::GetAllTemplateComponents<UMeshComponent>(Blueprint))
		{
			CollectRenderMaterials(*MeshTemplate, Materials);
		}
		return Materials;
	}

	static TSet<UMaterialInterface*> CollectRenderMaterials(AActor& Actor)
	{
		TArray<UMeshComponent*> Meshes;
		Actor.GetComponents(Meshes);
		TSet<UMaterialInterface*> Materials;
		Materials.Reserve(Meshes.Num());
		for (UMeshComponent* Mesh : Meshes)
		{
			CollectRenderMaterials(*Mesh, Materials);
		}
		return Materials;
	}

	static TSet<UMaterialInterface*> CollectRenderMaterials(UAGX_ModelSourceComponent* ModelSource)
	{
		// Determine if the Model Source Component is part of a Blueprint or an Actor.
		UBlueprint* Blueprint = FAGX_BlueprintUtilities::GetBlueprintFrom(*ModelSource);
		AActor* Owner = ModelSource->GetOwner();
		if (Blueprint != nullptr)
		{
			AGX_CHECKF(
				Owner == nullptr,
				TEXT("Material Replacer operating on a Model Source Component "
					 "that is both part of a Blueprint and an Actor. We did not expect this could "
					 "happen. User applications will use the Blueprint code to collect Materials "
					 "but not sure it will find all Materials."));
			return CollectRenderMaterials(*Blueprint);
		}
		else if (Owner != nullptr)
		{
			return CollectRenderMaterials(*Owner);
		}
		else
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Material Replacer could not collect in-use render Materials because the "
					 "Model Source Component is part of neither a Blueprint nor an Actor."));
			return TSet<UMaterialInterface*>();
		}
	}
}

void FAGX_ModelSourceComponentCustomization::CustomizeMaterialReplacer(
	UAGX_ModelSourceComponent* ModelSource)
{
	using namespace AGX_ModelSourceComponentCustomization_helpers;
	KnownMaterials = CollectRenderMaterials(ModelSource);

	IDetailCategoryBuilder& CategoryBuilder = DetailBuilder->EditCategory("Material Replacer");

	TSharedPtr<FAssetThumbnailPool> ThumbnailPool = DetailBuilder->GetThumbnailPool();
	CreateMaterialWidget(
		*this, CategoryBuilder, ThumbnailPool, LOCTEXT("CurrentMaterial", "Current Material"),
		LOCTEXT(
			"CurrentMaterialTooltip",
			"The Material currently set on Mesh Components that should be replaced with the new "
			"Material."),
		&FAGX_ModelSourceComponentCustomization::GetCurrentMaterialPath,
		&FAGX_ModelSourceComponentCustomization::OnCurrentMaterialSelected,
		&FAGX_ModelSourceComponentCustomization::IncludeOnlyUsedMaterials);
	CreateMaterialWidget(
		*this, CategoryBuilder, ThumbnailPool, LOCTEXT("NewMaterial", "New Material"),
		LOCTEXT(
			"NewMaterialTooltip",
			"The Material that should be assigned instead of the current Material."),
		&FAGX_ModelSourceComponentCustomization::GetNewMaterialPath,
		&FAGX_ModelSourceComponentCustomization::OnNewMaterialSelected,
		&FAGX_ModelSourceComponentCustomization::IncludeAllMaterials);

	// clang-format off
	CategoryBuilder.AddCustomRow(LOCTEXT("ReplaceMaterialButton", "Replace Material Button"))
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("ReplaceMaterialsButton", "Replace Materials"))
			.ToolTipText(LOCTEXT(
				"ReplaceMaterialsButtonTooltip",
				"Replace all occurrences of Current Material with New Material in this Blueprint or Actor"))
			.OnClicked(this, &FAGX_ModelSourceComponentCustomization::OnReplaceMaterialsButtonClicked)
		]
	];
	// clang-format on
}

// Since the Detail Customization can be recreated at any time, especially when switching between
// selecting different Components, the selection data is stored statically in a separate helper
// type. This means that the user can switch back and forth between the Model Source Component and
// Mesh Components and keep the selections made.

FString FAGX_ModelSourceComponentCustomization::GetCurrentMaterialPath() const
{
	return FAGX_MaterialReplacer::GetCurrentPathName();
}

FString FAGX_ModelSourceComponentCustomization::GetNewMaterialPath() const
{
	return FAGX_MaterialReplacer::GetNewPathName();
}

void FAGX_ModelSourceComponentCustomization::OnCurrentMaterialSelected(const FAssetData& AssetData)
{
	FAGX_MaterialReplacer::SetCurrent(AssetData);
}

void FAGX_ModelSourceComponentCustomization::OnNewMaterialSelected(const FAssetData& AssetData)
{
	FAGX_MaterialReplacer::SetNew(AssetData);
}

// Filter function that returns true for any asset that should be filtered out, i.e. hidden.
bool FAGX_ModelSourceComponentCustomization::IncludeOnlyUsedMaterials(const FAssetData& AssetData)
{
	const UMaterialInterface* Asset = Cast<UMaterialInterface>(AssetData.GetAsset());
	if (Asset == nullptr)
		return true; // Should never happen since AllowedClass is set.

	const bool HideAsset = !KnownMaterials.Contains(Asset);
	return HideAsset;
}

// Filter function that does not filter out, i.e. hides, anything.
bool FAGX_ModelSourceComponentCustomization::IncludeAllMaterials(const FAssetData&)
{
	return false;
}

FReply FAGX_ModelSourceComponentCustomization::OnReplaceMaterialsButtonClicked()
{
	auto Bail = [](const TCHAR* Message)
	{
		FAGX_NotificationUtilities::ShowNotification(Message, SNotificationItem::CS_Fail);
		return FReply::Handled();
	};

	if (DetailBuilder == nullptr)
	{
		return Bail(
			TEXT("Material replacing is currenly only supported while there is a Detail Builder."));
	}

	UAGX_ModelSourceComponent* ModelSource =
		FAGX_EditorUtilities::GetSingleObjectBeingCustomized<UAGX_ModelSourceComponent>(
			*DetailBuilder);
	if (ModelSource == nullptr)
	{
		return Bail(TEXT(
			"Material replacing is currenly only supported with a single Model Source Component."));
	}

	// Determine if the Model Source Component is part of a Blueprint or an Actor.
	UBlueprint* Blueprint = FAGX_BlueprintUtilities::GetBlueprintFrom(*ModelSource);
	AActor* Owner = ModelSource->GetOwner();
	if (Blueprint != nullptr)
	{
		AGX_CHECKF(
			Owner == nullptr,
			TEXT("Material Replacer operating on a Model Source Component that is both part of a "
				 "Blueprint and an Actor. We did not expect this could happen. User applications "
				 "will use the Blueprint code to replace Materials but not sure the change will be "
				 "applied completely and correctly."));
		FAGX_MaterialReplacer::ReplaceMaterials(*Blueprint);
	}
	else if (Owner != nullptr)
	{
		FAGX_MaterialReplacer::ReplaceMaterials(*Owner);
	}
	else
	{
		return Bail(
			TEXT("Material replacing failed because the Model Source Component has neither an "
				 "Owner nor a Blueprint"));
	}

	// The new Material is now in use, so add it to the set of known Materials.
	KnownMaterials.Add(FAGX_MaterialReplacer::NewMaterial.Get());

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
