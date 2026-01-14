// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "AGX_CableProperties.generated.h"

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
	UAGX_CableProperties* GetInstance();
	UAGX_CableProperties* GetAsset();

	UFUNCTION(BlueprintCallable, Category = "AGX Cable")
	void CommitToAsset();

private:
	/// The persistent asset that this runtime instance was created from. Nullptr for assets.
	TWeakObjectPtr<UAGX_CableProperties> Asset {nullptr};

	/// The runtime instance that was created from a persistent asset. Nullptr for instances.
	TWeakObjectPtr<UAGX_CableProperties> Instance {nullptr};
};
