// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"

#include "AGX_DistributedPossessionAsync.generated.h"

class AActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FAGX_DistributedPossessionOutput, AActor*, Entity, const FString&, ErrorMessage);

/** Await the WorldManager's possession result without changing simulation authority. */
UCLASS(meta = (HideThen))
class AGXUNREAL_API UAGX_DistributedPossessionAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FAGX_DistributedPossessionOutput Success;

	UPROPERTY(BlueprintAssignable)
	FAGX_DistributedPossessionOutput Fail;

	/**
	 * Acquire or release input control of an existing distributed Actor.
	 * Success runs after the WorldManager accepts the request, not merely after submission.
	 * Fail reports invalid targets, unavailable clients, overlapping requests and refused requests.
	 */
	UFUNCTION(
		BlueprintCallable, Category = "AGX Distributed",
		meta = (BlueprintInternalUseOnly = "true", DisplayName = "Request Distributed Possession"))
	static UAGX_DistributedPossessionAsync* RequestDistributedPossession(
		AActor* TargetActor, bool bPossess = true);

	virtual void Activate() override;

private:
	void Completed(bool bAccepted, AActor* CompletedEntity, const FString& ErrorMessage);

	TWeakObjectPtr<AActor> Entity;
	bool bRequestedPossession {true};
};
