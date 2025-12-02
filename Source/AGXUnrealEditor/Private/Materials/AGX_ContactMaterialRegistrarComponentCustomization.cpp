// Copyright 2025, Algoryx Simulation AB.

#include "Materials/AGX_ContactMaterialRegistrarComponentCustomization.h"

// Unreal Engine includes.
#include "DetailLayoutBuilder.h"

#define LOCTEXT_NAMESPACE "FAGX_ContactMaterialRegistrarComponentCustomization"

TSharedRef<IDetailCustomization> FAGX_ContactMaterialRegistrarComponentCustomization::MakeInstance()
{
	return MakeShareable(new FAGX_ContactMaterialRegistrarComponentCustomization);
}

void FAGX_ContactMaterialRegistrarComponentCustomization::CustomizeDetails(
	IDetailLayoutBuilder& DetailBuilder)
{
	DetailBuilder.EditCategory(
		"AGX Contact Material Registrar", FText::GetEmpty(), ECategoryPriority::Variable);
}

#undef LOCTEXT_NAMESPACE
