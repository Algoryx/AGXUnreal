// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_CameraOutputBase.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "AGX_CameraOutputColor.generated.h"

class UAGX_CameraSensorComponent;

USTRUCT(BlueprintType)
struct AGXUNREAL_API FAGX_CameraOutputColor : public FAGX_CameraOutputBase
{
	GENERATED_BODY()

public:
	virtual ~FAGX_CameraOutputColor() override = default;

	FAGX_CameraOutputColor& operator=(const FAGX_CameraOutputColor& Other);
	bool operator==(const FAGX_CameraOutputColor& Other) const;

protected:
	virtual TUniquePtr<FCameraOutputBarrier> CreateNativeBarrier() const override;
};

/**
 * This class acts as an API that exposes functions of FAGX_CameraOutputColor in Blueprints.
 */
UCLASS()
class AGXUNREAL_API UAGX_CameraOutputColor_LF : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "AGX Camera")
	static void AddTo(
		UPARAM(ref) FAGX_CameraOutputColor& Output, UAGX_CameraSensorComponent* Camera)
	{
		Output.AddTo(Camera);
	}
};
