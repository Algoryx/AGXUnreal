// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Wire/AGX_WireEnums.h"

// Unreal Engine includes.
#include "Math/Vector.h"

// Standard library includes.
#include <memory>

struct FWireNodeRef;
struct FRigidBodyBarrier;

class AGXUNREALBARRIER_API FWireNodeBarrier
{
public:
	FWireNodeBarrier();
	FWireNodeBarrier(const FWireNodeBarrier& InOther);
	FWireNodeBarrier(FWireNodeBarrier&& InOther);
	FWireNodeBarrier(std::unique_ptr<FWireNodeRef>&& InNative);
	~FWireNodeBarrier();

	FWireNodeBarrier& operator=(const FWireNodeBarrier& InOther);

	bool HasNative() const;
	void AllocateNativeFreeNode(const FVector& WorldLocation);
	void AllocateNativeEyeNode(FRigidBodyBarrier& RigidBody, const FVector& LocalLocation);
	void AllocateNativeBodyFixedNode(FRigidBodyBarrier& RigidBody, const FVector& LocalLocation);
	FWireNodeRef* GetNative();
	const FWireNodeRef* GetNative() const;
	void ReleaseNative();

	FVector GetWorldLocation() const;
	FVector GetTranslate() const;
	EWireNodeType GetType() const;
	FRigidBodyBarrier GetRigidBody() const;

private:
	std::unique_ptr<FWireNodeRef> NativeRef;
};
