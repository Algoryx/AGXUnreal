// Copyright 2025, Algoryx Simulation AB.

#include "Utilities/TestUtilities.h"

// AGX Dynamics for Unreal includes.
#include "TypeConversions.h"

// AGX Dynamics includes
#include "BeginAGXIncludes.h"
#include <agxUtil/agxUtil.h>
#include "EndAGXIncludes.h"

// Unreal Engine includes.
#include "Misc/Guid.h"

FString FTestUtilities::ConvertToAGXUuidStr(const FGuid& Guid)
{
	agx::Uuid UuidAGX = Convert(Guid);
	FString UuidStrUnreal;
	{
		agx::String UuidStrAGX = UuidAGX.str();
		UuidStrUnreal = Convert(UuidStrAGX);

		// Must be called to avoid crash due to different allocators used by AGX Dynamics and
		// Unreal Engine.
		agxUtil::freeContainerMemory(UuidStrAGX);
	}

	return UuidStrUnreal;
}

FGuid FTestUtilities::ConvertAGXUuidToGuid(const FString& AGXUuidStr)
{
	agx::Uuid UuidAGX(Convert(AGXUuidStr));
	return Convert(UuidAGX);
}
