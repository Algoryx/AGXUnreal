// Copyright 2026, Algoryx Simulation AB.

#include "Cable/CableNodeBarrier.h"

// AGX Dynamics for Unreal include.
#include "AGXBarrierFactories.h"
#include "BarrierOnly/AGXRefs.h"
#include "BarrierOnly/AGXTypeConversions.h"
#include "BarrierOnly/Cable/CableRef.h"
#include "RigidBodyBarrier.h"

FCableNodeBarrier::FCableNodeBarrier()
	: NativeRef {new FCableNodeRef()}
{
}

FCableNodeBarrier::FCableNodeBarrier(std::shared_ptr<FCableNodeRef> Native)
	: NativeRef(std::move(Native))
{
	check(NativeRef);
}

bool FCableNodeBarrier::HasNative() const
{
	return NativeRef->Native != nullptr;
}

void FCableNodeBarrier::AllocateNativeFreeNode(const FVector& WorldLocation)
{
	check(!HasNative());
	const agx::Vec3 WorldLocationAGX = ConvertDisplacement(WorldLocation);
	NativeRef->Native = new agxCable::FreeNode(WorldLocationAGX);
}

void FCableNodeBarrier::AllocateNativeBodyFixedNode(
	FRigidBodyBarrier& RigidBody, const FTransform& LocalTransform, bool LockRotationToBody)
{
	check(!HasNative());
	check(RigidBody.HasNative());
	const agx::AffineMatrix4x4 LocalTransformAGX = Convert(LocalTransform);
	agx::RigidBody* Body = RigidBody.GetNative()->Native;

	// For BodyFixedNodes in AGX, a lockjoint or a balljoint is created depending on which
	// constructor is called, see below.
	if (LockRotationToBody)
		NativeRef->Native = new agxCable::BodyFixedNode(Body, LocalTransformAGX);
	else
		NativeRef->Native = new agxCable::BodyFixedNode(Body, LocalTransformAGX.getTranslate());
}

FCableNodeRef* FCableNodeBarrier::GetNative()
{
	check(HasNative());
	return NativeRef.get();
}

const FCableNodeRef* FCableNodeBarrier::GetNative() const
{
	check(HasNative());
	return NativeRef.get();
}

void FCableNodeBarrier::ReleaseNative()
{
	check(HasNative());
	NativeRef->Native = nullptr;
}

EAGX_CableNodeType FCableNodeBarrier::GetType() const
{
	check(HasNative());
	return NativeRef->Native->is<agxCable::FreeNode>() ? EAGX_CableNodeType::Free
													   : EAGX_CableNodeType::BodyFixed;
}

FRigidBodyBarrier FCableNodeBarrier::GetRigidBody() const
{
	check(HasNative());
	agx::RigidBody* Body = NativeRef->Native->getRigidBody();
	return AGXBarrierFactories::CreateRigidBodyBarrier(Body);
}
