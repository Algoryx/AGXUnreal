// Copyright 2025, Algoryx Simulation AB.

#include "Constraints/AGX_ConstraintBodyAttachmentCustomization.h"

// AGX Dynamics for Unreal includes.
#include "AGX_RigidBodyComponent.h"
#include "Constraints/AGX_ConstraintComponent.h"
#include "Constraints/AGX_ConstraintBodyAttachment.h"
#include "Utilities/AGX_PropertyUtilities.h"

// Unreal Engine includes
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "FAGX_ConstraintBodyAttachmentCustomization"

TSharedRef<IPropertyTypeCustomization> FAGX_ConstraintBodyAttachmentCustomization::MakeInstance()
{
	return MakeShareable(new FAGX_ConstraintBodyAttachmentCustomization());
}

void FAGX_ConstraintBodyAttachmentCustomization::CustomizeHeader(
	TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow,
	IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	BodyAttachmentProperty = StructPropertyHandle;

	RigidBodyProperty = StructPropertyHandle->GetChildHandle(
		GET_MEMBER_NAME_CHECKED(FAGX_ConstraintBodyAttachment, RigidBody));

	// Use default visualization in the name column (left side).
	HeaderRow.NameContent()[StructPropertyHandle->CreatePropertyNameWidget()];

	// Show the name of chosen rigid body in the value column (right side).
	HeaderRow.ValueContent()
		.MinDesiredWidth(250.0f) // from SPropertyEditorAsset::GetDesiredWidth
		.MaxDesiredWidth(
			0)[SNew(STextBlock)
				   .Text(this, &FAGX_ConstraintBodyAttachmentCustomization::GetRigidBodyLabel)
				   .Font(IPropertyTypeCustomizationUtils::GetRegularFont())
				   .ColorAndOpacity(FLinearColor(1.0f, 0.45f, 0, 1.0f))
				   .MinDesiredWidth(250)];
}

void FAGX_ConstraintBodyAttachmentCustomization::CustomizeChildren(
	TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder,
	IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	FrameDefiningComponentProperty = StructPropertyHandle->GetChildHandle(
		GET_MEMBER_NAME_CHECKED(FAGX_ConstraintBodyAttachment, FrameDefiningComponent));

	uint32 NumChildren = 0;
	StructPropertyHandle->GetNumChildren(NumChildren);

	// Use default visualization for most of the structs properties.
	for (uint32 ChildIndex = 0; ChildIndex < NumChildren; ++ChildIndex)
	{
		if (TSharedPtr<IPropertyHandle> ChildHandle =
				StructPropertyHandle->GetChildHandle(ChildIndex))
		{
			// Add default visualization.
			IDetailPropertyRow& DefaultPropertyRow =
				StructBuilder.AddProperty(ChildHandle.ToSharedRef());

/// \todo Update 'Create New Frame Defining Actor' functionality to be FrameDefiningComponent
/// compatible.
// This code added a 'Create New Frame Defining Actor' entry in right-click context menu. Not sure
// how to best do that now that the property is a Component instead of an actor. The effect could
// still be the same, create a new Constriant Frame Actor and assign its RootComponent to the
// ConstraintFrameComponent property. But is it always safe to create a new Actor? It isn't when
// in the Blueprint editor, for example. Leaving this disabled for now.
#if 0
			// Add "Create New" option to context menu for the Frame Defining Actor.
			if (FAGX_PropertyUtilities::PropertyEquals(ChildHandle, FrameDefiningActorProperty))
			{
				// To do this, the default widgets need to be extracted, and the row needs
				// to be built up again using them, removing the additional Reset button,
				// and adding the additional context menu item.

				TSharedPtr<SWidget> DefaultNameWidget;
				TSharedPtr<SWidget> DefaultValueWidget;
				DefaultPropertyRow.GetDefaultWidgets(DefaultNameWidget, DefaultValueWidget, true);

				FAGX_SlateUtilities::RemoveChildWidgetByType(
					DefaultValueWidget, "SResetToDefaultPropertyEditor");

				FDetailWidgetRow& CustomPropertyRow =
					DefaultPropertyRow.CustomWidget(/*bShowChildren*/ true);

				CustomPropertyRow.AddCustomContextMenuAction(
					FUIAction(
						FExecuteAction::CreateSP(
							this, &FAGX_ConstraintBodyAttachmentCustomization::
									  CreateAndSetFrameDefiningActor),
						FCanExecuteAction::CreateLambda(
							[this] { return !HasFrameDefiningActor(); })),
					LOCTEXT("CreateFrameDefiningActorContextMenuItem", "Create New"));

				CustomPropertyRow.NameContent()[DefaultNameWidget.ToSharedRef()];

				CustomPropertyRow.ValueContent()
					.MinDesiredWidth(250.0f) // from SPropertyEditorAsset::GetDesiredWidth
					.MaxDesiredWidth(0)
						[SNew(SBox)
							 .VAlign(VAlign_Center)
							 .Padding(FMargin(
								 0, 0, 0,
								 0)) // Line up with the other properties due to having no reset
									 // to default button
								 [SNew(SVerticalBox) +
								  SVerticalBox::Slot().AutoHeight()
									  [SNew(SHorizontalBox) +
									   SHorizontalBox::Slot()[DefaultValueWidget.ToSharedRef()]]]];
			}
#endif
		}
	}

	StructBuilder.AddCustomRow(FText::FromString(""));
}

namespace
{
	FAGX_ConstraintBodyAttachment* GetConstraintBodyAttachment(
		const TSharedPtr<IPropertyHandle>& BodyAttachmentProperty)
	{
		return FAGX_PropertyUtilities::GetStructFromHandle<FAGX_ConstraintBodyAttachment>(
			BodyAttachmentProperty,
			FAGX_PropertyUtilities::GetParentObjectOfStruct(BodyAttachmentProperty));
	}
}

FText FAGX_ConstraintBodyAttachmentCustomization::GetRigidBodyLabel() const
{
	FAGX_ConstraintBodyAttachment* Attachment = GetConstraintBodyAttachment(BodyAttachmentProperty);
	check(Attachment != nullptr);

	USceneComponent* SceneComponent = Attachment->GetRigidBody();
	if (SceneComponent == nullptr)
	{
		if (Attachment->RigidBody.Name == NAME_None)
		{
			return FText::FromString(TEXT("<Nothing selected>"));
		}
		else
		{
			return FText::FromString(Attachment->RigidBody.Name.ToString());
		}
	}
	UAGX_RigidBodyComponent* Body = Cast<UAGX_RigidBodyComponent>(SceneComponent);
	if (Body == nullptr)
	{
		return FText::FromString(TEXT("<Something not a body selected>"));
	}
	return FText::FromString(Body->GetName());
}

bool FAGX_ConstraintBodyAttachmentCustomization::HasRigidBody() const
{
	return GetConstraintBodyAttachment(BodyAttachmentProperty)->GetRigidBody() != nullptr;
}

bool FAGX_ConstraintBodyAttachmentCustomization::HasFrameDefiningActor() const
{
	return FAGX_PropertyUtilities::GetObjectFromHandle(FrameDefiningComponentProperty) != nullptr;
}

FString GenerateFrameDefiningActorName(
	const UAGX_ConstraintComponent* Constraint, const UAGX_RigidBodyComponent* RigidBody)
{
	check(Constraint);
	return "Constraint Frame Actor for " + Constraint->GetName();
}

void FAGX_ConstraintBodyAttachmentCustomization::CreateAndSetFrameDefiningActor()
{
/// \todo Update 'Create New Frame Defining Actor' functionality to be FrameDefiningComponent
/// compatible.
// This is the implementation part of the constraint frame origin creation helper. See comment in
// CustomizeChildren above.
#if 0
	check(BodyAttachmentProperty);

	if (FAGX_PropertyUtilities::GetObjectFromHandle(FrameDefiningActorProperty))
	{
		// Already exists.
		return;
	}

	UAGX_ConstraintComponent* Constraint = Cast<UAGX_ConstraintComponent>(
		FAGX_PropertyUtilities::GetParentObjectOfStruct(BodyAttachmentProperty));

	check(Constraint);

	AActor* ParentActor = [this]() -> AActor* {
		FAGX_ConstraintBodyAttachment* Attachment =
			GetConstraintBodyAttachment(BodyAttachmentProperty);
		if (Attachment == nullptr)
		{
			return nullptr;
		}
		UAGX_RigidBodyComponent* RigidBody = Attachment->GetRigidBody();
		if (RigidBody == nullptr)
		{
			return nullptr;
		}
		return RigidBody->GetOwner();
	}();

	// Create the new Constraint Frame Actor.
	AActor* NewActor = FAGX_EditorUtilities::CreateConstraintFrameActor(
		ParentActor,
		/*Select*/ true,
		/*ShowNotification*/ true,
		/*InPlayingWorldIfAvailable*/ true);

	// Set the new actor to our property.
#if 0
	// This should work, but doesn't! Using worked-around below instead...
	FPropertyAccess::Result Result = FrameDefiningActorProperty->SetValue((UObject*)NewActor);
	check(Result == FPropertyAccess::Success);
#else
	FAGX_ConstraintBodyAttachment* BodyAttachment =
		FAGX_PropertyUtilities::GetStructFromHandle<FAGX_ConstraintBodyAttachment>(
			BodyAttachmentProperty, Constraint);

	BodyAttachment->FrameDefiningActor = NewActor;
	BodyAttachment->OnFrameDefiningActorChanged(Constraint);
#endif
#endif
}

#undef LOCTEXT_NAMESPACE
