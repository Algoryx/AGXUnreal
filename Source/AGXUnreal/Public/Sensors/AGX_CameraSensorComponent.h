// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_CameraBackendPropagator.h"
#include "Sensors/AGX_SceneCaptureComponent2DReference.h"
#include "Sensors/AGX_SensorComponentBase.h"

#include "AGX_CameraSensorComponent.generated.h"

struct FCameraBarrier;
struct FCameraLensBarrier;
struct FCameraLensSingleElementParameters;
struct FCameraPhotodetectorBarrier;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class UAGX_CameraLensBase;
class UAGX_CameraPhotodetectorBase;
class USceneCaptureComponent2D;
class UTextureRenderTarget2D;

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

	/**
	 * Camera photodetector to use when creating the native AGX Camera. If unset, a default AGX
	 * CMOS Sensor is used.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Camera", Meta = (ExposeOnSpawn))
	UAGX_CameraPhotodetectorBase* PhotoDetector {nullptr};

	/**
	 * Camera lens to use when creating the native AGX Camera. If unset, a default AGX single
	 * element lens is used.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Camera", Meta = (ExposeOnSpawn))
	UAGX_CameraLensBase* CameraLens {nullptr};

	/**
	 * Output resolution of the Camera Sensor [pixels].
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Camera", Meta = (ClampMin = "1"))
	FIntPoint Resolution {256, 256}; // TODO: remove this, resolution will be set on CameraCMOSSensor instead.

	/**
	 * Set the output resolution of the Camera Sensor.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Camera")
	void SetResolution(FIntPoint InResolution);

	/**
	 * Name of the texture parameter that receives the previous render pass output.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Camera")
	FName MaterialInputTextureParameterName {TEXT("InputTexture")};

	/**
	 * Set the name of the texture parameter that receives the previous render pass output.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Camera")
	void SetMaterialInputTextureParameterName(FName InParameterName);

	/**
	 * Materials used as full-screen render passes after the Scene Capture Component has captured
	 * the scene.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Camera")
	TArray<TObjectPtr<UMaterialInterface>> MaterialPasses;

	UFUNCTION(BlueprintCallable, Category = "AGX Camera")
	void AddMaterialPass(UMaterialInterface* Material);

	UFUNCTION(BlueprintCallable, Category = "AGX Camera")
	bool SetMaterialPass(int32 Index, UMaterialInterface* Material);

	UFUNCTION(BlueprintCallable, Category = "AGX Camera")
	bool RemoveMaterialPass(UMaterialInterface* Material);

	UFUNCTION(BlueprintCallable, Category = "AGX Camera")
	bool RemoveMaterialPassAt(int32 Index);

	UFUNCTION(BlueprintCallable, Category = "AGX Camera")
	void ClearMaterialPasses();

	void UpdateNativeTransform();

	/**
	 * Optional Scene Capture Component 2D to use instead of the one automatically created by this
	 * Camera Sensor Component. When set, the CaptureSourceOverride's existing render target is used
	 * as the camera pipeline input. Note that when using the CaptureSourceOverride, the transform
	 * of this AGX Camera Sensor Component has no effect.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, AdvancedDisplay, Category = "AGX Camera")
	FAGX_SceneCaptureComponent2DReference CaptureSourceOverride;

	UFUNCTION(BlueprintCallable, Category = "AGX Camera")
	void SetCaptureSourceOverride(USceneCaptureComponent2D* InCaptureSourceOverride);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Camera")
	bool HasCaptureSourceOverride() const;

	//~ Begin UAGX_SensorComponentBase Interface
	FSensorBarrier* CreateNativeImpl() override;
	//~ End UAGX_SensorComponentBase Interface

	/**
	 * Get the active Scene Capture Component 2D used by this Camera Sensor. Returns the
	 * CaptureSourceOverride if one is set, otherwise returns the internally created capture source.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Camera")
	USceneCaptureComponent2D* GetCaptureSource() const;

	/**
	 * Whether this Camera Sensor has both a Native object and a Scene Capture Component 2D. If
	 * CaptureSourceOverride is set, the referenced Scene Capture Component 2D must also have a
	 * render target.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Camera")
	bool IsCameraSensorValid() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Camera")
	UTextureRenderTarget2D* RenderCameraPipeline();

	//~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	virtual void PostApplyToComponent() override;
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;
#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif
	virtual void OnRegister() override;
	//~ End UActorComponent Interface

	//~ Begin UObject Interface
#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
	virtual void PostInitProperties() override;
#endif
	//~ End UObject Interface

	FCameraBarrier* GetNativeAsCamera();
	const FCameraBarrier* GetNativeAsCamera() const;

private:
	friend class FAGX_CameraBackendPropagator;

	//~ Begin UAGX_SensorComponentBase Interface
	virtual void MarkOutputAsRead() override;
	virtual void UpdateNativeProperties() override;
	//~ End UAGX_SensorComponentBase Interface

	void SetupSceneCapture();
	void SetupCameraBackendPropagator();
	void UpdateCameraPhotoDetector();
	void UpdateCameraLens();
	void SetupRenderPasses();
	void EnsureRenderTargets();

	/// The Resolution property or the Render Target size when CaptureSourceOverride is used.
	FIntPoint GetActiveResolution() const;

	UTextureRenderTarget2D* CreateRenderTarget(const FIntPoint& InResolution);
	bool IsRenderTargetUpToDate(
		const UTextureRenderTarget2D* RenderTarget, const FIntPoint& InResolution) const;
	static bool IsResolutionValid(const FIntPoint& InResolution);

#if WITH_EDITOR
	void InitPropertyDispatcher();
#endif

	/// Internal functions called by the Camera Backend.
	void OnBackendSetCameraLensSingleElement(const FCameraLensSingleElementParameters& Parameters);

	UPROPERTY(Transient)
	TObjectPtr<USceneCaptureComponent2D> OwnedCaptureComponent2D {nullptr};

	UPROPERTY(Transient)
	TObjectPtr<UTextureRenderTarget2D> SceneRenderTarget {nullptr};

	UPROPERTY(Transient)
	TArray<TObjectPtr<UTextureRenderTarget2D>> RenderTargets;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInstanceDynamic>> MaterialInstances;

	FAGX_CameraBackendPropagator CameraBackendPropagator;
};
