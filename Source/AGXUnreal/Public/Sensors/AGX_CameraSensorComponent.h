// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_SensorComponentBase.h"

#include "AGX_CameraSensorComponent.generated.h"

struct FCameraBarrier;

/**
 * Todo: add API comment.
 */
UCLASS(
	ClassGroup = "AGX_Sensor", Category = "AGX", Blueprintable,
	Meta = (BlueprintSpawnableComponent),
	Hidecategories = (Cooking, Collision, LOD, Physics, Rendering, Replication))
class AGXUNREAL_API UAGX_CameraSensorComponent : public UAGX_SensorComponentBase
{
	GENERATED_BODY()

public:
	UAGX_CameraSensorComponent();

	void UpdateNativeTransform();

	FSensorBarrier* CreateNativeImpl() override;

	//~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	//~ End UActorComponent Interface

	FCameraBarrier* GetNativeAsCamera();
	const FCameraBarrier* GetNativeAsCamera() const;

protected:
	virtual void MarkOutputAsRead() override;

private:
	virtual void UpdateNativeProperties() override;
};
