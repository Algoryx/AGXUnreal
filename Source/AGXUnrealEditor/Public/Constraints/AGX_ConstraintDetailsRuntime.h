// Copyright 2023, Algoryx Simulation AB.

#pragma once

class UAGX_Constraint1DofComponent;
class UAGX_Constraint2DofComponent;

// Unreal Engine includes.
#include "IDetailCustomNodeBuilder.h"

// Standard library includes.
#include <utility>

class IDetailLayoutBuilder;

/**
 * Provide a Details panel block showing AGX Dynamics runtime data from a 1 or 2 DOF constraint.
 */
class FAGX_ConstraintDetailsRuntime : public IDetailCustomNodeBuilder
{
public:
	FAGX_ConstraintDetailsRuntime(IDetailLayoutBuilder& InDetailBuilder);

	//~ Begin IDetailCustomNodeBuilder.
	virtual void GenerateHeaderRowContent(FDetailWidgetRow& NodeRow) override;
	virtual void GenerateChildContent(IDetailChildrenBuilder& ChildrenBuilder) override;
	virtual bool InitiallyCollapsed() const override;
	virtual void SetOnRebuildChildren(FSimpleDelegate InOnRegenerateChildren) override;
	virtual FName GetName() const override;
	virtual bool RequiresTick() const override;
	virtual void Tick(float DeltaTime) override;
	//~ End IDetailCustomNodeBuilder.

	void UpdateValues();

	/// Store text representations of the most recent values read from AGX Dynamics.
	struct FConstraintRuntimeState
	{
		FText Angle;

		/// Set all text fields to the same text. Used for error messages.
		void SetAll(const FText& Text)
		{
			Angle = Text;
		}
	} ConstraintState;

	/// Set all text fields to the same text. Used for error messages.
	void SetAll(const FText& Text)
	{
		ConstraintState.SetAll(Text);
	}

	using FConstraintTypes =
		std::pair<UAGX_Constraint1DofComponent*, UAGX_Constraint2DofComponent*>;

private:
	FConstraintTypes GetConstraint();

public:
	IDetailLayoutBuilder& DetailBuilder;

	// Call ExecuteIfBound on this delegate to trigger a complete Slate regeneration of this
	// part of the Details panel.
	FSimpleDelegate OnRegenerateChildren;
};
