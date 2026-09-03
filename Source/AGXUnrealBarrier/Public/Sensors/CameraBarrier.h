// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/CameraOutputBarrier.h"
#include "Sensors/SensorBarrier.h"

#include "CameraBarrier.generated.h"

struct FCameraLensBarrier;
struct FCameraLensSingleElementBarrier;
struct FCameraOutputColorBarrier;
struct FCameraPhotodetectorBarrier;
struct FAGX_CameraCaptureState;
struct FRigidBodyBarrier;
class FCameraBackendPropagatorBase;

USTRUCT(BlueprintType)
struct AGXUNREALBARRIER_API FCameraBarrier : public FSensorBarrier
{
	GENERATED_BODY()

	FCameraBarrier() = default;
	FCameraBarrier(
		std::shared_ptr<FSensorRef> Native, std::shared_ptr<FSensorGroupStepStrideRef> StepStride);
	virtual ~FCameraBarrier() override = default;

	/// Also calls RegisterWithBackend.
	void AllocateNative(
		const FTransform& Transform, FCameraLensBarrier* Lens,
		FCameraPhotodetectorBarrier* Photodetector);

	virtual void ReleaseNative() override;

	void RegisterWithBackend();
	void UnregisterFromBackend();

	void SetTransform(const FTransform& Transform);
	FTransform GetTransform() const;

	/// Returns the Rigid Body this Camera is attached to, if it exists.
	FRigidBodyBarrier GetRigidBody() const;

	/// Also registers the output with the CameraBackend.
	void AddOutput(FCameraOutputBarrier& Output);
	TArray<FCameraOutputBarrier> GetOutputs() const;

	void MarkOutputAsRead();

	void SetBackendPropagator(FCameraBackendPropagatorBase* InPropagator);
	FCameraBackendPropagatorBase* GetBackendPropagator() const;

	/** Internal functions, only called by the CameraBackendBarrier. */
	void OnBackendSynchronize(TArray<FAGX_CameraCaptureState>& CaptureStates, double DeltaTime);
	void OnBackendSetCameraLensSingleElement(const FCameraLensSingleElementBarrier& LensBarrier);
	void OnBackendSetCameraColorOutput(const FCameraOutputColorBarrier& OutputColorBarrier);
	void OnBackendRequestCapture(const FCameraOutputBarrier& OutputBarrier);

	static bool IsCamera(const FSensorBarrier& Sensor);

private:
	FCameraBackendPropagatorBase* BackendPropagator = nullptr;
};
