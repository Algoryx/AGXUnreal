// Copyright 2026, Algoryx Simulation AB.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"

#include "AGX_DistributedSettings.generated.h"

class UAGX_DistributedScenarioAsset;

/** Select a scenario asset under Project Settings > Plugins > AGX Distributed. */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "AGX Distributed"))
class AGXUNREAL_API UAGX_DistributedSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, Category = "Scenario")
	TSoftObjectPtr<UAGX_DistributedScenarioAsset> Scenario;

	virtual FName GetCategoryName() const override { return TEXT("Plugins"); }
};
