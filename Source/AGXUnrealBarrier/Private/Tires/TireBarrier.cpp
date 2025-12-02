// Copyright 2025, Algoryx Simulation AB.

#include "Tires/TireBarrier.h"

// AGX Dynamics for Unreal includes.
#include "BarrierOnly/AGXRefs.h"
#include "TypeConversions.h"

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

FGuid FTireBarrier::GetGuid() const
{
	check(HasNative());
	return Convert(NativeRef->Native->getUuid());
}
