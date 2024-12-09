// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_NativeOwner.h"
#include "AGX_RealInterval.h"
#include "Sensors/AGX_CustomPatternFetcher.h"
#include "Sensors/AGX_DistanceGaussianNoiseSettings.h"
#include "Sensors/AGX_LidarEnums.h"
#include "Sensors/AGX_LidarModelParameters.h"
#include "Sensors/AGX_RayAngleGaussianNoiseSettings.h"
#include "Sensors/LidarBarrier.h"

// Unreal Engine includes.
#include "Components/SceneComponent.h"
#include "CoreMinimal.h"
#include "Math/UnrealMathUtility.h"

#include "AGX_LidarSensorComponent.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;
class UTextureRenderTarget2D;

struct FAGX_LidarOutputBase;
struct FAGX_SensorMsgsPointCloud2;

DECLARE_DYNAMIC_DELEGATE_RetVal(TArray<FTransform>, FOnFetchRayTransforms);
DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(
	FAGX_CustomPatternInterval, FOnFetchNextPatternInterval, double, TimeStamp);

/**
 * Lidar Sensor Component, allowing to create point clouds at runtime.
 */
UCLASS(
	ClassGroup = "AGX_Sensor", Category = "AGX", Meta = (BlueprintSpawnableComponent),
	Hidecategories = (Cooking, Collision, LOD, Physics, Rendering, Replication))
class AGXUNREAL_API UAGX_LidarSensorComponent : public USceneComponent, public IAGX_NativeOwner
{
	GENERATED_BODY()

public:
	UAGX_LidarSensorComponent();

	/**
	 * The Model, or preset, of this Lidar.
	 * Changing this will assign Model specific properties to this Lidar.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Lidar", Meta = (ExposeOnSpawn))
	EAGX_LidarModel Model {EAGX_LidarModel::OusterOS1};

	UFUNCTION(BlueprintCallable, Category = "AGX Lidar")
	void SetModel(EAGX_LidarModel InModel);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Lidar")
	EAGX_LidarModel GetModel() const;

	/**
	 * Enable or disable this Lidar Sensor Component. If disabled, it will not perform raytracing
	 * and will thus not produce any output data.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Lidar")
	bool bEnabled {true};

	UFUNCTION(BlueprintCallable, Category = "AGX Lidar")
	void SetEnabled(bool InEnabled);

	UFUNCTION(BlueprintCallable, Category = "AGX Lidar")
	bool GetEnabled() const;

	// clang-format off
	/**
	 * The minimum and maximum range of the Lidar Sensor [cm].
	 * Objects outside this range will not be detected by this Lidar Sensor.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX Lidar",		
		Meta = (ClampMin = "0.0", EditCondition = "Model == EAGX_LidarModel::CustomRayPattern || Model == EAGX_LidarModel::GenericHorizontalSweep"))	
	FAGX_RealInterval Range {0.0, 10000.0};
	// clang-format on

	UFUNCTION(BlueprintCallable, Category = "AGX Lidar")
	void SetRange(FAGX_RealInterval InRange);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Lidar")
	FAGX_RealInterval GetRange() const;

	// clang-format off
	/**
	 * Divergence of the lidar laser light beam [deg].
	 * This the total "cone angle", i.e. the angle between a perfectly parallel beam of the same
	 * exit dimater to the cone surface is half this angle.
	 * This property affects the calculated intensity.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX Lidar",
		Meta = (ClampMin = "0.0", EditCondition = "Model == EAGX_LidarModel::CustomRayPattern || Model == EAGX_LidarModel::GenericHorizontalSweep"))
	FAGX_Real BeamDivergence {0.001 * 180.0 / PI};
	// clang-format on

	UFUNCTION(BlueprintCallable, Category = "AGX Lidar")
	void SetBeamDivergence(double InBeamDivergence);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Lidar")
	double GetBeamDivergence() const;

	// clang-format off
	/**
	 * The diameter of the lidar laser light beam as it exits the lidar [cm].
	 * This property affects the calculated intensity.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX Lidar",
		Meta = (ClampMin = "0.0", EditCondition = "Model == EAGX_LidarModel::CustomRayPattern || Model == EAGX_LidarModel::GenericHorizontalSweep"))
	FAGX_Real BeamExitRadius {0.5};
	// clang-format on

	UFUNCTION(BlueprintCallable, Category = "AGX Lidar")
	void SetBeamExitRadius(double InBeamExitRadius);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Lidar")
	double GetBeamExitRadius() const;

	/**
	 * Model Parameters used when creating this Lidar Sensor from a Lidar Model.
	 * The type of the Model Parameters asset must match the selected Lidar Model.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Lidar", Meta = (ExposeOnSpawn))
	UAGX_LidarModelParameters* ModelParameters {nullptr};

	/**
	 * Determines the number of maximum raytrace steps.
	 * The number of steps is one more than the number of bounces a ray will make,
	 * however, this number will generally not affect the size of the output data.
	 * It should be noted that the time and memory complexity of the raytrace will grow
	 * exponentially with the maximum number of raytrace steps.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Lidar", Meta = (ClampMin = "1"))
	int32 RaytraceDepth {1};

	UFUNCTION(BlueprintCallable, Category = "AGX Lidar")
	void SetRaytraceDepth(int32 Depth);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Lidar")
	int32 GetRaytraceDepth() const;

	/**
	 * Enables or disables removal of point misses, i.e. makes the output dense if set to true.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Lidar")
	bool bEnableRemovePointsMisses {true};

	UFUNCTION(BlueprintCallable, Category = "AGX Lidar")
	void SetEnableRemovePointsMisses(bool bEnable);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Lidar")
	bool GetEnableRemovePointsMisses() const;

	// clang-format off
	/**
	 * Enables or disables distance gaussian noise, adding an individual distance error to each
	 * measurements of Position.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX Lidar",
		Meta = (EditCondition = "Model == EAGX_LidarModel::CustomRayPattern || Model == EAGX_LidarModel::GenericHorizontalSweep"))
	bool bEnableDistanceGaussianNoise {false};
	// clang-format on

	UFUNCTION(BlueprintCallable, Category = "AGX Lidar")
	void SetEnableDistanceGaussianNoise(bool bEnable);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Lidar")
	bool GetEnableDistanceGaussianNoise() const;

	// clang-format off
	/**
	 * Determines the distance noise characteristics. The standard deviation is calculated as
	 * s = stdDev + d * stdDevSlope where d is the distance in centimeters.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX Lidar",
		Meta = (ClampMin = "0.0", EditCondition = "bEnableDistanceGaussianNoise"))
	FAGX_DistanceGaussianNoiseSettings DistanceNoiseSettings;
	// clang-format on

	UFUNCTION(BlueprintCallable, Category = "AGX Lidar")
	void SetDistanceNoiseSettings(FAGX_DistanceGaussianNoiseSettings Settings);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Lidar")
	FAGX_DistanceGaussianNoiseSettings GetDistanceNoiseSettings() const;

	// clang-format off
	/**
	 * Enables or disables angle ray gaussian noise, adding an individual angle error to each lidar
	 * ray.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX Lidar",
		Meta = (EditCondition = "Model == EAGX_LidarModel::CustomRayPattern || Model == EAGX_LidarModel::GenericHorizontalSweep"))
	bool bEnableRayAngleGaussianNoise {false};
	// clang-format on

	UFUNCTION(BlueprintCallable, Category = "AGX Lidar")
	void SetEnableRayAngleGaussianNoise(bool bEnable);

	UFUNCTION(BlueprintCallable, Category = "AGX Lidar")
	bool GetEnableRayAngleGaussianNoise() const;

	// clang-format off
	/**
	 * Determines the lidar ray noise characteristics.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX Lidar",
		Meta = (ClampMin = "0.0", EditCondition = "bEnableRayAngleGaussianNoise"))	
	FAGX_RayAngleGaussianNoiseSettings RayAngleNoiseSettings;
	// clang-format on

	UFUNCTION(BlueprintCallable, Category = "AGX Lidar")
	void SetRayAngleNoiseSettings(FAGX_RayAngleGaussianNoiseSettings Settings);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Lidar")
	FAGX_RayAngleGaussianNoiseSettings GetRayAngleNoiseSettings() const;

	/**
	 * Delegate that has to be assigned (bound to) by the user to support custom scan pattern.
	 * Only used if the Lidar Model is set to CustomRayPattern.
	 * Should return all ray transforms (lidar local coordinates) for the whole scan pattern.
	 * Rays are directed along the z-axis of the given transforms.
	 * This delegate is called only if no ray transforms has previously been returned by the
	 * funcion provided by the user, i.e. under normal conditions, it is called only once. The
	 * FetchNextPatternInterval is called each Step Forward and determines what part of the scan
	 * pattern to use next, see OnFetchNextPatternInterval. The signature of the function assigned
	 * must be: TArray<FTransform> MyFunc().
	 */
	UPROPERTY(
		BlueprintReadWrite, Category = "AGX Lidar",
		Meta = (EditCondition = "ScanPattern == EAGX_LidarRayPattern::Custom"))
	FOnFetchRayTransforms OnFetchRayTransforms;

	/**
	 * Delegate that has to be assigned (bound to) by the user to support custom scan pattern.
	 * Only used if the Lidar Model is set to CustomRayPattern.
	 * Should return the next AGX Custom Pattern Interval to use.
	 * This delegate is called each Step Forward and determines what part of the total scan pattern
	 * to use in that Step Forward. See also OnFetchRayTransforms. The signature of the function
	 * assigned must be: FAGX_CustomPatternInterval MyFunc(double TimeStamp).
	 */
	UPROPERTY(
		BlueprintReadWrite, Category = "AGX Lidar",
		Meta = (EditCondition = "ScanPattern == EAGX_LidarRayPattern::Custom"))
	FOnFetchNextPatternInterval OnFetchNextPatternInterval;

	/** Whether lidar data rendering should be enabled or not. */
	UPROPERTY(EditAnywhere, Category = "AGX Lidar")
	bool bEnableRendering {true};

	UPROPERTY(EditAnywhere, Category = "AGX Lidar", Meta = (EditCondition = "bEnableRendering"))
	UNiagaraSystem* NiagaraSystemAsset {nullptr};

	/**
	 * If a Niagara System Component has been spawned by the Lidar, this function will return
	 * it. Returns nullptr otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Lidar")
	UNiagaraComponent* GetSpawnedNiagaraSystemComponent();

	void UpdateNativeTransform();

	bool AddOutput(FAGX_LidarOutputBase& InOutput);

	bool IsCustomParametersSupported() const;

	FLidarBarrier* GetOrCreateNative();
	FLidarBarrier* GetNative();
	const FLidarBarrier* GetNative() const;

	// ~Begin AGX NativeOwner interface.
	virtual bool HasNative() const override;
	virtual uint64 GetNativeAddress() const override;
	virtual void SetNativeAddress(uint64 NativeAddress) override;
	// ~/End IAGX_NativeOwner interface.

	void CopyFrom(const UAGX_LidarSensorComponent& Source);

	//~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	virtual void DestroyComponent(bool bPromoteChildren) override;
	virtual TStructOnScope<FActorComponentInstanceData> GetComponentInstanceData() const override;
#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif
	//~ End UActorComponent Interface

	//~ Begin UObject interface.
#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
	virtual void PostInitProperties() override;
#endif
	//~ End UObject interface.

	friend class FAGX_CustomPatternFetcher;

private:
#if WITH_EDITOR
	void InitPropertyDispatcher();
#endif

	void UpdateNativeProperties();

	TArray<FTransform> FetchRayTransforms();
	FAGX_CustomPatternInterval FetchNextInterval();

	UNiagaraComponent* NiagaraSystemComponent = nullptr;
	FAGX_CustomPatternFetcher PatternFetcher;
	FLidarBarrier NativeBarrier;
};
