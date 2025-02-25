// Copyright 2024, Algoryx Simulation AB.

#include "Constraints/AGX_ConstraintCustomizationRuntime.h"

// AGX Dynamics for Unreal includes.
#include "Constraints/AGX_ConstraintComponent.h"
#include "Constraints/AGX_Constraint1DofComponent.h"
#include "Constraints/AGX_Constraint2DofComponent.h"

// Unreal Engine includes.
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "IDetailGroup.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "AGX_ConstraintCustomizationRuntime"

FAGX_ConstraintCustomizationRuntime::FAGX_ConstraintCustomizationRuntime(
	IDetailLayoutBuilder& InDetailBuilder)
	: DetailBuilder(InDetailBuilder)
{
	UpdateValues();
}

void FAGX_ConstraintCustomizationRuntime::GenerateHeaderRowContent(FDetailWidgetRow& NodeRow)
{
	// By having an empty header row Slate won't generate a collapsable section for the node
	// details.
	/// @todo Maybe we do want one in this case?
}

namespace FAGX_ConstraintCustomizationRuntime_helpers
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

void FAGX_ConstraintCustomizationRuntime::GenerateChildContent(
	IDetailChildrenBuilder& ChildrenBuilder)
{
	using namespace FAGX_ConstraintCustomizationRuntime_helpers;

	CreateRuntimeDisplay(
		ChildrenBuilder, LOCTEXT("HasNative", "Has Native"), [this]() { return HasNative; });
	CreateRuntimeDisplay(
		ChildrenBuilder, LOCTEXT("CurrentForce", "Current Force"),
		[this]() { return CurrentForce; });

	IDetailGroup& FirstDofGroup = ChildrenBuilder.AddGroup(
		TEXT("FirstDegreeOfFreedom"),
		LOCTEXT("First Degree Of Freedom", "First Degree Of Freedom"));
	CreateRuntimeDisplay(
		FirstDofGroup, LOCTEXT("MotorController", "Motor Controller"),
		[this]() { return FirstDof.MotorControllerForce; });
	CreateRuntimeDisplay(
		FirstDofGroup, LOCTEXT("LockController", "Lock Controller"),
		[this]() { return FirstDof.LockControllerForce; });
}

bool FAGX_ConstraintCustomizationRuntime::InitiallyCollapsed() const
{
	return false;
}

void FAGX_ConstraintCustomizationRuntime::SetOnRebuildChildren(
	FSimpleDelegate InOnRegenerateChildren)
{
	OnRegenerateChildren = InOnRegenerateChildren;
}

FName FAGX_ConstraintCustomizationRuntime::GetName() const
{
	return TEXT("Runtime");
}

bool FAGX_ConstraintCustomizationRuntime::RequiresTick() const
{
	return true;
}

void FAGX_ConstraintCustomizationRuntime::Tick(float DeltaTime)
{
	UpdateValues();
}

void FAGX_ConstraintCustomizationRuntime::UpdateValues()
{
	static const FText NoObject = LOCTEXT("NoObject", "No Object");
	static const FText MultipleObject = LOCTEXT("MultipleObjects", "Multiple Objects");
	static const FText NoNative = LOCTEXT("NoNative", "No Native");

	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailBuilder.GetObjectsBeingCustomized(Objects);
	if (Objects.Num() < 1)
	{
		HasNative = NoObject;
		return;
	}
	if (Objects.Num() > 1)
	{
		HasNative = MultipleObject;
		return;
	}

	TWeakObjectPtr<UObject> Object = Objects[0];
	if (!Object.IsValid())
	{
		HasNative = NoObject;
		return;
	}

	UAGX_ConstraintComponent* Constraint = Cast<UAGX_ConstraintComponent>(Object.Get());
	if (Constraint == nullptr)
	{
		// Not really true. There is an object, it just isn't a Rigid Body.
		HasNative = NoObject;
		return;
	}

	if (!Constraint->HasNative())
	{
		HasNative = NoNative;
		return;
	}

	HasNative = Constraint->HasNative() ? LOCTEXT("True", "True") : LOCTEXT("False", "False");
	CurrentForce = FText::Format(
		LOCTEXT("ExternalForceValue", "{0} N"),
		FText::AsNumber(Constraint->GetCurrentForce(EGenericDofIndex::Translational1)));

	if (UAGX_Constraint1DofComponent* OneDof = Cast<UAGX_Constraint1DofComponent>(Constraint))
	{
		FirstDof.MotorControllerForce = FText::AsNumber(OneDof->TargetSpeedController.GetForce());
		FirstDof.LockControllerForce = FText::AsNumber(OneDof->LockController.GetForce());
	}
	if (UAGX_Constraint2DofComponent* TwoDof = Cast<UAGX_Constraint2DofComponent>(Constraint))
	{
		FirstDof.MotorControllerForce = FText::AsNumber(TwoDof->TargetSpeedController1.GetForce());
		SecondDof.MotorControllerForce = FText::AsNumber(TwoDof->TargetSpeedController2.GetForce());
	}
}

#undef LOCTEXT_NAMESPACE
