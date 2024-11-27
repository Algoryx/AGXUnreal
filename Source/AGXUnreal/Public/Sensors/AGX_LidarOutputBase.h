// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_Real.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_LidarOutputBase.generated.h"

class FLidarOutputBarrier;
class UAGX_LidarSensorComponent;

USTRUCT(BlueprintType, Meta = (HiddenByDefault))
struct AGXUNREAL_API FAGX_LidarOutputBase
{
	GENERATED_BODY()

public:
	virtual ~FAGX_LidarOutputBase() = default;

	virtual bool HasNative() const PURE_VIRTUAL(FAGX_LidarOutputBase::HasNative, return false;);

	virtual FLidarOutputBarrier* GetOrCreateNative()
		PURE_VIRTUAL(FAGX_LidarOutputBase::GetOrCreateNative, return nullptr;);

	virtual const FLidarOutputBarrier* GetNative() const
		PURE_VIRTUAL(FAGX_LidarOutputBase::GetNative, return nullptr;);

	virtual FLidarOutputBarrier* GetNative()
		PURE_VIRTUAL(FAGX_LidarOutputBase::GetNative, return nullptr;);

	// Making UAGX_LidarSensorComponent::AddOutput Blueprint friendly was not so easy since
	// non-const references becomes out-variables, and pointers to structs are not permitted as
	// input argument.
	bool AddTo(UAGX_LidarSensorComponent* Lidar);

	bool operator==(const FAGX_LidarOutputBase& Other) const;
};
