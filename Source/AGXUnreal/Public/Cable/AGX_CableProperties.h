// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/Object.h"

// AGX Dynamics for Unreal includes.
#include "Cable/CablePropertiesBarrier.h"

#include "AGX_CableProperties.generated.h"

class UWorld;

struct FAGX_ImportContext;

/**
 * An asset used to hold properties defining the physical behaviour of a Cable.
 */
UCLASS(ClassGroup = "AGX", Category = "AGX")
class AGXUNREAL_API UAGX_CableProperties : public UObject
{
	GENERATED_BODY()

public:
	UAGX_CableProperties() = default;

	UAGX_CableProperties* GetOrCreateInstance(UWorld* PlayingWorld);
	bool IsInstance() const;
	UAGX_CableProperties* CreateInstanceFromAsset(
		const UWorld* PlayingWorld, UAGX_CableProperties* Source);
	UAGX_CableProperties* GetInstance();
	UAGX_CableProperties* GetAsset();

	UFUNCTION(BlueprintCallable, Category = "AGX Cable")
	void CommitToAsset();

	void CopyFrom(const FCablePropertiesBarrier& Source, FAGX_ImportContext* Context);
	void CopyFrom(const UAGX_CableProperties* Source);

	bool HasNative() const;
	FCablePropertiesBarrier* GetNative();
	const FCablePropertiesBarrier* GetNative() const;
	FCablePropertiesBarrier* GetOrCreateNative();

	void UpdateNativeProperties();

	// ~Begin UObject interface.
	virtual void PostInitProperties() override;
#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
#endif
	
private:
#if WITH_EDITOR
	virtual void InitPropertyDispatcher();
#endif

	void CreateNative();

private:
	/// The persistent asset that this runtime instance was created from. Nullptr for assets.
	TWeakObjectPtr<UAGX_CableProperties> Asset {nullptr};

	/// The runtime instance that was created from a persistent asset. Nullptr for instances.
	TWeakObjectPtr<UAGX_CableProperties> Instance {nullptr};

	FCablePropertiesBarrier NativeBarrier;
};
