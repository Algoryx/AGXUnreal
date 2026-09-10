// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/CameraPhotodetectorBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_CameraPhotodetectorBase.generated.h"

class UWorld;

/**
 * Base class for AGX Camera Photodetector assets.
 */
UCLASS(Abstract, ClassGroup = "AGX_Sensor", Category = "AGX", BlueprintType)
class AGXUNREAL_API UAGX_CameraPhotodetectorBase : public UObject
{
	GENERATED_BODY()

public:
	bool HasNative() const;
	FCameraPhotodetectorBarrier* GetNative();
	const FCameraPhotodetectorBarrier* GetNative() const;
	void ReleaseNative();

	void CommitToAsset();

	static UAGX_CameraPhotodetectorBase* CreateInstanceFromAsset(
		UWorld* PlayingWorld, UAGX_CameraPhotodetectorBase& Source);

	UAGX_CameraPhotodetectorBase* GetOrCreateInstance(UWorld* PlayingWorld);
	FCameraPhotodetectorBarrier* GetOrCreateNative();

	bool IsInstance() const;

	UAGX_CameraPhotodetectorBase* GetInstance();
	const UAGX_CameraPhotodetectorBase* GetInstance() const;

	UAGX_CameraPhotodetectorBase* GetAsset();
	const UAGX_CameraPhotodetectorBase* GetAsset() const;

	virtual void CopyProperties(const UAGX_CameraPhotodetectorBase& Source);

protected:
	virtual void CreateNative() PURE_VIRTUAL(UAGX_CameraPhotodetectorBase::CreateNative, );

protected:
	TWeakObjectPtr<UAGX_CameraPhotodetectorBase> Asset;
	TWeakObjectPtr<UAGX_CameraPhotodetectorBase> Instance;
	TUniquePtr<FCameraPhotodetectorBarrier> NativeBarrier;
};
