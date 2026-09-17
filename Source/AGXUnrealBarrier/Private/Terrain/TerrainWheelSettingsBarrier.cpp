// Copyright 2026, Algoryx Simulation AB.

#include "Terrain/TerrainWheelSettingsBarrier.h"

// AGX Dynamics for Unreal includes.
#include "BarrierOnly/AGXRefs.h"
#include "BarrierOnly/AGXTypeConversions.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxTerrain/TerrainWheelSettings.h>
#include "EndAGXIncludes.h"

FTerrainWheelSettingsBarrier::FTerrainWheelSettingsBarrier()
	: NativeRef {new FTerrainWheelSettingsRef}
{
}

FTerrainWheelSettingsBarrier::FTerrainWheelSettingsBarrier(
	std::shared_ptr<FTerrainWheelSettingsRef> Native)
	: NativeRef {std::move(Native)}
{
}

bool FTerrainWheelSettingsBarrier::HasNative() const
{
	return NativeRef && NativeRef->Native;
}

FTerrainWheelSettingsRef* FTerrainWheelSettingsBarrier::GetNative()
{
	return NativeRef.get();
}

const FTerrainWheelSettingsRef* FTerrainWheelSettingsBarrier::GetNative() const
{
	return NativeRef.get();
}

void FTerrainWheelSettingsBarrier::AllocateNative()
{
	check(!HasNative());
	NativeRef->Native = new agxTerrain::TerrainWheelSettings();
}

void FTerrainWheelSettingsBarrier::ReleaseNative()
{
	check(HasNative());
	NativeRef->Native = nullptr;
}

FGuid FTerrainWheelSettingsBarrier::GetGuid() const
{
	check(HasNative());
	return Convert(NativeRef->Native->getUuid());
}

void FTerrainWheelSettingsBarrier::SetSlipRatioVxAngularEquivalentThreshold(double InThreshold)
{
	check(HasNative());
	NativeRef->Native->setSlipRatioVxAngularEquivalentThreshold(ConvertDistanceToAGX(InThreshold));
}

double FTerrainWheelSettingsBarrier::GetSlipRatioVxAngularEquivalentThreshold() const
{
	check(HasNative());
	return ConvertDistanceToUnreal<double>(
		NativeRef->Native->getSlipRatioVxAngularEquivalentThreshold());
}

void FTerrainWheelSettingsBarrier::SetSlipRatioOmegaYThreshold(double InThreshold)
{
	check(HasNative());
	NativeRef->Native->setSlipRatioOmegaYThreshold(ConvertDistanceToAGX(InThreshold));
}

double FTerrainWheelSettingsBarrier::GetSlipRatioOmegaYThreshold() const
{
	check(HasNative());
	return ConvertDistanceToUnreal<double>(NativeRef->Native->getSlipRatioOmegaYThreshold());
}

void FTerrainWheelSettingsBarrier::SetSlipRatioSmoothingAngularSpeed(double InSpeed)
{
	check(HasNative());
	NativeRef->Native->setSlipRatioSmoothingAngularSpeed(ConvertDistanceToAGX(InSpeed));
}

double FTerrainWheelSettingsBarrier::GetSlipRatioSmoothingAngularSpeed() const
{
	check(HasNative());
	return ConvertDistanceToUnreal<double>(NativeRef->Native->getSlipRatioSmoothingAngularSpeed());
}

void FTerrainWheelSettingsBarrier::SetAngularIntegrationStep(double InStep)
{
	check(HasNative());
	NativeRef->Native->setAngularIntegrationStep(ConvertAngleToAGX(InStep));
}

double FTerrainWheelSettingsBarrier::GetAngularIntegrationStep() const
{
	check(HasNative());
	return ConvertAngleToUnreal<double>(NativeRef->Native->getAngularIntegrationStep());
}

void FTerrainWheelSettingsBarrier::SetPressureSinkageModel(
	EAGX_TerrainWheelPressureSinkageModel InModel)
{
	check(HasNative());
	NativeRef->Native->setPressureSinkageModel(Convert(InModel));
}

EAGX_TerrainWheelPressureSinkageModel FTerrainWheelSettingsBarrier::GetPressureSinkageModel() const
{
	check(HasNative());
	return Convert(NativeRef->Native->getPressureSinkageModel());
}

void FTerrainWheelSettingsBarrier::SetEnableComputeRearAngleFromFrontAngle(bool InEnable)
{
	check(HasNative());
	NativeRef->Native->setEnableComputeRearAngleFromFrontAngle(InEnable);
}

bool FTerrainWheelSettingsBarrier::GetEnableComputeRearAngleFromFrontAngle() const
{
	check(HasNative());
	return NativeRef->Native->getEnableComputeRearAngleFromFrontAngle();
}

void FTerrainWheelSettingsBarrier::SetEnableComputeMaximumNormalStressAngleFromFrontAngle(
	bool InEnable)
{
	check(HasNative());
	NativeRef->Native->setEnableComputeMaximumNormalStressAngleFromFrontAngle(InEnable);
}

bool FTerrainWheelSettingsBarrier::GetEnableComputeMaximumNormalStressAngleFromFrontAngle() const
{
	check(HasNative());
	return NativeRef->Native->getEnableComputeMaximumNormalStressAngleFromFrontAngle();
}

void FTerrainWheelSettingsBarrier::SetEnableAGXDebugRendering(bool InEnable)
{
	check(HasNative());
	NativeRef->Native->setEnableDebugRegressionPlanes(InEnable);
}

bool FTerrainWheelSettingsBarrier::GetEnableAGXDebugRendering() const
{
	check(HasNative());
	return NativeRef->Native->getEnableDebugRegressionPlanes();
}

void FTerrainWheelSettingsBarrier::SetEnableForceFrameTransformation(bool InEnable)
{
	check(HasNative());
	NativeRef->Native->setEnableForceFrameTransformation(InEnable);
}

bool FTerrainWheelSettingsBarrier::GetEnableForceFrameTransformation() const
{
	check(HasNative());
	return NativeRef->Native->getEnableForceFrameTransformation();
}

void FTerrainWheelSettingsBarrier::SetForceFeedbackHeightFieldSamplingMode(
	EAGX_TerrainWheelSamplingMode InMode)
{
	check(HasNative());
	NativeRef->Native->setForceFeedbackHeightFieldSamplingMode(Convert(InMode));
}

EAGX_TerrainWheelSamplingMode
FTerrainWheelSettingsBarrier::GetForceFeedbackHeightFieldSamplingMode() const
{
	check(HasNative());
	return Convert(NativeRef->Native->getForceFeedbackHeightFieldSamplingMode());
}

void FTerrainWheelSettingsBarrier::SetForceReferencePlaneSamplingMode(
	EAGX_TerrainWheelSamplingMode InMode)
{
	check(HasNative());
	NativeRef->Native->setForceReferencePlaneSamplingMode(Convert(InMode));
}

EAGX_TerrainWheelSamplingMode
FTerrainWheelSettingsBarrier::GetForceReferencePlaneSamplingMode() const
{
	check(HasNative());
	return Convert(NativeRef->Native->getForceReferencePlaneSamplingMode());
}

void FTerrainWheelSettingsBarrier::SetRearAndFrontAngleReferencePlaneSamplingMode(
	EAGX_TerrainWheelSamplingMode InMode)
{
	check(HasNative());
	NativeRef->Native->setRearAndFrontAngleReferencePlaneSamplingMode(Convert(InMode));
}

EAGX_TerrainWheelSamplingMode
FTerrainWheelSettingsBarrier::GetRearAndFrontAngleReferencePlaneSamplingMode() const
{
	check(HasNative());
	return Convert(NativeRef->Native->getRearAndFrontAngleReferencePlaneSamplingMode());
}

void FTerrainWheelSettingsBarrier::SetEnableRearAndFrontAngleSmoothing(bool InEnable)
{
	check(HasNative());
	NativeRef->Native->setEnableRearAndFrontAngleSmoothing(InEnable);
}

bool FTerrainWheelSettingsBarrier::GetEnableRearAndFrontAngleSmoothing() const
{
	check(HasNative());
	return NativeRef->Native->getEnableRearAndFrontAngleSmoothing();
}

void FTerrainWheelSettingsBarrier::SetRearAndFrontAngleSmoothingAngularSpeed(double InSpeed)
{
	check(HasNative());
	NativeRef->Native->setRearAndFrontAngleSmoothingAngularSpeed(ConvertAngleToAGX(InSpeed));
}

double FTerrainWheelSettingsBarrier::GetRearAndFrontAngleSmoothingAngularSpeed() const
{
	check(HasNative());
	return ConvertAngleToUnreal<double>(
		NativeRef->Native->getRearAndFrontAngleSmoothingAngularSpeed());
}

void FTerrainWheelSettingsBarrier::SetEnableTractionRollingResistanceCoupling(bool InEnable)
{
	check(HasNative());
	NativeRef->Native->setEnableTractionRollingResistanceCoupling(InEnable);
}

bool FTerrainWheelSettingsBarrier::GetEnableTractionRollingResistanceCoupling() const
{
	check(HasNative());
	return NativeRef->Native->getEnableTractionRollingResistanceCoupling();
}
