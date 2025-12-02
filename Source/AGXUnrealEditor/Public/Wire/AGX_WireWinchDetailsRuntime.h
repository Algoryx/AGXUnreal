// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Wire/AGX_WireWinch.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "IDetailCustomNodeBuilder.h"
#include "UObject/WeakObjectPtrTemplates.h"

class IDetailLayoutBuilder;

/**
 * A Wire Winch can be owned by many different types of objects, and any one such object may contain
 * multiple Wire Winches. When a Details Panel is created for a particular type a callback of this
 * type is passed to the Wire Winch Details Runtime and the callback knows how to get the right Wire
 * Winch from the currently customized UObject.
 */
using FWireWinchGetter = TFunction<FAGX_WireWinch*(UObject*)>;

class FAGX_WireWinchDetailsRuntime : public IDetailCustomNodeBuilder
{
public:
	FAGX_WireWinchDetailsRuntime(
		IDetailLayoutBuilder& InDetailBuilder, FWireWinchGetter InWireWinchGetter);

	//~ Begin IDetailCustomNodeBuilder interface
	virtual void GenerateHeaderRowContent(FDetailWidgetRow& NodeRow) override;
	virtual void GenerateChildContent(IDetailChildrenBuilder& ChildrenBuilder) override;
	virtual bool InitiallyCollapsed() const override;
	virtual void SetOnRebuildChildren(FSimpleDelegate InOnRegenerateChildren) override;
	virtual FName GetName() const override;
	virtual bool RequiresTick() const override;
	virtual void Tick(float DeltaTime) override;
	//~ End IDetailCustomNodeBuilder interface

public:
	void UpdateValues();

	FText Speed;
	FText PulledInLength;
	FText MotorForce;
	FText BrakeForce;

	void SetAll(const FText& Text)
	{
		Speed = Text;
		PulledInLength = Text;
		MotorForce = Text;
		BrakeForce = Text;
	}

public:
	FWireWinchGetter WireWinchGetter;
	IDetailLayoutBuilder& DetailBuilder;
	FSimpleDelegate OnRegenerateChildren;
	TWeakObjectPtr<FAGX_WireWinch> CurrentWinch;
};
