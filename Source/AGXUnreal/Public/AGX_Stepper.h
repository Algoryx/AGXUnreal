#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AGX_Stepper.generated.h"

UCLASS(ClassGroup = "AGX", Category = "AGX")
class AGXUNREAL_API AAGX_Stepper : public AActor
{
	GENERATED_BODY()
public:
	AAGX_Stepper();
	~AAGX_Stepper();

	void Tick(float DeltaTime) override;
};