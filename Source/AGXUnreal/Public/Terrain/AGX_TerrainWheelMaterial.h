// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Terrain/TerrainWheelMaterialBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_TerrainWheelMaterial.generated.h"

struct FAGX_ImportContext;

/**
 * Terrain Wheel Material that can be assigned to a Terrain Wheel Component.
 * Affects the phyiscal behaviour of the Terrain Wheel.
 */
UCLASS(ClassGroup = "AGX_Sensor", Category = "AGX")
class AGXUNREAL_API UAGX_TerrainWheelMaterial : public UObject
{
	GENERATED_BODY()

public:
	bool operator==(const UAGX_TerrainWheelMaterial& Other) const;

	/**
	 * The 'n_0' in n = n_0 + n_1 * slip_ratio, where 'n' is sinkage exponent.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Wheel")
	double SinkageExponentParameterA {1.0};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	void SetSinkageExponentParameterA(double Value);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	double GetSinkageExponentParameterA() const;

	/**
	 * The 'n_1' in n = n_0 + n_1 * slip_ratio, where 'n' is sinkage exponent.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Wheel")
	double SinkageExponentParameterB {0.0};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	void SetSinkageExponentParameterB(double Value);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	double GetSinkageExponentParameterB() const;

	/**
	 * The 'c' in tau_max = c + sigma*tan(phi).
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Wheel")
	double Cohesion {800.0};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	void SetCohesion(double Value);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	double GetCohesion() const;

	/**
	 * The 'phi' in tau = c + sigma*tan(phi).
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Wheel")
	double AngleOfInternalFriction {0.611};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	void SetAngleOfInternalFriction(double Value);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	double GetAngleOfInternalFriction() const;

	/**
	 * The 'a_0' in K_x = a_0 + slip_angle * a_1, where
	 * tau_x = [c + sigma*tan(phi)][1-exp(-j_x/K_x)].
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Wheel")
	double ShearModulusXParameterA {0.036};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	void SetShearModulusXParameterA(double Value);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	double GetShearModulusXParameterA() const;

	/**
	 * The 'a_1' in K_x = a_0 + slip_angle * a_1.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Wheel")
	double ShearModulusXParameterB {0.043};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	void SetShearModulusXParameterB(double Value);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	double GetShearModulusXParameterB() const;

	/**
	 * The 'a_0' in K_y = a_0 + slip_angle * a_1, where
	 * tau_y = [c + sigma*tan(phi)][1-exp(-j_y/K_y)].
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Wheel")
	double ShearModulusYParameterA {0.013};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	void SetShearModulusYParameterA(double Value);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	double GetShearModulusYParameterA() const;

	/**
	 * The 'a_1' in K_y = a_0 + slip_angle * a_1.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Wheel")
	double ShearModulusYParameterB {0.020};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	void SetShearModulusYParameterB(double Value);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	double GetShearModulusYParameterB() const;

	/**
	 * The 'k_c' in p = (k_c/b + k_phi)z^n.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Wheel")
	double CohesiveModulusBekker {1000.0};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	void SetCohesiveModulusBekker(double Value);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	double GetCohesiveModulusBekker() const;

	/**
	 * The 'k_phi' in p = (k_c/b + k_phi)z^n.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Wheel")
	double FrictionalModulusBekker {800000.0};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	void SetFrictionalModulusBekker(double Value);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	double GetFrictionalModulusBekker() const;

	/**
	 * The 'k_c_prime' in
	 * p = (c*k_c_prime + rho*g*b*k_phi_prime)(z/b)^n.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Wheel")
	double CohesiveModulusReece {670.0};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	void SetCohesiveModulusReece(double Value);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	double GetCohesiveModulusReece() const;

	/**
	 * The 'k_phi_prime' in
	 * p = (c*k_c_prime + rho*g*b*k_phi_prime)(z/b)^n.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Wheel")
	double FrictionalModulusReece {200.0};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	void SetFrictionalModulusReece(double Value);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	double GetFrictionalModulusReece() const;

	/**
	 * The 'rho' in
	 * p = (c*k_c_prime + rho*g*b*k_phi_prime)z^n.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Wheel")
	double MassDensity {1500.0};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	void SetMassDensity(double Value);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	double GetMassDensity() const;

	/**
	 * The 'a_0' in theta_m = (a_0 + a_1*slip_ratio) * theta_f.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Wheel")
	double MaximumNormalStressAngleParameterA {0.4};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	void SetMaximumNormalStressAngleParameterA(double Value);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	double GetMaximumNormalStressAngleParameterA() const;

	/**
	 * The 'a_1' in theta_m = (a_0 + a_1*slip_ratio) * theta_f.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Wheel")
	double MaximumNormalStressAngleParameterB {0.15};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	void SetMaximumNormalStressAngleParameterB(double Value);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	double GetMaximumNormalStressAngleParameterB() const;

	/**
	 * The 'a_0' in theta_r = (a_0 + a_1*slip_ratio) * theta_f.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Wheel")
	double RearAngleParameterA {-0.12};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	void SetRearAngleParameterA(double Value);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	double GetRearAngleParameterA() const;

	/**
	 * The 'a_1' in theta_r = (a_0 + a_1*slip_ratio) * theta_f.
	 */
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
	void CopyFrom(const FTerrainWheelMaterialBarrier& Source, FAGX_ImportContext* Context);
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
