// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "RigidBodyBarrier.h"
#include "Vehicle/AGX_TrackEnums.h"

// Standard library includes.
#include <memory>

struct FTrackWheelCreationData;
struct FTrackWheelRef;


/*
 * This is currently mostly used by the Import pipeline.
 * In the future, it would be possible to let the FAGX_TrackWheel UStruct own an instance of this
 * class and be responsible to keep it in sync, and expose relevant functions to the user in the
 * Editor.
 */
class AGXUNREALBARRIER_API FTrackWheelBarrier
{
public:
	FTrackWheelBarrier();
	FTrackWheelBarrier(std::unique_ptr<FTrackWheelRef> Native);
	FTrackWheelBarrier(FTrackWheelBarrier&& Other);
	~FTrackWheelBarrier();

	void AllocateNative(const FTrackWheelCreationData& Data);

	FRigidBodyBarrier GetRigidBody() const;

	double GetRadius() const;

	EAGX_TrackWheelModel GetModel() const;

	bool GetSplitSegments() const;

	bool GetMoveNodesToRotationPlane() const;

	bool GetMoveNodesToWheel() const;

	FVector GetRelativeLocation() const;
	FRotator GetRelativeRotation() const;

	// Native Handling.
	bool HasNative() const;
	FTrackWheelRef* GetNative();
	const FTrackWheelRef* GetNative() const;

private:
	std::unique_ptr<FTrackWheelRef> NativeRef;
};

struct AGXUNREALBARRIER_API FTrackWheelCreationData
{
	uint8 Model {0};
	double Radius {0.0};

	FRigidBodyBarrier RigidBody;
	FVector RelativePosition;
	FQuat RelativeRotation;

	bool bSplitSegments {false};
	bool bMoveNodesToRotationPlane {false};
	bool bMoveNodesToWheel {false};
};
