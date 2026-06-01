// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Containers/UnrealString.h"

// Standard library includes.
#include <memory>

#include "ObserverFrameBarrier.generated.h"

struct FObserverFrameRef;
struct FRigidBodyBarrier;

USTRUCT(BlueprintType)
struct AGXUNREALBARRIER_API FObserverFrameBarrier
{
	GENERATED_BODY()

	FObserverFrameBarrier();
	FObserverFrameBarrier(std::shared_ptr<FObserverFrameRef> Native);

	void SetEnabled(bool Enabled);
	bool GetEnabled() const;

	void SetPosition(const FVector& Position);
	FVector GetPosition() const;

	void SetLocalPosition(const FVector& Position);
	FVector GetLocalPosition() const;

	void SetRotation(const FQuat& Rotation);
	FQuat GetRotation() const;

	void SetLocalRotation(const FQuat& Rotation);
	FQuat GetLocalRotation() const;

	FVector GetVelocity() const;
	FVector GetAngularVelocity() const;
	FVector GetAcceleration() const;
	FVector GetAngularAcceleration() const;

	FRigidBodyBarrier GetRigidBody() const;

	void SetName(const FString& NewName);
	FString GetName() const;

	FGuid GetGuid() const;

	bool HasNative() const;
	void AllocateNative(FRigidBodyBarrier& Body);
	FObserverFrameRef* GetNative();
	const FObserverFrameRef* GetNative() const;

	/// @return The address of the underlying AGX Dynamics object.
	uintptr_t GetNativeAddress() const;

	/// Re-assign this Barrier to the given native address. The address must be an existing AGX
	/// Dynamics object of the correct type.
	void SetNativeAddress(uintptr_t NativeAddress);

	void ReleaseNative();

private:
	std::shared_ptr<FObserverFrameRef> NativeRef;
};
