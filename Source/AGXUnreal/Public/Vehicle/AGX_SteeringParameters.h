// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Vehicle/AGX_SteeringParametersData.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "AGX_SteeringParameters.generated.h"

/**
 * An asset used to hold configuration properties for steering mechanisms.
 */
UCLASS(
	ClassGroup = "AGX_Vehicle", Category = "AGX", abstract,
	AutoExpandCategories = ("AGX Steering"))
class AGXUNREAL_API UAGX_SteeringParameters : public UObject
{
	GENERATED_BODY()

public:
	UAGX_SteeringParameters() = default;

	/**
	 * Steering Parameters data, describing the configuration of the steering used.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Steering")
	FAGX_SteeringParametersData SteeringData;

	UAGX_SteeringParameters* GetOrCreateInstance(UWorld* PlayingWorld);
	bool IsInstance() const;
	UAGX_SteeringParameters* GetInstance();
	UAGX_SteeringParameters* GetAsset();

	UFUNCTION(BlueprintCallable, Category = "AGX Steering")
	void CommitToAsset();

private:
	/// The persistent asset that this runtime instance was created from. Nullptr for assets.
	TWeakObjectPtr<UAGX_SteeringParameters> Asset {nullptr};

	/// The runtime instance that was created from a persistent asset. Nullptr for instances.
	TWeakObjectPtr<UAGX_SteeringParameters> Instance {nullptr};
};
