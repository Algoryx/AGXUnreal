// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include "agxCollide/Contacts.h"
#include "EndAGXIncludes.h"

struct FShapeContactEntity
{
	agxCollide::GeometryContact Native;

	FShapeContactEntity()
		// Explicitly passing an empty GeometryContactPtr to ensure that an entity isn't created
		// in the thread's default storage.
		: Native(agx::Physics::GeometryContactPtr())
	{
	}

	FShapeContactEntity(agxCollide::GeometryContact InNative)
		: Native(InNative)
	{
	}
};
