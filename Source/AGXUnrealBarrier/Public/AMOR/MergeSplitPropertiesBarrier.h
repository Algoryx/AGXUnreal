// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AMOR/ConstraintMergeSplitThresholdsBarrier.h"
#include "AMOR/ShapeContactMergeSplitThresholdsBarrier.h"
#include "AMOR/WireMergeSplitThresholdsBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes
#include <memory>

class FConstraintBarrier;
class FShapeBarrier;
struct FRigidBodyBarrier;
class FWireBarrier;

struct FMergeSplitPropertiesPtr;

class AGXUNREALBARRIER_API FMergeSplitPropertiesBarrier
{
public:
	FMergeSplitPropertiesBarrier();
	FMergeSplitPropertiesBarrier(std::unique_ptr<FMergeSplitPropertiesPtr>&& Native);
	FMergeSplitPropertiesBarrier(FMergeSplitPropertiesBarrier&& Other) noexcept;
	~FMergeSplitPropertiesBarrier();

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

	FMergeSplitPropertiesBarrier& operator=(const FMergeSplitPropertiesBarrier& Other) = delete;

	std::unique_ptr<FMergeSplitPropertiesPtr> NativePtr;
};
