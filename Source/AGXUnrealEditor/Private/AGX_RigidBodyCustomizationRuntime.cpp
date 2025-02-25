// Copyright 2024, Algoryx Simulation AB.

#include "AGX_RigidBodyCustomizationRuntime.h"

// AGX Dynamics for Unreal includes.
#include "AGX_RigidBodyComponent.h"

// Unreal Engine includes.
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "IDetailGroup.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "AGX_RigidBodyCustomizationRuntime"

FAGX_RigidBodyCustomizationRuntime::FAGX_RigidBodyCustomizationRuntime(
	IDetailLayoutBuilder& InDetailBuilder)
	: DetailBuilder(InDetailBuilder)
{
	UpdateValues();
}

void FAGX_RigidBodyCustomizationRuntime::GenerateHeaderRowContent(FDetailWidgetRow& NodeRow)
{
	// By having an empty header row Slate won't generate a collapsable section for the node
	// details.
	/// @todo Maybe we do want one in this case?
}

namespace FAGX_RigidBodyCustomizationRuntime_helpers
{
	template <class FOnGet>
	void CreateRuntimeDisplay(IDetailGroup& Group, const FText& Name, FOnGet OnGet)
	{
		// clang-format off
		Group.AddWidgetRow()
		.NameContent()
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.Text(Name)
		]
		.ValueContent()
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.Text_Lambda(OnGet)
		];
		// clang-format on
	}

	template <typename FOnGet>
	void CreateRuntimeDisplay(IDetailChildrenBuilder& Builder, const FText& Name, FOnGet OnGet)
	{
		// clang-format off
		Builder.AddCustomRow(Name)
		.NameContent()
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.Text(Name)
		]
		.ValueContent()
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.Text_Lambda(OnGet)
		];
		// clang-format on
	}
}

void FAGX_RigidBodyCustomizationRuntime::GenerateChildContent(
	IDetailChildrenBuilder& ChildrenBuilder)
{
	using namespace FAGX_RigidBodyCustomizationRuntime_helpers;

	// IDetailGroup& Group = ChildrenBuilder.AddGroup(TEXT("Runtime"), LOCTEXT("Runtime",
	// "Runtime"));
	CreateRuntimeDisplay(
		ChildrenBuilder, LOCTEXT("HasNative", "Has Native"), [this]() { return HasNative; });

// External force is problematic since it is cleared at the end of the step. So how do we read it?
// Pre Step is too early because gravity hasn't been added yet. Post step is tool late because that
// is after the Clear Forces kernel.
#if 0
	CreateRuntimeDisplay(
		ChildrenBuilder, LOCTEXT("ExternalForce", "External Force"),
		[this]() { return ExternalForce; });
#endif
}

bool FAGX_RigidBodyCustomizationRuntime::InitiallyCollapsed() const
{
	return false;
}

void FAGX_RigidBodyCustomizationRuntime::SetOnRebuildChildren(
	FSimpleDelegate InOnRegenerateChildren)
{
	OnRegenerateChildren = InOnRegenerateChildren;
}

FName FAGX_RigidBodyCustomizationRuntime::GetName() const
{
	return TEXT("Runtime");
}

bool FAGX_RigidBodyCustomizationRuntime::RequiresTick() const
{
	return true;
}

void FAGX_RigidBodyCustomizationRuntime::Tick(float DeltaTime)
{
	UpdateValues();
}

void FAGX_RigidBodyCustomizationRuntime::UpdateValues()
{
	static const FText NoObject = LOCTEXT("NoObject", "No Object");
	static const FText MultipleObject = LOCTEXT("MultipleObjects", "Multiple Objects");
	static const FText NoNative = LOCTEXT("NoNative", "No Native");

	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailBuilder.GetObjectsBeingCustomized(Objects);
	if (Objects.Num() < 1)
	{
		SetAll(NoObject);
		return;
	}
	if (Objects.Num() > 1)
	{
		SetAll(MultipleObject);
		return;
	}

	TWeakObjectPtr<UObject> Object = Objects[0];
	if (!Object.IsValid())
	{
		SetAll(NoObject);
		return;
	}

	UAGX_RigidBodyComponent* Body = Cast<UAGX_RigidBodyComponent>(Object.Get());
	if (Body == nullptr)
	{
		// Not really true. There is an object, it just isn't a Rigid Body.
		SetAll(NoObject);
		return;
	}

	if (!Body->HasNative())
	{
		SetAll(NoNative);
		return;
	}

	HasNative = Body->HasNative() ? LOCTEXT("True", "True") : LOCTEXT("False", "False");
	ExternalForce = FText::Format(
		LOCTEXT("ExternalForceValue", "{0} N"), FText::FromString(Body->GetForce().ToString()));
}

#undef LOCTEXT_NAMESPACE
