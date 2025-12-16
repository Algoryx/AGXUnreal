// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "Components/ActorComponent.h"

#include "AGX_NativeOwnerInstanceData.generated.h"


/**
 * Component Instance Data for Actor Components.
 * Do not use for SceneComponents! Instead, use the FAGX_NativeOwnerSceneComponentInstanceData
 * for those.
 */
USTRUCT()
struct AGXUNREAL_API FAGX_NativeOwnerInstanceData : public FActorComponentInstanceData
{
	GENERATED_BODY();

	FAGX_NativeOwnerInstanceData() = default;
	FAGX_NativeOwnerInstanceData(
		const IAGX_NativeOwner* NativeOwner, const UActorComponent* SourceComponent,
		TFunction<IAGX_NativeOwner*(UActorComponent*)> InDowncaster);
	virtual ~FAGX_NativeOwnerInstanceData() = default;

	//~ Begin FComponentInstanceData interface.
	virtual void ApplyToComponent(
		UActorComponent* Component, const ECacheApplyPhase CacheApplyPhase) override;

	virtual bool ContainsData() const override;
	//~ End FComponentInstanceData interface.

private:
	UPROPERTY()
	uint64 NativeAddress {0};

	TFunction<IAGX_NativeOwner*(UActorComponent*)> Downcaster;
};
