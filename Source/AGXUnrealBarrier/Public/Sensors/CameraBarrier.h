// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/SensorBarrier.h"

#include "CameraBarrier.generated.h"

struct FCameraBackendBarrier;
struct FCameraLensBarrier;
struct FCameraLensSingleElementParameters;
struct FCameraOutputBarrier;
struct FCameraPhotodetectorBarrier;
class FCameraBackendPropagatorBase;

struct FCameraLatestImage // TODO: this will be completely removed.
{
	TArray<FColor> Pixels;
	FIntPoint Resolution {0, 0};
	bool bHasImage {false};

	void Reset()
	{
		Pixels.Empty();
		Resolution = {0, 0};
		bHasImage = false;
	}
};

USTRUCT(BlueprintType)
struct AGXUNREALBARRIER_API FCameraBarrier : public FSensorBarrier
{
	GENERATED_BODY()

	FCameraBarrier() = default;
	FCameraBarrier(
		std::shared_ptr<FSensorRef> Native, std::shared_ptr<FSensorGroupStepStrideRef> StepStride);
	virtual ~FCameraBarrier() override = default;

	void AllocateNative(
		const FTransform& Transform, FCameraBackendBarrier& CameraBackend,
		FCameraLensBarrier* Lens, FCameraPhotodetectorBarrier* Photodetector);
	virtual void ReleaseNative() override;

	void SetTransform(const FTransform& Transform);
	FTransform GetTransform() const;

	void AddOutput(FCameraOutputBarrier& Output);

	void MarkOutputAsRead();

	void SetBackendPropagator(FCameraBackendPropagatorBase* InPropagator);
	FCameraBackendPropagatorBase* GetBackendPropagator() const;

	/** Internal functions, only called by the CameraBackendBarrier. */
	void OnBackendSetCameraLensSingleElement(const FCameraLensSingleElementParameters& Parameters);

	FCameraLatestImage LatestImage; // TODO: this will be completely removed.

private:
	FCameraBackendPropagatorBase* BackendPropagator = nullptr;
};
