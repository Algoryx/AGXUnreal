#include "Tires/TireBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGXRefs.h"
#include "TypeConversions.h"

FTireBarrier::FTireBarrier()
	: NativeRef {new FTireRef}
{
}

FTireBarrier::FTireBarrier(std::unique_ptr<FTireRef> Native)
	: NativeRef(std::move(Native))
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