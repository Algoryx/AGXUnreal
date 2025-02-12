// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Templates/SharedPointer.h"

class FMenuBuilder;
class FText;

/**
 * Creates the AGX Top Menu on the Unreal Main Menu Bar.
 */
class AGXUNREALEDITOR_API FAGX_TopMenu
{
public:
	FAGX_TopMenu();
	virtual ~FAGX_TopMenu();

private:
	static void CreateTopMenu(FMenuBarBuilder& Builder);
	virtual void FillTopMenu(FMenuBuilder& Builder); // Must be virtual because of dirty hack (see
													 // comment in CreateTopMenu)!
	void FillConstraintMenu(FMenuBuilder& Builder);

	void FillFileMenu(FMenuBuilder& Builder);

	void FillLicenseMenu(FMenuBuilder& Builder);

	void FillGrabModeMenu(FMenuBuilder& Builder);

	template <typename Function>
	void AddFileMenuEntry(
		FMenuBuilder& Builder, const FText& Label, const FText& Tooltip,
		Function MenuItemClickCallbackFunction);

	void OnCreateConstraintClicked(UClass* ConstraintClass);
	void OnVisitDemoPageClicked();
	void OnVisitUserManualPageClicked();
	void OnOpenAboutDialogClicked();

	void OnOpenLicenseActivationDialogClicked();
	void OnOpenOfflineActivationDialogClicked();
	void OnOpenGenerateRuntimeActivationDialogClicked();

	void OnStartGrabModeDialogClicked();
	void OnStopGrabModeDialogClicked();

	TSharedPtr<class FExtender> Extender;
	TSharedPtr<const class FExtensionBase> UnrealMenuBarExtension;
};

template <typename Function>
void FAGX_TopMenu::AddFileMenuEntry(
	FMenuBuilder& Builder, const FText& Label, const FText& Tooltip,
	Function MenuItemClickCallbackFunction)
{
	Builder.AddMenuEntry(
		Label, Tooltip, FSlateIcon(), FExecuteAction::CreateLambda(MenuItemClickCallbackFunction),
		NAME_None, EUserInterfaceActionType::Button);
}
