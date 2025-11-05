// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_NativeOwner.h"
#include "ObserverFrameBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Components/SceneComponent.h"

#include "AGX_ObserverFrameComponent.generated.h"

class UAGX_RigidBodyComponent;

struct FAGX_ImportContext;

/**
 * A frame attached to a RigidBody with an optional relative transform.
 * During runtime, it is possible to get its position, velocity, angular velocity, acceleration
 * etc.
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", Meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_ObserverFrameComponent : public USceneComponent, public IAGX_NativeOwner
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "AGX Observer Frame")
	bool bEnabled {true};

	UFUNCTION(BlueprintCallable, Category = "AGX Observer Frame")
	void SetEnabled(bool InEnabled);

	UFUNCTION(BlueprintCallable, Category = "AGX Observer Frame")
	bool IsEnabled() const;

	/**
	 * Set the world position of the Observer Frame [cm].
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Observer Frame")
	void SetPosition(FVector Position);

	/**
	 * Get the world position of the Observer Frame [cm].
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Observer Frame")
	FVector GetPosition() const;

	/**
	 * Set the local position of the Observer Frame [cm].
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Observer Frame")
	void SetLocalPosition(FVector Position);

	/**
	 * Get the local position of the Observer Frame [cm].
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Observer Frame")
	FVector GetLocalPosition() const;

	/**
	 * Set the world rotation of the Observer Frame.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Observer Frame")
	void SetRotation(FQuat Rotation);

	/**
	 * Get the world rotation of the Observer Frame.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Observer Frame")
	FQuat GetRotation() const;

	/**
	 * Set the local rotation of the Observer Frame.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Observer Frame")
	void SetLocalRotation(FQuat Rotation);

	/**
	 * Get the local rotation of the Observer Frame.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Observer Frame")
	FQuat GetLocalRotation() const;

	/**
	 * Set the world rotator of the Observer Frame.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Observer Frame")
	void SetRotator(FRotator Rotator);

	/**
	 * Get the world rotator of the Observer Frame.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Observer Frame")
	FRotator GetRotator() const;

	/**
	 * Set the local rotator of the Observer Frame.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Observer Frame")
	void SetLocalRotator(FRotator Rotator);

	/**
	 * Get the local rotator of the Observer Frame.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Observer Frame")
	FRotator GetLocalRotator() const;

	/**
	 * Get the world velocity of the Observer Frame [cm/s].
	 * Only valid during Play. If this Observer Frame does not have a native AGX Dynamics Observer
	 * Frame, zero vector is returned.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Observer Frame")
	FVector GetVelocity() const;

	/**
	 * Get the world angular velocity of the Observer Frame [deg/s].
	 * Only valid during Play. If this Observer Frame does not have a native AGX Dynamics Observer
	 * Frame, zero vector is returned.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Observer Frame")
	FVector GetAngularVelocity() const;

	/**
	 * Get the world acceleration of the Observer Frame [cm/s^2].
	 * Only valid during Play. If this Observer Frame does not have a native AGX Dynamics Observer
	 * Frame, zero vector is returned.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Observer Frame")
	FVector GetAcceleration() const;

	/**
	 * Get the world angular acceleration of the Observer Frame [deg/s^2].
	 * Only valid during Play. If this Observer Frame does not have a native AGX Dynamics Observer
	 * Frame, zero vector is returned.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Observer Frame")
	FVector GetAngularAcceleration() const;

	/**
	 * Get the Rigid Body that owns this Observer Frame. Will return None / nullptr if this Observer
	 * Frame does not belong to a Rigid Body.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Observer Frame")
	UAGX_RigidBodyComponent* GetRigidBody() const;

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

	void CopyFrom(const FObserverFrameBarrier& Barrier, FAGX_ImportContext* Context);

	FObserverFrameBarrier* GetNative();
	const FObserverFrameBarrier* GetNative() const;

	FObserverFrameBarrier* GetOrCreateNative();

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

	FObserverFrameBarrier NativeBarrier;
};
