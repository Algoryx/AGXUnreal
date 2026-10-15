// Copyright 2026, Algoryx Simulation AB.

#include "Utilities/AGXUtilities.h"

// AGX Dynamics for Unreal includes.
#include "BarrierOnly/AGXRefs.h"
#include "RigidBodyBarrier.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxUtil/Convert/Convert.h>
#include <agxUtil/agxUtil.h>
#include "EndAGXIncludes.h"

double FAGXUtilities::ConvertSpookDampingToAttenuation(double SpookDamping, double TimeStep)
{
	return agxUtil::convert::convertSpookDampingToAttenuation(SpookDamping, TimeStep);
}

double FAGXUtilities::ConvertAttenuationToSpookDamping(double Attenuation, double TimeStep)
{
	return agxUtil::convert::convertAttenuationToSpookDamping(Attenuation, TimeStep);
}

void FAGXUtilities::AddParentVelocity(const FRigidBodyBarrier& Parent, FRigidBodyBarrier& Body)
{
	check(Parent.HasNative());
	check(Body.HasNative());

	agxUtil::addParentVelocity(Parent.GetNative()->Native, Body.GetNative()->Native);
}
