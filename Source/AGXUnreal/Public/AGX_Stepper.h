// Copyright 2025, Algoryx Simulation AB.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AGX_Stepper.generated.h"

UCLASS(ClassGroup = "AGX", Category = "AGX", NotPlaceable)
class AGXUNREAL_API AAGX_Stepper : public AActor
{
	GENERATED_BODY()
public:
	AAGX_Stepper();
	virtual ~AAGX_Stepper() override;

	virtual void BeginPlay() override;
	void Tick(float DeltaTime) override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
