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

namespace CameraLensSingleElementBarrier_helpers
{
	agxSensor::CameraLensSingleElement* GetNative(FCameraLensSingleElementBarrier& Barrier)
	{
		check(Barrier.HasNative());
		agxSensor::CameraLensSingleElement* Lens =
			Barrier.GetNative()->Native->asSafe<agxSensor::CameraLensSingleElement>();
		AGX_CHECK(Lens != nullptr);
		return Lens;
	}

	const agxSensor::CameraLensSingleElement* GetNative(
		const FCameraLensSingleElementBarrier& Barrier)
	{
		check(Barrier.HasNative());
		const agxSensor::CameraLensSingleElement* Lens =
			Barrier.GetNative()->Native->asSafe<agxSensor::CameraLensSingleElement>();
		AGX_CHECK(Lens != nullptr);
		return Lens;
	}
}

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
	using namespace CameraLensSingleElementBarrier_helpers;
	GetNative(*this)->setFocalLength(ConvertDistanceToAGX(InFocalLength));
}

double FCameraLensSingleElementBarrier::GetFocalLength() const
{
	using namespace CameraLensSingleElementBarrier_helpers;
	return ConvertDistanceToUnreal<double>(GetNative(*this)->getFocalLength());
}

void FCameraLensSingleElementBarrier::SetFStop(double InFStop)
{
	using namespace CameraLensSingleElementBarrier_helpers;
	GetNative(*this)->setFStop(InFStop);
}

double FCameraLensSingleElementBarrier::GetFStop() const
{
	using namespace CameraLensSingleElementBarrier_helpers;
	return GetNative(*this)->getFStop();
}

void FCameraLensSingleElementBarrier::SetAutofocus(double InMinimumFocusDistance)
{
	using namespace CameraLensSingleElementBarrier_helpers;
	GetNative(*this)->setAutofocus(ConvertDistanceToAGX(InMinimumFocusDistance));
}

bool FCameraLensSingleElementBarrier::GetUseAutofocus() const
{
	using namespace CameraLensSingleElementBarrier_helpers;
	return GetNative(*this)->isAutofocusEnabled();
}

void FCameraLensSingleElementBarrier::SetFocusDistance(double InFocusDistance)
{
	using namespace CameraLensSingleElementBarrier_helpers;
	GetNative(*this)->setFocusDistance(ConvertDistanceToAGX(InFocusDistance));
}

double FCameraLensSingleElementBarrier::GetFocusDistance() const
{
	using namespace CameraLensSingleElementBarrier_helpers;
	const auto FocusDistance = GetNative(*this)->getFocusDistance();
	return FocusDistance.has_value() ? ConvertDistanceToUnreal<double>(*FocusDistance) : 0.0;
}

double FCameraLensSingleElementBarrier::GetMinimumFocusDistance() const
{
	using namespace CameraLensSingleElementBarrier_helpers;
	const auto MinimumFocusDistance = GetNative(*this)->getMinimumFocusDistance();
	return MinimumFocusDistance.has_value()
			   ? ConvertDistanceToUnreal<double>(*MinimumFocusDistance)
			   : 0.0;
}

bool FCameraLensSingleElementBarrier::IsSingleElement(const FCameraLensBarrier& Lens)
{
	if (!Lens.HasNative())
		return false;

	return Lens.GetNative()->Native->is<agxSensor::CameraLensSingleElement>();
}
