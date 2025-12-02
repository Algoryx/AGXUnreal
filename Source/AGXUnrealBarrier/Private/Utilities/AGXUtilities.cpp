// Copyright 2025, Algoryx Simulation AB.

#include "Utilities/AGXUtilities.h"

// AGX Dynamics for Unreal includes.
#include "BarrierOnly/AGXRefs.h"
#include "RigidBodyBarrier.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxUtil/agxUtil.h>
#include "EndAGXIncludes.h"


void FAGXUtilities::AddParentVelocity(const FRigidBodyBarrier& Parent, FRigidBodyBarrier& Body)
{
	check(Parent.HasNative());
	check(Body.HasNative());

	agxUtil::addParentVelocity(Parent.GetNative()->Native, Body.GetNative()->Native);
}
