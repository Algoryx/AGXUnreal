// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_CameraBackendPropagator.h"
#include "Sensors/AGX_CameraOutputColor.h"
#include "Sensors/AGX_CameraSensorCaptureHelper.h"
#include "Sensors/AGX_SceneCaptureComponent2DReference.h"
#include "Sensors/AGX_SensorComponentBase.h"

#include "AGX_CameraSensorComponent.generated.h"

struct FCameraBarrier;
struct FCameraLensBarrier;
struct FCameraLensSingleElementBarrier;
struct FCameraOutputBarrier;
struct FCameraOutputColorBarrier;
struct FCameraPhotodetectorBarrier;
struct FAGX_CameraOutputBase;
struct FAGX_ImportContext;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class UAGX_CameraLensBase;
class UAGX_CameraPhotodetectorBase;
class USceneCaptureComponent2D;
class UTextureRenderTarget2D;

USTRUCT()
struct AGXUNREAL_API FCameraOutputRenderContext
{
	GENERATED_BODY()

	// Given to SceneCaptureComponent before capture.
	UPROPERTY(Transient)
	TObjectPtr<UTextureRenderTarget2D> SceneRenderTarget;

	// Holds the result of Material Passes.
	UPROPERTY(Transient)
	TArray<TObjectPtr<UTextureRenderTarget2D>> RenderTargets;

	// MaterialInstances created from the set MaterialPasses used when executing the Material
	// Passes.
	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInstanceDynamic>> MaterialInstances;

	FAGX_CameraSensorCaptureHelper CaptureHelper;
};

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

	bool AddOutput(FAGX_CameraOutputBase& InOutput);

	virtual void CopyFrom(const FSensorBarrier& Barrier, FAGX_ImportContext* Context) override;

	/**
	 * Optional Scene Capture Component 2D to use instead of the one automatically created by this
	 * Camera Sensor Component. When set, the CaptureSourceOverride's existing render target is used
	 * as the camera material passes input. Note that when using the CaptureSourceOverride, the
	 * transform of this AGX Camera Sensor Component has no effect.
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

	/**
	 * Get the Render Target containing the latest output from the Material Passes.
	 * The returned Render Target may stop being the active output if MaterialPasses is modified
	 * during Play.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Camera")
	UTextureRenderTarget2D* GetOutputRenderTarget(UPARAM(ref)
													  const FAGX_CameraOutputColor& Output) const;

	//~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	virtual void PostApplyToComponent() override;
	virtual void TickComponent(
		float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;
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

	/// Executes the MaterialPasses and returns the final render target.
	UTextureRenderTarget2D* RenderMaterialPasses(
		FCameraOutputRenderContext& OutputRenderContext,
		const FCameraOutputColorBarrier& OutputColorBarrier);

	bool RequestCapture(const FCameraOutputColorBarrier& OutputColorBarrier);
	void PollCaptures();

	UTextureRenderTarget2D* CreateRenderTarget(
		const FIntPoint& InResolution, EAGX_CameraOutputChannelType ChannelType,
		uint8 ChannelCount);
	static bool IsResolutionValid(const FIntPoint& InResolution);

#if WITH_EDITOR
	void InitPropertyDispatcher();
#endif

	FCameraOutputRenderContext* GetOrCreateOutputRenderContext(
		const FCameraOutputColorBarrier& OutputColorBarrier);

	// Updates the FCameraOutputRenderContext according to the given OutputColorBarrier. Does not
	// set material parameters, see UpdateMaterialParameters for that.
	bool UpdateOutputRenderContextNoParams(
		FCameraOutputRenderContext& OutputRenderContext,
		const FCameraOutputColorBarrier& OutputColorBarrier, bool bLogWarnings = false);

	/// Write output specific parameters to the given OutMaterials.
	void UpdateMaterialParameters(
		const FCameraOutputColorBarrier& OutputColorBarrier,
		TArray<TObjectPtr<UMaterialInstanceDynamic>>& OutMaterials);

	/// Internal functions called by the Camera Backend.
	void OnBackendSetCameraLensSingleElement(const FCameraLensSingleElementBarrier& LensBarrier);
	void OnBackendSetCameraColorOutput(const FCameraOutputColorBarrier& OutputColorBarrier);
	void OnBackendRequestCapture(const FCameraOutputBarrier& OutputBarrier);

	UPROPERTY(Transient)
	TObjectPtr<USceneCaptureComponent2D> OwnedCaptureComponent2D {nullptr};

	// Per-output render context. Key is native Output address.
	UPROPERTY(Transient)
	TMap<uint64, FCameraOutputRenderContext> OutputRenderContexts;

	FAGX_CameraBackendPropagator CameraBackendPropagator;
};
