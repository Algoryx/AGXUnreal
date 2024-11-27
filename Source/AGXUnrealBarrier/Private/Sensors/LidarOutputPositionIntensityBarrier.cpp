// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/LidarOutputPositionIntensityBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "Sensors/AGX_LidarOutputTypes.h"
#include "Sensors/SensorRef.h"
#include "TypeConversions.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxSensor/RaytraceOutput.h>
#include "EndAGXIncludes.h"

namespace LidarOutputPositionIntensityBarrier_helpers
{
	struct LidarPositionIntensityData
	{
		float X;
		float Y;
		float Z;
		float Intensity;
	};
}

void FLidarOutputPositionIntensityBarrier::AllocateNative()
{
	check(!HasNative());

	NativeRef->Native = new agxSensor::RtOutput();
	NativeRef->Native->add(agxSensor::RtOutput::XYZ_VEC3_F32);
	NativeRef->Native->add(agxSensor::RtOutput::INTENSITY_F32);
}

void FLidarOutputPositionIntensityBarrier::GetData(
	TArray<FAGX_LidarOutputPositionIntensityData>& OutData) const
{
	using namespace LidarOutputPositionIntensityBarrier_helpers;

	check(HasNative());
	AGX_CHECK(sizeof(LidarPositionIntensityData) == GetNative()->Native->getElementSize());

	agxSensor::BinaryOutputView<LidarPositionIntensityData> ViewAGX =
		GetNative()->Native->view<LidarPositionIntensityData>();

	OutData.SetNumUninitialized(ViewAGX.size(), false);
	for (int32 I = 0; I < ViewAGX.size(); I++)
	{
		OutData[I] = {
			ConvertDisplacement(ViewAGX[I].X, ViewAGX[I].Y, ViewAGX[I].Z), ViewAGX[I].Intensity};
	}
}
