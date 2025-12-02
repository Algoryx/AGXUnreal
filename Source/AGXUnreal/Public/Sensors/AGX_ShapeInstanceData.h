// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "Sensors/RtShapeInstanceBarrier.h"
#include "Sensors/RtShapeBarrier.h"


struct FAGX_RtInstanceData
{
	FRtShapeInstanceBarrier Instance;
	FTransform Transform;
	size_t RefCount {1};

	void SetTransform(const FTransform& InTransform)
	{
		Transform = InTransform;

		if (Instance.HasNative())
			Instance.SetTransform(InTransform);
	}
};

struct FAGX_RtShapeInstanceData
{
	FRtShapeBarrier Shape;
	FAGX_RtInstanceData InstanceData;
};

struct FAGX_RtInstancedShapeInstanceData
{
	FRtShapeBarrier Shape;
	TMap<int32, FAGX_RtInstanceData> InstancesData;
};
