// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <memory>

#include "TerrainWheelMaterialBarrier.generated.h"

struct FTerrainWheelMaterialPtr;


USTRUCT(BlueprintType)
struct AGXUNREALBARRIER_API FTerrainWheelMaterialBarrier
{
	GENERATED_BODY()

	FTerrainWheelMaterialBarrier();
	FTerrainWheelMaterialBarrier(std::shared_ptr<FTerrainWheelMaterialPtr> Native);

	void AllocateNative();

	void SetSinkageExponentParameterA(double Value);
	double GetSinkageExponentParameterA() const;

	void SetSinkageExponentParameterB(double Value);
	double GetSinkageExponentParameterB() const;

	void SetCohesion(double Value);
	double GetCohesion() const;

	void SetAngleOfInternalFriction(double Value);
	double GetAngleOfInternalFriction() const;

	void SetShearModulusXParameterA(double Value);
	double GetShearModulusXParameterA() const;

	void SetShearModulusXParameterB(double Value);
	double GetShearModulusXParameterB() const;

	void SetShearModulusYParameterA(double Value);
	double GetShearModulusYParameterA() const;

	void SetShearModulusYParameterB(double Value);
	double GetShearModulusYParameterB() const;

	void SetCohesiveModulusBekker(double Value);
	double GetCohesiveModulusBekker() const;

	void SetFrictionalModulusBekker(double Value);
	double GetFrictionalModulusBekker() const;

	void SetCohesiveModulusReece(double Value);
	double GetCohesiveModulusReece() const;

	void SetFrictionalModulusReece(double Value);
	double GetFrictionalModulusReece() const;

	void SetMassDensity(double Value);
	double GetMassDensity() const;

	void SetMaximumNormalStressAngleParameterA(double Value);
	double GetMaximumNormalStressAngleParameterA() const;

	void SetMaximumNormalStressAngleParameterB(double Value);
	double GetMaximumNormalStressAngleParameterB() const;

	void SetRearAngleParameterA(double Value);
	double GetRearAngleParameterA() const;

	void SetRearAngleParameterB(double Value);
	double GetRearAngleParameterB() const;


	bool HasNative() const;
	FTerrainWheelMaterialPtr* GetNative();
	const FTerrainWheelMaterialPtr* GetNative() const;
	void ReleaseNative();

private:
	std::shared_ptr<FTerrainWheelMaterialPtr> NativePtr;
};
