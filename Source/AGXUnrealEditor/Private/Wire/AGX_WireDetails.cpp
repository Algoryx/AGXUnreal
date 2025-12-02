// Copyright 2025, Algoryx Simulation AB.

#include "Wire/AGX_WireDetails.h"

// AGX Dynamics for Unreal includes.
#include "Wire/AGX_WireComponent.h"
#include "Wire/AGX_WireDetailsRuntime.h"
#include "Wire/AGX_WireNodeDetails.h"

// Unreal Engine includes.
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"

#define LOCTEXT_NAMESPACE "AGX_WireDetails"

TSharedRef<IDetailCustomization> FAGX_WireDetails::MakeInstance()
{
	return MakeShareable(new FAGX_WireDetails());
}

void FAGX_WireDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	ECategoryPriority::Type Order = ECategoryPriority::TypeSpecific;
	DetailBuilder.EditCategory("AGX Wire", LOCTEXT("AGXWire", "AGX Wire"), Order);
	DetailBuilder.EditCategory(
		"AGX Wire Begin Winch", LOCTEXT("AGXWireBeginWinch", "AGX Wire Begin Winch"), Order);
	DetailBuilder.EditCategory(
		"AGX Wire End Winch", LOCTEXT("AGXWireEndWinch", "AGX Wire End Winch"), Order);
	DetailBuilder.EditCategory("AGX Wire Route", LOCTEXT("AGXWireRoute", "AGX Wire Route"), Order);
	DetailBuilder.EditCategory("Selected Node", LOCTEXT("SelectedNode", "Selected Node"), Order);
	DetailBuilder.EditCategory("AGX Runtime", LOCTEXT("AGXRuntime", "AGX Runtime"), Order);

	IDetailCategoryBuilder& RuntimeCategory = DetailBuilder.EditCategory("AGX Runtime");
	RuntimeCategory.AddCustomBuilder(MakeShareable(new FAGX_WireDetailsRuntime(DetailBuilder)));
	IDetailCategoryBuilder& SelectedNodeCategory = DetailBuilder.EditCategory("Selected Node");
	SelectedNodeCategory.AddCustomBuilder(MakeShareable(new FAGX_WireNodeDetails(DetailBuilder)));
}

#undef LOCTEXT_NAMESPACE
