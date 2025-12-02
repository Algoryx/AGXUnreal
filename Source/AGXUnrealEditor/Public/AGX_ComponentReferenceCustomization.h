// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"
#include "Templates/SharedPointer.h"
#include "UObject/NameTypes.h"
#include "Widgets/Input/SComboBox.h"

class SEditableTextBox;

struct FAGX_ComponentReference;

/**
 * Handles the creation and logic of the Details panel widgets for an FAGX_ComponentReference.
 */
class FAGX_ComponentReferenceCustomization : public IPropertyTypeCustomization
{
public:
	/// Factory function registered with FPropertyEditorModule::RegisterCustomPropertyTypeLayout.
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	// ~Begin IPropertyTypeCustomization interface.
	virtual void CustomizeHeader(
		TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow,
		IPropertyTypeCustomizationUtils& CustomizationUtils) override;

	virtual void CustomizeChildren(
		TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder,
		IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	// ~End IPropertyTypeCustomization interface.

private:
	/// Callback called to write a summary of the reference to the header. Also shown when
	/// collapsed.
	FText GetHeaderText() const;

	/**
	 * Re-populate the Component name combo-box with names from either the Owning Actor or the
	 * current Blueprint.
	 */
	void RebuildComboBox();

	/// Callback called by Slate when the user selects a new name in the combo-box.
	void OnComboBoxChanged(TSharedPtr<FName> NewSelection, ESelectInfo::Type SelectionInfo);

	/// Callback called by Slate when the user ends exiting the name text box.
	void OnComponentNameCommitted(const FText& InText, ETextCommit::Type InCommitType);

	/// The names of the known compatible Components in the Owning Actor plus any extra entries
	/// such as the None entry and the currently selected name, if not part of the Components.
	TArray<TSharedPtr<FName>> ComponentNames;

	/// True if the ComponentNames array contains names found in the Owning Actor or Blueprint.
	/// There are also other name in the array, so checking if empty is not correct.
	bool bFoundComponents {false};

	// @todo Is this needed? Can we skip it and always ask NameHandle?
	/// The name of the Component that is currently selected. Can be NAME_None.
	FName SelectedComponent;

	/// The combo-box listing the names of the compatible Components found, and the extra entries.
	TSharedPtr<SComboBox<TSharedPtr<FName>>> ComboBoxPtr;

	/// Combo box identical to ComboBoxPtr, but shown in the header instead of as a child.
	TSharedPtr<SComboBox<TSharedPtr<FName>>> HeaderComboBoxPtr;

	/// Delegate called when the search settings, i.e. Owning Actor and Search Child Actors, are
	/// changed. Repopulates the ComponentNames array.
	FSimpleDelegate RebuildComboBoxDelegate;

	/// The text box where the user can write a name by hand.
	TSharedPtr<SEditableTextBox> ComponentNameBoxPtr;

	/**
	 * We must not write through property handles while the Details panel is being constructed, that
	 * causes Unreal Editor to crash when editing an instance of a Blueprint because the edit causes
	 * a Blueprint reconstruction, which causes the Details panel to be re-initialized, which causes
	 * the name combo-bax changed callback to be called, which, if we do write, causes another
	 * Blueprint Reconstruction while the first one is still ongoing. This leads to a failed assert
	 * in RegisterCustomPropertyTypeLayout
	 *
	 * Assertion failed: !LayoutData.DetailLayout.IsValid() || LayoutData.DetailLayout.IsUnique()
	 *
	 * The purpose of this flag is to make sure we don't trigger that, or similar, crashes.
	 *
	 * I hope this doesn't cause us to miss any updates.
	 */
	bool bInCustomize {false};

	/**
	 * Re-fetch the property handles to the underlying data stores.
	 * This should be called at the start of every Customize.+ function.
	 */
	bool RefetchPropertyHandles(const TSharedRef<IPropertyHandle>& InComponentReferenceHandle);

	/**
	 * Set all stored IPropertyHandles to nullptr.
	 */
	void ClearPropertyHandles();

	/// Read the Component Reference through the Component Reference Handle.
	FAGX_ComponentReference* GetComponentReference() const;

	/// Read the Owning Actor through the Owning Actor handle.
	AActor* GetOwningActor() const;

	/**
	 *  Get the Actor that is searched for the referenced Component. Is the Owning Actor if one
	 *  has been set, otherwise the fallback Local Scope is used.
	 */
	AActor* GetScope() const;

	/// Read the Component name through the Name Handle.
	FName GetName() const;

	/// Read the Search Child Actors flag through Search Child Actors Handle.
	bool GetSearchChildActors() const;

	TSharedPtr<IPropertyHandle> ComponentReferenceHandle;
	TSharedPtr<IPropertyHandle> OwningActorHandle;
	TSharedPtr<IPropertyHandle> NameHandle;
	TSharedPtr<IPropertyHandle> SearchChildActorsHandle;

	friend struct FAGX_ComponentReferenceCustomizationOperations;
};
