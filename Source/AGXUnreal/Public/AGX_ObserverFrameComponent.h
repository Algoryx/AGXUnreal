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
struct FObserverFrameData;

UCLASS(ClassGroup = "AGX", Category = "AGX", Meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_ObserverFrameComponent : public USceneComponent, public IAGX_NativeOwner
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "AGX Dynamics")
	bool bEnabled {true};

	UFUNCTION(BlueprintCallable, Category = "AGX Dynamics")
	void SetEnabled(bool InEnabled);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Dynamics")
	bool IsEnabled() const;

	/**
	 * Get the Rigid Body that owns this Observer Frame. Will return None / nullptr if this Observer
	 * Frame does not belong to a Rigid Body.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Dynamics")
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

	void CopyFrom(const FObserverFrameData& Data, FAGX_ImportContext* Context);

	FObserverFrameBarrier* GetNative();
	const FObserverFrameBarrier* GetNative() const;

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
