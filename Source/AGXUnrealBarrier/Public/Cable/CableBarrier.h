// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <memory>

#include "CableBarrier.generated.h"

struct FAGX_CableNodeInfo;
struct FCableNodeBarrier;
struct FCableRef;

USTRUCT(BlueprintType)
struct AGXUNREALBARRIER_API FCableBarrier
{
	GENERATED_BODY()

	FCableBarrier();
	FCableBarrier(std::shared_ptr<FCableRef> Native);

	void AllocateNative(double Radius, double SegmentLength);
	bool Add(FCableNodeBarrier& Node);

	TArray<FAGX_CableNodeInfo> GetNodeInfo() const;

	double GetRadius() const;

	FGuid GetGuid() const;

	void SetName(const FString& NewName);
	FString GetName() const;

	bool HasNative() const;
	FCableRef* GetNative();
	const FCableRef* GetNative() const;

	/// @return The address of the underlying AGX Dynamics object.
	uintptr_t GetNativeAddress() const;

	/// Re-assign this Barrier to the given native address. The address must be an existing AGX
	/// Dynamics object of the correct type.
	void SetNativeAddress(uintptr_t NativeAddress);

	void ReleaseNative();

private:
	std::shared_ptr<FCableRef> NativeRef;
};
