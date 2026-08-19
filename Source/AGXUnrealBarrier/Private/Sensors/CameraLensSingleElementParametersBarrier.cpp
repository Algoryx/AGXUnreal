// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/CameraLensSingleElementParametersBarrier.h"

// AGX Dynamics for Unreal includes.
#include "BarrierOnly/AGXTypeConversions.h"
#include "Sensors/SensorRef.h"

// Standard library includes.
#include <utility>

FCameraLensSingleElementParametersBarrier::FCameraLensSingleElementParametersBarrier()
	: NativeRef {new FCameraLensSingleElementParametersRef}
{
}

FCameraLensSingleElementParametersBarrier::FCameraLensSingleElementParametersBarrier(
	std::shared_ptr<FCameraLensSingleElementParametersRef> Native)
	: NativeRef(std::move(Native))
{
	check(NativeRef);
}

bool FCameraLensSingleElementParametersBarrier::HasNative() const
{
	return NativeRef->Native != nullptr;
}

void FCameraLensSingleElementParametersBarrier::SetFocalLength(double FocalLength)
{
	check(HasNative());
	NativeRef->Native->focalLength = ConvertDistanceToAGX(FocalLength);
}

double FCameraLensSingleElementParametersBarrier::GetFocalLength() const
{
	check(HasNative());
	return ConvertDistanceToUnreal<double>(NativeRef->Native->focalLength);
}

void FCameraLensSingleElementParametersBarrier::SetFStop(double FStop)
{
	check(HasNative());
	NativeRef->Native->fStop = FStop;
}

double FCameraLensSingleElementParametersBarrier::GetFStop() const
{
	check(HasNative());
	return NativeRef->Native->fStop;
}

void FCameraLensSingleElementParametersBarrier::SetMinimumFocusDistance(double MinimumFocusDistance)
{
	check(HasNative());
	NativeRef->Native->autofocus = true;
	NativeRef->Native->focus.minimumDistance = ConvertDistanceToAGX(MinimumFocusDistance);
}

void FCameraLensSingleElementParametersBarrier::SetFocusDistance(double FocusDistance)
{
	check(HasNative());
	NativeRef->Native->autofocus = false;
	NativeRef->Native->focus.distance = ConvertDistanceToAGX(FocusDistance);
}

bool FCameraLensSingleElementParametersBarrier::IsAutofocusEnabled() const
{
	check(HasNative());
	return NativeRef->Native->autofocus;
}

double FCameraLensSingleElementParametersBarrier::GetFocusDistance() const
{
	check(HasNative());
	if (IsAutofocusEnabled())
		return -1.0;

	return ConvertDistanceToUnreal<double>(NativeRef->Native->focus.distance);
}

double FCameraLensSingleElementParametersBarrier::GetMinimumFocusDistance() const
{
	check(HasNative());
	if (!IsAutofocusEnabled())
		return -1.0;

	return ConvertDistanceToUnreal<double>(NativeRef->Native->focus.minimumDistance);
}
