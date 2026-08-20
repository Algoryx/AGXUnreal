// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/CameraOutputBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_CameraOutputBase.generated.h"

class UAGX_CameraSensorComponent;

USTRUCT(BlueprintType, BlueprintInternalUseOnly)
struct AGXUNREAL_API FAGX_CameraOutputBase
{
	GENERATED_BODY()

public:
	FAGX_CameraOutputBase() = default;
	FAGX_CameraOutputBase(const FAGX_CameraOutputBase& Other);
	virtual ~FAGX_CameraOutputBase();

	bool HasNative() const;
	FCameraOutputBarrier* GetOrCreateNative();
	const FCameraOutputBarrier* GetNative() const;
	FCameraOutputBarrier* GetNative();

	// Making UAGX_CameraSensorComponent::AddOutput Blueprint friendly was not so easy since
	// non-const references becomes out-variables, and pointers to structs are not permitted as
	// input argument.
	bool AddTo(UAGX_CameraSensorComponent* Camera);

	FAGX_CameraOutputBase& operator=(const FAGX_CameraOutputBase& Other);
	bool operator==(const FAGX_CameraOutputBase& Other) const;

protected:
	virtual TUniquePtr<FCameraOutputBarrier> CreateNativeBarrier() const
		PURE_VIRTUAL(FAGX_CameraOutputBase::CreateNativeBarrier, return nullptr;);

private:
	TUniquePtr<FCameraOutputBarrier> NativeBarrier;
};
