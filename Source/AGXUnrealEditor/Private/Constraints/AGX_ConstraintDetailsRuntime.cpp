// Copyright 2023, Algoryx Simulation AB.

#include "Constraints/AGX_ConstraintDetailsRuntime.h"

// AGX Dynamics for Unreal includes.
#include "Constraints/AGX_Constraint1DofComponent.h"
#include "Constraints/AGX_Constraint2DofComponent.h"
#include "Constraints/AGX_Constraint2DOFFreeDOF.h"
#include "Constraints/AGX_ConstraintActor.h"
#include "Constraints/AGX_ConstraintComponent.h"

// Unreal Engine includes.
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "IDetailGroup.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "AGX_ConstraintDetailsRuntime"

FAGX_ConstraintDetailsRuntime::FAGX_ConstraintDetailsRuntime(IDetailLayoutBuilder& InDetailBuilder)
	: DetailBuilder(InDetailBuilder)
{
	UpdateValues();
}

void FAGX_ConstraintDetailsRuntime::GenerateHeaderRowContent(FDetailWidgetRow& NodeRow)
{
	// By having an empty header row Slate won't generate a collapsable section for the runtime
	// data. Do we want a collapsable section?
}

namespace AGX_ConstraintDetailsRuntime_helpers
{
	/**
	 * Populate a single row in the Details panel, displaying one AGX Dynamics value.
	 * @param Row The row to populate.
	 * @param Name Text to display in the name column.
	 * @param OnGet Lambda that should return an FText representation of the current value.
	 */
	template <class FOnGet>
	void CreateRuntimeDisplay(FDetailWidgetRow& Row, const FText& Name, FOnGet OnGet)
	{
		// clang-format off
		Row
		.NameContent()
		[
			SNew(STextBlock)
			.Text(Name)
		]
		.ValueContent()
		[
			SNew(STextBlock)
			.Text_Lambda(OnGet)
		];
		// clang-format on
	}

	/**
	 * Create a single row in the Details panel, displaying one AGX Dynamics value.
	 * @param Group Group to create a row in.
	 * @param Name Text to display in the name column.
	 * @param OnGet Lambda that should return an FText representation of the current value.
	 */
	template <class FOnGet>
	void CreateRuntimeDisplay(IDetailGroup& Group, const FText& Name, FOnGet OnGet)
	{
		CreateRuntimeDisplay(Group.AddWidgetRow(), Name, OnGet);
	}

	template <class FOnGet>
	/**
	 * Create a single row in the Details panel, displaying one AGX Dynamics value.
	 * @param ChildrenBuilder Details panel node to create the row in.
	 * @param Name Text to display in the name column.
	 * @param OnGet Lambda that should return an FText representation of the current value.
	 */
	void CreateRuntimeDisplay(
		IDetailChildrenBuilder& ChildrenBuilder, const FText& Name, FOnGet OnGet)
	{
		CreateRuntimeDisplay(
			ChildrenBuilder.AddCustomRow(LOCTEXT("AGXRuntime", "AGX Runtime")), Name, OnGet);
	}
}

void FAGX_ConstraintDetailsRuntime::GenerateChildContent(IDetailChildrenBuilder& ChildrenBuilder)
{
	using namespace AGX_ConstraintDetailsRuntime_helpers;

	{
		CreateRuntimeDisplay(
			ChildrenBuilder, LOCTEXT("Angle", "Angle"), [this]() { return ConstraintState.Angle; });
	}
}

bool FAGX_ConstraintDetailsRuntime::InitiallyCollapsed() const
{
	return false;
}

void FAGX_ConstraintDetailsRuntime::SetOnRebuildChildren(FSimpleDelegate InOnRegenerateChildren)
{
	OnRegenerateChildren = InOnRegenerateChildren;
}

FName FAGX_ConstraintDetailsRuntime::GetName() const
{
	return TEXT("Constraint Runtime");
}

bool FAGX_ConstraintDetailsRuntime::RequiresTick() const
{
	return true;
}

void FAGX_ConstraintDetailsRuntime::Tick(float DeltaTime)
{
	UpdateValues();
}

FAGX_ConstraintDetailsRuntime::FConstraintTypes MakeConstraintTypes(
	UAGX_Constraint1DofComponent* Constraint)
{
	return {Constraint, nullptr};
}

FAGX_ConstraintDetailsRuntime::FConstraintTypes MakeConstraintTypes(
	UAGX_Constraint2DofComponent* Constraint)
{
	return {nullptr, Constraint};
}

FAGX_ConstraintDetailsRuntime::FConstraintTypes FAGX_ConstraintDetailsRuntime::GetConstraint()
{
	static const FText NoObject = LOCTEXT("NoObject", "No Object");
	static const FText MultipleObjects = LOCTEXT("MultipleOjects", "Multiple Objects");
	static const FText NotConstraint = LOCTEXT("NotConstraint", "Not a 1 or 2 DOF constraint");
	static const FText NoNative = LOCTEXT("NoNative", "No Native");

	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailBuilder.GetObjectsBeingCustomized(Objects);

	// Make sure only a single object is selected. It is highly unlikely that multiple constraints
	// will have the same state, and we don't want to add multi-value displays just yet.
	if (Objects.Num() < 1)
	{
		SetAll(NoObject);
		return FConstraintTypes();
	}
	if (Objects.Num() > 1)
	{
		SetAll(MultipleObjects);
		return FConstraintTypes();
	}

	// Get the Constraint Component, regardless of if the selected object itself is the Component,
	// or if the selected object is a Constraint Actor that has a Constraint Component.
	TWeakObjectPtr<UObject> Object = Objects[0];
	if (!Object.IsValid())
	{
		SetAll(NoObject);
		return FConstraintTypes();
	}
	UAGX_ConstraintComponent* Constraint = Cast<UAGX_ConstraintComponent>(Object.Get());
	if (Constraint == nullptr)
	{
		AAGX_ConstraintActor* Actor = Cast<AAGX_ConstraintActor>(Object.Get());
		if (Actor != nullptr)
		{
			Constraint = Actor->GetConstraintComponent();
		}
		if (Constraint == nullptr)
		{
			SetAll(NotConstraint);
			return FConstraintTypes();
		}
	}

	if (!Constraint->HasNative())
	{
		SetAll(NoNative);
		return FConstraintTypes();
	}

	UAGX_Constraint1DofComponent* Constraint1Dof = Cast<UAGX_Constraint1DofComponent>(Constraint);
	if (Constraint1Dof != nullptr)
	{
		return MakeConstraintTypes(Constraint1Dof);
	}

	UAGX_Constraint2DofComponent* Constraint2Dof = Cast<UAGX_Constraint2DofComponent>(Constraint);
	if (Constraint2Dof != nullptr)
	{
		return MakeConstraintTypes(Constraint2Dof);
	}

	SetAll(NotConstraint);
	return FConstraintTypes();
}

void FAGX_ConstraintDetailsRuntime::UpdateValues()
{
	FConstraintTypes Constraint = GetConstraint();

	if (const UAGX_Constraint1DofComponent* Constraint1Dof = Constraint.first)
	{
		static const FText LocUnit = LOCTEXT("LocUnit", "cm");
		static const FText RotUnit = LOCTEXT("RotUnit", "deg");
		const FText& AngleUnit = Constraint1Dof->IsRotational() ? RotUnit : LocUnit;
		ConstraintState.Angle = FText::Format(
			LOCTEXT("AngleValue", "{0} {1}"), FText::AsNumber(Constraint1Dof->GetAngle()),
			AngleUnit);
	}
	if (const UAGX_Constraint2DofComponent* Constraint2Dof = Constraint.second)
	{
		// This assumes that the first degree of freedom is translational and the second is
		// rotational. This assumption is based on the implementation of
		// agx::addSecondaryConstraints2DOF, which adds secondary constraints for the translational
		// degree of freedom before the rotational degrees of freedom.
		ConstraintState.Angle = FText::Format(
			LOCTEXT("angleValue2dof", "{0} cm, {1} deg"),
			FText::AsNumber(Constraint2Dof->GetAngle(EAGX_Constraint2DOFFreeDOF::FIRST)),
			FText::AsNumber(Constraint2Dof->GetAngle(EAGX_Constraint2DOFFreeDOF::SECOND)));
	}
}

#undef LOCTEXT_NAMESPACE
