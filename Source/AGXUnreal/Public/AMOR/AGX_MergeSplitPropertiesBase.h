// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AMOR/MergeSplitPropertiesBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "AGX_MergeSplitPropertiesBase.generated.h"

class UAGX_MergeSplitThresholdsBase;

struct FAGX_ImportContext;

USTRUCT(BlueprintType)
struct AGXUNREAL_API FAGX_MergeSplitPropertiesBase
{
	GENERATED_BODY()

public:
	virtual ~FAGX_MergeSplitPropertiesBase() = default;

	// We must provide operator = because the Unreal framework will attempt to invoke it.
	FAGX_MergeSplitPropertiesBase& operator=(const FAGX_MergeSplitPropertiesBase& Other);

	bool operator==(const FAGX_MergeSplitPropertiesBase& Other) const;

	UPROPERTY(EditAnywhere, Category = "AGX AMOR")
	bool bEnableMerge = false;

	UPROPERTY(EditAnywhere, Category = "AGX AMOR")
	bool bEnableSplit = false;

	void SetEnableMerge(bool bEnable);
	bool GetEnableMerge() const;

	void SetEnableSplit(bool bEnable);
	bool GetEnableSplit() const;

	bool HasNative() const;
	const FMergeSplitPropertiesBarrier* GetNative() const;
	FMergeSplitPropertiesBarrier* GetNative();

	virtual void CopyFrom(const FMergeSplitPropertiesBarrier& Barrier, FAGX_ImportContext* Context);

	virtual UAGX_MergeSplitThresholdsBase* GetThresholds()
		PURE_VIRTUAL(UAGX_MergeSplitThresholdsBase::GetThresholds, return nullptr;);

protected:
	FMergeSplitPropertiesBarrier NativeBarrier;
};
