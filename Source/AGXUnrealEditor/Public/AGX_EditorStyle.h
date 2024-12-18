// Copyright 2024, Algoryx Simulation AB.

#pragma once

#include "CoreMinimal.h"

/**
 * Style that can be used by Editor-only AGX objects.
 */
class AGXUNREALEDITOR_API FAGX_EditorStyle
{
public: // Names of common resources
	static const FName AgxIcon;
	static const FName AgxIconSmall;
	static const FName AgxIconTiny;
	static const FName FileIcon;
	static const FName FileIconSmall;
	static const FName GrabIcon;
	static const FName JointIcon;
	static const FName JointIconSmall;
	static const FName LicenseKeyIcon;
	static const FName TerrainIcon;
	static const FName TerrainIconSmall;

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
