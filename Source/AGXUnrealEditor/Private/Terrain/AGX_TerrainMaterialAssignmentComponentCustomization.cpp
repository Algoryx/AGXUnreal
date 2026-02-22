// Copyright 2026, Algoryx Simulation AB.

#include "Terrain/AGX_TerrainMaterialAssignmentComponentCustomization.h"

// AGX Dynamics for Unreal includes.
#include "Terrain/AGX_TerrainMaterialAssignmentComponent.h"
#include "Utilities/AGX_EditorUtilities.h"

// Unreal Engine includes.
#include "AssetRegistry/AssetData.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"

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
	UAGX_TerrainMaterialAssignmentComponent* AssignmentComponent =
		FAGX_EditorUtilities::GetSingleObjectBeingCustomized<
			UAGX_TerrainMaterialAssignmentComponent>(InDetailBuilder, false);

	if (AssignmentComponent == nullptr)
		return;

	AssignmentComponent->UpdateTerrainMaterialAssignments();
}

#undef LOCTEXT_NAMESPACE
