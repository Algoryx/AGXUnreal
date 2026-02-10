// Copyright 2026, Algoryx Simulation AB.

#include "Vehicle/TrackWheelBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGX_AgxDynamicsObjectsAccess.h"
#include "BarrierOnly/AGXRefs.h"
#include "BarrierOnly/AGXTypeConversions.h"
#include "BarrierOnly/Vehicle/TrackWheelRef.h"
#include "RigidBodyBarrier.h"

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

void FTrackWheelBarrier::AllocateNative(const FTrackWheelCreationData& Data)
{
	check(Data.RigidBody.HasNative()); // \todo More gentle check and return false?

	agxVehicle::TrackWheel::Model ModelAGX = static_cast<agxVehicle::TrackWheel::Model>(Data.Model);
	agx::Real RadiusAGX = ConvertDistanceToAGX<agx::Real>(Data.Radius);
	agx::RigidBody* RigidBodyAGX = FAGX_AgxDynamicsObjectsAccess::GetFrom(&Data.RigidBody);
	agx::AffineMatrix4x4 RelTransformAGX =
		ConvertMatrix(Data.RelativePosition, Data.RelativeRotation);

	NativeRef->Native =
		new agxVehicle::TrackWheel(ModelAGX, RadiusAGX, RigidBodyAGX, RelTransformAGX);

	// Set properties.
	// \remark MERGE_NODES seems to be automatically set by AGX Dynamics depending on Model
	//         (set to true for Sprocket and Idler). Not sure if there is any purpose for letting
	//         the user override it, so ignoring it for now..
	// WheelAGX->setEnableProperty(agxVehicle::TrackWheel::Property::MERGE_NODES, ...);
	NativeRef->Native->setEnableProperty(
		agxVehicle::TrackWheel::Property::SPLIT_SEGMENTS, Data.bSplitSegments);
	NativeRef->Native->setEnableProperty(
		agxVehicle::TrackWheel::Property::MOVE_NODES_TO_ROTATION_PLANE,
		Data.bMoveNodesToRotationPlane);
	NativeRef->Native->setEnableProperty(
		agxVehicle::TrackWheel::Property::MOVE_NODES_TO_WHEEL, Data.bMoveNodesToWheel);
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
