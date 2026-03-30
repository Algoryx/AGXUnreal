// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/AGX_IMUSensorComponentCustomization.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_IMUSensorComponent.h"
#include "Utilities/AGX_EditorUtilities.h"

// Unreal Engine includes.
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"

#define LOCTEXT_NAMESPACE "FAGX_IMUSensorComponentCustomization"

TSharedRef<IDetailCustomization> FAGX_IMUSensorComponentCustomization::MakeInstance()
{
	return MakeShareable(new FAGX_IMUSensorComponentCustomization);
}

void FAGX_IMUSensorComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& InDetailBuilder)
{
	// Ensure high priority for our categories.
	InDetailBuilder.EditCategory("AGX Sensor", FText::GetEmpty(), ECategoryPriority::Important);
	InDetailBuilder.EditCategory("AGX IMU", FText::GetEmpty(), ECategoryPriority::Important);

	InDetailBuilder.HideCategory(FName("Sockets"));
	InDetailBuilder.HideCategory(FName("Visualisation"));
}

#undef LOCTEXT_NAMESPACE
