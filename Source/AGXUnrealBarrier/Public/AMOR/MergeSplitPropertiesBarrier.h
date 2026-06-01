// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AMOR/ConstraintMergeSplitThresholdsBarrier.h"
#include "AMOR/ShapeContactMergeSplitThresholdsBarrier.h"
#include "AMOR/WireMergeSplitThresholdsBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes
#include <memory>

#include "MergeSplitPropertiesBarrier.generated.h"

class FConstraintBarrier;
struct FRigidBodyBarrier;
struct FShapeBarrier;
class FWireBarrier;

struct FMergeSplitPropertiesPtr;

USTRUCT(BlueprintType)
struct AGXUNREALBARRIER_API FMergeSplitPropertiesBarrier
{
	GENERATED_BODY()

	FMergeSplitPropertiesBarrier();
	FMergeSplitPropertiesBarrier(std::shared_ptr<FMergeSplitPropertiesPtr> Native);

	bool HasNative() const;
	FMergeSplitPropertiesPtr* GetNative();
	const FMergeSplitPropertiesPtr* GetNative() const;

	template <typename T>
	void AllocateNative(T& Owner);

	void ReleaseNative();

	template <typename T>
	static FMergeSplitPropertiesBarrier CreateFrom(T& Barrier);

	void SetEnableMerge(bool bEnable);
	bool GetEnableMerge() const;

	void SetEnableSplit(bool bEnable);
	bool GetEnableSplit() const;

	void SetShapeContactMergeSplitThresholds(FShapeContactMergeSplitThresholdsBarrier* Thresholds);
	FShapeContactMergeSplitThresholdsBarrier GetShapeContactMergeSplitThresholds() const;

	void SetConstraintMergeSplitThresholds(FConstraintMergeSplitThresholdsBarrier* Thresholds);
	FConstraintMergeSplitThresholdsBarrier GetConstraintMergeSplitThresholds() const;

	void SetWireMergeSplitThresholds(FWireMergeSplitThresholdsBarrier* Thresholds);
	FWireMergeSplitThresholdsBarrier GetWireMergeSplitThresholds() const;

	/*
	 * This Barrier should not have a Native at the time of calling BindToNewOwner.
	 * Ensure it is released prior.
	 */
	void BindToNewOwner(FRigidBodyBarrier& NewOwner);
	void BindToNewOwner(FShapeBarrier& NewOwner);
	void BindToNewOwner(FConstraintBarrier& NewOwner);
	void BindToNewOwner(FWireBarrier& NewOwner);

private:
	template <typename T>
	void BindToNewOwnerImpl(T& NewOwner);

	std::shared_ptr<FMergeSplitPropertiesPtr> NativePtr;
};
