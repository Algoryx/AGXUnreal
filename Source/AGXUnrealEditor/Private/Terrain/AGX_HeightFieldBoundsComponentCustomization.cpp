// Copyright 2025, Algoryx Simulation AB.

#include "Terrain/AGX_HeightFieldBoundsComponentCustomization.h"

// Unreal Engine includes.
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"

#define LOCTEXT_NAMESPACE "FAGX_HeightFieldBoundsComponentCustomization"

TSharedRef<IDetailCustomization> FAGX_HeightFieldBoundsComponentCustomization::MakeInstance()
{
	return MakeShareable(new FAGX_HeightFieldBoundsComponentCustomization);
}

void FAGX_HeightFieldBoundsComponentCustomization::CustomizeDetails(
	IDetailLayoutBuilder& InDetailBuilder)
{
	InDetailBuilder.HideCategory(FName("Lighting"));
	InDetailBuilder.HideCategory(FName("Activation"));
	InDetailBuilder.HideCategory(FName("Rendering"));
	InDetailBuilder.HideCategory(FName("Rendering"));
	InDetailBuilder.HideCategory(FName("Tags"));
	InDetailBuilder.HideCategory(FName("Collision"));
	InDetailBuilder.HideCategory(FName("Cooking"));
	InDetailBuilder.HideCategory(FName("AssetUserData"));
}

#undef LOCTEXT_NAMESPACE
