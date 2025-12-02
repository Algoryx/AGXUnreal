// Copyright 2025, Algoryx Simulation AB.

#include "AgxEdMode/AGX_AgxEdModeTerrain.h"

// AGX Dynamics for Unreal includes.
#include "AGX_EditorStyle.h"

// Unreal Engine includes.
#include "Textures/SlateIcon.h"

#define LOCTEXT_NAMESPACE "UAGX_AgxEdModeTerrain"

UAGX_AgxEdModeTerrain* UAGX_AgxEdModeTerrain::GetInstance()
{
	static UAGX_AgxEdModeTerrain* TerrainSubmode = nullptr;
	if (TerrainSubmode == nullptr)
	{
		TerrainSubmode = GetMutableDefault<UAGX_AgxEdModeTerrain>();
	}
	return TerrainSubmode;
}

FText UAGX_AgxEdModeTerrain::GetDisplayName() const
{
	return LOCTEXT("DisplayName", "Terrain");
}

FText UAGX_AgxEdModeTerrain::GetTooltip() const
{
	return LOCTEXT("Tooltip", "Management of AGX Terrain assets.");
}

FSlateIcon UAGX_AgxEdModeTerrain::GetIcon() const
{
	static FSlateIcon Icon(
		FAGX_EditorStyle::GetStyleSetName(), FAGX_EditorStyle::TerrainIcon,
		FAGX_EditorStyle::TerrainIconSmall);
	return Icon;
}

#undef LOCTEXT_NAMESPACE
