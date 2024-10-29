// Copyright 2024, Algoryx Simulation AB.

#include "PowerLine/PowerLineBarrier.h"

// AGX Dynamics for Unreal includes.
#include "BarrierOnly/AGXRefs.h"
#include "BarrierOnly/PowerLine/PowerLineRefs.h"
#include "BarrierOnly/PowerLine/PowerLineRefs.h"
#include "PowerLine/PowerLineUnitBarrier.h"
#include "SimulationBarrier.h"
#include "Utilities/BarrierBase.inl.h"

FPowerLineBarrier::FPowerLineBarrier()
	: NativeRef {new FPowerLineRef()}
{
}

FPowerLineBarrier::FPowerLineBarrier(std::unique_ptr<FPowerLineRef> Native)
	: NativeRef  {std::move(Native)}
{
}

FPowerLineBarrier::FPowerLineBarrier(const FPowerLineBarrier& Other)
	: NativeRef {new FPowerLineRef(Other.NativeRef->Native)}
{
}

FPowerLineBarrier::FPowerLineBarrier(FPowerLineBarrier&& Other)
	: NativeRef {std::move(Other.NativeRef)}
{
	Other.NativeRef.reset(new FPowerLineRef());
}

FPowerLineBarrier& FPowerLineBarrier::operator=(const FPowerLineBarrier& Other)
{
	NativeRef->Native = Other.NativeRef->Native;
	return *this;
}

FPowerLineBarrier::~FPowerLineBarrier()
{
	NativeRef->Native = nullptr;
}

void FPowerLineBarrier::AllocateNative()
{
	check(!HasNative());
	NativeRef->Native = new agxPowerLine::PowerLine();
}

bool FPowerLineBarrier::HasNative() const
{
	return NativeRef->Native != nullptr;
}

FPowerLineRef* FPowerLineBarrier::GetNative()
{
	check(HasNative());
	return NativeRef.get();
}

const FPowerLineRef* FPowerLineBarrier::GetNative() const
{
	check(HasNative());
	return NativeRef.get();
}

uintptr_t FPowerLineBarrier::GetNativeAddress() const
{
	if (!HasNative())
	{
		return 0;
	}
	return reinterpret_cast<uintptr_t>(NativeRef->Native.get());
}

void FPowerLineBarrier::SetNativeAddress(uintptr_t NativeAddress)
{
	if (NativeAddress == GetNativeAddress())
	{
		return;
	}

	if  (HasNative())
	{
		ReleaseNative();
	}

	if (NativeAddress == 0)
	{
		NativeRef->Native = nullptr;
	}
	else
	{
		NativeRef->Native = reinterpret_cast<agxPowerLine::PowerLine*>(NativeAddress);
	}
}

void FPowerLineBarrier::ReleaseNative()
{
	NativeRef->Native = nullptr;
}

void FPowerLineBarrier::Add(FPowerLineUnitBarrier& Unit)
{
	check(HasNative());
	check(Unit.HasNative());
	NativeRef->Native->add(Unit.GetNative()->Native);
}

bool FPowerLineBarrier::AddTo(FSimulationBarrier& Simulation)
{
	check(HasNative());
	check(Simulation.HasNative());
	return Simulation.GetNative()->Native->add(NativeRef->Native);
}
