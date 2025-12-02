// Copyright 2025, Algoryx Simulation AB.

#include "Wire/AGX_WireWinchDetails.h"

// AGX Dynamics for Unreal includes.
#include "Wire/AGX_WireWinchComponent.h"
#include "Wire/AGX_WireWinchDetailsRuntime.h"

// Unreal Engine includes.
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"

#define LOCTEXT_NAMESPACE "AGX_WireWinchDetails"

TSharedRef<IDetailCustomization> FAGX_WireWinchDetails::MakeInstance()
{
	return MakeShareable(new FAGX_WireWinchDetails());
}

namespace AGX_WireWinchDetail_helpers
{
	FAGX_WireWinch* WinchGetter(UObject* Object)
	{
		UAGX_WireWinchComponent* Winch = Cast<UAGX_WireWinchComponent>(Object);
		return Winch != nullptr ? &Winch->WireWinch : nullptr;
	}
}

void FAGX_WireWinchDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& RuntimeCategory = DetailBuilder.EditCategory("AGX Runtime");
	RuntimeCategory.AddCustomBuilder(MakeShareable(
		new FAGX_WireWinchDetailsRuntime(DetailBuilder, AGX_WireWinchDetail_helpers::WinchGetter)));
}

#undef LOCTEXT_NAMESPACE
