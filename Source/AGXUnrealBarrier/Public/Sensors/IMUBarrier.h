// Copyright 2025, Algoryx Simulation AB.

#pragma once


// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <memory>

struct FIMURef;

class AGXUNREALBARRIER_API FIMUBarrier
{
public:
	FIMUBarrier();
	FIMUBarrier(std::unique_ptr<FIMURef> Native);
	FIMUBarrier(FIMUBarrier&& Other);
	~FIMUBarrier();

private:
	FIMUBarrier(const FIMUBarrier&) = delete;
	void operator=(const FIMUBarrier&) = delete;

private:
	std::unique_ptr<FIMURef> NativeRef;
};
