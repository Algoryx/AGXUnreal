// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <memory>

#include "SensorBarrier.generated.h"

class FSensorEnvironmentBarrier;

struct FSensorGroupStepStrideRef;
struct FSensorRef;

USTRUCT(BlueprintType)
struct AGXUNREALBARRIER_API FSensorBarrier
{
	GENERATED_BODY()

	FSensorBarrier();
	FSensorBarrier(
		std::shared_ptr<FSensorRef> Native, std::shared_ptr<FSensorGroupStepStrideRef> StepStride);
	virtual ~FSensorBarrier();

	void AllocateStepStride();
	bool HasStepStrideNative() const;

	bool HasNative() const;
	FSensorRef* GetNative();
	const FSensorRef* GetNative() const;
	void ReleaseNative();

	void SetEnabled(bool Enabled);
	bool GetEnabled() const;

	void SetStepStride(uint32 Stride);
	uint32 GetStepStride() const;

	/**
	 * Increment the reference count of the AGX Dynamics object. This should always be paired with
	 * a call to DecrementRefCount, and the count should only be artificially incremented for a
	 * very well specified duration.
	 *
	 * One use-case is during a Blueprint Reconstruction, when the Unreal Engine objects are
	 * destroyed and then recreated. During this time the AGX Dynamics objects are retained and
	 * handed between the old and the new Unreal Engine objects through a Component Instance Data.
	 * This Component Instance Data instance is considered the owner of the AGX Dynamics object
	 * during this transition period and the reference count is therefore increment during its
	 * lifetime. We're lending out ownership of the AGX Dynamics object to the Component Instance
	 * Data instance for the duration of the Blueprint Reconstruction.
	 *
	 * These functions can be const even though they have observable side effects because the
	 * reference count is not a salient part of the AGX Dynamics objects, and they are thread-safe.
	 */
	void IncrementRefCount() const;
	void DecrementRefCount() const;

	uint64 GetNativeAddress() const;
	void SetNativeAddress(uint64 Address);

	bool AddToEnvironment(FSensorEnvironmentBarrier& Environment);
	bool RemoveFromEnvironment(FSensorEnvironmentBarrier& Environment);

	FGuid GetGuid() const;
	FString GetName() const;

protected:
	std::shared_ptr<FSensorRef> NativeRef;
	std::shared_ptr<FSensorGroupStepStrideRef> StepStrideRef;
};
