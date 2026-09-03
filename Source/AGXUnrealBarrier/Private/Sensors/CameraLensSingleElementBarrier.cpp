// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/CameraLensSingleElementBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "BarrierOnly/AGXTypeConversions.h"
#include "Sensors/SensorRef.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxSensor/CameraLensSingleElement.h>
#include "EndAGXIncludes.h"

FCameraLensSingleElementBarrier::FCameraLensSingleElementBarrier(
	std::shared_ptr<FCameraLensRef> Native)
	: FCameraLensBarrier(std::move(Native))
{
}

void FCameraLensSingleElementBarrier::AllocateNative()
{
	check(!HasNative());
	NativeRef->Native = new agxSensor::CameraLensSingleElement();
}

void FCameraLensSingleElementBarrier::SetFocalLength(double InFocalLength)
{
	check(HasNative());
	agxSensor::CameraLensSingleElement* Lens =
		GetNative()->Native->asSafe<agxSensor::CameraLensSingleElement>();
	AGX_CHECK(Lens != nullptr);
	Lens->setFocalLength(ConvertDistanceToAGX(InFocalLength));
}

double FCameraLensSingleElementBarrier::GetFocalLength() const
{
	check(HasNative());
	const agxSensor::CameraLensSingleElement* Lens =
		GetNative()->Native->asSafe<agxSensor::CameraLensSingleElement>();
	AGX_CHECK(Lens != nullptr);
	return ConvertDistanceToUnreal<double>(Lens->getFocalLength());
}

bool FCameraLensSingleElementBarrier::IsSingleElement(const FCameraLensBarrier& Lens)
{
	if (!Lens.HasNative())
		return false;

	return Lens.GetNative()->Native->is<agxSensor::CameraLensSingleElement>();
}
