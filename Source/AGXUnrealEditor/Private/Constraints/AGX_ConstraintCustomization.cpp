// Copyright 2025, Algoryx Simulation AB.

#include "Constraints/AGX_ConstraintCustomization.h"

// AGX Dynamics for Unreal includes.
#include "Constraints/AGX_ConstraintComponent.h"
#include "Constraints/AGX_ConstraintCustomizationRuntime.h"
#include "Utilities/AGX_EditorUtilities.h"

// Unreal Engine includes.
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "FAGX_ConstraintCustomization"

TSharedRef<IDetailCustomization> FAGX_ConstraintCustomization::MakeInstance()
{
	return MakeShareable(new FAGX_ConstraintCustomization);
}

void FAGX_ConstraintCustomization::CustomizeDetails(IDetailLayoutBuilder& InDetailBuilder)
{
	DetailBuilder = &InDetailBuilder;

	// Fix category ordering by priority by the order they are often edited.
	IDetailCategoryBuilder& BodiesCategory = DetailBuilder->EditCategory(
		"AGX Constraint Bodies", FText::GetEmpty(), ECategoryPriority::Important);
	DetailBuilder->EditCategory("AGX Constraint", FText::GetEmpty(), ECategoryPriority::Important);
	DetailBuilder->EditCategory(
		"AGX Secondary Constraint", FText::GetEmpty(), ECategoryPriority::Important);
	DetailBuilder->EditCategory(
		"AGX Secondary Constraints", FText::GetEmpty(), ECategoryPriority::Important);

	// clang-format off
	BodiesCategory.AddCustomRow(LOCTEXT("Error","Error"))
	[
		// Switch to SListView and populate a list of FTexts if need more performance.
		SNew(STextBlock)
		.Text(this, &FAGX_ConstraintCustomization::GetBodySetupErrorText)
		.ToolTipText(this, &FAGX_ConstraintCustomization::GetBodySetupErrorText)
		.Font(IDetailLayoutBuilder::GetDetailFont())
		.ColorAndOpacity(FLinearColor::Red)
	]
	.Visibility(TAttribute<EVisibility>(
		this, &FAGX_ConstraintCustomization::VisibleWhenBodySetupError));
	// clang-format on

	IDetailCategoryBuilder& Runtime =
		DetailBuilder->EditCategory(TEXT("AGX Runtime"), LOCTEXT("AGXRuntime", "AGX Runtime"));
	Runtime.AddCustomBuilder(
		MakeShareable(new FAGX_ConstraintCustomizationRuntime(*DetailBuilder)));
}

namespace AGX_ConstraintCustomization_helpers
{
	enum class EAGX_AttachmentSetupError : int
	{
		NoError = 0,
		NoFirstBody = 1 << 0,
		SameBody = 1 << 1
	};

	EAGX_AttachmentSetupError& operator|=(
		EAGX_AttachmentSetupError& InOutLhs, EAGX_AttachmentSetupError InRhs)
	{
		int Lhs = (int) InOutLhs;
		int Rhs = (int) InRhs;
		int result = Lhs | Rhs;
		InOutLhs = (EAGX_AttachmentSetupError) result;
		return InOutLhs;
	}

	EAGX_AttachmentSetupError operator&(
		EAGX_AttachmentSetupError InLhs, EAGX_AttachmentSetupError InRhs)
	{
		int Lhs = (int) InLhs;
		int Rhs = (int) InRhs;
		int Result = Lhs & Rhs;
		return (EAGX_AttachmentSetupError) Result;
	}

	bool HasError(EAGX_AttachmentSetupError Error, EAGX_AttachmentSetupError Flag)
	{
		return (Error & Flag) != EAGX_AttachmentSetupError::NoError;
	}

	EAGX_AttachmentSetupError GetBodySetupError(const IDetailLayoutBuilder& DetailBuilder)
	{
		EAGX_AttachmentSetupError Error {EAGX_AttachmentSetupError::NoError};

		TArray<TWeakObjectPtr<UObject>> Objects;
		DetailBuilder.GetObjectsBeingCustomized(Objects);
		for (auto& Object : Objects)
		{
			UAGX_ConstraintComponent* Constraint = Cast<UAGX_ConstraintComponent>(Object.Get());
			if (Constraint == nullptr)
				continue;

			const FAGX_RigidBodyReference& Attachment1 = Constraint->BodyAttachment1.RigidBody;
			const FName Name1 = Attachment1.Name;
			const AActor* Scope1 = Attachment1.GetScope();

			const FAGX_RigidBodyReference& Attachment2 = Constraint->BodyAttachment2.RigidBody;
			const FName Name2 = Attachment2.Name;
			const AActor* Scope2 = Attachment2.GetScope();

			if (Name1.IsNone())
			{
				Error |= EAGX_AttachmentSetupError::NoFirstBody;
			}
			else if (Name1 == Name2 && Scope1 == Scope2)
			{
				Error |= EAGX_AttachmentSetupError::SameBody;
			}
		}

		return Error;
	}
}

EVisibility FAGX_ConstraintCustomization::VisibleWhenBodySetupError() const
{
	using namespace AGX_ConstraintCustomization_helpers;
	if (DetailBuilder == nullptr)
		return EVisibility::Collapsed;
	EAGX_AttachmentSetupError Error = GetBodySetupError(*DetailBuilder);
	return FAGX_EditorUtilities::VisibleIf(Error != EAGX_AttachmentSetupError::NoError);
}

FText FAGX_ConstraintCustomization::GetBodySetupErrorText() const
{
	using namespace AGX_ConstraintCustomization_helpers;
	if (DetailBuilder == nullptr)
		return FText();

	EAGX_AttachmentSetupError Error = GetBodySetupError(*DetailBuilder);
	FString Message;
	auto Append = [&Message](const FText& Text)
	{
		if (!Message.IsEmpty())
			Message += TEXT("\n");
		Message += Text.ToString();
	};

	if (HasError(Error, EAGX_AttachmentSetupError::NoFirstBody))
	{
		static FText Text = LOCTEXT("NoFirstBody", "A Constraint must have a first Rigid Body .");
		Append(Text);
	}
	if (HasError(Error, EAGX_AttachmentSetupError::SameBody))
	{
		static FText Text = LOCTEXT(
			"BothBodiesSame", "Both Body Attachments may not reference the same Rigid Body");
		Append(Text);
	}

	return FText::FromString(Message);
}

#undef LOCTEXT_NAMESPACE
