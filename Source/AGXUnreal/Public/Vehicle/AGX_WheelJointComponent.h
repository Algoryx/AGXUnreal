// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Constraints/AGX_ConstraintComponent.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_WheelJointComponent.generated.h"

class FWheelJointBarrier;

/**
 * Locks all degrees of freedom except for rotation around the Z-axis.
 */
UCLASS(
	ClassGroup = "AGX_Vehicle", Category = "AGX", Blueprintable,
	Meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_WheelJointComponent : public UAGX_ConstraintComponent
{
	GENERATED_BODY()

public:
	using FBarrierType = FWheelJointBarrier;

public:
	UAGX_WheelJointComponent();
	virtual ~UAGX_WheelJointComponent() override;

	FWheelJointBarrier* GetNativeWheelJoint();
	const FWheelJointBarrier* GetNativeWheelJoint() const;

private:
	virtual void CreateNativeImpl() override;
};
