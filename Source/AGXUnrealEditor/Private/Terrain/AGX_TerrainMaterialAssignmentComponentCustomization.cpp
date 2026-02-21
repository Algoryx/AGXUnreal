// Copyright 2026, Algoryx Simulation AB.

#include "Terrain/AGX_TerrainMaterialAssignmentComponentCustomization.h"

// AGX Dynamics for Unreal includes.
#include "Materials/AGX_ShapeMaterial.h"
#include "Materials/AGX_TerrainMaterial.h"
#include "Terrain/AGX_TerrainMaterialAssignmentComponent.h"
#include "Utilities/AGX_EditorUtilities.h"

// Unreal Engine includes.
#include "AssetRegistry/AssetData.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "PropertyCustomizationHelpers.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "FAGX_TerrainMaterialAssignmentComponentCustomization"

namespace AGX_TerrainMaterialAssignmentComponentCustomization_helpers
{
	const FText TerrainMaterialTooltip = LOCTEXT(
		"TerrainMaterialTooltip",
		"This Terrain Material will be assigned to the voxels of the Terrain that this Shape overlaps.");

	const FText ShapeMaterialTooltip = LOCTEXT(
		"ShapeMaterialTooltip",
		"This Shape Material will be associated with the selected Terrain Material for this Terrain.");
}

TSharedRef<IDetailCustomization>
FAGX_TerrainMaterialAssignmentComponentCustomization::MakeInstance()
{
	return MakeShareable(new FAGX_TerrainMaterialAssignmentComponentCustomization);
}

void FAGX_TerrainMaterialAssignmentComponentCustomization::CustomizeDetails(
	IDetailLayoutBuilder& InDetailBuilder)
{
	IDetailCategoryBuilder& Category = InDetailBuilder.EditCategory(
		"AGX Terrain Material Assignment", FText::GetEmpty(), ECategoryPriority::Important);

	UAGX_TerrainMaterialAssignmentComponent* AssignmentComponent =
		FAGX_EditorUtilities::GetSingleObjectBeingCustomized<
			UAGX_TerrainMaterialAssignmentComponent>(InDetailBuilder, false);

	if (AssignmentComponent == nullptr)
		return;

	AssignmentComponent->UpdateTerrainMaterialAssignments();

	TArray<FAGX_TerrainMaterialAssignmentData>& Assignments =
		AssignmentComponent->GetTerrainMaterialAssignments();
	if (Assignments.IsEmpty())
	{
		Category.AddCustomRow(LOCTEXT("NoShapesFilter", "No AGX Shape Components"))
			.WholeRowContent()
				[SNew(STextBlock)
					 .Text(LOCTEXT(
						 "NoShapesText",
						 "Attach AGX Shape Components to create material assignments."))];
		return;
	}

	TWeakObjectPtr<UAGX_TerrainMaterialAssignmentComponent> WeakAssignmentComponent =
		AssignmentComponent;
	for (int32 Index = 0; Index < Assignments.Num(); ++Index)
	{
		const FName ShapeName = Assignments[Index].ShapeComponentName;
		const FText ShapeNameText = ShapeName.IsNone() ? LOCTEXT("UnnamedShape", "Unnamed Shape")
													   : FText::FromName(ShapeName);
		Category.AddCustomRow(ShapeNameText)
			.NameContent()
				[SNew(STextBlock).Font(IDetailLayoutBuilder::GetDetailFont()).Text(ShapeNameText)]
			.ValueContent()
				[SNew(SVerticalBox) +
				 SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 2.0f)
					 [SNew(SHorizontalBox) +
					  SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.0f, 0.0f, 8.0f, 0.0f)
						  [SNew(STextBlock)
							   .Font(IDetailLayoutBuilder::GetDetailFont())
							   .Text(LOCTEXT("TerrainMaterialLabel", "Terrain Material"))
							   .ToolTipText(
								   AGX_TerrainMaterialAssignmentComponentCustomization_helpers::
									   TerrainMaterialTooltip)] +
					  SHorizontalBox::Slot().FillWidth(1.0f)
						  [SNew(SObjectPropertyEntryBox)
							   .AllowedClass(UAGX_TerrainMaterial::StaticClass())
							   .ToolTipText(
								   AGX_TerrainMaterialAssignmentComponentCustomization_helpers::
									   TerrainMaterialTooltip)
							   .ObjectPath_Lambda(
								   [WeakAssignmentComponent, Index]()
								   {
									   const UAGX_TerrainMaterialAssignmentComponent* Component =
										   WeakAssignmentComponent.Get();
									   if (Component == nullptr)
									   {
										   return FString();
									   }

									   const TArray<FAGX_TerrainMaterialAssignmentData>&
										   CurrentAssignments =
											   Component->GetTerrainMaterialAssignments();
									   if (!CurrentAssignments.IsValidIndex(Index))
									   {
										   return FString();
									   }

									   const UAGX_TerrainMaterial* TerrainMaterial =
										   CurrentAssignments[Index].TerrainMaterial;
									   return TerrainMaterial != nullptr ? TerrainMaterial->GetPathName()
																		 : FString();
								   })
							   .OnObjectChanged_Lambda(
								   [WeakAssignmentComponent, Index](const FAssetData& AssetData)
								   {
									   UAGX_TerrainMaterialAssignmentComponent* Component =
										   WeakAssignmentComponent.Get();
									   if (Component == nullptr)
									   {
										   return;
									   }

									   TArray<FAGX_TerrainMaterialAssignmentData>& CurrentAssignments =
										   Component->GetTerrainMaterialAssignments();
									   if (!CurrentAssignments.IsValidIndex(Index))
									   {
										   return;
									   }

									   Component->Modify();
									   CurrentAssignments[Index].TerrainMaterial =
										   Cast<UAGX_TerrainMaterial>(AssetData.GetAsset());
									   Component->MarkPackageDirty();
								   })]] +
				 SVerticalBox::Slot().AutoHeight()
					 [SNew(SHorizontalBox) +
					  SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.0f, 0.0f, 8.0f, 0.0f)
						  [SNew(STextBlock)
							   .Font(IDetailLayoutBuilder::GetDetailFont())
							   .Text(LOCTEXT("ShapeMaterialLabel", "Shape Material"))
							   .ToolTipText(
								   AGX_TerrainMaterialAssignmentComponentCustomization_helpers::
									   ShapeMaterialTooltip)] +
					  SHorizontalBox::Slot().FillWidth(1.0f)
						  [SNew(SObjectPropertyEntryBox)
							   .AllowedClass(UAGX_ShapeMaterial::StaticClass())
							   .ToolTipText(
								   AGX_TerrainMaterialAssignmentComponentCustomization_helpers::
									   ShapeMaterialTooltip)
							   .ObjectPath_Lambda(
								   [WeakAssignmentComponent, Index]()
								   {
									   const UAGX_TerrainMaterialAssignmentComponent* Component =
										   WeakAssignmentComponent.Get();
									   if (Component == nullptr)
									   {
										   return FString();
									   }

									   const TArray<FAGX_TerrainMaterialAssignmentData>&
										   CurrentAssignments =
											   Component->GetTerrainMaterialAssignments();
									   if (!CurrentAssignments.IsValidIndex(Index))
									   {
										   return FString();
									   }

									   const UAGX_ShapeMaterial* ShapeMaterial =
										   CurrentAssignments[Index].ShapeMaterial;
									   return ShapeMaterial != nullptr ? ShapeMaterial->GetPathName()
																	   : FString();
								   })
							   .OnObjectChanged_Lambda(
								   [WeakAssignmentComponent, Index](const FAssetData& AssetData)
								   {
									   UAGX_TerrainMaterialAssignmentComponent* Component =
										   WeakAssignmentComponent.Get();
									   if (Component == nullptr)
									   {
										   return;
									   }

									   TArray<FAGX_TerrainMaterialAssignmentData>& CurrentAssignments =
										   Component->GetTerrainMaterialAssignments();
									   if (!CurrentAssignments.IsValidIndex(Index))
									   {
										   return;
									   }

									   Component->Modify();
									   CurrentAssignments[Index].ShapeMaterial =
										   Cast<UAGX_ShapeMaterial>(AssetData.GetAsset());
									   Component->MarkPackageDirty();
								   })]]];
	}
}

#undef LOCTEXT_NAMESPACE
