// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include "agxCollide/Contacts.h"
#include "EndAGXIncludes.h"

struct FContactPointEntity
{
	agxCollide::ContactPoint Native;

	FContactPointEntity()
		// Must explicitly pass empty ContactPointPtr because the default
		// constructor creates a new entity in the current threads default
		// storage.
		: Native(agx::Physics::ContactPointPtr())
	{
	}

	FContactPointEntity(agxCollide::ContactPoint InNative)
		: Native(InNative)
	{
	}
};
