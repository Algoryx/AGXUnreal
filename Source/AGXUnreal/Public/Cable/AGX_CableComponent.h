// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_NativeOwner.h"
#include "Cable/CableBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Components/SceneComponent.h"

#include "AGX_CableComponent.generated.h"

/**
 * A frame attached to a RigidBody with an optional relative transform.
 * During runtime, it is possible to get its position, velocity, angular velocity, acceleration
 * etc.
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", Meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_CableComponent : public USceneComponent, public IAGX_NativeOwner
{
	GENERATED_BODY()

public:
	/**
	 * The radius of the Cable [cm].
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX Cable",
		Meta = (ClampMin = "0", UIMin = "0"))
	double Radius {3.0};

	/**
	 * The resolution of the Cable, i.e. the number of lumped elements per centimeter.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX Cable",
		Meta = (ClampMin = "0", UIMin = "0"))
	double ResolutionPerUnitLength {0.01};

	/*
	 * The import name of this Component. Only used by the AGX Dynamics for Unreal import system.
	 * Should never be assigned manually.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Dynamics Import Name")
	FString ImportName;

	FCableBarrier* GetNative();
	const FCableBarrier* GetNative() const;

	FCableBarrier* GetOrCreateNative();

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

	#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif

	FCableBarrier NativeBarrier;
};
