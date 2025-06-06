// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "IDetailCustomNodeBuilder.h"

class IDetailLayoutBuilder;

/**
 * A group in the Details panel for Constraint that displays data that isn't an Unreal Engine
 * property, things that are internal to the simulation but still interesting for the user.
 */
class FAGX_ConstraintCustomizationRuntime : public IDetailCustomNodeBuilder
{
public:
	FAGX_ConstraintCustomizationRuntime(IDetailLayoutBuilder& InDetailBuilder);

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
	FText HasNative;

	FText CurrentForce;

	struct FFirstDof
	{
		FText MotorControllerForce;
		FText LockControllerForce;
	} FirstDof;

	struct FSecondDof
	{
		FText MotorControllerForce;
		FText LockControllerForce;
	} SecondDof;

public:
	IDetailLayoutBuilder& DetailBuilder;
	FSimpleDelegate OnRegenerateChildren;
};
