// Copyright 2025, Algoryx Simulation AB.

#include "Cable/CablePropertiesBarrier.h"

// AGX Dynamics for Unreal includes.
#include "BarrierOnly/AGXTypeConversions.h"
#include "BarrierOnly/Cable/CableRef.h"

FCablePropertiesBarrier::FCablePropertiesBarrier()
	: NativeRef {new FCablePropertiesRef}
{
}

FCablePropertiesBarrier::FCablePropertiesBarrier(std::shared_ptr<FCablePropertiesRef> Native)
	: NativeRef(std::move(Native))
{
}

bool FCablePropertiesBarrier::HasNative() const
{
	return NativeRef && NativeRef->Native;
}

FCablePropertiesRef* FCablePropertiesBarrier::GetNative()
{
	return NativeRef.get();
}

const FCablePropertiesRef* FCablePropertiesBarrier::GetNative() const
{
	return NativeRef.get();
}

void FCablePropertiesBarrier::AllocateNative()
{
	check(!HasNative());

	// CableProperties constructor is protected, so we use a dummy Cable to get one, and store it in
	// a Ref, so that even though the Cable goes out of scope, the Cable Properties object will
	// still be valid. Cable Properties is designed to handle being shared.
	agxCable::CableRef TempCable = new agxCable::Cable(0.1, 1.0);
	NativeRef->Native = TempCable->getCableProperties();
}

void FCablePropertiesBarrier::ReleaseNative()
{
	check(HasNative());
	NativeRef->Native = nullptr;
}

FGuid FCablePropertiesBarrier::GetGuid() const
{
	check(HasNative());
	FGuid Guid = Convert(NativeRef->Native->getUuid());
	return Guid;
}
