// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <memory>

struct FPowerLineRef;

class FPowerLineUnitBarrier;

class AGXUNREALBARRIER_API FPowerLineBarrier
{
public:
	FPowerLineBarrier();
	FPowerLineBarrier(std::unique_ptr<FPowerLineRef> Native);
	FPowerLineBarrier(const FPowerLineBarrier& Other);
	FPowerLineBarrier(FPowerLineBarrier&& Other);
	FPowerLineBarrier& operator=(const FPowerLineBarrier& Other);
	virtual ~FPowerLineBarrier();

	void AllocateNative();
	bool HasNative() const;
	FPowerLineRef* GetNative();
	const FPowerLineRef* GetNative() const;
	uintptr_t GetNativeAddress() const;
	void SetNativeAddress(uintptr_t NativeAddress);
	void ReleaseNative();

	void Add(FPowerLineUnitBarrier& Unit);

private:
	std::unique_ptr<FPowerLineRef> NativeRef;
};
