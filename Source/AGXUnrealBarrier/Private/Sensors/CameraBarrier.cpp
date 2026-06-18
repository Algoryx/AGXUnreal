// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/CameraBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "BarrierOnly/AGXTypeConversions.h"
#include "Sensors/SensorRef.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxSensor/Camera.h>
#include <agxSensor/CameraCMOSSensor.h>
#include <agxSensor/CameraLensSingleElement.h>
#include <agxSensor/CameraModel.h>
#include "EndAGXIncludes.h"

namespace CameraBarrier_helpers
{
	agxSensor::Camera* GetCameraNative(FCameraBarrier& Camera)
	{
		AGX_CHECK(Camera.HasNative());
		return Camera.GetNative()->Native->asSafe<agxSensor::Camera>();
	}

	agxSensor::Camera* GetCameraNative(const FCameraBarrier& Camera)
	{
		AGX_CHECK(Camera.HasNative());
		return Camera.GetNative()->Native->asSafe<agxSensor::Camera>();
	}
}

FCameraBarrier::FCameraBarrier(
	std::shared_ptr<FSensorRef> Native, std::shared_ptr<FSensorGroupStepStrideRef> StepStride)
	: FSensorBarrier(std::move(Native), std::move(StepStride))
{
}

void FCameraBarrier::AllocateNative(const FTransform& Transform)
{
	check(!HasNative());

	NativeRef->Native = new agxSensor::Camera(
		new agx::Frame(Convert(Transform)),
		new agxSensor::CameraModel(
			new agxSensor::CameraLensSingleElement(), new agxSensor::CameraCMOSSensor()));
}

void FCameraBarrier::SetTransform(const FTransform& Transform)
{
	check(HasNative());
	CameraBarrier_helpers::GetCameraNative(*this)->getFrame()->setMatrix(Convert(Transform));
}

FTransform FCameraBarrier::GetTransform() const
{
	check(HasNative());
	return Convert(CameraBarrier_helpers::GetCameraNative(*this)->getFrame()->getMatrix());
}
