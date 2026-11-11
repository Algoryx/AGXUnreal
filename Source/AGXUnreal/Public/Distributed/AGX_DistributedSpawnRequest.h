// Copyright 2026, Algoryx Simulation AB.

#pragma once

#include "CoreMinimal.h"
#include "AGX_DistributedSpawnRequest.generated.h"

class AActor;
class UAGX_DistributedAsset;

UENUM(BlueprintType)
enum class EAGX_DistributedSpawnState : uint8
{
	Queued,
	Submitted,
	Succeeded,
	Failed,
	Cancelled
};

/** One local caller's interest in a distributed world object. */
USTRUCT(BlueprintType)
struct AGXUNREAL_API FAGX_DistributedSpawnOptions
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distributed")
	TObjectPtr<UAGX_DistributedAsset> Asset;
	/** World-space placement in Unreal centimeters. Non-unit scale is unsupported. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distributed")
	FTransform Transform;
	/** Stable across clients for authored objects. Empty creates a new runtime object identity. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distributed")
	FString ObjectKey;
	/** Requests authority separately after the local Actor is ready. Does not request possession.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distributed")
	bool bRequestSimulationAuthority {false};
	/** Seconds allowed for joining, submission and local Actor creation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distributed", meta = (ClampMin = "1"))
	double TimeoutSeconds {120.0};
};

USTRUCT(BlueprintType)
struct AGXUNREAL_API FAGX_DistributedSpawnResult
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly, Category = "Distributed")
	FGuid RequestId;
	UPROPERTY(BlueprintReadOnly, Category = "Distributed")
	EAGX_DistributedSpawnState State {EAGX_DistributedSpawnState::Queued};
	UPROPERTY(BlueprintReadOnly, Category = "Distributed")
	int64 EntityId {-1};
	UPROPERTY(BlueprintReadOnly, Category = "Distributed")
	TObjectPtr<AActor> Entity {nullptr};
	UPROPERTY(BlueprintReadOnly, Category = "Distributed")
	FString ErrorMessage;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FAGX_DistributedSpawnCompleted, const FAGX_DistributedSpawnResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
	FAGX_DistributedAuthorityCompleted, int64, EntityId, bool, bSimulate, bool, bAccepted,
	const FString&, ErrorMessage);
DECLARE_DELEGATE_OneParam(FAGX_DistributedSpawnCallback, const FAGX_DistributedSpawnResult&);
