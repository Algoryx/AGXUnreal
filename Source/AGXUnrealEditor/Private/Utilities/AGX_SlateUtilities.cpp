// Copyright 2023, Algoryx Simulation AB.

#include "Utilities/AGX_SlateUtilities.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"

// Unreal Engine includes.
#include "Fonts/SlateFontInfo.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SWidget.h"

FSlateFontInfo FAGX_SlateUtilities::CreateFont(int Size)
{
	FSlateFontInfo F = IPropertyTypeCustomizationUtils::GetRegularFont();
	F.Size = Size;
	return F;
}



bool FAGX_SlateUtilities::RemoveChildWidgetByType(
	const TSharedPtr<SWidget>& Parent, const FString& TypeNameToRemove, bool Recursive)
{
	if (!Parent)
		return false;

	FChildren* Children = Parent->GetChildren();

	for (int32 ChildIndex = 0; ChildIndex < Children->Num(); ++ChildIndex)
	{
		TSharedRef<SWidget> Child = Children->GetChildAt(ChildIndex);

		if (Child->GetTypeAsString() == TypeNameToRemove)
		{
			const FString ParentType = Parent->GetTypeAsString();
			if (ParentType == "SHorizontalBox" || ParentType == "SVerticalBox" ||
				ParentType == "SBoxPanel")
			{
				SBoxPanel* BoxPanel = static_cast<SBoxPanel*>(Parent.Get());

				if (BoxPanel->RemoveSlot(Child) != -1)
					return true;
			}
		}

		if (Recursive && RemoveChildWidgetByType(Child, TypeNameToRemove, Recursive))
			return true;
	}

	return false;
}

void FAGX_SlateUtilities::LogChildWidgets(
	const TSharedPtr<SWidget>& Parent, bool Recursive, const FString& Prefix)
{
	if (!Parent)
		return;

	FChildren* Children = Parent->GetChildren();

	for (int32 ChildIndex = 0; ChildIndex < Children->Num(); ++ChildIndex)
	{
		TSharedRef<SWidget> Child = Children->GetChildAt(ChildIndex);

		UE_LOG(LogAGX, Log, TEXT("%s%s"), *Prefix, *Child->GetTypeAsString());

		if (Recursive)
			LogChildWidgets(Child, Recursive, Prefix + "  ");
	}
}