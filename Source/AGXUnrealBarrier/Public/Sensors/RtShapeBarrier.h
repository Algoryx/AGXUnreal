// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <memory>

struct FRtShapeRef;

class AGXUNREALBARRIER_API FRtShapeBarrier
{
public:
	FRtShapeBarrier();
	FRtShapeBarrier(std::unique_ptr<FRtShapeRef> Native);
	FRtShapeBarrier(FRtShapeBarrier&& Other);
	~FRtShapeBarrier();

	bool HasNative() const;
	void AllocateNative(const TArray<FVector>& Vertices, const TArray<FTriIndices>& Indices);
	FRtShapeRef* GetNative();
	const FRtShapeRef* GetNative() const;

	void ReleaseNative();

private:
	FRtShapeBarrier(const FRtShapeBarrier&) = delete;
	void operator=(const FRtShapeBarrier&) = delete;

private:
	std::unique_ptr<FRtShapeRef> NativeRef;
};
