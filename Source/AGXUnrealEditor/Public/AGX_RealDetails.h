#pragma once

// Unreal Engine includes.
#include "IPropertyTypeCustomization.h"
#include "Layout/Visibility.h"

class FAGX_RealDetails : public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	virtual void CustomizeHeader(
		TSharedRef<class IPropertyHandle> StructPropertyHandle,
		class FDetailWidgetRow& HeaderRow,
		IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

	virtual void CustomizeChildren(
		TSharedRef<class IPropertyHandle> StructPropertyHandle,
		class IDetailChildrenBuilder& StructBuilder,
		IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

private:
	EVisibility VisibleWhenSingleSelection() const;
	EVisibility VisibleWhenMultiSelection() const;
	EVisibility VisibleWhenNoSelectionOrInvalidHandle() const;
	double GetDoubleValue() const;
	FText GetTextValue() const;
	void OnSpinChanged(double NewValue);
	void OnSpinCommitted(double NewValue, ETextCommit::Type CommitInfo);
	void OnTextChanged(const FText& NewText);
	void OnTextCommitted(const FText& NewText, ETextCommit::Type CommitInfo);

private:
	TSharedPtr<IPropertyHandle> StructHandle;
	TSharedPtr<IPropertyHandle> ValueHandle;
};