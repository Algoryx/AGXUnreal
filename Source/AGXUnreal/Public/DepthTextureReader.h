// Copyright 2023, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "Engine/TextureRenderTarget2D.h"

#include "DepthTextureReader.generated.h"


UCLASS(
	ClassGroup = "AGX", Category = "AGX", Experimental, Meta = (BlueprintSpawnableComponent),
	Hidecategories = (Cooking, Collision, LOD, Physics, Rendering, Replication))
class AGXUNREAL_API UDepthTextureReader : public UActorComponent
{
	GENERATED_BODY()

public:
	UDepthTextureReader();

	UPROPERTY(EditAnywhere, Category = "Depth")
	UTextureRenderTarget2D* DepthTexture;
	
	UFUNCTION(BlueprintCallable, Category = "Depth")
	void Execute(const FVector& RefPos, const FQuat& RefRot);

	UFUNCTION(BlueprintCallable, Category = "Depth")
	void DrawDebugPoints(const TArray<FVector4>& Points);
};
