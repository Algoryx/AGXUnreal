// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "IPropertyTypeCustomization.h"
#include "Layout/Visibility.h"

struct FAGX_Frame;

/**
 * A Property Type Customization that displays FAGX_Frame Properties.
 */
class FAGX_FrameCustomization : public IPropertyTypeCustomization
{
public:
	using Super = IPropertyTypeCustomization;

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
	 * Re-fetch the Property Handles to the underlying data stores.
	 * This should be called at the start of every Customize.* function.
	 */
	bool RefetchPropertyHandles(TSharedRef<IPropertyHandle>& InFrameHandle);

	/// Set all stored IPropertyHandles to nullptr.
	void ClearPropertyHandles();

	/// Read the Component Reference through the Component Reference Handle.
	FAGX_Frame* GetFrame() const;

	// Handle to the entire AGX_Frame struct.
	TSharedPtr<IPropertyHandle> FrameHandle;

	// Handle to the Parent struct member.
	TSharedPtr<IPropertyHandle> ParentHandle;

	// Handle to the Owning Actor struct member in the Parent member.
	TSharedPtr<IPropertyHandle> OwningActorHandle;

	// Handle to the Local Location struct member.
	TSharedPtr<IPropertyHandle> LocalLocationHandle;

	// Handle to the Local Rotation struct member.
	TSharedPtr<IPropertyHandle> LocalRotationHandle;
};
