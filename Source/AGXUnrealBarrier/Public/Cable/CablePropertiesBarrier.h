// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <memory>

#include "CablePropertiesBarrier.generated.h"

struct FCablePropertiesRef;


USTRUCT(BlueprintType)
struct AGXUNREALBARRIER_API FCablePropertiesBarrier
{
	GENERATED_BODY()

	FCablePropertiesBarrier();
	FCablePropertiesBarrier(std::shared_ptr<FCablePropertiesRef> Native);

	bool HasNative() const;
	FCablePropertiesRef* GetNative();
	const FCablePropertiesRef* GetNative() const;

	void AllocateNative();
	void ReleaseNative();

	FGuid GetGuid() const;

private:
	std::shared_ptr<FCablePropertiesRef> NativeRef;
};
