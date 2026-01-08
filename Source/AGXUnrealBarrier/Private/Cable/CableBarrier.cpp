// Copyright 2025, Algoryx Simulation AB.

#include "Cable/CableBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGXBarrierFactories.h"
#include "BarrierOnly/AGXTypeConversions.h"
#include "BarrierOnly/Cable/CableRef.h"

FCableBarrier::FCableBarrier()
	: NativeRef {new FCableRef}
{
}

FCableBarrier::FCableBarrier(std::shared_ptr<FCableRef> Native)
	: NativeRef(std::move(Native))
{
	check(NativeRef);
}

void FCableBarrier::AllocateNative(double Radius, double ResolutionPerUnitLength)
{
	check(!HasNative());
	agx::Real RadiusAGX = ConvertDistanceToAGX(Radius);
	agx::Real ResolutionPerUnitLengthAGX = ConvertDistanceInvToAGX(ResolutionPerUnitLength);
	NativeRef->Native = new agxCable::Cable(RadiusAGX, ResolutionPerUnitLengthAGX);
}

double FCableBarrier::GetRadius() const
{
	check(HasNative());
	const agx::Real RadiusAGX = NativeRef->Native->getRadius();
	const double Radius = ConvertDistanceToUnreal<double>(RadiusAGX);
	return Radius;
}

FGuid FCableBarrier::GetGuid() const
{
	check(HasNative());
	FGuid Guid = Convert(NativeRef->Native->getUuid());
	return Guid;
}

void FCableBarrier::SetName(const FString& NameUnreal)
{
	check(HasNative());
	agx::String NameAGX = Convert(NameUnreal);
	NativeRef->Native->setName(NameAGX);
}

FString FCableBarrier::GetName() const
{
	check(HasNative());
	FString NameUnreal(Convert(NativeRef->Native->getName()));
	return NameUnreal;
}

bool FCableBarrier::HasNative() const
{
	return NativeRef->Native != nullptr;
}

FCableRef* FCableBarrier::GetNative()
{
	check(HasNative());
	return NativeRef.get();
}

const FCableRef* FCableBarrier::GetNative() const
{
	check(HasNative());
	return NativeRef.get();
}

uintptr_t FCableBarrier::GetNativeAddress() const
{
	if (!HasNative())
	{
		return 0;
	}

	return reinterpret_cast<uintptr_t>(NativeRef->Native.get());
}

void FCableBarrier::SetNativeAddress(uintptr_t NativeAddress)
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

	NativeRef->Native = reinterpret_cast<agxCable::Cable*>(NativeAddress);
}

void FCableBarrier::ReleaseNative()
{
	NativeRef->Native = nullptr;
}
