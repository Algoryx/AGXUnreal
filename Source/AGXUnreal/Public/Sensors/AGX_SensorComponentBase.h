// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_NativeOwner.h"
#include "Sensors/SensorBarrier.h"

// Unreal Engine includes.
#include "Components/SceneComponent.h"
#include "CoreMinimal.h"

// Standard Library includes.
#include <memory>

#include "AGX_SensorComponentBase.generated.h"

/**
 * Base class for Sensor Components such as Lidar and IMU.
 */
UCLASS(Category = "AGX", ClassGroup = "AGX_Sensor", NotPlaceable)
class AGXUNREAL_API UAGX_SensorComponentBase : public USceneComponent, public IAGX_NativeOwner
{
	GENERATED_BODY()

public:
	UAGX_SensorComponentBase();

	/**
	 * Enable or disable this Sensor Component.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Sensor")
	bool bEnabled {true};

	UFUNCTION(BlueprintCallable, Category = "AGX Sensor")
	void SetEnabled(bool InEnabled);

	UFUNCTION(BlueprintCallable, Category = "AGX Sensor")
	bool IsEnabled() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Sensor",
		Meta = (DeprecatedFunction, DeprecationMessage = "Use IsEnabled instead."))
	bool GetEnabled() const;

	/**
	 * How often this Sensor generates Output, in number of AGX Time Steps.
	 * For example, if StepStride is set to 1, Output is produces each AGX Step Forward.
	 * If StepStride is set to 5, Output is produced every 5 AGX Step Forward executions.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Sensor", Meta = (ClampMin = "1"))
	int32 StepStride {1};

	UFUNCTION(BlueprintCallable, Category = "AGX Sensor")
	void SetStepStride(int32 Stride);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Sensor")
	int32 GetStepStride() const;

	// ~Begin AGX NativeOwner interface.
	virtual uint64 GetNativeAddress() const override;
	virtual void SetNativeAddress(uint64 NativeAddress) override;
	virtual bool HasNative() const override;
	// ~End IAGX_NativeOwner interface.

	FSensorBarrier* GetOrCreateNative();
	FSensorBarrier* GetNative();
	const FSensorBarrier* GetNative() const;

	//~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	virtual TStructOnScope<FActorComponentInstanceData> GetComponentInstanceData() const override;
	//~ End UActorComponent Interface

	//~ Begin UObject interface.
#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
	virtual void PostInitProperties() override;
#endif
	//~ End UObject interface.

protected:
	virtual void UpdateNativeProperties();
	virtual FSensorBarrier* CreateNativeImpl();
	virtual void MarkOutputAsRead()
		PURE_VIRTUAL(UAGX_SensorComponentBase::MarkOutputAsRead,);

	TUniquePtr<FSensorBarrier> NativeBarrier;

private:
#if WITH_EDITOR
	void InitPropertyDispatcher();
#endif
	FDelegateHandle PreStepForwardHandle;
};
