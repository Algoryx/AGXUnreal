// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_NativeOwner.h"
#include "Model/DynamicWaterBarrier.h"

// Unreal Engine includes.
#include "Components/SceneComponent.h"
#include "CoreMinimal.h"

#include "AGX_DynamicWaterComponent.generated.h"

UCLASS(ClassGroup = "AGX", Category = "AGX", BlueprintType, Blueprintable,
	meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_DynamicWaterComponent : public USceneComponent, public IAGX_NativeOwner
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UAGX_DynamicWaterComponent();

	/**
	 * Get the Native Barrier. Create the native AGX Dynamics object if it does not
	 * already exist.
	 *
	 * @return The Native Barrier for this controller.
	 */
	FDynamicWaterBarrier* GetOrCreateNative();

	/// Return the native AGX Dynamics representation of this controller. May return nullptr.
	FDynamicWaterBarrier* GetNative();

	const FDynamicWaterBarrier* GetNative() const;

	// ~Begin IAGX_NativeOwner interface.
	virtual bool HasNative() const override;
	virtual uint64 GetNativeAddress() const override;
	virtual void SetNativeAddress(uint64 NativeAddress) override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type Reason) override;

public:
	virtual double FindHeightFromSurface(const FVector& WorldPoint, const FVector& UpVector, const double& Time) const;
	virtual double GetDensity() const;
	virtual FVector GetVelocity(const FVector& WorldPoint) const;

private:
	// Create the native AGX Dynamics object.
	void InitializeNative();

	TUniquePtr<FDynamicWaterBarrier> NativeBarrier;
};
