// Copyright 2025, Algoryx Simulation AB.

#include "Wire/AGX_WireWinchDetailsRuntime.h"

// Unreal Engine includes.
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "IDetailGroup.h"
#include "DetailLayoutBuilder.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "AGX_WireWinchDetailsRuntime"

FAGX_WireWinchDetailsRuntime::FAGX_WireWinchDetailsRuntime(
	IDetailLayoutBuilder& InDetailBuilder, FWireWinchGetter InWireWinchGetter)
	: WireWinchGetter(InWireWinchGetter)
	, DetailBuilder(InDetailBuilder)
{
	UpdateValues();
}

void FAGX_WireWinchDetailsRuntime::GenerateHeaderRowContent(FDetailWidgetRow& NodeRow)
{
	// By having an empty header row Slate won't generate a collapsable section for the node
	// details.
	/// @todo Maybe we do want one in this case?
}

namespace AGX_WireWinchDetailsRuntime_helpers
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

void FAGX_WireWinchDetailsRuntime::GenerateChildContent(IDetailChildrenBuilder& ChildrenBuilder)
{
	IDetailGroup& Group =
		ChildrenBuilder.AddGroup(TEXT("Wire winch"), LOCTEXT("WireWinch", "Wire Winch"));

	using namespace AGX_WireWinchDetailsRuntime_helpers;
	CreateRuntimeDisplay(Group, LOCTEXT("Speed", "Speed"), [this]() { return Speed; });
	CreateRuntimeDisplay(
		Group, LOCTEXT("PulledInLength", "Pulled In Length"), [this]() { return PulledInLength; });
	CreateRuntimeDisplay(
		Group, LOCTEXT("MotorForce", "Motor Force"), [this]() { return MotorForce; });
	CreateRuntimeDisplay(
		Group, LOCTEXT("BrakeForce", "Brake Force"), [this]() { return BrakeForce; });
}

bool FAGX_WireWinchDetailsRuntime::InitiallyCollapsed() const
{
	return false;
}

void FAGX_WireWinchDetailsRuntime::SetOnRebuildChildren(FSimpleDelegate InOnRegenerateChildren)
{
	OnRegenerateChildren = InOnRegenerateChildren;
}

FName FAGX_WireWinchDetailsRuntime::GetName() const
{
	return TEXT("Wire Winch Runtime");
}

bool FAGX_WireWinchDetailsRuntime::RequiresTick() const
{
	return true;
}

void FAGX_WireWinchDetailsRuntime::Tick(float DeltaTime)
{
	UpdateValues();
}

void FAGX_WireWinchDetailsRuntime::UpdateValues()
{
	static const FText NoObject = LOCTEXT("NoObject", "No Object");
	static const FText MultipleObjects = LOCTEXT("MultipleObject", "Multiple Objects");
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
		SetAll(MultipleObjects);
		return;
	}

	TWeakObjectPtr<UObject> Object = Objects[0];
	if (!Object.IsValid())
	{
		SetAll(NoObject);
		return;
	}

	if (!WireWinchGetter)
	{
		/// @todo What to do in this case?
		return;
	}

	FAGX_WireWinch* Winch = WireWinchGetter(Object.Get());
	if (Winch == nullptr)
	{
		SetAll(NoObject);
		return;
	}

	if (!Winch->HasNative())
	{
		SetAll(NoNative);
		return;
	}

	Winch->ReadPropertiesFromNative();
	Speed =
		FText::Format(LOCTEXT("SpeedValue", "{0} cm/s"), FText::AsNumber(Winch->GetCurrentSpeed()));
	PulledInLength = FText::Format(
		LOCTEXT("PullInLengthValue", "{0} cm"), FText::AsNumber(Winch->GetPulledInLength()));
	MotorForce = FText::Format(
		LOCTEXT("MotorForceValue", "{0} N"), FText::AsNumber(Winch->GetCurrentMotorForce()));
	BrakeForce = FText::Format(
		LOCTEXT("BrakeForceValue", "{0} N"), FText::AsNumber(Winch->GetCurrentBrakeForce()));
}

#undef LOCTEXT_NAMESPACE
