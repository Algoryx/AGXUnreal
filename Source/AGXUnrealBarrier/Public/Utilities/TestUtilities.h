// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

struct FGuid;

/**
 * Convenience class made for internal Unit Tests to access parts of the plugin otherwise
 * unreachable from the AGXUnrealTest module.
 */
class AGXUNREALBARRIER_API FTestUtilities
{
public:
#if WITH_DEV_AUTOMATION_TESTS
	/** Run a test with a private manager, optionally starting with an empty asset catalog. */
	static bool WithDistributedTestManager(
		uint16 Port, TFunctionRef<bool()> Test, FString& OutError, bool bEmptyCatalog = false);
	/** Exercise real control transport and barrier polling without a physics step. */
	static bool TestDistributedControlMessages(FString& OutError);
	/** Exercise state serialization, authority broadcasts and release over real transport. */
	static bool TestDistributedEntityState(FString& OutError);
	/** Exercise typed signal transport, exact payloads and authority-handoff replay. */
	static bool TestDistributedSignals(FString& OutError);
#endif
	static FString ConvertToAGXUuidStr(const FGuid& Guid);

	static FGuid ConvertAGXUuidToGuid(const FString& AGXUuidStr);

	static bool DecodeDistributedBodyTransforms(
		const TArray<uint8>& Data, TArray<FTransform>& OutTransforms);
};
