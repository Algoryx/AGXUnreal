// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Terrain/AGX_TerrainWheelEnums.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <memory>

#include "TerrainWheelSettingsBarrier.generated.h"

struct FTerrainWheelSettingsRef;

/**
 * Acts as an interface to native AGX Terrain Wheel Settings, and encapsulates it so that it is
 * completely hidden from code that includes this file.
 */
USTRUCT(BlueprintType)
struct AGXUNREALBARRIER_API FTerrainWheelSettingsBarrier
{
	GENERATED_BODY()

public:
	FTerrainWheelSettingsBarrier();
	FTerrainWheelSettingsBarrier(std::shared_ptr<FTerrainWheelSettingsRef> Native);

	bool HasNative() const;
	FTerrainWheelSettingsRef* GetNative();
	const FTerrainWheelSettingsRef* GetNative() const;

	void AllocateNative();
	void ReleaseNative();

	FGuid GetGuid() const;

	void SetSlipRatioVxAngularEquivalentThreshold(double InThreshold);
	double GetSlipRatioVxAngularEquivalentThreshold() const;

	void SetSlipRatioOmegaYThreshold(double InThreshold);
	double GetSlipRatioOmegaYThreshold() const;

	void SetSlipRatioSmoothingAngularSpeed(double InSpeed);
	double GetSlipRatioSmoothingAngularSpeed() const;

	void SetAngularIntegrationStep(double InStep);
	double GetAngularIntegrationStep() const;

	void SetPressureSinkageModel(EAGX_TerrainWheelPressureSinkageModel InModel);
	EAGX_TerrainWheelPressureSinkageModel GetPressureSinkageModel() const;

	void SetEnableComputeRearAngleFromFrontAngle(bool InEnable);
	bool GetEnableComputeRearAngleFromFrontAngle() const;

	void SetEnableComputeMaximumNormalStressAngleFromFrontAngle(bool InEnable);
	bool GetEnableComputeMaximumNormalStressAngleFromFrontAngle() const;

	void SetEnableAGXDebugRendering(bool InEnable); // No getter in AGX.

private:
	std::shared_ptr<FTerrainWheelSettingsRef> NativeRef;
};
