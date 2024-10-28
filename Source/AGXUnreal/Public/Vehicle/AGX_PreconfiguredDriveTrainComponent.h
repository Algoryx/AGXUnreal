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
 *
 */
UCLASS(Category = "AGX", ClassGroup = "AGX_Vehicle", Meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_PreconfiguredDriveTrainComponent : public UActorComponent,
															public IAGX_NativeOwner
{
	GENERATED_BODY()

public:
	UAGX_PreconfiguredDriveTrainComponent();
	virtual ~UAGX_PreconfiguredDriveTrainComponent() = default;

	UPROPERTY(EditAnywhere, Category = "Combustion Engine")
	FAGX_CombustionEngineParameters CombustionEngineParameters; // TODO Make an asset.

	UPROPERTY(EditAnywhere, Category = "Combustion Engine")
	FAGX_Real Throttle {0.0};

	UFUNCTION(BlueprintCallable, Category = "Preconfigured Drive-Train|Combustion Engine")
	void SetThrottle(double InThrottle);

	UPROPERTY(EditAnywhere, Category = "Wheels")
	FAGX_HingeReference FrontLeftHinge;

	UPROPERTY(EditAnywhere, Category = "Wheels")
	FAGX_HingeReference FrontRightHinge;

	void UpdateNativeProperties();


	// ~Begin Native Owner interface.
	virtual bool HasNative() const override;
	virtual uint64 GetNativeAddress() const override;
	virtual void SetNativeAddress(uint64 NativeAddress) override;
	// ~End Native Owner interface.

	void CreateNative();
	bool HasNativeCombustionEngine() const;

	//~ Begin UActorComponent interface.
	virtual void OnRegister() override;
	virtual void BeginPlay() override;
	//~ End UActorComponent interface.

private:
	void AllocateNative();

private:
	FPreconfiguredDriveTrainBarriers NativeBarriers;
};
