// Copyright 2026, Algoryx Simulation AB.

#include "OpenPLX/OpenPLX_IMUOutputViewFunctionLibrary.h"

bool UOpenPLX_IMUOutputView::GetAccelerometerDataWorld(
	FOpenPLXIMUOutputView& View, const FTransform& IMUTransform, FVector& OutAccelerometerData)
{
	if (!View.GetAccelerometerData(OutAccelerometerData))
		return false;

	OutAccelerometerData = IMUTransform.TransformVectorNoScale(OutAccelerometerData);
	return true;
}

bool UOpenPLX_IMUOutputView::GetGyroscopeDataWorld(
	FOpenPLXIMUOutputView& View, const FTransform& IMUTransform, FVector& OutGyroscopeData)
{
	if (!View.GetGyroscopeData(OutGyroscopeData))
		return false;

	OutGyroscopeData = IMUTransform.TransformVectorNoScale(OutGyroscopeData);
	return true;
}

bool UOpenPLX_IMUOutputView::GetMagnetometerDataWorld(
	FOpenPLXIMUOutputView& View, const FTransform& IMUTransform, FVector& OutMagnetometerData)
{
	if (!View.GetMagnetometerData(OutMagnetometerData))
		return false;

	OutMagnetometerData = IMUTransform.TransformVectorNoScale(OutMagnetometerData);
	return true;
}
