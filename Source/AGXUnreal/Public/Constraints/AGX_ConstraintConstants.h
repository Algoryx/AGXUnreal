// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_RealInterval.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <limits>

/**
 * Constants that reflect the initial values used in the native constraint implementations.
 * Therefore, do not change these!
 */
struct ConstraintConstants
{
public:
	static constexpr double DefaultCompliance()
	{
		return 1.0e-8;
	}
	static constexpr double DefaultElasticity()
	{
		return 1.0 / DefaultCompliance();
	};

	static constexpr double StrongCompliance()
	{
		return 1.0e-10;
	}
	static constexpr double StrongElasticity()
	{
		return 1.0 / StrongCompliance();
	};

	static constexpr double DefaultSpookDamping()
	{
		return 2.0 / 60.0;
	}

	static FAGX_RealInterval DefaultForceRange()
	{
		return FAGX_RealInterval(
			-std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity());
	}
};
