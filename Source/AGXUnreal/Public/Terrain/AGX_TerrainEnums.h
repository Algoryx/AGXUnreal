// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"

/**
 * All frames that a Shovel keeps track of.
 */
UENUM()
enum class EAGX_ShovelFrame : uint8
{
	None,
	TopEdgeBegin,
	TopEdgeEnd,
	CuttingEdgeBegin,
	CuttingEdgeEnd,
	ToothDirection
};

inline bool IsTranslatable(EAGX_ShovelFrame ShovelFrame)
{
	switch (ShovelFrame)
	{
		// Even directions are transaltable, even though that doesn't mean anything.
		// Is helpful in order to line the direction vector up with geometry on the shovel.
		case EAGX_ShovelFrame::None:
			return false;
		case EAGX_ShovelFrame::ToothDirection:
		case EAGX_ShovelFrame::CuttingEdgeBegin:
		case EAGX_ShovelFrame::CuttingEdgeEnd:
		case EAGX_ShovelFrame::TopEdgeBegin:
		case EAGX_ShovelFrame::TopEdgeEnd:
			return true;
	}

	// Unknown shovel frame passed, should never happen. Crash in unit tests, assume not
	// translatable in user builds.
	AGX_CHECK(false);
	return false;
}

inline bool IsRotatable(EAGX_ShovelFrame ShovelFrame)
{
	switch (ShovelFrame)
	{
		case EAGX_ShovelFrame::None:
			return false;
		case EAGX_ShovelFrame::ToothDirection:
			return true;
		case EAGX_ShovelFrame::CuttingEdgeBegin:
		case EAGX_ShovelFrame::CuttingEdgeEnd:
		case EAGX_ShovelFrame::TopEdgeBegin:
		case EAGX_ShovelFrame::TopEdgeEnd:
			return false;
	}

	// Unknown shovel frame passed, should never happen. Crash in unit tests, assume not rotatable
	// in user builds.
	AGX_CHECK(false);
	return false;
}
