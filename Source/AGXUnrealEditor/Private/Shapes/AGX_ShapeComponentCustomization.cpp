// Copyright 2024, Algoryx Simulation AB.

#include "Shapes/AGX_ShapeComponentCustomization.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Shapes/AGX_AutoFitShapeDetails.h"
#include "Shapes/AGX_BoxShapeComponent.h"
#include "Shapes/AGX_CylinderShapeComponent.h"
#include "Shapes/AGX_CapsuleShapeComponent.h"
#include "Shapes/AGX_ShapeComponent.h"
#include "Utilities/AGX_EditorUtilities.h"

// Unreal Engine includes.
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Engine/SCS_Node.h"
#include "ScopedTransaction.h"

#define LOCTEXT_NAMESPACE "FAGX_ShapeComponentCustomization"

namespace AGX_ShapeComponentCustomization_helpers
{
	bool SupportsAutoFit(UAGX_ShapeComponent* Shape)
	{
		if (Cast<UAGX_BoxShapeComponent>(Shape))
		{
			return true;
		}
		if (Cast<UAGX_CapsuleShapeComponent>(Shape))
		{
			return true;
		}
		if (Cast<UAGX_CylinderShapeComponent>(Shape))
		{
			return true;
		}
		return false;
	}
}

TSharedRef<IDetailCustomization> FAGX_ShapeComponentCustomization::MakeInstance()
{
	return MakeShareable(new FAGX_ShapeComponentCustomization);
}

void FAGX_ShapeComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	UAGX_ShapeComponent* ShapeComponent =
		FAGX_EditorUtilities::GetSingleObjectBeingCustomized<UAGX_ShapeComponent>(DetailBuilder);

	if (!ShapeComponent)
	{
		return;
	}

	if (AGX_ShapeComponentCustomization_helpers::SupportsAutoFit(ShapeComponent))
	{
		// Ensure correct category order in the Details panel.
		DetailBuilder.EditCategory("AGX Shape", FText::GetEmpty(), ECategoryPriority::TypeSpecific);

		// Build the auto-fit category.
		IDetailCategoryBuilder& CategoryBuilder = DetailBuilder.EditCategory(
			"AGX Shape Auto-fit", FText::GetEmpty(), ECategoryPriority::TypeSpecific);
		CategoryBuilder.AddCustomBuilder(
			MakeShareable(new FAGX_AutoFitShapeDetails(DetailBuilder)));
	}
}

#undef LOCTEXT_NAMESPACE
