// Copyright 2025, Algoryx Simulation AB.

#include "Tires/TireBarrier.h"

// AGX Dynamics for Unreal includes.
#include "BarrierOnly/AGXRefs.h"
#include "BarrierOnly/AGXTypeConversions.h"

FTireBarrier::FTireBarrier()
	: NativeRef {new FTireRef}
{
}

FTireBarrier::FTireBarrier(std::unique_ptr<FTireRef>&& Native)
	: NativeRef(std::move(Native))
{
}

FTireBarrier::FTireBarrier(FTireBarrier&& Other)
	: NativeRef(std::move(Other.NativeRef))
{
}

FTireBarrier::~FTireBarrier()
{
}

bool FTireBarrier::HasNative() const
{
	return NativeRef && NativeRef->Native;
}

FTireRef* FTireBarrier::GetNative()
{
	return NativeRef.get();
}

const FTireRef* FTireBarrier::GetNative() const
{
	return NativeRef.get();
}

void FTireBarrier::SetName(const FString& NewName)
{
	check(HasNative());
	agx::String NameAGX = Convert(NewName);
	NativeRef->Native->setName(NameAGX);
}

FString FTireBarrier::GetName() const
{
	check(HasNative());
	FString NameUnreal(Convert(NativeRef->Native->getName()));
	return NameUnreal;
}

void FTireBarrier::ReleaseNative()
{
	check(HasNative());
	NativeRef->Native = nullptr;
}

uintptr_t FTireBarrier::GetNativeAddress() const
{
	if (!HasNative())
		return 0;

	return reinterpret_cast<uintptr_t>(NativeRef->Native.get());
}

void FTireBarrier::SetNativeAddress(uintptr_t NativeAddress)
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

	NativeRef->Native = reinterpret_cast<agxModel::Tire* > (NativeAddress);
}

FGuid FTireBarrier::GetGuid() const
{
	check(HasNative());
	return Convert(NativeRef->Native->getUuid());
}
