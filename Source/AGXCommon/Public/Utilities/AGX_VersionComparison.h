// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_BuildInfo.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#if AGXUNREAL_HAS_VERSIONS

// Helper for AGXUNREAL_VERSION_NEWER_THAN and AGXUNREAL_VERSION_OLDER_THAN.
#define AGXUNREAL_GREATER_SORT(Value, ValueToBeGreaterThan, TieBreaker) \
	(((Value) > (ValueToBeGreaterThan)) || (((Value) == (ValueToBeGreaterThan)) && (TieBreaker)))

// Determine if the current AGX Dynamics for Unreal version is newer than the given version.
// Usage:
//    #if AGXUNREAL_VERSION_NEWER_THAN(1.16.0)
//        // Use the new API.
//    #else
//        // Use the old API.
//    #endif
#define AGXUNREAL_VERSION_NEWER_THAN(MajorVersion, MinorVersion, PatchVersion) \
	AGXUNREAL_GREATER_SORT(                                                    \
		AGXUNREAL_MAJOR_VERSION, MajorVersion,                                 \
		AGXUNREAL_GREATER_SORT(                                                \
			AGXUNREAL_MINOR_VERSION, MinorVersion,                             \
			AGXUNREAL_GREATER_SORT(AGXUNREAL_PATCH_VERSION, PatchVersion, false)))

// Determine if the current AGX Dynamics for Unreal version is newer than or equal to the given
// version.
// Usage:
//    #if AGXUNREAL_VERSION_NEWER_THAN_OR_EQUAL(1.16.0)
//        // Use the new API.
//    #else
//        // Use the old API.
//    #endif
#define AGXUNREAL_VERSION_NEWER_THAN_OR_EQUAL(MajorVersion, MinorVersion, PatchVersion) \
	AGXUNREAL_GREATER_SORT(                                                             \
		AGXUNREAL_MAJOR_VERSION, MajorVersion,                                          \
		AGXUNREAL_GREATER_SORT(                                                         \
			AGXUNREAL_MINOR_VERSION, MinorVersion,                                      \
			AGXUNREAL_GREATER_SORT(AGXUNREAL_PATCH_VERSION, PatchVersion, true)))

// Determine if the current AGX Dynamics for Unreal version is older than the given version.
// Usage:
//    #if AGXUNREAL_VERSION_OLDER_THAN(1.16.0)
//        // Use the old API.
//    #else
//        // Use the new API.
//    #endif
#define AGXUNREAL_VERSION_OLDER_THAN(MajorVersion, MinorVersion, PatchVersion) \
	AGXUNREAL_GREATER_SORT(                                                    \
		MajorVersion, AGXUNREAL_MAJOR_VERSION,                                 \
		AGXUNREAL_GREATER_SORT(                                                \
			MinorVersion, AGXUNREAL_MINOR_VERSION,                             \
			AGXUNREAL_GREATER_SORT(PatchVersion, AGXUNREAL_PATCH_VERSION, false)))

// Determine if the current AGX Dynamics for Unreal version is older than or equal to the given
// version.
// Usage:
//    #if AGXUNREAL_VERSION_OLDER_THAN_OR_EQUAL(1.16.0)
//        // Use the old API.
//    #else
//        // Use the new API.
//    #endif
#define AGXUNREAL_VERSION_OLDER_THAN_OR_EQUAL(MajorVersion, MinorVersion, PatchVersion) \
	AGXUNREAL_GREATER_SORT(                                                             \
		MajorVersion, AGXUNREAL_MAJOR_VERSION,                                          \
		AGXUNREAL_GREATER_SORT(                                                         \
			MinorVersion, AGXUNREAL_MINOR_VERSION,                                      \
			AGXUNREAL_GREATER_SORT(PatchVersion, AGXUNREAL_PATCH_VERSION, true)))

#else

#pragma message "AGX Dynamics for Unreal version information not available, AGXUNREAL_VERSION_(NEWER|OLDER)_THAN macros will not work."


#endif
