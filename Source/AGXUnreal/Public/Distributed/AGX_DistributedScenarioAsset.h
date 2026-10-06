// Copyright 2026, Algoryx Simulation AB.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "AGX_DistributedScenarioAsset.generated.h"

class AActor;

/** Project-authored settings for joining an externally managed AGX Distributed scenario. */
UCLASS(BlueprintType)
class AGXUNREAL_API UAGX_DistributedScenarioAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Empty uses AGX Distributed's default discovery. Otherwise connect to this Zenoh endpoint. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Connection")
	FString ZenohEndpoint;

	/** Scenario asset path to the Blueprint imported from that OpenPLX model. */
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
