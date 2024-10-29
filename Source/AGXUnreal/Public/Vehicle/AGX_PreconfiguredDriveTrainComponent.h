// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_NativeOwner.h"
#include "DriveTrain/CombustionEngineParameters.h"
#include "Constraints/AGX_ConstraintReferences.h"
#include "Vehicle/PreconfiguredDriveTrainBarriers.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "AGX_PreconfiguredDriveTrainComponent.generated.h"

/**
 * A convenience class for creating a drive-train preconfigured for the common case of a
 * four-wheeled vehicle powered by a combustion engine. For other setups it is recommended to create
 * a custom drive-train.
 */
UCLASS(Category = "AGX", ClassGroup = "AGX_Vehicle", Meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_PreconfiguredDriveTrainComponent : public UActorComponent,
															public IAGX_NativeOwner
{
	GENERATED_BODY()

public:
	UAGX_PreconfiguredDriveTrainComponent();
	virtual ~UAGX_PreconfiguredDriveTrainComponent() = default;

	/**
	 * Parameters describing the fixed properties of the combustion engine.
	 */
	UPROPERTY(EditAnywhere, Category = "Combustion Engine")
	FAGX_CombustionEngineParameters CombustionEngineParameters; // TODO Make an asset.

	/**
	 * Current engine throttle. A value between 0.0 and 1.0.
	 */
	UPROPERTY(EditAnywhere, Category = "Combustion Engine")
	FAGX_Real Throttle {0.0};

	/**
	 * Set the engine throttle. A value between 0.0 and 1.0.
	 * @param InThrottle Engine throttle.
	 */
	UFUNCTION(BlueprintCallable, Category = "Preconfigured Drive-Train|Combustion Engine")
	void SetThrottle(double InThrottle);

	/**
	 * Get the current engine throttle. A value between 0.0 and 1.0.
	 * @return The current engine throttle.
	 */
	UFUNCTION(BlueprintCallable, Category = "Preconfigured Drive-Train|Combustion Engine")
	double GetThrottle() const;

	/**
	 * The hinge attaching the front left wheel of the vehicle.
	 */
	UPROPERTY(EditAnywhere, Category = "Wheels")
	FAGX_HingeReference FrontLeftHinge;

	/**
	 * The hinge attaching the front left wheel of the vehicle.
	 */
	UPROPERTY(EditAnywhere, Category = "Wheels")
	FAGX_HingeReference FrontRightHinge;

	/**
	 * Write the Unreal Engine state to the AGX Dynamics native state.
	 */
	void UpdateNativeProperties();

	// ~Begin Native Owner interface.
	// These all deal with the power-line native, not the individual components.
	virtual bool HasNative() const override;
	virtual uint64 GetNativeAddress() const override;
	virtual void SetNativeAddress(uint64 NativeAddress) override;
	// ~End Native Owner interface.

	FPowerLineBarrier* GetNative();
	const FPowerLineBarrier* GetNative() const;
	FPowerLineBarrier* GetOrCreateNative();
	bool HasNativeCombustionEngine() const;

	//~ Begin UActorComponent interface.
	virtual void OnRegister() override;
	virtual void BeginPlay() override;
	//~ End UActorComponent interface.

private:
	void CreateNative();

private:
	FPreconfiguredDriveTrainBarriers NativeBarriers;
};
