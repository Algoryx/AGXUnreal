// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/CustomPatternGenerator.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Sensors/CustomPatternFetcherBase.h"
#include "TypeConversions.h"

agxSensor::LidarRayPatternInterval FCustomPatternGenerator::getNextInterval(agx::Real /*dt*/)
{
	if (PatternFetcher == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("getNextInterval called on Custom Pattern Generator without a PatternFetcher. "
				 "Lidar Custom pattern cannot be generated."));
		return agxSensor::LidarRayPatternInterval();
	}

	if (getNumRays() == 0)
	{
		const TArray<FTransform> Rays = PatternFetcher->GetRayTransforms();
		setRayTransforms(Convert(Rays));
	}

	const FAGX_CustomPatternInterval Interval = PatternFetcher->GetNextInterval();
	return Convert(Interval);
}

FCustomPatternGenerator::FCustomPatternGenerator(FCustomPatternFetcherBase* InFetcher)
	: PatternFetcher(InFetcher)
{
}

void FCustomPatternGenerator::SetCustomPatternFetcher(FCustomPatternFetcherBase* InFetcher)
{
	PatternFetcher = InFetcher;
}

FCustomPatternFetcherBase* FCustomPatternGenerator::GetCustomPatternFetcher() const
{
	return PatternFetcher;
}
