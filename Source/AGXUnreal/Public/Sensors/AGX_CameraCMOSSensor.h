// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_CameraPhotodetectorBase.h"

#include "AGX_CameraCMOSSensor.generated.h"

struct FCameraCMOSSensorBarrier;

/**
 * AGX Camera CMOS Sensor photodetector asset.
 */
UCLASS(ClassGroup = "AGX_Sensor", Category = "AGX", BlueprintType)
class AGXUNREAL_API UAGX_CameraCMOSSensor : public UAGX_CameraPhotodetectorBase
{
	GENERATED_BODY()

public:
	/**
	 * Rectangular physical size of the CMOS sensor in the camera XY-plane [cm].
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Camera CMOS Sensor", Meta = (ClampMin = "0.0"))
	FVector2D Size {0.27288, 0.15498};

	UFUNCTION(BlueprintCallable, Category = "AGX Camera CMOS Sensor")
	void SetSize(FVector2D InSize);

	UFUNCTION(BlueprintCallable, Category = "AGX Camera CMOS Sensor")
	FVector2D GetSize() const;

	/**
	 * Sensor amplifier ISO value.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Camera CMOS Sensor", Meta = (ClampMin = "0.0"))
	double ISO {100.0};

	UFUNCTION(BlueprintCallable, Category = "AGX Camera CMOS Sensor")
	void SetISO(double InISO);

	UFUNCTION(BlueprintCallable, Category = "AGX Camera CMOS Sensor")
	double GetISO() const;

	/**
	 * Shutter speed of the sensor electronic shutter [s].
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Camera CMOS Sensor", Meta = (ClampMin = "0.0"))
	double ShutterSpeed {5.6e-3};

	UFUNCTION(BlueprintCallable, Category = "AGX Camera CMOS Sensor")
	void SetShutterSpeed(double InShutterSpeed);

	UFUNCTION(BlueprintCallable, Category = "AGX Camera CMOS Sensor")
	double GetShutterSpeed() const;

	/**
	 * Whether to use automatic exposure control. If false, manual exposure compensation is used.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Camera CMOS Sensor")
	bool bUseAutoExposure {true};

	UFUNCTION(BlueprintCallable, Category = "AGX Camera CMOS Sensor")
	void SetUseAutoExposure(bool bInUseAutoExposure);

	UFUNCTION(BlueprintCallable, Category = "AGX Camera CMOS Sensor")
	bool GetUseAutoExposure() const;

	/**
	 * Maximum exposure value used by auto exposure.
	 * Only used when bUseAutoExposure is true.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Camera CMOS Sensor",
		Meta = (ClampMin = "0.0", EditCondition = "bUseAutoExposure"))
	double DynamicRange {11.3};

	UFUNCTION(BlueprintCallable, Category = "AGX Camera CMOS Sensor")
	void SetDynamicRange(double InDynamicRange);

	UFUNCTION(BlueprintCallable, Category = "AGX Camera CMOS Sensor")
	double GetDynamicRange() const;

	/**
	 * Exposure compensation value for manual exposure control.
	 * Only used when bUseAutoExposure is false.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Camera CMOS Sensor",
		Meta = (EditCondition = "!bUseAutoExposure"))
	double ExposureCompensation {0.0};

	UFUNCTION(BlueprintCallable, Category = "AGX Camera CMOS Sensor")
	void SetExposureCompensation(double InExposureCompensation);

	UFUNCTION(BlueprintCallable, Category = "AGX Camera CMOS Sensor")
	double GetExposureCompensation() const;

	FCameraCMOSSensorBarrier* GetNativeAsCMOSSensor();
	const FCameraCMOSSensorBarrier* GetNativeAsCMOSSensor() const;

	virtual void CopyProperties(const UAGX_CameraPhotodetectorBase& Source) override;

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
