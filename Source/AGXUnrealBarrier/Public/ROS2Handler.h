// Copyright 2023, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"


class AGXUNREALBARRIER_API FROS2Handler
{
public:
	static void SendPointCloud(const TArray<FVector4>& Points);
};
