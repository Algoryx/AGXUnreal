// Copyright 2024, Algoryx Simulation AB.

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
