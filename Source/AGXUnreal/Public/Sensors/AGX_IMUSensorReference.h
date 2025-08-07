// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_ComponentReference.h"

// Unreal Engine includes.
#include "Kismet/BlueprintFunctionLibrary.h"

#include "AGX_IMUSensorReference.generated.h"

class UAGX_IMUSensorComponent;

USTRUCT(BlueprintType)
struct AGXUNREAL_API FAGX_IMUSensorReference : public FAGX_ComponentReference
{
	GENERATED_BODY()

	FAGX_IMUSensorReference();

	UAGX_IMUSensorComponent* GetIMUComponent() const;
};

#if UE_BUILD_DEBUG
uint32 GetTypeHash(const FAGX_IMUSensorReference& Thing);
#else // optimize by inlining in shipping and development builds
FORCEINLINE uint32 GetTypeHash(const FAGX_IMUSensorReference& Thing)
{
	uint32 Hash = FCrc::MemCrc32(&Thing, sizeof(FAGX_IMUSensorReference));
	return Hash;
}
#endif

// Blueprint API

UCLASS()
class AGXUNREAL_API UAGX_IMUSensorReference_FL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "AGX IMU")
	static void SetIMUComponent(
		UPARAM(Ref) FAGX_IMUSensorReference& Reference, UAGX_IMUSensorComponent* Component);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX IMU")
	static UAGX_IMUSensorComponent* GetIMUComponent(UPARAM(Ref)
															FAGX_IMUSensorReference& Reference);
};
