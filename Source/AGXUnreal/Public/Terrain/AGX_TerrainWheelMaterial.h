// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Terrain/TerrainWheelMaterialBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_TerrainWheelMaterial.generated.h"

/**
 * Terrain Wheel Material that can be assigned to a Terrain Wheel Component.
 * Affects the phyiscal behaviour of the Terrain Wheel.
 */
UCLASS(ClassGroup = "AGX_Sensor", Category = "AGX")
class AGXUNREAL_API UAGX_TerrainWheelMaterial : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, Category = "AGX Terrain Wheel")
	double SinkageExponentParameterA {1.0};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	void SetSinkageExponentParameterA(double Value);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	double GetSinkageExponentParameterA() const;

	UPROPERTY(EditAnywhere, Category = "AGX Terrain Wheel")
	double SinkageExponentParameterB {0.0};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	void SetSinkageExponentParameterB(double Value);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	double GetSinkageExponentParameterB() const;

	UPROPERTY(EditAnywhere, Category = "AGX Terrain Wheel")
	double Cohesion {800.0};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	void SetCohesion(double Value);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	double GetCohesion() const;

	UPROPERTY(EditAnywhere, Category = "AGX Terrain Wheel")
	double AngleOfInternalFriction {0.611};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	void SetAngleOfInternalFriction(double Value);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	double GetAngleOfInternalFriction() const;

	UPROPERTY(EditAnywhere, Category = "AGX Terrain Wheel")
	double ShearModulusXParameterA {0.036};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	void SetShearModulusXParameterA(double Value);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	double GetShearModulusXParameterA() const;

	UPROPERTY(EditAnywhere, Category = "AGX Terrain Wheel")
	double ShearModulusXParameterB {0.043};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	void SetShearModulusXParameterB(double Value);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	double GetShearModulusXParameterB() const;

	UPROPERTY(EditAnywhere, Category = "AGX Terrain Wheel")
	double ShearModulusYParameterA {0.013};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	void SetShearModulusYParameterA(double Value);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	double GetShearModulusYParameterA() const;

	UPROPERTY(EditAnywhere, Category = "AGX Terrain Wheel")
	double ShearModulusYParameterB {0.020};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	void SetShearModulusYParameterB(double Value);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	double GetShearModulusYParameterB() const;

	UPROPERTY(EditAnywhere, Category = "AGX Terrain Wheel")
	double CohesiveModulusBekker {1000.0};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	void SetCohesiveModulusBekker(double Value);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	double GetCohesiveModulusBekker() const;

	UPROPERTY(EditAnywhere, Category = "AGX Terrain Wheel")
	double FrictionalModulusBekker {800000.0};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	void SetFrictionalModulusBekker(double Value);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	double GetFrictionalModulusBekker() const;

	UPROPERTY(EditAnywhere, Category = "AGX Terrain Wheel")
	double CohesiveModulusReece {670.0};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	void SetCohesiveModulusReece(double Value);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	double GetCohesiveModulusReece() const;

	UPROPERTY(EditAnywhere, Category = "AGX Terrain Wheel")
	double FrictionalModulusReece {200.0};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	void SetFrictionalModulusReece(double Value);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	double GetFrictionalModulusReece() const;

	UPROPERTY(EditAnywhere, Category = "AGX Terrain Wheel")
	double MassDensity {1500.0};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	void SetMassDensity(double Value);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	double GetMassDensity() const;

	UPROPERTY(EditAnywhere, Category = "AGX Terrain Wheel")
	double MaximumNormalStressAngleParameterA {0.4};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	void SetMaximumNormalStressAngleParameterA(double Value);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	double GetMaximumNormalStressAngleParameterA() const;

	UPROPERTY(EditAnywhere, Category = "AGX Terrain Wheel")
	double MaximumNormalStressAngleParameterB {0.15};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	void SetMaximumNormalStressAngleParameterB(double Value);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	double GetMaximumNormalStressAngleParameterB() const;

	UPROPERTY(EditAnywhere, Category = "AGX Terrain Wheel")
	double RearAngleParameterA {-0.12};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	void SetRearAngleParameterA(double Value);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	double GetRearAngleParameterA() const;

	UPROPERTY(EditAnywhere, Category = "AGX Terrain Wheel")
	double RearAngleParameterB {0.0};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	void SetRearAngleParameterB(double Value);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	double GetRearAngleParameterB() const;


	bool HasNative() const;
	FTerrainWheelMaterialBarrier* GetNative();
	const FTerrainWheelMaterialBarrier* GetNative() const;
	void ReleaseNative();

	void CommitToAsset();

	static UAGX_TerrainWheelMaterial* CreateInstanceFromAsset(
		UWorld* PlayingWorld, UAGX_TerrainWheelMaterial& Source);

	UAGX_TerrainWheelMaterial* GetOrCreateInstance(UWorld* PlayingWorld);

	FTerrainWheelMaterialBarrier* GetOrCreateNative();

	void UpdateNativeProperties();

	bool IsInstance() const;

	void CopyFrom(const FTerrainWheelMaterialBarrier& Source);
	void CopyProperties(const UAGX_TerrainWheelMaterial& Source);

private:
	void CreateNative();

#if WITH_EDITOR
	virtual void PostInitProperties() override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
	void InitPropertyDispatcher();
#endif

private:
	TWeakObjectPtr<UAGX_TerrainWheelMaterial> Asset;
	TWeakObjectPtr<UAGX_TerrainWheelMaterial> Instance;
	FTerrainWheelMaterialBarrier NativeBarrier;
};
