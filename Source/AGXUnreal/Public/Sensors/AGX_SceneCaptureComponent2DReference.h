// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_ComponentReference.h"

// Unreal Engine includes.
#include "Kismet/BlueprintFunctionLibrary.h"

#include "AGX_SceneCaptureComponent2DReference.generated.h"

class USceneCaptureComponent2D;

USTRUCT(BlueprintType)
struct AGXUNREAL_API FAGX_SceneCaptureComponent2DReference : public FAGX_ComponentReference
{
	GENERATED_BODY()

	FAGX_SceneCaptureComponent2DReference();

	USceneCaptureComponent2D* GetSceneCaptureComponent2D() const;
};

FORCEINLINE uint32 GetTypeHash(const FAGX_SceneCaptureComponent2DReference& Thing)
{
	uint32 Hash = FCrc::MemCrc32(&Thing, sizeof(FAGX_SceneCaptureComponent2DReference));
	return Hash;
}

// Blueprint API

UCLASS()
class AGXUNREAL_API UAGX_SceneCaptureComponent2DReference_FL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "AGX Scene Capture Component 2D")
	static void SetSceneCaptureComponent2D(
		UPARAM(Ref) FAGX_SceneCaptureComponent2DReference& Reference,
		USceneCaptureComponent2D* Component);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Scene Capture Component 2D")
	static USceneCaptureComponent2D* GetSceneCaptureComponent2D(
		UPARAM(Ref) FAGX_SceneCaptureComponent2DReference& Reference);
};
