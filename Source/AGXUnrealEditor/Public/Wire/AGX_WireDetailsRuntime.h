// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "IDetailCustomNodeBuilder.h"

class IDetailLayoutBuilder;

class FAGX_WireDetailsRuntime : public IDetailCustomNodeBuilder
{
public:
	FAGX_WireDetailsRuntime(IDetailLayoutBuilder& InDetailBuilder);

	//~ Begin IDetailCustomNodeBuilder.
	virtual void GenerateHeaderRowContent(FDetailWidgetRow& NodeRow) override;
	virtual void GenerateChildContent(IDetailChildrenBuilder& ChildrenBuilder) override;
	virtual bool InitiallyCollapsed() const override;
	virtual void SetOnRebuildChildren(FSimpleDelegate InOnRegenerateChildren) override;
	virtual FName GetName() const override;
	virtual bool RequiresTick() const override;
	virtual void Tick(float DeltaTime) override;
	//~ End IDetailCustomNodeBuilder.

public:
	void UpdateValues();

public:
	struct FWireRuntimeState
	{
		FText RestLength;
		FText Mass;
		FText Tension;

		void SetAll(const FText& Text)
		{
			RestLength = Text;
			Mass = Text;
			Tension = Text;
		}
	} WireState;

	struct FWinchRuntimeState
	{
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
	} BeginWinchState, EndWinchState;

	void SetAll(const FText& Text)
	{
		WireState.SetAll(Text);
		BeginWinchState.SetAll(Text);
		EndWinchState.SetAll(Text);
	}

public:
	IDetailLayoutBuilder& DetailBuilder;
	FSimpleDelegate OnRegenerateChildren;
};
