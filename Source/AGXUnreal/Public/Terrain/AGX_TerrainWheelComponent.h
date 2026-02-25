// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_NativeOwner.h"
#include "AGX_RigidBodyReference.h"
#include "Terrain/TerrainWheelBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "AGX_TerrainWheelComponent.generated.h"

struct FAGX_ImportContext;

UCLASS(ClassGroup = "AGX_Terrain", meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_TerrainWheelComponent : public UActorComponent, public IAGX_NativeOwner
{
	GENERATED_BODY()

public:
	UAGX_TerrainWheelComponent();

	/**
	 * Reference to the Rigid Body to be used for this Terrain Wheel Component.
	 * This Rigid Body must contain a Cylinder Shape to act as the contacting Shape of this wheel.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX Terrain Wheel", Meta = (ExposeOnSpawn))
	FAGX_RigidBodyReference RigidBody;

	/**
	 * Determines whether this Terrain Wheeel will deform the Terrain it is in contact with.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Wheel")
	bool bEnableTerrainDeformation {true};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	void SetTerrainDeformationEnabled(bool InEnable);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	bool IsTerrainDeformationEnabled() const;

	/**
	 * Determines whether this Terrain Wheeel will displace Terrain soil (to create ridges).
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Wheel")
	bool bEnableTerrainDisplacement {true};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	void SetTerrainDisplacementEnabled(bool InEnable);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	bool IsTerrainDisplacementEnabled() const;

	/**
	 * Longitudinal velocity threshold used by the slip-ratio dead-band [cm/s].
	 * The slip ratio is clamped to zero when both |vX| and |omegaY * radius| are below their
	 * respective thresholds.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Wheel")
	double SlipRatioVxThreshold {1.0};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	void SetSlipRatioVxThreshold(double InThreshold);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	double GetSlipRatioVxThreshold() const;

	/**
	 * Tangential surface-speed threshold used by the slip-ratio dead-band [cm/s].
	 * This corresponds to |omegaY * radius| in the slip-ratio logic.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Wheel")
	double SlipRatioOmegaYRThreshold {1.0};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	void SetSlipRatioOmegaYRThreshold(double InThreshold);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	double GetSlipRatioOmegaYRThreshold() const;

	/**
	 * Minimum velocity scale used to smooth slip-ratio computation [cm/s].
	 *
	 * At very low wheel speeds this value is used to attenuate the slip-ratio expression
	 * to improve numerical stability close to standstill.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Wheel")
	double SlipRatioSmoothingSpeed {0.01};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	void SetSlipRatioSmoothingSpeed(double InSpeed);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	double GetSlipRatioSmoothingSpeed() const;

	/**
	 * If set to true, the size of the wheel is used as a basis for the
	 * scaling of the regression plane used by the Terrain Wheel.
	 * If set to false, te element size of the terrain is used instead.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Wheel")
	bool bRegressionPlaneStepSizeScaleUsingWheel {false};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	void SetRegressionPlaneStepSizeScaleUsingWheelEnabled(bool InEnable);

	/**
	 * Enable or disable computation of the rear contact angle from the front contact angle.
	 *
	 * When enabled, the rear contact angle theta_r is not computed from the wheel-terrain
	 * geometry directly. Instead it is derived from the current front contact angle
	 * theta_f using an empirical slip-dependent relation
	 * (see computeRearAngleFromFrontAngle()).
	 * This can be useful when a simplified or more stable trailing-edge estimate is desired.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Wheel")
	bool bEnableComputeRearAngleFromFrontAngle {false};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	void SetComputeRearAngleFromFrontAngleEnabled(bool InEnable);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	bool IsComputeRearAngleFromFrontAngleEnabled() const;

	/**
	 * Determines whether detailed debug rendering in AGX for this Terrain Wheel is active. This
	 * will be visible in the AGX Web Debugger.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Wheel", AdvancedDisplay)
	bool bEnableAGXDebugRendering {false};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel")
	void SetAGXDebugRenderingEnabled(bool InEnable);

	UPROPERTY(EditAnywhere, Category = "Rendering")
	bool Visible {true};

	/*
	 * The import Guid of this Component. Only used by the AGX Dynamics for Unreal import system.
	 * Should never be assigned manually.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Dynamics Import")
	FGuid ImportGuid;

	/*
	 * The import name of this Component. Only used by the AGX Dynamics for Unreal import system.
	 * Should never be assigned manually.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Dynamics Import")
	FString ImportName;

	void CopyFrom(const FTerrainWheelBarrier& Barrier, FAGX_ImportContext* Context);

	// ~Begin UObject interface.
	virtual void PostInitProperties() override;
#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
#endif
	// ~End UObject interface.

	//~ Begin ActorComponent Interface
	virtual void OnRegister() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual TStructOnScope<FActorComponentInstanceData> GetComponentInstanceData() const override;
	// ~End UActorComponent interface.

	// ~Begin AGX NativeOwner interface.
	virtual bool HasNative() const override;
	virtual uint64 GetNativeAddress() const override;
	virtual void SetNativeAddress(uint64 NativeAddress) override;
	// ~/End IAGX_NativeOwner interface.

	/// Get the native AGX Dynamics representation of this TerrainWheel. Create it if necessary.
	FTerrainWheelBarrier* GetOrCreateNative();

	/// Return the native AGX Dynamics representation of this TerrainWheel. May return nullptr.
	FTerrainWheelBarrier* GetNative();

	/// Return the native AGX Dynamics representation of this TerrainWheel. May return nullptr.
	const FTerrainWheelBarrier* GetNative() const;

private:
#if WITH_EDITOR
	void InitPropertyDispatcher();
	virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif

	// To allow usage of Dispatcher macro.
	void SetEnableTerrainDisplacement(bool InEnable);
	void SetEnableTerrainDeformation(bool InEnable);
	void SetRegressionPlaneStepSizeScaleUsingWheel(bool InEnable);
	void SetEnableComputeRearAngleFromFrontAngle(bool InEnable);
	void SetEnableAGXDebugRendering(bool InEnable);

	void CreateNative();

private:
	FTerrainWheelBarrier NativeBarrier;
};
