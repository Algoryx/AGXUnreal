// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "Components/SceneComponent.h"

#include "AGX_WireInstanceData.generated.h"

class UAGX_WireComponent;

USTRUCT()
struct AGXUNREAL_API FAGX_WireInstanceData : public FSceneComponentInstanceData
{
	GENERATED_BODY();

public:
	FAGX_WireInstanceData() = default;
	FAGX_WireInstanceData(const UAGX_WireComponent* Wire);
	virtual ~FAGX_WireInstanceData() = default;

	//~ Begin FComponentInstanceData interface.
	virtual bool ContainsData() const override;
	virtual void ApplyToComponent(
		UActorComponent* Component, const ECacheApplyPhase CacheApplyPhase) override;
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual void FindAndReplaceInstances(
		const TMap<UObject*, UObject*>& OldToNewInstanceMap) override;
	//~ End FComponentInstanceData interface.

	bool HasNativeAddress() const;

private:
	UPROPERTY()
	uint64 NativeWireAddress {0};

	UPROPERTY()
	uint64 NativeBeginWinchAddress {0};

	UPROPERTY()
	uint64 NativeEndWinchAddress {0};
};
