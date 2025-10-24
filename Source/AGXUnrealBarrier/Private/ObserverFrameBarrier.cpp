// Copyright 2025, Algoryx Simulation AB.

#include "ObserverFrameBarrier.h"

// AGX Dynamics for Unreal includes.
#include "BarrierOnly/AGXRefs.h"
#include "TypeConversions.h"

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

void FObserverFrameBarrier::AllocateNative()
{
	check(!HasNative());
	NativeRef->Native = new agx::ObserverFrame();
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
