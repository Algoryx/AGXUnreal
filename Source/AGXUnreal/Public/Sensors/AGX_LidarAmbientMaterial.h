// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/RtAmbientMaterialBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_LidarAmbientMaterial.generated.h"

/**
 * Lidar Ambient Material that can be assigned to a Sensor Environment.
 * Used to simulate different weather conditions such as fog or rain.
 */
UCLASS(ClassGroup = "AGX_Sensor", Category = "AGX")
class AGXUNREAL_API UAGX_LidarAmbientMaterial : public UObject
{
	GENERATED_BODY()

public:
	bool operator==(const UAGX_LidarAmbientMaterial& Other) const;

	UPROPERTY(EditAnywhere, Category = "AGX Lidar")
	float RefractiveIndex {1.000273f};

	UFUNCTION(BlueprintCallable, Category = "AGX Lidar")
	void SetRefractiveIndex(float InRefractiveIndex);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Lidar")
	float GetRefractiveIndex() const;

	UPROPERTY(EditAnywhere, Category = "AGX Lidar")
	float AttenuationCoefficient {0.000402272f};

	UFUNCTION(BlueprintCallable, Category = "AGX Lidar")
	void SetAttenuationCoefficient(float InAttenuationCoefficient);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Lidar")
	float GetAttenuationCoefficient() const;

	UPROPERTY(EditAnywhere, Category = "AGX Lidar")
	float ReturnProbabilityScaling {1.58899e-05f};

	UFUNCTION(BlueprintCallable, Category = "AGX Lidar")
	void SetReturnProbabilityScaling(float InScalingParameter);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Lidar")
	float GetReturnProbabilityScaling() const;

	UPROPERTY(EditAnywhere, Category = "AGX Lidar")
	float ReturnGammaDistributionShapeParameter {9.5f};

	UFUNCTION(BlueprintCallable, Category = "AGX Lidar")
	void SetReturnGammaDistributionShapeParameter(float InShapeParameter);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Lidar")
	float GetReturnGammaDistributionShapeParameter() const;

	UPROPERTY(EditAnywhere, Category = "AGX Lidar")
	float ReturnGammaDistributionScaleParameter {0.52f};

	UFUNCTION(BlueprintCallable, Category = "AGX Lidar")
	void SetReturnGammaDistributionScaleParameter(float InScaleParameter);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Lidar")
	float GetReturnGammaDistributionScaleParameter() const;

	/**
	 * Configure this Material as Air with the set visibility in kilometers. The Material parameters
	 * will update accordingly.
	 * This function only works during Play. To make permanent changes to a Lidar Ambient Material
	 * Asset, use the helpers in the Lidar Ambient Material Asset Editor.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Lidar", Meta = (ClampMin = "0.0"))
	bool ConfigureAsAir(float VisibilityKm);

	/**
	 * Configure this Material as Fog with the set visibility in kilometers and Lidar wavelength in
	 * nanometers. The Material parameters will update accordingly.
	 * This function only works during Play. To make permanent changes to a Lidar Ambient Material
	 * Asset, use the helpers in the Lidar Ambient Material Asset Editor.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Lidar", Meta = (ClampMin = "0.0"))
	bool ConfigureAsFog(float VisibilityKm, float WavelengthNm);

	/**
	 * Configure this Material to simulate rainfall based on the rain rate in millimeters per hour.
	 * This function only works during Play. To make permanent changes to a Lidar Ambient Material
	 * Asset, use the helpers in the Lidar Ambient Material Asset Editor.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Lidar", Meta = (ClampMin = "0.0"))
	bool ConfigureAsRainfall(float RateMmPerHour);

	/**
	 * Configure this Material to simulate snowfall with snowfall rate in millimeters per hour and
	 * Lidar wavelength in nanometers.
	 * This function only works during Play. To make permanent changes to a Lidar Ambient Material
	 * Asset, use the helpers in the Lidar Ambient Material Asset Editor.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Lidar", Meta = (ClampMin = "0.0"))
	bool ConfigureAsSnowfall(float RateMmPerHour, float WavelengthNm);

	bool HasNative() const;
	FRtAmbientMaterialBarrier* GetNative();
	const FRtAmbientMaterialBarrier* GetNative() const;
	void ReleaseNative();

	void CommitToAsset();

	static UAGX_LidarAmbientMaterial* CreateInstanceFromAsset(
		UWorld* PlayingWorld, UAGX_LidarAmbientMaterial& Source);

	UAGX_LidarAmbientMaterial* GetOrCreateInstance(UWorld* PlayingWorld);

	FRtAmbientMaterialBarrier* GetOrCreateNative();

	void UpdateNativeProperties();

	bool IsInstance() const;

	void CopyFrom(const FRtAmbientMaterialBarrier& Source);
	void CopyProperties(const UAGX_LidarAmbientMaterial& Source);

private:
	void CreateNative();

#if WITH_EDITOR
	virtual void PostInitProperties() override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
	void InitPropertyDispatcher();
#endif

private:
	TWeakObjectPtr<UAGX_LidarAmbientMaterial> Asset;
	TWeakObjectPtr<UAGX_LidarAmbientMaterial> Instance;
	FRtAmbientMaterialBarrier NativeBarrier;
};
