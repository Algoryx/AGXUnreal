// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "Misc/AutomationTest.h"

/** Changes only the loaded test world; fixtures and source assets are never saved. */
class FAGX_PrepareLandscapeScenario final : public IAutomationLatentCommand
{
public:
	FAGX_PrepareLandscapeScenario(
		FAutomationTestBase& InTest, bool InCoarse = false, bool InNormals = false,
		bool InLargeRadius = false, int32 InCollisionMip = 0)
		: Test(InTest)
		, bCoarse(InCoarse)
		, bNormals(InNormals)
		, bLargeRadius(InLargeRadius)
		, CollisionMip(InCollisionMip)
	{
	}
	virtual bool Update() override;

private:
	FAutomationTestBase& Test;
	bool bCoarse;
	bool bNormals;
	bool bLargeRadius;
	int32 CollisionMip;
};
