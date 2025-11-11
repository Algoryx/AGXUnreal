// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_NativeOwner.h"
#include "Vehicle/SteeringBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "AGX_SteeringComponent.generated.h"

struct FAGX_ImportContext;

/**
 * The Steering Component provides a steering mechanism designed to align wheels simultaneously to
 * minimize slip, making them move along the tangents of concentric circles determined by the
 * distances between the wheels and the distance between front and rear.
 *
 * It provides:
 *  - Ackermann mechanism with direct steering input on one of the wheels.
 *  - Bell-crank (central-lever) linkage, used when the steering column is centered.
 *  - Rack and pinion mechanism, commonly used in passenger cars.
 *  - Davis mechanism, which provides ideal steering geometry, but is not used in practice.
 */
UCLASS(ClassGroup = "AGX_Vehicle", Category = "AGX", Meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_SteeringComponent : public UActorComponent, public IAGX_NativeOwner
{
	GENERATED_BODY()

public:
	/*
	 * Determines whether this Steering Component is enabled (active) or not.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Steering")
	bool bEnabled {true};

	UFUNCTION(BlueprintCallable, Category = "AGX Steering")
	void SetEnabled(bool InEnabled);

	UFUNCTION(BlueprintCallable, Category = "AGX Steering")
	bool IsEnabled() const;

	/*
	 * The import Guid of this Component. Only used by the AGX Dynamics for Unreal import system.
	 * Should never be assigned manually.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Dynamics Import Guid")
	FGuid ImportGuid;

	/*
	 * The import name of this Component. Only used by the AGX Dynamics for Unreal import system.
	 * Should never be assigned manually.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Dynamics Import Name")
	FString ImportName;

	void CopyFrom(const FSteeringBarrier& Barrier, FAGX_ImportContext* Context);

	FSteeringBarrier* GetNative();
	const FSteeringBarrier* GetNative() const;

	//~ Begin IAGX_NativeObject interface.
	virtual bool HasNative() const override;
	virtual uint64 GetNativeAddress() const override;
	virtual void SetNativeAddress(uint64 NativeAddress) override;
	//~ End IAGX_NativeObject interface.

	//~ Begin UActorComponent interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	virtual TStructOnScope<FActorComponentInstanceData> GetComponentInstanceData() const override;
	//~ End UActorComponent interface

#if WITH_EDITOR
	// ~Begin UObject interface.
	virtual void PostInitProperties() override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
	// ~End UObject interface.
#endif

private:
#if WITH_EDITOR
	void InitPropertyDispatcher();
#endif
	void UpdateNativeProperties();
	void CreateNative();
	bool GetEnabled() const; // To be able to use AGX_COMPONENT_DEFAULT_DISPATCHER_BOOL.

	FSteeringBarrier NativeBarrier;
};
