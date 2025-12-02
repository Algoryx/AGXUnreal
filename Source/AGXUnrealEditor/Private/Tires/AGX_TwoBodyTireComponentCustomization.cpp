// Copyright 2025, Algoryx Simulation AB.

#include "Tires/AGX_TwoBodyTireComponentCustomization.h"

// AGX Dynamics for Unreal includes.
#include "Tires/AGX_TwoBodyTireComponent.h"
#include "Utilities/AGX_EditorUtilities.h"

// Unreal Engine includes.
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"

#define LOCTEXT_NAMESPACE "FAGX_TwoBodyTireComponentCustomization"

TSharedRef<IDetailCustomization> FAGX_TwoBodyTireComponentCustomization::MakeInstance()
{
	return MakeShareable(new FAGX_TwoBodyTireComponentCustomization);
}

void FAGX_TwoBodyTireComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	UAGX_TwoBodyTireComponent* TwoBodyTireComponent =
		FAGX_EditorUtilities::GetSingleObjectBeingCustomized<UAGX_TwoBodyTireComponent>(
			DetailBuilder);

	if (!TwoBodyTireComponent)
	{
		return;
	}

	// In case this UAGX_TwoBodyTireComponent is the default UAGX_TwoBodyTireComponent of an
	// AAGX_TwoBodyTireActor, the tire and hub Rigid Body references are hidden since they are
	// already set up by the AAGX_TwoBodyTireActor and should not be changed.
	if (TwoBodyTireComponent->IsDefaultSubObjectOfTwoBodyTireActor())
	{
		DetailBuilder.HideProperty(DetailBuilder.GetProperty(
			GET_MEMBER_NAME_CHECKED(UAGX_TwoBodyTireComponent, TireRigidBody)));

		DetailBuilder.HideProperty(DetailBuilder.GetProperty(
			GET_MEMBER_NAME_CHECKED(UAGX_TwoBodyTireComponent, HubRigidBody)));
	}
}

#undef LOCTEXT_NAMESPACE
