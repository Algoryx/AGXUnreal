// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_ComponentReference.h"

// Unreal Engine includes.
#include "Kismet/BlueprintFunctionLibrary.h"


#include "AGX_LidarSensorReference.generated.h"

class UAGX_LidarSensorComponent;

USTRUCT(BlueprintType)
struct AGXUNREAL_API FAGX_LidarSensorReference : public FAGX_ComponentReference
{
	GENERATED_BODY()

	FAGX_LidarSensorReference();

	UAGX_LidarSensorComponent* GetLidarComponent() const;
};

#if UE_BUILD_DEBUG
uint32 GetTypeHash(const FAGX_LidarSensorReference& Thing);
#else // optimize by inlining in shipping and development builds
FORCEINLINE uint32 GetTypeHash(const FAGX_LidarSensorReference& Thing)
{
	uint32 Hash = FCrc::MemCrc32(&Thing, sizeof(FAGX_LidarSensorReference));
	return Hash;
}
#endif


// Blueprint API

UCLASS()
class AGXUNREAL_API UAGX_LidarSensorReference_FL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "AGX Lidar")
	static void SetLidarComponent(
		UPARAM(Ref) FAGX_LidarSensorReference& Reference, UAGX_LidarSensorComponent* Component);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Lidar")
	static UAGX_LidarSensorComponent* GetLidarComponent(UPARAM(Ref)
															FAGX_LidarSensorReference& Reference);
};
