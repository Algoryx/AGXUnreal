// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Containers/UnrealString.h"

// Standard library includes.
#include <memory>

#include "ObserverFrameBarrier.generated.h"

struct FObserverFrameRef;

USTRUCT(BlueprintType)
struct AGXUNREALBARRIER_API FObserverFrameBarrier
{
	GENERATED_BODY()

	FObserverFrameBarrier();
	FObserverFrameBarrier(std::shared_ptr<FObserverFrameRef> Native);

	void SetName(const FString& NewName);
	FString GetName() const;

	FGuid GetGuid() const;

	bool HasNative() const;
	void AllocateNative();
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
