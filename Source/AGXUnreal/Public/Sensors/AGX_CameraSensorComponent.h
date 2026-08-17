// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_SensorComponentBase.h"

#include "AGX_CameraSensorComponent.generated.h"

struct FCameraBarrier;
class UMaterialInstanceDynamic;
class UMaterialInterface;
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
	 * Output resolution of the Camera Sensor [pixels].
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Camera", Meta = (ClampMin = "1"))
	FIntPoint Resolution {256, 256};

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

	UFUNCTION(BlueprintCallable, Category = "AGX Camera")
	UTextureRenderTarget2D* RenderCameraPipeline();

	//~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	virtual void PostApplyToComponent() override;
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;
	//~ End UActorComponent Interface

	//~ Begin UObject Interface
#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
	virtual void PostInitProperties() override;
#endif
	//~ End UObject Interface

	FCameraBarrier* GetNativeAsCamera();
	const FCameraBarrier* GetNativeAsCamera() const;

protected:
	virtual void MarkOutputAsRead() override;

private:
	virtual void UpdateNativeProperties() override;

	void SetupSceneCapture();
	void SetupRenderPasses();
	void EnsureRenderTargets();
	UTextureRenderTarget2D* CreateRenderTarget();
	bool IsRenderTargetUpToDate(const UTextureRenderTarget2D* RenderTarget) const;
	static bool IsResolutionValid(const FIntPoint& InResolution);

#if WITH_EDITOR
	void InitPropertyDispatcher();
#endif

	UPROPERTY(Transient)
	TObjectPtr<USceneCaptureComponent2D> CaptureComponent2D {nullptr};

	UPROPERTY(Transient)
	TObjectPtr<UTextureRenderTarget2D> SceneRenderTarget {nullptr};

	UPROPERTY(Transient)
	TArray<TObjectPtr<UTextureRenderTarget2D>> RenderTargets;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInstanceDynamic>> MaterialInstances;
};
