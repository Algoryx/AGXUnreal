// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_LensDistortionBase.h"

#include "AGX_LensDistortionBrownConrady.generated.h"

struct FLensDistortionBrownConradyBarrier;

/**
 * AGX five parameter Brown-Conrady lens distortion asset.
 */
UCLASS(ClassGroup = "AGX_Sensor", Category = "AGX", BlueprintType)
class AGXUNREAL_API UAGX_LensDistortionBrownConrady : public UAGX_LensDistortionBase
{
	GENERATED_BODY()

public:
	/**
	 * First radial distortion coefficient.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Lens Distortion")
	double K1 {0.0};

	UFUNCTION(BlueprintCallable, Category = "AGX Lens Distortion")
	void SetK1(double InK1);

	UFUNCTION(BlueprintCallable, Category = "AGX Lens Distortion")
	double GetK1() const;

	/**
	 * Second radial distortion coefficient.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Lens Distortion")
	double K2 {0.0};

	UFUNCTION(BlueprintCallable, Category = "AGX Lens Distortion")
	void SetK2(double InK2);

	UFUNCTION(BlueprintCallable, Category = "AGX Lens Distortion")
	double GetK2() const;

	/**
	 * Third radial distortion coefficient.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Lens Distortion")
	double K3 {0.0};

	UFUNCTION(BlueprintCallable, Category = "AGX Lens Distortion")
	void SetK3(double InK3);

	UFUNCTION(BlueprintCallable, Category = "AGX Lens Distortion")
	double GetK3() const;

	/**
	 * First tangential distortion coefficient.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Lens Distortion")
	double P1 {0.0};

	UFUNCTION(BlueprintCallable, Category = "AGX Lens Distortion")
	void SetP1(double InP1);

	UFUNCTION(BlueprintCallable, Category = "AGX Lens Distortion")
	double GetP1() const;

	/**
	 * Second tangential distortion coefficient.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Lens Distortion")
	double P2 {0.0};

	UFUNCTION(BlueprintCallable, Category = "AGX Lens Distortion")
	void SetP2(double InP2);

	UFUNCTION(BlueprintCallable, Category = "AGX Lens Distortion")
	double GetP2() const;

	FLensDistortionBrownConradyBarrier* GetNativeAsBrownConrady();
	const FLensDistortionBrownConradyBarrier* GetNativeAsBrownConrady() const;

	virtual void CopyProperties(const UAGX_LensDistortionBase& Source) override;

	// ~Begin UObject interface.
	virtual void PostInitProperties() override;
#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
#endif
	// ~End UObject interface.

private:
#if WITH_EDITOR
	void InitPropertyDispatcher();
#endif

	void UpdateNativeProperties();

protected:
	virtual void CreateNative() override;
};
