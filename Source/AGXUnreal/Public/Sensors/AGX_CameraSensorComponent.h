// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_SensorComponentBase.h"

#include "AGX_CameraSensorComponent.generated.h"

struct FCameraBarrier;
class USceneCaptureComponent2D;

/**
 * Todo: add API comment.
 */
UCLASS(
	ClassGroup = "AGX_Sensor", Category = "AGX", Blueprintable,
	Meta = (BlueprintSpawnableComponent),
	Hidecategories = (Cooking, Collision, LOD, Physics, Rendering, Replication))
class AGXUNREAL_API UAGX_CameraSensorComponent : public UAGX_SensorComponentBase
{
	GENERATED_BODY()

public:
	UAGX_CameraSensorComponent();

	void UpdateNativeTransform();

	FSensorBarrier* CreateNativeImpl() override;

	/**
	 * Get the Scene Capture Component 2D used by this Camera Sensor.
	 * Only valid during Play.
	 * Returns nullptr if the Camera Sensor has not been initialized.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Camera")
	USceneCaptureComponent2D* GetSceneCaptureComponent2D() const;

	/**
	 * Whether this Camera Sensor has both a Native object and a Scene Capture Component 2D.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Camera")
	bool IsCameraSensorValid() const;

	//~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	virtual void PostApplyToComponent() override;
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;
	//~ End UActorComponent Interface

	FCameraBarrier* GetNativeAsCamera();
	const FCameraBarrier* GetNativeAsCamera() const;

protected:
	virtual void MarkOutputAsRead() override;

private:
	virtual void UpdateNativeProperties() override;

	void SetupSceneCapture();

	UPROPERTY(Transient)
	TObjectPtr<USceneCaptureComponent2D> CaptureComponent2D {nullptr};
};
