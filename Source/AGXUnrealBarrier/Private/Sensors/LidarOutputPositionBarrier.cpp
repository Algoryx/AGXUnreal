// Copyright 2025, Algoryx Simulation AB.

#include "Sensors/LidarOutputPositionBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "BarrierOnly/AGXTypeConversions.h"
#include "Sensors/AGX_LidarOutputTypes.h"
#include "Sensors/SensorRef.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxSensor/RaytraceOutput.h>
#include "EndAGXIncludes.h"

// Unreal Engine includes.
#include "Misc/EngineVersionComparison.h"

namespace LidarOutputPositionBarrier_helpers
{
	struct LidarPositionData
	{
		float X;
		float Y;
		float Z;
	};
}

void FLidarOutputPositionBarrier::AllocateNative()
{
	check(!HasNative());

	NativeRef->Native = new agxSensor::RtOutput();
	NativeRef->Native->add(agxSensor::RtOutput::XYZ_VEC3_F32);
}

void FLidarOutputPositionBarrier::GetData(TArray<FAGX_LidarOutputPositionData>& OutData) const
{
	using namespace LidarOutputPositionBarrier_helpers;

	check(HasNative());
	AGX_CHECK(sizeof(LidarPositionData) == GetNative()->Native->getElementSize());

	if (!GetNative()->Native->hasUnreadData(/*markAsRead*/ false))
	{
#if UE_VERSION_OLDER_THAN(5, 5, 0)
		OutData.SetNumUninitialized(0, false);
#else
		OutData.SetNumUninitialized(0, EAllowShrinking::No);
#endif
		return;
	}

	agxSensor::BinaryOutputView<LidarPositionData> ViewAGX =
		GetNative()->Native->view<LidarPositionData>();

#if UE_VERSION_OLDER_THAN(5, 5, 0)
	OutData.SetNumUninitialized(ViewAGX.size(), false);
#else
	OutData.SetNumUninitialized(ViewAGX.size(), EAllowShrinking::No);
#endif
	for (int32 I = 0; I < ViewAGX.size(); I++)
	{
		OutData[I] = {ConvertDisplacement(ViewAGX[I].X, ViewAGX[I].Y, ViewAGX[I].Z)};
	}
}
