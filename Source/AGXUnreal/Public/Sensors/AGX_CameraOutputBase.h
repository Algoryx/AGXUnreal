// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/CameraOutputBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_CameraOutputBase.generated.h"

class UAGX_CameraSensorComponent;

USTRUCT(BlueprintType, BlueprintInternalUseOnly)
struct AGXUNREAL_API FAGX_CameraOutputBase
{
	GENERATED_BODY()

public:
	FAGX_CameraOutputBase() = default;
	FAGX_CameraOutputBase(const FAGX_CameraOutputBase& Other);
	virtual ~FAGX_CameraOutputBase();

	/**
	 * Output resolution [pixels].
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Camera", Meta = (ClampMin = "1"))
	FIntPoint Resolution {256, 256};

	void SetResolution(FIntPoint InResolution);
	FIntPoint GetResolution() const;

	/**
	 * How often the Camera Sensor should capture a frame [Hz].
	 * This is only used when Contant Capture is enabled.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX Camera",
		Meta = (ClampMin = "0.0", EditCondition = "bConstantCapture"))
	double FrameRate {10.0};

	void SetFrameRate(double InFrameRate);
	double GetFrameRate() const;

	/**
	 * Whether the Camera Sensor captures continuously according to the set Frame Rate or waits for
	 * manual capture requests.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Camera")
	bool bConstantCapture {true};

	void SetConstantCapture(bool bInConstantCapture);
	bool GetConstantCapture() const;

	bool HasNative() const;
	FCameraOutputBarrier* GetOrCreateNative();
	const FCameraOutputBarrier* GetNative() const;
	FCameraOutputBarrier* GetNative();

	// Making UAGX_CameraSensorComponent::AddOutput Blueprint friendly was not so easy since
	// non-const references becomes out-variables, and pointers to structs are not permitted as
	// input argument.
	bool AddTo(UAGX_CameraSensorComponent* Camera);

	FAGX_CameraOutputBase& operator=(const FAGX_CameraOutputBase& Other);
	bool operator==(const FAGX_CameraOutputBase& Other) const;

protected:
	void ApplyBasePropertiesToNative(FCameraOutputBarrier& Native) const;

	virtual TUniquePtr<FCameraOutputBarrier> CreateNativeBarrier() const
		PURE_VIRTUAL(FAGX_CameraOutputBase::CreateNativeBarrier, return nullptr;);

private:
	TUniquePtr<FCameraOutputBarrier> NativeBarrier;
};
