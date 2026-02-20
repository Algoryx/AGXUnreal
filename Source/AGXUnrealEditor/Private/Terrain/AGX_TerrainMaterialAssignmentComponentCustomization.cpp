// Copyright 2026, Algoryx Simulation AB.

#include "Terrain/AGX_TerrainMaterialAssignmentComponentCustomization.h"

// Unreal Engine includes.
#include "DetailLayoutBuilder.h"

#define LOCTEXT_NAMESPACE "FAGX_TerrainMaterialAssignmentComponentCustomization"

TSharedRef<IDetailCustomization> FAGX_TerrainMaterialAssignmentComponentCustomization::MakeInstance()
{
	return MakeShareable(new FAGX_TerrainMaterialAssignmentComponentCustomization);
}

void FAGX_TerrainMaterialAssignmentComponentCustomization::CustomizeDetails(
	IDetailLayoutBuilder& InDetailBuilder)
{
	InDetailBuilder.EditCategory(
		"AGX Terrain Material Assignment", FText::GetEmpty(), ECategoryPriority::Important);
}

#undef LOCTEXT_NAMESPACE
