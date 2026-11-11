// Copyright 2026, Algoryx Simulation AB.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "AGX_DistributedScenarioAsset.generated.h"

class AActor;
class UAGX_DistributedAsset;

/** Project-authored settings for joining an externally managed AGX Distributed scenario. */
UCLASS(BlueprintType)
class AGXUNREAL_API UAGX_DistributedScenarioAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Limit network submissions per world tick to keep large placement sets responsive. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning", meta = (ClampMin = "1"))
	int32 MaxSpawnRequestsPerTick {32};

	/** Limit potentially expensive Actor/Blueprint creation per world tick. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning", meta = (ClampMin = "1"))
	int32 MaxActorCreationsPerTick {16};

	/** Register all descriptors used by this scenario, including assets spawned by other nodes. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Assets")
	TArray<TObjectPtr<UAGX_DistributedAsset>> Assets;

	/** Shared namespace for authored placement keys; must agree on all clients. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Assets")
	FString WorldNamespace {TEXT("Default")};

	/**
	 *	Empty uses AGX Distributed's default discovery. Otherwise connect to this Zenoh endpoint.
	 *
	 *	Format: protocol/hostname:port
	 *
	 *	Example: tcp/localhost:7447
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Connection")
	FString ZenohEndpoint;

	/**
	 * Map from AGX Distributed asset paths, such as MyModel.openplx, to the Blueprint that should
	 * be used when creating instances of each asset.
	 *
	 * Has lower priority than the Assets entries.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Assets")
	TMap<FString, TSoftClassPtr<AActor>> AssetBlueprints;

	/** Scenario asset ID to imported Blueprint. Takes precedence over the path mapping. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Assets")
	TMap<int64, TSoftClassPtr<AActor>> AssetBlueprintsById;

	/** Negative disables the automatic possession request after joining. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Client", meta = (ClampMin = "-1"))
	int64 EntityIdToPossess {-1};

	/** Placeholder for a later WorldManager launcher; ignored by this implementation. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Manager")
	bool bLaunchWorldManager {false};
};
