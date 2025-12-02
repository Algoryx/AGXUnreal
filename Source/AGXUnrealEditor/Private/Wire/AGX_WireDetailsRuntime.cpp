// Copyright 2025, Algoryx Simulation AB.

#include "Wire/AGX_WireDetailsRuntime.h"

// AGX Dynamics for Unreal includes.
#include "Wire/AGX_WireComponent.h"

// Unreal Engine includes.
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "IDetailGroup.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "AGX_WireDetailsRuntime"

FAGX_WireDetailsRuntime::FAGX_WireDetailsRuntime(IDetailLayoutBuilder& InDetailBuilder)
	: DetailBuilder(InDetailBuilder)
{
	UpdateValues();
}

void FAGX_WireDetailsRuntime::GenerateHeaderRowContent(FDetailWidgetRow& NodeRow)
{
	// By having an empty header row Slate won't generate a collapsable section for the node
	// details.
	/// @todo Maybe we do want one in this case?
}

namespace AGX_WireDetailsRuntime_helpers
{
	template <class FOnGet>
	void CreateRuntimeDisplay(IDetailGroup& Group, const FText& Name, FOnGet OnGet)
	{
		// clang-format off
		Group.AddWidgetRow()
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
}

void FAGX_WireDetailsRuntime::GenerateChildContent(IDetailChildrenBuilder& ChildrenBuilder)
{
	using namespace AGX_WireDetailsRuntime_helpers;

	{
		IDetailGroup& Group = ChildrenBuilder.AddGroup(TEXT("Wire"), LOCTEXT("Wire", "Wire"));
		CreateRuntimeDisplay(
			Group, LOCTEXT("RestLength", "RestLength"), [this]() { return WireState.RestLength; });
		CreateRuntimeDisplay(Group, LOCTEXT("Mass", "Mass"), [this]() { return WireState.Mass; });
		CreateRuntimeDisplay(
			Group, LOCTEXT("Tension", "Tension"), [this]() { return WireState.Tension; });
	}

	{
		IDetailGroup& Group =
			ChildrenBuilder.AddGroup(TEXT("Begin Winch"), LOCTEXT("BeginWinch", "Begin Winch"));
		CreateRuntimeDisplay(
			Group, LOCTEXT("CurrentSpeed", "Current Speed"),
			[this]() { return BeginWinchState.Speed; });
		CreateRuntimeDisplay(
			Group, LOCTEXT("PulledInLength", "Pulled in Length"),
			[this]() { return BeginWinchState.PulledInLength; });
		CreateRuntimeDisplay(
			Group, LOCTEXT("MotorForce", "Motor Force"),
			[this]() { return BeginWinchState.MotorForce; });
		CreateRuntimeDisplay(
			Group, LOCTEXT("BrakeForce", "Brake Force"),
			[this] { return BeginWinchState.BrakeForce; });
	}

	{
		IDetailGroup& Group =
			ChildrenBuilder.AddGroup(TEXT("End Winch"), LOCTEXT("EndWinch", "End Winch"));
		CreateRuntimeDisplay(
			Group, LOCTEXT("CurrentSpeed", "Current Speed"),
			[this]() { return EndWinchState.Speed; });
		CreateRuntimeDisplay(
			Group, LOCTEXT("PulledInLength", "Pulled in Length"),
			[this]() { return EndWinchState.PulledInLength; });
		CreateRuntimeDisplay(
			Group, LOCTEXT("MotorForce", "Motor Force"),
			[this]() { return EndWinchState.MotorForce; });
		CreateRuntimeDisplay(
			Group, LOCTEXT("BrakeForce", "Brake Force"),
			[this] { return EndWinchState.BrakeForce; });
	}
}

bool FAGX_WireDetailsRuntime::InitiallyCollapsed() const
{
	return false;
}

void FAGX_WireDetailsRuntime::SetOnRebuildChildren(FSimpleDelegate InOnRegenerateChildren)
{
	OnRegenerateChildren = InOnRegenerateChildren;
}

FName FAGX_WireDetailsRuntime::GetName() const
{
	return TEXT("Wire Runtime");
}

bool FAGX_WireDetailsRuntime::RequiresTick() const
{
	return true;
}

void FAGX_WireDetailsRuntime::Tick(float DeltaTime)
{
	UpdateValues();
}

void FAGX_WireDetailsRuntime::UpdateValues()
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

	UAGX_WireComponent* Wire = Cast<UAGX_WireComponent>(Object.Get());
	if (Wire == nullptr)
	{
		// Not really true. There is an object, it just isn't a wire.
		SetAll(NoObject);
		return;
	}

	if (!Wire->HasNative())
	{
		SetAll(NoNative);
		return;
	}

	WireState.RestLength =
		FText::Format(LOCTEXT("RestLengthValue", "{0} cm"), FText::AsNumber(Wire->GetRestLength()));
	WireState.Mass =
		FText::Format(LOCTEXT("MassValue", "{0} kg"), FText::AsNumber(Wire->GetMass()));
	WireState.Tension =
		FText::Format(LOCTEXT("TensionValue", "{0} N"), FText::AsNumber(Wire->GetTension()));

	auto UpdateWinchState = [](const FAGX_WireWinch* const Winch, FWinchRuntimeState& State)
	{
		if (Winch == nullptr)
		{
			State.SetAll(NoObject);
			return;
		}
		if (!Winch->HasNative())
		{
			State.SetAll(NoNative);
			return;
		}

		State.Speed = FText::Format(
			LOCTEXT("SpeedValue", "{0} cm/s"), FText::AsNumber(Winch->GetCurrentSpeed()));
		State.PulledInLength = FText::Format(
			LOCTEXT("PulledInLengthValue", "{0} cm"), FText::AsNumber(Winch->GetPulledInLength()));
		State.MotorForce = FText::Format(
			LOCTEXT("MotorForceValue", "{0} N"), FText::AsNumber(Winch->GetCurrentMotorForce()));
		State.BrakeForce = FText::Format(
			LOCTEXT("BrakeForceValue", "{0} N"), FText::AsNumber(Winch->GetCurrentBrakeForce()));
	};

	UpdateWinchState(Wire->GetBeginWinch(), BeginWinchState);
	UpdateWinchState(Wire->GetEndWinch(), EndWinchState);
}

#undef LOCTEXT_NAMESPACE
