// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

template <typename BarrierT>
class AGXUNREALBARRIER_API FBarrierBase
{
public:
	bool HasNative() const;
	uintptr_t GetNativeAddress() const;
	template <typename AGXType>
	void SetNativeAddress(uintptr_t NativeAddress);
	void ReleaseNative();

private:
	auto GetNative()
	{
		return Barrier().GetNative();
	}

	auto GetNative() const
	{
		return Barrier().GetNative();
	}

	BarrierT& Barrier();
	const BarrierT& Barrier() const;
};
