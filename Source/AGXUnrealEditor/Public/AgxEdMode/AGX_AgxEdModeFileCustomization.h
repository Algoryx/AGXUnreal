// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailCustomization.h"
#include "Widgets/Input/SButton.h"

class IDetailLayoutBuilder;

/**
 *
 */
class FAGX_AgxEdModeFileCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

public:
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
	void CustomizeFileImporterCategory(IDetailLayoutBuilder& DetailBuilder);
	void CustomizeFileExporterCategory(IDetailLayoutBuilder& DetailBuilder);
	void CustomizeMaterialLibraryCategory(IDetailLayoutBuilder& DetailBuilder);

	template <typename Function>
	void AddCustomButton(
		IDetailCategoryBuilder& CategoryBuilder, const FText& ButtonText, const FText& ToolTip,
		Function ButtonClickCallbackFunction);
};

template <typename Function>
void FAGX_AgxEdModeFileCustomization::AddCustomButton(
	IDetailCategoryBuilder& CategoryBuilder, const FText& ButtonText, const FText& ToolTip,
	Function ButtonClickCallbackFunction)
{
	/** Create custom button */
	CategoryBuilder.AddCustomRow(FText::GetEmpty())
		[SNew(SHorizontalBox) + SHorizontalBox::Slot().AutoWidth() +
		 SHorizontalBox::Slot().AutoWidth()[SNew(SButton)
												.Text(ButtonText)
												.ToolTipText(ToolTip)
												.OnClicked_Lambda(ButtonClickCallbackFunction)]];
}
