// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Terrain/TerrainWheelSettingsBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "AGX_TerrainWheelSettings.generated.h"

struct FAGX_ImportContext;
struct FTerrainWheelBarrier;

/**
 * Contains configuration properties for AGX Terrain Wheel. Several Terrain Wheel Components can
 * share the same Terrain Wheel Settings.
 */
UCLASS(ClassGroup = "AGX_Terrain", Category = "AGX", BlueprintType)
class AGXUNREAL_API UAGX_TerrainWheelSettings : public UObject
{
	GENERATED_BODY()

public:
	UAGX_TerrainWheelSettings() = default;

	/**
	 * Longitudinal velocity threshold used by the slip-ratio dead-band [cm/s].
	 * The slip ratio is clamped to zero when both |vX| and |omegaY * radius| are below their
	 * respective thresholds.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Wheel Settings")
	double SlipRatioVxAngularEquivalentThreshold {1.0};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel Settings")
	void SetSlipRatioVxAngularEquivalentThreshold(double InThreshold);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel Settings")
	double GetSlipRatioVxAngularEquivalentThreshold() const;

	/**
	 * Tangential surface-speed threshold used by the slip-ratio dead-band [cm/s].
	 * This corresponds to |omegaY * radius| in the slip-ratio logic.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Wheel Settings")
	double SlipRatioOmegaYThreshold {1.0};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel Settings")
	void SetSlipRatioOmegaYThreshold(double InThreshold);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel Settings")
	double GetSlipRatioOmegaYThreshold() const;

	/**
	 * Minimum velocity scale used to smooth slip-ratio computation [cm/s].
	 *
	 * At very low wheel speeds this value is used to attenuate the slip-ratio expression
	 * to improve numerical stability close to standstill.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Wheel Settings")
	double SlipRatioSmoothingAngularSpeed {0.01};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel Settings")
	void SetSlipRatioSmoothingAngularSpeed(double InSpeed);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel Settings")
	double GetSlipRatioSmoothingAngularSpeed() const;

	/**
	 * Enable or disable computation of the rear contact angle from the front contact
	 * angle.
	 *
	 * When enabled, the rear contact angle theta_r is not computed from the
	 * wheel-terrain geometry directly. Instead it is derived from the current front contact angle
	 * theta_f using an empirical slip-dependent relation
	 * (see computeRearAngleFromFrontAngle()).
	 * This can be useful when a simplified or more stable trailing-edge estimate is desired.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Wheel Settings")
	bool bEnableComputeRearAngleFromFrontAngle {false};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel Settings")
	void SetEnableComputeRearAngleFromFrontAngle(bool InEnable);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel Settings")
	bool GetEnableComputeRearAngleFromFrontAngle() const;

	/**
	 * Determines whether detailed debug rendering in AGX for this Terrain Wheel is active. This
	 * will be visible in the AGX Web Debugger.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Wheel Settings", AdvancedDisplay)
	bool bEnableAGXDebugRendering {false};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel Settings")
	void SetEnableAGXDebugRendering(bool InEnable);

	/*
	 * The import Guid of this Asset. Only used by the AGX Dynamics for Unreal import system.
	 * Should never be assigned manually.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Dynamics Import")
	FGuid ImportGuid;

	/**
	 * Copy property values from the runtime instance to the Terrain Wheel Settings asset the
	 * instance was created from.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel Settings")
	void CommitToAsset();

	void CopyFrom(const FTerrainWheelBarrier& Source, FAGX_ImportContext* Context);
	void CopyFrom(const FTerrainWheelSettingsBarrier& Source);
	void CopyFrom(const UAGX_TerrainWheelSettings* Source);

	UAGX_TerrainWheelSettings* GetInstance();

	/**
	 * If PlayingWorld is an in-game World and this TerrainWheelSettings is an asset, returns a
	 * TerrainWheelSettings instance representing the asset throughout the lifetime of the
	 * GameInstance. If this is already an instance it returns itself.
	 */
	UAGX_TerrainWheelSettings* GetOrCreateInstance(const UWorld* PlayingWorld);

	/**
	 * If this TerrainWheelSettings is an instance, returns the asset it was created from. Else
	 * returns itself.
	 */
	UAGX_TerrainWheelSettings* GetAsset();

	bool IsInstance() const;

	bool HasNative() const;
	FTerrainWheelSettingsBarrier* GetNative();
	const FTerrainWheelSettingsBarrier* GetNative() const;
	FTerrainWheelSettingsBarrier* GetOrCreateNative();

	void UpdateNativeProperties();

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

	void CreateNative();

private:
	TWeakObjectPtr<UAGX_TerrainWheelSettings> Asset;
	TWeakObjectPtr<UAGX_TerrainWheelSettings> Instance;
	FTerrainWheelSettingsBarrier NativeBarrier;
};
