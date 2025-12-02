// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"

/**
 * Specifies in what solvers the normal and friction equations will be calculated.
 */
UENUM(BlueprintType)
enum class EAGX_ContactSolver : uint8
{
	NotDefined,

	/** Normal and friction equations calculated only in the DIRECT solver. */
	Direct,

	/** Normal and friction equations calculated only in the ITERATIVE solver. */
	Iterative,

	/**
	 * First Normal equation is calculated in the DIRECT solver, then in a second pass the normal
	 * and the friction equations are solved in the ITERATIVE solver.
	 */
	Split,

	/** Normal and friction equation calculated both in the ITERATIVE and the DIRECT solver. */
	DirectAndIterative UMETA(DisplayName = "Direct & Iterative")
};

/**
 * Specifies in what way friction is modelled.
 */
UENUM(BlueprintType)
enum class EAGX_FrictionModel : uint8
{
	NotDefined = 0,

	/**
	 * Box friction. Static bounds during solve. The friction box is aligned with the world axes.
	 * The normal force used by the box friction model is for new contacts estimated by the relative
	 * impact speed, mass, and gravity, or for continuous contacts equal to the the last normal
	 * force.
	 */
	BoxFriction = 1,

	/**
	 * This model uses the current (i.e. correct) normal force received by the solver.
	 *
	 * It is computationally more expensive than the box friction model but with a more realistic
	 * dry friction.
	 */
	ScaledBoxFriction = 2,

	/**
	 * This friction model is the default in AGX Dynamics.
	 *
	 * When this method is used with Solve type SPLIT (which is default), it will split the
	 * normal-tangential equations. I.e., the normal forces are first solved with a direct solve,
	 * and then both the normal and tangential equations are solved iteratively.
	 *
	 * For complex systems this could lead to viscous friction. This can be resolved by using
	 * 'Direct & Iterative' solve model for other constraints (hinges, etc.) involved in the system.
	 *
	 * Iterative projected cone friction model is computationally cheap. Friction forces are
	 * projected onto the friction cone, i.e., you will always get friction_force =
	 * friction_coefficient * normal_force.
	 */
	IterativeProjectedConeFriction = 3,

	/**
	 * Box friction model with oriented friction box.
	 */
	OrientedBoxFriction = 4,

	/**
	 * Scale box friction model with oriented friction box.
	 */
	OrientedScaledBoxFriction = 5,

	/**
	 * Iterative projected cone friction model with oriented friction box.
	 */
	OrientedIterativeProjectedConeFriction = 6,

	/**
	 * Oriented box friction model that uses the same normal force magnitude for all contact points
	 * associated to this friction model.
	 *
	 * This means that the size of the friction box always will be:
	 *   Primary Direction   = (Primary) Friction Coefficient * Normal Force Magnitude
	 *   Secondary Direction = Secondary Friction Coefficient * Normal Force Magnitude
	 *
	 * The given normal force can also be scaled with the contact point depth by setting
	 * 'Scale Normal Force With Depth' to true.
	 */
	OrientedConstantNormalForceBoxFriction = 7,

};

inline bool IsConstantNormalForceFrictionModel(EAGX_FrictionModel FrictionModel)
{
	return FrictionModel == EAGX_FrictionModel::OrientedConstantNormalForceBoxFriction;
}

inline bool IsOrientedFrictionModel(EAGX_FrictionModel FrictionModel)
{
	return FrictionModel == EAGX_FrictionModel::OrientedBoxFriction ||
		   FrictionModel == EAGX_FrictionModel::OrientedScaledBoxFriction ||
		   FrictionModel == EAGX_FrictionModel::OrientedIterativeProjectedConeFriction ||
		   FrictionModel == EAGX_FrictionModel::OrientedConstantNormalForceBoxFriction;
}

/**
 * Specifies to what extent contact reduction will be used.
 */
UENUM(BlueprintType)
enum class EAGX_ContactReductionMode : uint8
{
	/** No contact reduction enabled. */
	None,

	/** Reduce contacts between geometries. */
	Geometry,

	/** Two step reduction: first between geometries, and then between rigid bodies. */
	All
};

/**
 * Specifies the level of contact reduction used.
 */
UENUM(BlueprintType)
enum class EAGX_ContactReductionLevel : uint8
{
	/** AGX Dynamics default contact reduction level. */
	Default = 0,

	/** Remove contacts aggressively. */
	Aggressive = 1,

	/** Moderate contact reduction. */
	Moderate = 2,

	/** Remove few contacts. */
	Minimal = 3
};
