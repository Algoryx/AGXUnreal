// Copyright 2024, Algoryx Simulation AB.

#include "AGX_RigidBodyComponentCustomization.h"

// AGX Dynamics for Unreal includes.
#include "AGX_RigidBodyCustomizationRuntime.h"
#include "Utilities/AGX_EditorUtilities.h"

// Unreal Engine includes.
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "FAGX_RigidBodyComponentCustomization"

TSharedRef<IDetailCustomization> FAGX_RigidBodyComponentCustomization::MakeInstance()
{
	return MakeShareable(new FAGX_RigidBodyComponentCustomization);
}

void FAGX_RigidBodyComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& InDetailBuilder)
{
	DetailBuilder = &InDetailBuilder;

	UAGX_RigidBodyComponent* RigidBodyComponent =
		FAGX_EditorUtilities::GetSingleObjectBeingCustomized<UAGX_RigidBodyComponent>(
			*DetailBuilder);

	if (!RigidBodyComponent)
		return;

	RigidBodyComponent->OnComponentView();

	IDetailCategoryBuilder& Runtime =
		DetailBuilder->EditCategory(TEXT("AGX Runtime"), LOCTEXT("AGXRuntime", "AGX Runtime"));
	Runtime.AddCustomBuilder(MakeShareable(new FAGX_RigidBodyCustomizationRuntime(*DetailBuilder)));
}

#undef LOCTEXT_NAMESPACE
