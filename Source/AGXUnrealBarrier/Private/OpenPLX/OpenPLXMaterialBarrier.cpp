// Copyright 2026, Algoryx Simulation AB.

#include "OpenPLX/OpenPLXMaterialBarrier.h"

// AGX Dynamics for Unreal includes.
#include "BarrierOnly/AGXTypeConversions.h"
#include "BarrierOnly/OpenPLX/OpenPLXRefs.h"

FOpenPLXMaterialBarrier::FOpenPLXMaterialBarrier()
	: NativeRef {new FOpenPLXMaterialRef}
{
}

FOpenPLXMaterialBarrier::FOpenPLXMaterialBarrier(std::shared_ptr<FOpenPLXMaterialRef> Native)
	: NativeRef(std::move(Native))
{
	check(NativeRef);
}

bool FOpenPLXMaterialBarrier::HasNative() const
{
	return NativeRef != nullptr && NativeRef->Native != nullptr;
}

FOpenPLXMaterialRef* FOpenPLXMaterialBarrier::GetNative()
{
	check(NativeRef);
	return NativeRef.get();
}

const FOpenPLXMaterialRef* FOpenPLXMaterialBarrier::GetNative() const
{
	check(NativeRef);
	return NativeRef.get();
}

FString FOpenPLXMaterialBarrier::GetName() const
{
	check(HasNative());
	return Convert(NativeRef->Native->getName());
}
