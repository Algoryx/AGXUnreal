// Copyright 2026, Algoryx Simulation AB.

#include "Terrain/AGX_TerrainMaterialPatchComponentCustomization.h"

// AGX Dynamics for Unreal includes.
#include "Terrain/AGX_TerrainMaterialPatchComponent.h"
#include "Utilities/AGX_EditorUtilities.h"

// Unreal Engine includes.
#include "AssetRegistry/AssetData.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"

#define LOCTEXT_NAMESPACE "FAGX_TerrainMaterialPatchComponentCustomization"

namespace AGX_TerrainMaterialPatchComponentCustomization_helpers
{
	const FText TerrainMaterialTooltip = LOCTEXT(
		"TerrainMaterialTooltip",
		"This Terrain Material will be assigned to the voxels of the Terrain that this Shape overlaps.");

	const FText ShapeMaterialTooltip = LOCTEXT(
		"ShapeMaterialTooltip",
		"This Shape Material will be associated with the selected Terrain Material for this Terrain.");
}

TSharedRef<IDetailCustomization>
FAGX_TerrainMaterialPatchComponentCustomization::MakeInstance()
{
	return MakeShareable(new FAGX_TerrainMaterialPatchComponentCustomization);
}

void FAGX_TerrainMaterialPatchComponentCustomization::CustomizeDetails(
	IDetailLayoutBuilder& InDetailBuilder)
{
	UAGX_TerrainMaterialPatchComponent* AssignmentComponent =
		FAGX_EditorUtilities::GetSingleObjectBeingCustomized<
			UAGX_TerrainMaterialPatchComponent>(InDetailBuilder, false);

	if (AssignmentComponent == nullptr)
		return;

	AssignmentComponent->UpdateTerrainMaterialPatches();
}

#undef LOCTEXT_NAMESPACE

