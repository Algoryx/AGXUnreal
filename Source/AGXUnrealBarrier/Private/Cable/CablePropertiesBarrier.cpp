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

void FCablePropertiesBarrier::SetSpookDampingBend(double SpookDamping)
{
	check(HasNative());
	NativeRef->Native->setDamping(SpookDamping, agxCable::BEND);
}

double FCablePropertiesBarrier::GetSpookDampingBend() const
{
	check(HasNative());
	return NativeRef->Native->getDamping(agxCable::BEND);
}

void FCablePropertiesBarrier::SetSpookDampingTwist(double SpookDamping)
{
	check(HasNative());
	NativeRef->Native->setDamping(SpookDamping, agxCable::TWIST);
}

double FCablePropertiesBarrier::GetSpookDampingTwist() const
{
	check(HasNative());
	return NativeRef->Native->getDamping(agxCable::TWIST);
}

void FCablePropertiesBarrier::SetSpookDampingStretch(double SpookDamping)
{
	check(HasNative());
	NativeRef->Native->setDamping(SpookDamping, agxCable::STRETCH);
}

double FCablePropertiesBarrier::GetSpookDampingStretch() const
{
	check(HasNative());
	return NativeRef->Native->getDamping(agxCable::STRETCH);
}

void FCablePropertiesBarrier::SetPoissonsRatioBend(double PoissonsRatio)
{
	check(HasNative());
	NativeRef->Native->setPoissonsRatio(PoissonsRatio, agxCable::BEND);
}

double FCablePropertiesBarrier::GetPoissonsRatioBend() const
{
	check(HasNative());
	return NativeRef->Native->getPoissonsRatio(agxCable::BEND);
}

void FCablePropertiesBarrier::SetPoissonsRatioTwist(double PoissonsRatio)
{
	check(HasNative());
	NativeRef->Native->setPoissonsRatio(PoissonsRatio, agxCable::TWIST);
}

double FCablePropertiesBarrier::GetPoissonsRatioTwist() const
{
	check(HasNative());
	return NativeRef->Native->getPoissonsRatio(agxCable::TWIST);
}

void FCablePropertiesBarrier::SetPoissonsRatioStretch(double PoissonsRatio)
{
	check(HasNative());
	NativeRef->Native->setPoissonsRatio(PoissonsRatio, agxCable::STRETCH);
}

double FCablePropertiesBarrier::GetPoissonsRatioStretch() const
{
	check(HasNative());
	return NativeRef->Native->getPoissonsRatio(agxCable::STRETCH);
}

void FCablePropertiesBarrier::SetYoungsModulusBend(double YoungsModulus)
{
	check(HasNative());
	NativeRef->Native->setYoungsModulus(YoungsModulus, agxCable::BEND);
}

double FCablePropertiesBarrier::GetYoungsModulusBend() const
{
	check(HasNative());
	return NativeRef->Native->getYoungsModulus(agxCable::BEND);
}

void FCablePropertiesBarrier::SetYoungsModulusTwist(double YoungsModulus)
{
	check(HasNative());
	NativeRef->Native->setYoungsModulus(YoungsModulus, agxCable::TWIST);
}

double FCablePropertiesBarrier::GetYoungsModulusTwist() const
{
	check(HasNative());
	return NativeRef->Native->getYoungsModulus(agxCable::TWIST);
}

void FCablePropertiesBarrier::SetYoungsModulusStretch(double YoungsModulus)
{
	check(HasNative());
	NativeRef->Native->setYoungsModulus(YoungsModulus, agxCable::STRETCH);
}

double FCablePropertiesBarrier::GetYoungsModulusStretch() const
{
	check(HasNative());
	return NativeRef->Native->getYoungsModulus(agxCable::STRETCH);
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
