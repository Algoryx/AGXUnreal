// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Tires/TireBarrier.h"

// Unreal Engine includes.
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "AGX_TireComponent.generated.h"

/**
 * Base class for Tire model components.
 */
UCLASS(Category = "AGX", ClassGroup = "AGX", NotPlaceable)
class AGXUNREAL_API UAGX_TireComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAGX_TireComponent();
	virtual ~UAGX_TireComponent() = default;

	UPROPERTY(EditAnywhere, Category = "Rendering")
	bool Visible = true;

	bool HasNative() const;

	FTireBarrier* GetOrCreateNative();

	FTireBarrier* GetNative();

	const FTireBarrier* GetNative() const;

protected:
	TUniquePtr<FTireBarrier> NativeBarrier;

	// ~Begin UActorComponent interface.
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	// ~End UActorComponent interface.

	virtual void AllocateNative() PURE_VIRTUAL(UAGX_TireComponent::CreateNativeImpl, );

	virtual void UpdateNativeProperties()
		PURE_VIRTUAL(UAGX_TireComponent::UpdateNativeProperties, );

private:
	void CreateNative();
};
