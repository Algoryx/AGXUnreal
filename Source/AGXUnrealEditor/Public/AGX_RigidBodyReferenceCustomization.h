#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"
#include "Templates/SharedPointer.h"
#include "Widgets/Input/SComboBox.h"

struct FAGX_RigidBodyReference;

class AActor;
class FDetailWidgetRow;
class PropertyHandle;

class FAGX_RigidBodyReferenceCustomization : public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	virtual void CustomizeHeader(
		TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow,
		IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

	virtual void CustomizeChildren(
		TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder,
		IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

private:
	/**
	 * Re-fetch the pointers/handles to the underlying data stores.
	 * This should be called at the start of every Customize.+ function.
	 */
	void RefreshStoreReferences(IPropertyHandle& StructPropertyHandle);

	FText GetHeaderText() const;

	/**
	 * Rebuild the contents of the name selection combo box widget. Will fetch the
	 * current names and then repopulate the combo box. This member function
	 * should be called whenever the owning Actor has been changed.
	 */
	void RebuildComboBox();

	/**
	 * Fetch all names of RigidBodyComponents held by the pointed-to owning
	 * Actor. Optionally searches child actors if enabled in the
	 * RigidBodyReference instance. The found names are stored internally in
	 * this Customization. This member function should be called whenever the
	 * owning Actor has been changed, often via RebuildComboBox.
	 */
	void FetchBodyNames();

	/**
	 * Called when the user selects a RigidBodyComponent name from the name ComboBox.
	 */
	void OnComboBoxChanged(TSharedPtr<FName> NewSelection, ESelectInfo::Type SelectionInfo);

	/**
	 * Return the the currently selected body-owning Actor, if exactly one RigidBodyReference is
	 * selected and that reference has a selected owning Actor.
	 */
	AActor* GetOwningActor();

private:
	/// List of known body names in the selected owning Actor. Updated when FetchBodyNames is
	/// called.
	TArray<TSharedPtr<FName>> BodyNames;

	/// The name of the currently selected body.
	FName SelectedBody;

	/// GUI widget for presenting body names and selecting which particular body in the owning
	/// Actor that this reference should reference.
	/// \todo Do not use raw pointer here. Either SharedPtr or WeakPtr.
	SComboBox<TSharedPtr<FName>>* ComboBoxPtr;

	FSimpleDelegate RebuildComboBoxDelegate;

	FAGX_RigidBodyReference* RigidBodyReference;

	TSharedPtr<IPropertyHandle> OwningActorHandle;
	TSharedPtr<IPropertyHandle> BodyNameHandle;
	TSharedPtr<IPropertyHandle> SearchChildActorsHandle;
};