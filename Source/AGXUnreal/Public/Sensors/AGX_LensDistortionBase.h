// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/LensDistortionBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_LensDistortionBase.generated.h"

class UWorld;

/**
 * Base class for AGX lens distortion assets.
 */
UCLASS(Abstract, ClassGroup = "AGX_Sensor", Category = "AGX", BlueprintType)
class AGXUNREAL_API UAGX_LensDistortionBase : public UObject
{
	GENERATED_BODY()

public:
	bool HasNative() const;
	FLensDistortionBarrier* GetNative();
	const FLensDistortionBarrier* GetNative() const;
	void ReleaseNative();

	void CommitToAsset();

	static UAGX_LensDistortionBase* CreateInstanceFromAsset(
		UWorld* PlayingWorld, UAGX_LensDistortionBase& Source);

	UAGX_LensDistortionBase* GetOrCreateInstance(UWorld* PlayingWorld);
	FLensDistortionBarrier* GetOrCreateNative();

	bool IsInstance() const;

	UAGX_LensDistortionBase* GetInstance();
	const UAGX_LensDistortionBase* GetInstance() const;

	UAGX_LensDistortionBase* GetAsset();
	const UAGX_LensDistortionBase* GetAsset() const;

	virtual void CopyProperties(const UAGX_LensDistortionBase& Source);

protected:
	virtual void CreateNative() PURE_VIRTUAL(UAGX_LensDistortionBase::CreateNative, );

protected:
	TWeakObjectPtr<UAGX_LensDistortionBase> Asset;
	TWeakObjectPtr<UAGX_LensDistortionBase> Instance;
	TUniquePtr<FLensDistortionBarrier> NativeBarrier;
};
