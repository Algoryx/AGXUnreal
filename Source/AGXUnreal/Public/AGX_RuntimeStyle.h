// Copyright 2025, Algoryx Simulation AB.

#pragma once

#include "CoreMinimal.h"

/**
 * Style that can be used by both Runtime and Editor AGX objects.
 */
class AGXUNREAL_API FAGX_RuntimeStyle
{
public:
	static void Initialize();
	static void Shutdown();
	static void ReloadTextures();
	static TSharedPtr<class ISlateStyle> Get();
	static FName GetStyleSetName();

private:
	static TSharedRef<class FSlateStyleSet> Create();

private:
	static TSharedPtr<class FSlateStyleSet> StyleInstance;
};
