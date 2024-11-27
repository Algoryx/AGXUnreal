// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxSensor/Environment.h>
#include <agxSensor/Lidar.h>
#include <agxSensor/RaytraceAmbientMaterial.h>
#include <agxSensor/RaytraceDistanceGaussianNoise.h>
#include <agxSensor/RaytraceHandles.h>
#include <agxSensor/RaytraceOutput.h>
#include <agxSensor/RaytraceShapeInstance.h>
#include <agxSensor/RaytraceLambertianOpaqueMaterial.h>
#include "EndAGXIncludes.h"

#include <memory>

struct FSensorEnvironmentRef
{
	agxSensor::EnvironmentRef Native;
	FSensorEnvironmentRef() = default;
	FSensorEnvironmentRef(agxSensor::Environment* InNative)
		: Native(InNative)
	{
	}
};

struct FRtShapeRef
{
	agxSensor::RtShapeRef Native;
	FRtShapeRef() = default;
	FRtShapeRef(agxSensor::RtShapeRef InNative)
		: Native(InNative)
	{
	}
};

struct FRtShapeInstance
{
	agxSensor::RtShapeInstance Native;
};

struct FLidarRef
{
	agxSensor::LidarRef Native;
	FLidarRef() = default;
	FLidarRef(agxSensor::Lidar* InNative)
		: Native(InNative)
	{
	}
};

struct FLidarOutputRef
{
	agxSensor::RtOutputRef Native;
	FLidarOutputRef() = default;
	FLidarOutputRef(agxSensor::RtOutput* InNative)
		: Native(InNative)
	{
	}
};

struct FRtLambertianOpaqueMaterial
{
	std::shared_ptr<agxSensor::RtLambertianOpaqueMaterial> Native;
	FRtLambertianOpaqueMaterial() = default;
};

struct FRtAmbientMaterial
{
	std::shared_ptr<agxSensor::RtAmbientMaterial> Native;
	FRtAmbientMaterial() = default;
};
