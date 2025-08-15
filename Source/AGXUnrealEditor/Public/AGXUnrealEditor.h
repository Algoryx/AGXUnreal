// Copyright 2025, Algoryx Simulation AB.

#pragma once

#include "CoreMinimal.h"
#include "Containers/Array.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

class FComponentVisualizer;
class FToolBarBuilder;
class FMenuBuilder;
class IAssetTools;
class IAssetTypeActions;

struct FAssetData;

class FAGXUnrealEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/// \todo Should this really return a reference to a shared pointer, or would a TAGX_TopMenu* be
	/// better?
	const TSharedPtr<class FAGX_TopMenu>& GetAgxTopMenu() const;

private:
	/**
	 * Registers settings exposed in the Project Settings window.
	 * This is typically the default simulation settings.
	 */
	void RegisterProjectSettings();

	/**
	 * Unregisters settings exposed in the Project Settings window.
	 */
	void UnregisterProjectSettings();

	void RegisterCommands();
	void UnregisterCommands();

	void RegisterAssetTypeActions();
	void UnregisterAssetTypeActions();

	void RegisterAssetTypeAction(
		IAssetTools& AssetTools, const TSharedPtr<IAssetTypeActions>& Action);

	/**
	 * Registers property type customizations (IPropertyTypeCustomization),
	 * and class detail customizations (IDetailCustomization).
	 */
	void RegisterCustomizations();

	/**
	 * Unregisters property type customizations and class detail customizations.
	 */
	void UnregisterCustomizations();

	void RegisterComponentVisualizers();
	void UnregisterComponentVisualizers();

	void RegisterComponentVisualizer(
		const FName& ComponentClassName, TSharedPtr<FComponentVisualizer> Visualizer);
	void UnregisterComponentVisualizer(const FName& ComponentClassName);

	void RegisterModes();
	void UnregisterModes();

	void RegisterPlacementCategory();
	void UnregisterPlacementCategory();

	void InitializeAssets();

	void OnGrabModeCommand() const;
	bool OnCanExecuteGrabModeCommand() const;

	void OnAssetRemoved(const FAssetData& AssetData);

private:
	TSharedPtr<class FUICommandList> PluginCommands;
	TSharedPtr<class FAGX_TopMenu> AgxTopMenu;

	TArray<TSharedPtr<IAssetTypeActions>> RegisteredAssetTypeActions;
};
