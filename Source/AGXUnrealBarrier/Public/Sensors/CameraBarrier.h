// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/SensorBarrier.h"

#include "CameraBarrier.generated.h"

struct FCameraBackendBarrier;

USTRUCT(BlueprintType)
struct AGXUNREALBARRIER_API FCameraBarrier : public FSensorBarrier
{
	GENERATED_BODY()

	FCameraBarrier() = default;
	FCameraBarrier(
		std::shared_ptr<FSensorRef> Native, std::shared_ptr<FSensorGroupStepStrideRef> StepStride);
	virtual ~FCameraBarrier() override = default;

	void AllocateNative(const FTransform& Transform, FCameraBackendBarrier& CameraBackend);
	virtual void ReleaseNative() override;

	void SetTransform(const FTransform& Transform);
	FTransform GetTransform() const;

	void MarkOutputAsRead();
};
