// Copyright 2025, Algoryx Simulation AB.

#include "AGX_FrameCustomization.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Frame.h"
#include "Utilities/AGX_EditorUtilities.h"
#include "Utilities/AGX_StringUtilities.h"

// Unreal Engine includes.
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "AGX_FrameCustomization"

TSharedRef<IPropertyTypeCustomization> FAGX_FrameCustomization::MakeInstance()
{
	return MakeShareable(new FAGX_FrameCustomization());
}

void FAGX_FrameCustomization::CustomizeHeader(
	TSharedRef<IPropertyHandle> InComponentReferenceHandle, FDetailWidgetRow& HeaderRow,
	IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	if (!RefetchPropertyHandles(InComponentReferenceHandle))
	{
		// There is something wrong with the Property Handles that we need. Generate a warning text
		// in the header.

		// clang-format off
		HeaderRow
		.WholeRowContent()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("NoPropertyHandle", "Invalid property handle."))
			.Font(IPropertyTypeCustomizationUtils::GetRegularFont())
			.MinDesiredWidth(250.0f)
		];
		// clang-format on

		return;
	}

	// clang-format off
	HeaderRow
	.NameContent()
	[
		FrameHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	.MinDesiredWidth(250.0f)
	[
		SNew(STextBlock)
		.Text(this, &FAGX_FrameCustomization::GetHeaderText)
		.ToolTipText(this, &FAGX_FrameCustomization::GetHeaderText)
		.Font(IPropertyTypeCustomizationUtils::GetRegularFont())
		.MinDesiredWidth(250.0f)
	];
	// clang-format on
}

void FAGX_FrameCustomization::CustomizeChildren(
	TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder,
	IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	// Create default widgets for the Properties that don't need any special attention.
	ChildBuilder.AddProperty(ParentHandle.ToSharedRef());
	ChildBuilder.AddProperty(LocalLocationHandle.ToSharedRef());
	ChildBuilder.AddProperty(LocalRotationHandle.ToSharedRef());
}

bool FAGX_FrameCustomization::RefetchPropertyHandles(TSharedRef<IPropertyHandle>& InFrameHandle)
{
	ClearPropertyHandles();

	FrameHandle = InFrameHandle;
	if (!FrameHandle.IsValid() || !FrameHandle->IsValidHandle())
	{
		ClearPropertyHandles();
		return false;
	}

	ParentHandle = FrameHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FAGX_Frame, Parent));
	if (!ParentHandle.IsValid() || !ParentHandle->IsValidHandle())
	{
		ClearPropertyHandles();
		return false;
	}

	OwningActorHandle =
		ParentHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FAGX_ComponentReference, OwningActor));
	LocalLocationHandle =
		FrameHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FAGX_Frame, LocalLocation));
	LocalRotationHandle =
		FrameHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FAGX_Frame, LocalRotation));

	if (!OwningActorHandle.IsValid() || !OwningActorHandle->IsValidHandle() ||
		!LocalLocationHandle.IsValid() || !LocalLocationHandle->IsValidHandle() ||
		!LocalRotationHandle.IsValid() || !LocalRotationHandle->IsValidHandle())
	{
		ClearPropertyHandles();
		return false;
	}

	return true;
}

void FAGX_FrameCustomization::ClearPropertyHandles()
{
	FrameHandle = nullptr;
	OwningActorHandle = nullptr;
	LocalLocationHandle = nullptr;
	LocalRotationHandle = nullptr;
}

FAGX_Frame* FAGX_FrameCustomization::GetFrame() const
{
	if (!FrameHandle.IsValid() || !FrameHandle->IsValidHandle())
	{
		return nullptr;
	}

	void* UntypedPointer = nullptr;
	const FPropertyAccess::Result Result = FrameHandle->GetValueData(UntypedPointer);
	if (Result != FPropertyAccess::Success || UntypedPointer == nullptr)
	{
		return nullptr;
	}

	return reinterpret_cast<FAGX_Frame*>(UntypedPointer);
}

FText FAGX_FrameCustomization::GetHeaderText() const
{
	const FAGX_Frame* Frame = GetFrame();
	if (Frame == nullptr)
	{
		return LOCTEXT(
			"NoComponentReferece",
			"Component Reference Customization does not have a valid Component Reference");
	}

	const AActor* OwningActor = Frame->Parent.OwningActor;
	const FName ComponentName = Frame->Parent.Name;
	const FString ActorName = OwningActor ? GetLabelSafe(OwningActor) : TEXT("(Self)");
	return FText::Format(
		LOCTEXT("HeaderText", "{0} in {1}"), FText::FromName(ComponentName),
		FText::FromString(ActorName));
}

#undef LOCTEXT_NAMESPACE
