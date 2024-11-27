// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxSensor/LidarRayPatternGenerator.h>
#include "EndAGXIncludes.h"

class FCustomPatternFetcherBase;

class FCustomPatternGenerator : public agxSensor::LidarRayPatternGenerator
{
public:
	FCustomPatternGenerator() = default;
	FCustomPatternGenerator(FCustomPatternFetcherBase* InFetcher);

	void SetCustomPatternFetcher(FCustomPatternFetcherBase* InFetcher);
	FCustomPatternFetcherBase* GetCustomPatternFetcher() const;

	virtual agxSensor::LidarRayPatternInterval getNextInterval(agx::Real dt) override;

private:
	FCustomPatternFetcherBase* PatternFetcher {nullptr};
};
