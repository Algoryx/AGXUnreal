// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"

#include "AGX_DistributedSimulationAuthorityAsync.generated.h"

class AActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FAGX_DistributedSimulationAuthorityOutput, AActor*, Entity, const FString&, ErrorMessage);

/** Await the WorldManager's simulation authority result without changing possession. */
UCLASS(meta = (HideThen))
class AGXUNREAL_API UAGX_DistributedSimulationAuthorityAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FAGX_DistributedSimulationAuthorityOutput Success;

	UPROPERTY(BlueprintAssignable)
	FAGX_DistributedSimulationAuthorityOutput Fail;

	/**
	 * Acquire or release simulation authority over an existing distributed Actor.
	 * Success runs after the WorldManager accepts the request, not merely after submission.
	 * Fail reports invalid targets, unavailable clients, overlapping requests and refused requests.
	 */
	UFUNCTION(
		BlueprintCallable, Category = "AGX Distributed",
		meta =
			(BlueprintInternalUseOnly = "true",
			 DisplayName = "Request Distributed Simulation Authority"))
	static UAGX_DistributedSimulationAuthorityAsync* RequestDistributedSimulationAuthority(
		AActor* TargetActor, bool bSimulate = true);

	virtual void Activate() override;

private:
	void Completed(bool bAccepted, AActor* CompletedEntity, const FString& ErrorMessage);

	TWeakObjectPtr<AActor> Entity;
	bool bRequestedSimulationAuthority {true};
};
