// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_CameraLensBase.h"

#include "AGX_CameraLensSingleElement.generated.h"

struct FCameraLensSingleElementBarrier;

/**
 * AGX Camera single element lens asset.
 */
UCLASS(ClassGroup = "AGX_Sensor", Category = "AGX", BlueprintType)
class AGXUNREAL_API UAGX_CameraLensSingleElement : public UAGX_CameraLensBase
{
	GENERATED_BODY()

public:
	/**
	 * Focal length of the lens [cm].
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Camera Lens", Meta = (ClampMin = "0.0"))
	double FocalLength {0.188};

	UFUNCTION(BlueprintCallable, Category = "AGX Camera Lens")
	void SetFocalLength(double InFocalLength);

	UFUNCTION(BlueprintCallable, Category = "AGX Camera Lens")
	double GetFocalLength() const;

	/**
	 * Lens aperture f-stop value.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Camera Lens", Meta = (ClampMin = "0.0"))
	double FStop {2.0};

	UFUNCTION(BlueprintCallable, Category = "AGX Camera Lens")
	void SetFStop(double InFStop);

	UFUNCTION(BlueprintCallable, Category = "AGX Camera Lens")
	double GetFStop() const;

	/**
	 * Whether to use autofocus. If false, manual focus is used.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Camera Lens")
	bool bUseAutofocus {true};

	UFUNCTION(BlueprintCallable, Category = "AGX Camera Lens")
	void SetUseAutofocus(bool bInUseAutofocus);

	UFUNCTION(BlueprintCallable, Category = "AGX Camera Lens")
	bool GetUseAutofocus() const;

	/**
	 * Minimum focus distance for autofocus [cm].
	 * Only used when bUseAutofocus is true.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Camera Lens",
		Meta = (ClampMin = "0.0", EditCondition = "bUseAutofocus"))
	double MinimumFocusDistance {10.0};

	UFUNCTION(BlueprintCallable, Category = "AGX Camera Lens")
	void SetMinimumFocusDistance(double InMinimumFocusDistance);

	UFUNCTION(BlueprintCallable, Category = "AGX Camera Lens")
	double GetMinimumFocusDistance() const;

	/**
	 * Focus distance for manual focus control [cm].
	 * Only used when bUseAutofocus is false.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Camera Lens",
		Meta = (ClampMin = "0.0", EditCondition = "!bUseAutofocus"))
	double FocusDistance {150.0};

	UFUNCTION(BlueprintCallable, Category = "AGX Camera Lens")
	void SetFocusDistance(double InFocusDistance);

	UFUNCTION(BlueprintCallable, Category = "AGX Camera Lens")
	double GetFocusDistance() const;

	FCameraLensSingleElementBarrier* GetNativeAsSingleElement();
	const FCameraLensSingleElementBarrier* GetNativeAsSingleElement() const;

	virtual void CopyProperties(const UAGX_CameraLensBase& Source) override;

	// ~Begin UObject interface.
	virtual void PostInitProperties() override;
#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
#endif
	// ~End UObject interface.

private:
#if WITH_EDITOR
	void InitPropertyDispatcher();
#endif

	void UpdateNativeProperties();

protected:
	virtual void CreateNative() override;
};
