// Copyright 2025, Algoryx Simulation AB.

#include "Vehicle/TrackWheelBarrier.h"

// AGX Dynamics for Unreal includes.
#include "BarrierOnly/AGXRefs.h"
#include "BarrierOnly/Vehicle/TrackWheelRef.h"
#include "RigidBodyBarrier.h"
#include "TypeConversions.h"

FTrackWheelBarrier::FTrackWheelBarrier()
	: NativeRef {new FTrackWheelRef}
{
}

FTrackWheelBarrier::FTrackWheelBarrier(std::unique_ptr<FTrackWheelRef> Native)
	: NativeRef(std::move(Native))
{
	check(NativeRef);
}

FTrackWheelBarrier::FTrackWheelBarrier(FTrackWheelBarrier&& Other)
	: NativeRef {std::move(Other.NativeRef)}
{
	Other.NativeRef.reset(new FTrackWheelRef);
}

FTrackWheelBarrier::~FTrackWheelBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the
	// std::unique_ptr NativeRef's destructor must be able to see the definition,
	// not just the forward declaration, of FTrackWheelRef.
}

FRigidBodyBarrier FTrackWheelBarrier::GetRigidBody() const
{
	check(HasNative());

	return FRigidBodyBarrier(std::make_unique<FRigidBodyRef>(NativeRef->Native->getRigidBody()));
}

double FTrackWheelBarrier::GetRadius() const
{
	check(HasNative());
	return ConvertDistanceToUnreal<double>(NativeRef->Native->getRadius());
}

EAGX_TrackWheelModel FTrackWheelBarrier::GetModel() const
{
	check(HasNative());
	return Convert(NativeRef->Native->getModel());
}

bool FTrackWheelBarrier::GetSplitSegments() const
{
	check(HasNative());
	return NativeRef->Native->getProperties().Is(agxVehicle::TrackWheel::SPLIT_SEGMENTS);
}

bool FTrackWheelBarrier::GetMoveNodesToRotationPlane() const
{
	check(HasNative());
	return NativeRef->Native->getProperties().Is(
		agxVehicle::TrackWheel::MOVE_NODES_TO_ROTATION_PLANE);
}

bool FTrackWheelBarrier::GetMoveNodesToWheel() const
{
	check(HasNative());
	return NativeRef->Native->getProperties().Is(agxVehicle::TrackWheel::MOVE_NODES_TO_WHEEL);
}

FVector FTrackWheelBarrier::GetRelativeLocation() const
{
	check(HasNative());
	agx::Frame* FrameAGX = NativeRef->Native->getLocalFrame();
	if (FrameAGX == nullptr)
	{
		return FVector::ZeroVector;
	}

	return ConvertDisplacement(FrameAGX->getLocalTranslate());
}

FRotator FTrackWheelBarrier::GetRelativeRotation() const
{
	check(HasNative());
	agx::Frame* FrameAGX = NativeRef->Native->getLocalFrame();
	if (FrameAGX == nullptr)
	{
		return FRotator::ZeroRotator;
	}

	return Convert(FrameAGX->getLocalRotate()).Rotator();
}

bool FTrackWheelBarrier::HasNative() const
{
	return NativeRef != nullptr && NativeRef->Native != nullptr;
}

FTrackWheelRef* FTrackWheelBarrier::GetNative()
{
	if (!HasNative())
	{
		return nullptr;
	}
	return NativeRef.get();
}

const FTrackWheelRef* FTrackWheelBarrier::GetNative() const
{
	if (!HasNative())
	{
		return nullptr;
	}
	return NativeRef.get();
}
