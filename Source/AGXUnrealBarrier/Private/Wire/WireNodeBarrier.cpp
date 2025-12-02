// Copyright 2025, Algoryx Simulation AB.

#include "Wire/WireNodeBarrier.h"

// AGX Dynamics for Unreal include.
#include "AGXBarrierFactories.h"
#include "BarrierOnly/AGXRefs.h"
#include "RigidBodyBarrier.h"
#include "TypeConversions.h"
#include "BarrierOnly/Wire/WireNodeRef.h"

FWireNodeBarrier::FWireNodeBarrier()
	: NativeRef {new FWireNodeRef()}
{
}

FWireNodeBarrier::FWireNodeBarrier(const FWireNodeBarrier& Other)
	: NativeRef(new FWireNodeRef())
{
	NativeRef->Native = Other.NativeRef->Native;
}

FWireNodeBarrier::FWireNodeBarrier(FWireNodeBarrier&& Other)
	: NativeRef {std::move(Other.NativeRef)}
{
	Other.NativeRef.reset();
}

FWireNodeBarrier::FWireNodeBarrier(std::unique_ptr<FWireNodeRef>&& Native)
	: NativeRef {std::move(Native)}
{
	Native.reset();
}

FWireNodeBarrier::~FWireNodeBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the
	// std::unique_ptr NativeRef's destructor must be able to see the definition,
	// not just the forward declaration, of FWireNodeRef.
}

FWireNodeBarrier& FWireNodeBarrier::operator=(const FWireNodeBarrier& InOther)
{
	NativeRef->Native = InOther.NativeRef->Native;
	return *this;
}

bool FWireNodeBarrier::HasNative() const
{
	return NativeRef->Native != nullptr;
}

void FWireNodeBarrier::AllocateNativeFreeNode(const FVector& WorldLocation)
{
	check(!HasNative());
	const agx::Vec3 WorldLocationAGX = ConvertDisplacement(WorldLocation);
	NativeRef->Native = new agxWire::FreeNode(WorldLocationAGX);
}

void FWireNodeBarrier::AllocateNativeEyeNode(
	FRigidBodyBarrier& RigidBody, const FVector& LocalLocation)
{
	check(!HasNative());
	check(RigidBody.HasNative());
	const agx::Vec3 LocalLocationAGX = ConvertDisplacement(LocalLocation);
	agx::RigidBody* Body = RigidBody.GetNative()->Native;
	NativeRef->Native = new agxWire::EyeNode(Body, LocalLocationAGX);
}

void FWireNodeBarrier::AllocateNativeBodyFixedNode(
	FRigidBodyBarrier& RigidBody, const FVector& LocalLocation)
{
	check(!HasNative());
	check(RigidBody.HasNative());
	const agx::Vec3 LocalLocationAGX = ConvertDisplacement(LocalLocation);
	agx::RigidBody* Body = RigidBody.GetNative()->Native;
	NativeRef->Native = new agxWire::BodyFixedNode(Body, LocalLocationAGX);
}

FWireNodeRef* FWireNodeBarrier::GetNative()
{
	check(HasNative());
	return NativeRef.get();
}

const FWireNodeRef* FWireNodeBarrier::GetNative() const
{
	check(HasNative());
	return NativeRef.get();
}

void FWireNodeBarrier::ReleaseNative()
{
	check(HasNative());
	NativeRef->Native = nullptr;
}

FVector FWireNodeBarrier::GetWorldLocation() const
{
	check(HasNative());
	return ConvertDisplacement(NativeRef->Native->getWorldPosition());
}

FVector FWireNodeBarrier::GetTranslate() const
{
	check(HasNative());
	return ConvertDisplacement(NativeRef->Native->getTranslate());
}

EWireNodeType FWireNodeBarrier::GetType() const
{
	check(HasNative());
	agxWire::Node::Type TypeAGX = NativeRef->Native->getType();
	return Convert(TypeAGX);
}

FRigidBodyBarrier FWireNodeBarrier::GetRigidBody() const
{
	check(HasNative());
	agx::RigidBody* Body = NativeRef->Native->getRigidBody();
	return AGXBarrierFactories::CreateRigidBodyBarrier(Body);
}
