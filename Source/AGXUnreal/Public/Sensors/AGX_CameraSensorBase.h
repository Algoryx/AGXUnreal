// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "Components/SceneComponent.h"
#include "CoreMinimal.h"

#include "AGX_CameraSensorBase.generated.h"

class USceneCaptureComponent2D;
class UTextureRenderTarget2D;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNewImageROS2, const FAGX_SensorMsgsImage&, Image);

/**
 * Camera Sensor Component, allowing to extract camera pixel information in runtime.
 */
UCLASS(ClassGroup = "AGX_Sensor", Category = "AGX", Abstract, NotPlaceable)
class AGXUNREAL_API UAGX_CameraSensorBase : public USceneComponent
{
	GENERATED_BODY()

public:
	UAGX_CameraSensorBase();

	/**
	 * Horizontal Field of View (FOV) of the Camera Sensor [deg].
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX Camera",
		meta = (ClampMin = "0.0", ClampMax = "170.0"))
	float FOV {90.f};

	UFUNCTION(BlueprintCallable, Category = "AGX Camera")
	void SetFOV(float InFOV);

	/**
	 * Output resolution of the Camera Sensor [pixels].
	 * The first element is the horizontal resolution, and the second vertical resolution.
	 * Note: using a large resolution will come with a performance hit.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Camera", meta = (ClampMin = "1"))
	FIntPoint Resolution {256, 256};

	/**
	 * Sets the resolution of the Camera Sensor.
	 * Also updates the RenderTarget if available.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Camera")
	void SetResolution(FIntPoint InResolution);

	/**
	 * Render Target used by the Camera Sensor to write pixel data to.
	 * It is recommended to use the 'Generate Runtime Assets' button in the Details Panel to
	 * generate it.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Camera")
	UTextureRenderTarget2D* RenderTarget {nullptr};

	/**
	 * Access the Scene Capture Component 2D used by the AGX Camera Sensor.
	 * If the Scene Capture Component 2D has not been created, this function returns nullptr.
	 * The Scene Capture Component 2D is only available during Play.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Camera")
	USceneCaptureComponent2D* GetSceneCaptureComponent2D() const;

	static bool IsFovValid(float FOV);
	static bool IsResolutionValid(const FIntPoint& Resolution);

	//~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	virtual void PostApplyToComponent() override;
	//~ End UActorComponent Interface

	//~ Begin UObject Interface
#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* InProperty) const override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
	virtual void PostInitProperties() override;
#endif
	//~ End UObject Interface

protected:
	virtual bool CheckValid() const;
	virtual void Init();

protected:
	bool bIsValid {false};

private:
	USceneCaptureComponent2D* CaptureComponent2D;
	void InitCaptureComponent();
};
