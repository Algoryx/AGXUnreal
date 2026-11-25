// Copyright 2026, Algoryx Simulation AB.

#pragma once

#include "CoreMinimal.h"

#include "AGX_DistributedEnums.generated.h"

/** Shared asset behavior registered with the World Manager. Must agree on all clients. */
UENUM(BlueprintType)
enum class EAGX_DistributedAssetType : uint8
{
	Static,
	Dynamic,
	Possessable
};
