// Copyright 2026, Algoryx Simulation AB.

#include "OpenPLX/OpenPLXCameraColorOutputView.h"

// AGX Dynamics for Unreal includes.
#include "BarrierOnly/OpenPLX/OpenPLXRefs.h"

// Standard library includes.
#include <utility>

FOpenPLXCameraColorOutputView::FOpenPLXCameraColorOutputView()
	: NativeRef {new FOpenPLXCameraColorOutputViewRef}
{
}

FOpenPLXCameraColorOutputView::FOpenPLXCameraColorOutputView(
	std::shared_ptr<FOpenPLXCameraColorOutputViewRef> Native)
	: NativeRef(std::move(Native))
{
	check(NativeRef);
}

bool FOpenPLXCameraColorOutputView::HasNative() const
{
	return NativeRef != nullptr && NativeRef->Marshalling != nullptr;
}

bool FOpenPLXCameraColorOutputView::MakePersistant()
{
	if (!HasNative())
		return false;

	std::shared_ptr<openplx::Marshalling> Detached = NativeRef->Marshalling->detach_copy();
	if (Detached == nullptr)
		return false;

	NativeRef->Marshalling = std::move(Detached);
	return true;
}

FOpenPLXCameraColorOutputViewRef* FOpenPLXCameraColorOutputView::GetNative()
{
	check(NativeRef);
	return NativeRef.get();
}

const FOpenPLXCameraColorOutputViewRef* FOpenPLXCameraColorOutputView::GetNative() const
{
	check(NativeRef);
	return NativeRef.get();
}
