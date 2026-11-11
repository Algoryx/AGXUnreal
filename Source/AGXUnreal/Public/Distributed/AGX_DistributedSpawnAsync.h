// Copyright 2026, Algoryx Simulation AB.

#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Distributed/AGX_DistributedSpawnRequest.h"
#include "AGX_DistributedSpawnAsync.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(
	FAGX_DistributedSpawnOutput, bool, QueuedResult, int64, EntityId, AActor*, Entity,
	const FString&, ErrorMessage, FGuid, RequestId);

/** Queue now, complete after the distributed entity and its local Actor both exist. */
UCLASS(meta = (HideThen))
class AGXUNREAL_API UAGX_DistributedSpawnAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable) FAGX_DistributedSpawnOutput Queued;
	UPROPERTY(BlueprintAssignable) FAGX_DistributedSpawnOutput Success;
	UPROPERTY(BlueprintAssignable) FAGX_DistributedSpawnOutput Fail;
	UFUNCTION(
		BlueprintCallable, Category = "AGX Distributed",
		meta =
			(BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject",
			 DisplayName = "Spawn Distributed Entity"))
	static UAGX_DistributedSpawnAsync* SpawnDistributedEntity(
		UObject* WorldContextObject, FAGX_DistributedSpawnOptions Options);
	virtual void Activate() override;

private:
	UPROPERTY() TObjectPtr<UObject> Context;
	UPROPERTY() FAGX_DistributedSpawnOptions SpawnOptions;
	void Completed(const FAGX_DistributedSpawnResult& Result);
};
