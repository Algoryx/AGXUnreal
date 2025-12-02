// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <memory>

struct FLidarOutputRef;


class AGXUNREALBARRIER_API FLidarOutputBarrier
{
public:
	FLidarOutputBarrier();
	FLidarOutputBarrier(std::unique_ptr<FLidarOutputRef> Native);
	FLidarOutputBarrier(FLidarOutputBarrier&& Other);
	virtual ~FLidarOutputBarrier();

	virtual void AllocateNative() = 0;

	bool HasNative() const;
	FLidarOutputRef* GetNative();
	const FLidarOutputRef* GetNative() const;
	void ReleaseNative();

private:
	FLidarOutputBarrier(const FLidarOutputBarrier&) = delete;
	void operator=(const FLidarOutputBarrier&) = delete;

protected:
	std::unique_ptr<FLidarOutputRef> NativeRef;
};
