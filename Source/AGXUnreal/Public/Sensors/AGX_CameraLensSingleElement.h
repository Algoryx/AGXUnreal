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
