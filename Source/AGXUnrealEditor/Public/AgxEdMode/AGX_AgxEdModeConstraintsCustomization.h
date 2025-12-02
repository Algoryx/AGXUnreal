// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_AgxEdModeConstraints.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "Styling/SlateTypes.h"
#include "Types/SlateEnums.h"

class IDetailLayoutBuilder;
class IDetailCategoryBuilder;
class UAGX_AgxEdModeConstraints;

/**
 * Defines the design of the Constraints Sub-Mode of AGX Editor Mode.
 */
class FAGX_AgxEdModeConstraintsCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

public:
	FAGX_AgxEdModeConstraintsCustomization();

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private: // Constraint Creator
	void CreateConstraintCreatorCategory(
		IDetailLayoutBuilder& DetailBuilder, UAGX_AgxEdModeConstraints* ConstraintsSubMode);

	void CreateConstraintTypeComboBox(
		IDetailCategoryBuilder& CategoryBuilder, UAGX_AgxEdModeConstraints* ConstraintsSubMode);

	void CreateGetFromSelectedActorsButton(
		IDetailCategoryBuilder& CategoryBuilder, UAGX_AgxEdModeConstraints* ConstraintsSubMode);

	void CreateFrameSourceRadioButtons(
		IDetailCategoryBuilder& CategoryBuilder, UAGX_AgxEdModeConstraints* ConstraintsSubMode);

	void OnConstraintTypeComboBoxChanged(
		UClass* NewSelectedItem, ESelectInfo::Type InSeletionInfo,
		UAGX_AgxEdModeConstraints* ConstraintsSubMode);

	void OnFrameSourceRadioButtonChanged(
		ECheckBoxState NewCheckedState, EAGX_ConstraintCreationFrameSource RadioButton,
		UAGX_AgxEdModeConstraints* ConstraintsSubMode);

	TArray<UClass*> ConstraintClasses;

private: // Constraint Browser
	void CreateConstraintBrowserCategory(
		IDetailLayoutBuilder& DetailBuilder, UAGX_AgxEdModeConstraints* ConstraintsSubMode);

	void CreateConstraintBrowserListView(
		IDetailCategoryBuilder& CategoryBuilder, UAGX_AgxEdModeConstraints* ConstraintsSubMode);
};
