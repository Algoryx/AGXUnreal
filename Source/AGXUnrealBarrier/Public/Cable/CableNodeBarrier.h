// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Cable/AGX_CableEnums.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Math/Vector.h"

// Standard library includes.
#include <memory>

#include "CableNodeBarrier.generated.h"

struct FCableNodeRef;
struct FRigidBodyBarrier;

USTRUCT(BlueprintType)
struct AGXUNREALBARRIER_API FCableNodeBarrier
{
	GENERATED_BODY()

	FCableNodeBarrier();
	FCableNodeBarrier(std::shared_ptr<FCableNodeRef> Native);

	bool HasNative() const;
	void AllocateNativeFreeNode(const FVector& WorldLocation);
	void AllocateNativeBodyFixedNode(FRigidBodyBarrier& RigidBody, const FVector& LocalLocation);
	FCableNodeRef* GetNative();
	const FCableNodeRef* GetNative() const;
	void ReleaseNative();

	FVector GetWorldLocation() const;
	FVector GetTranslate() const;
	EAGX_CableNodeType GetType() const;
	FRigidBodyBarrier GetRigidBody() const;

private:
	std::shared_ptr<FCableNodeRef> NativeRef;
};
