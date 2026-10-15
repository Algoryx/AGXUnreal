// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

struct FRigidBodyBarrier;

class AGXUNREALBARRIER_API FAGXUtilities
{
public:
	/** Convert SPOOK damping [s] to attenuation [steps]. TimeStep must be positive [s]. */
	static double ConvertSpookDampingToAttenuation(double SpookDamping, double TimeStep);

	/** Convert attenuation [steps] to SPOOK damping [s]. TimeStep must be positive [s]. */
	static double ConvertAttenuationToSpookDamping(double Attenuation, double TimeStep);

	/**
	* Body will get the parents velocity added to its velocity/angular velocity as if it was
	* rigidly attached.
	*/
	static void AddParentVelocity(const FRigidBodyBarrier& Parent, FRigidBodyBarrier& Body);
};
