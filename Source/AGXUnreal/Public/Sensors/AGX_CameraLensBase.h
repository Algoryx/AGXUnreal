// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/CameraLensBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_CameraLensBase.generated.h"

class UWorld;

/**
 * Base class for AGX Camera Lens assets.
 */
UCLASS(Abstract, ClassGroup = "AGX_Sensor", Category = "AGX", BlueprintType)
class AGXUNREAL_API UAGX_CameraLensBase : public UObject
{
	GENERATED_BODY()

public:
	bool HasNative() const;
	FCameraLensBarrier* GetNative();
	const FCameraLensBarrier* GetNative() const;
	void ReleaseNative();

	void CommitToAsset();

	static UAGX_CameraLensBase* CreateInstanceFromAsset(
		UWorld* PlayingWorld, UAGX_CameraLensBase& Source);

	UAGX_CameraLensBase* GetOrCreateInstance(UWorld* PlayingWorld);
	FCameraLensBarrier* GetOrCreateNative();

	bool IsInstance() const;

	UAGX_CameraLensBase* GetInstance();
	const UAGX_CameraLensBase* GetInstance() const;

	UAGX_CameraLensBase* GetAsset();
	const UAGX_CameraLensBase* GetAsset() const;

	virtual void CopyProperties(const UAGX_CameraLensBase& Source);

protected:
	virtual void CreateNative() PURE_VIRTUAL(UAGX_CameraLensBase::CreateNative, );

protected:
	TWeakObjectPtr<UAGX_CameraLensBase> Asset;
	TWeakObjectPtr<UAGX_CameraLensBase> Instance;
	TUniquePtr<FCameraLensBarrier> NativeBarrier;
};
