// Copyright 2025, Algoryx Simulation AB.

#include "ObserverFrameBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGXBarrierFactories.h"
#include "BarrierOnly/AGXRefs.h"
#include "BarrierOnly/AGXTypeConversions.h"
#include "RigidBodyBarrier.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agx/ObserverFrame.h>
#include "EndAGXIncludes.h"

FObserverFrameBarrier::FObserverFrameBarrier()
	: NativeRef {new FObserverFrameRef}
{
}

FObserverFrameBarrier::FObserverFrameBarrier(std::shared_ptr<FObserverFrameRef> Native)
	: NativeRef(std::move(Native))
{
	check(NativeRef);
}

void FObserverFrameBarrier::SetEnabled(bool Enabled)
{
	check(HasNative());
	NativeRef->Native->setEnable(Enabled);
}

bool FObserverFrameBarrier::GetEnabled() const
{
	check(HasNative());
	return NativeRef->Native->getEnable();
}

void FObserverFrameBarrier::SetPosition(const FVector& PositionUnreal)
{
	check(HasNative());
	agx::Vec3 PositionAGX = ConvertDisplacement(PositionUnreal);
	NativeRef->Native->setPosition(PositionAGX);
}

FVector FObserverFrameBarrier::GetPosition() const
{
	check(HasNative());
	agx::Vec3 PositionAGX = NativeRef->Native->getPosition();
	return ConvertDisplacement(PositionAGX);
}

void FObserverFrameBarrier::SetLocalPosition(const FVector& PositionUnreal)
{
	check(HasNative());
	agx::Vec3 PositionAGX = ConvertDisplacement(PositionUnreal);
	NativeRef->Native->setLocalPosition(PositionAGX);
}

FVector FObserverFrameBarrier::GetLocalPosition() const
{
	check(HasNative());
	agx::Vec3 PositionAGX = NativeRef->Native->getLocalPosition();
	return ConvertDisplacement(PositionAGX);
}

void FObserverFrameBarrier::SetRotation(const FQuat& RotationUnreal)
{
	check(HasNative());
	agx::Quat RotationAGX = Convert(RotationUnreal);
	NativeRef->Native->setRotation(RotationAGX);
}

FQuat FObserverFrameBarrier::GetRotation() const
{
	check(HasNative());
	agx::Quat RotationAGX = NativeRef->Native->getRotation();
	return Convert(RotationAGX);
}

void FObserverFrameBarrier::SetLocalRotation(const FQuat& RotationUnreal)
{
	check(HasNative());
	agx::Quat RotationAGX = Convert(RotationUnreal);
	NativeRef->Native->setLocalRotation(RotationAGX);
}

FQuat FObserverFrameBarrier::GetLocalRotation() const
{
	check(HasNative());
	agx::Quat RotationAGX = NativeRef->Native->getLocalRotation();
	return Convert(RotationAGX);
}

FVector FObserverFrameBarrier::GetVelocity() const
{
	check(HasNative());
	agx::Vec3 VelocityAGX = NativeRef->Native->getVelocity();
	return ConvertDisplacement(VelocityAGX);
}

FVector FObserverFrameBarrier::GetAngularVelocity() const
{
	check(HasNative());
	agx::Vec3 AngularVelocityAGX = NativeRef->Native->getAngularVelocity();
	return ConvertAngularVelocity(AngularVelocityAGX);
}

FVector FObserverFrameBarrier::GetAcceleration() const
{
	check(HasNative());
	agx::Vec3 AccelerationAGX = NativeRef->Native->getAcceleration();
	return ConvertDisplacement(AccelerationAGX);
}

FVector FObserverFrameBarrier::GetAngularAcceleration() const
{
	check(HasNative());
	agx::Vec3 AngularAccelerationAGX = NativeRef->Native->getAngularAcceleration();
	return ConvertAngularAcceleration(AngularAccelerationAGX);
}

FRigidBodyBarrier FObserverFrameBarrier::GetRigidBody() const
{
	check(HasNative());
	return AGXBarrierFactories::CreateRigidBodyBarrier(NativeRef->Native->getRigidBody());
}

void FObserverFrameBarrier::SetName(const FString& NameUnreal)
{
	check(HasNative());
	agx::String NameAGX = Convert(NameUnreal);
	NativeRef->Native->setName(NameAGX);
}

FString FObserverFrameBarrier::GetName() const
{
	check(HasNative());
	FString NameUnreal(Convert(NativeRef->Native->getName()));
	return NameUnreal;
}

FGuid FObserverFrameBarrier::GetGuid() const
{
	check(HasNative());
	FGuid Guid = Convert(NativeRef->Native->getUuid());
	return Guid;
}

bool FObserverFrameBarrier::HasNative() const
{
	return NativeRef->Native != nullptr;
}

void FObserverFrameBarrier::AllocateNative(FRigidBodyBarrier& Body)
{
	check(!HasNative());
	check(Body.HasNative());
	NativeRef->Native = new agx::ObserverFrame(Body.GetNative()->Native);
}

FObserverFrameRef* FObserverFrameBarrier::GetNative()
{
	check(HasNative());
	return NativeRef.get();
}

const FObserverFrameRef* FObserverFrameBarrier::GetNative() const
{
	check(HasNative());
	return NativeRef.get();
}

uintptr_t FObserverFrameBarrier::GetNativeAddress() const
{
	if (!HasNative())
	{
		return 0;
	}

	return reinterpret_cast<uintptr_t>(NativeRef->Native.get());
}

void FObserverFrameBarrier::SetNativeAddress(uintptr_t NativeAddress)
{
	if (NativeAddress == GetNativeAddress())
		return;

	if (HasNative())
		ReleaseNative();

	if (NativeAddress == 0)
	{
		NativeRef->Native = nullptr;
		return;
	}

	NativeRef->Native = reinterpret_cast<agx::ObserverFrame*>(NativeAddress);
}

void FObserverFrameBarrier::ReleaseNative()
{
	NativeRef->Native = nullptr;
}
