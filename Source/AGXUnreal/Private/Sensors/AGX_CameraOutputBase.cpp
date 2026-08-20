// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/AGX_CameraOutputBase.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_CameraSensorComponent.h"
#include "Sensors/CameraOutputBarrier.h"

FAGX_CameraOutputBase::FAGX_CameraOutputBase(const FAGX_CameraOutputBase&)
{
	// This is needed to be able to declare e.g. TArray's containing this struct.
	// It is assumed not to be called during play, therefore no members are copied here.
}

FAGX_CameraOutputBase::~FAGX_CameraOutputBase() = default;

bool FAGX_CameraOutputBase::HasNative() const
{
	return NativeBarrier != nullptr && NativeBarrier->HasNative();
}

FCameraOutputBarrier* FAGX_CameraOutputBase::GetOrCreateNative()
{
	if (NativeBarrier == nullptr)
		NativeBarrier = CreateNativeBarrier();

	if (NativeBarrier == nullptr)
		return nullptr;

	if (!NativeBarrier->HasNative())
		NativeBarrier->AllocateNative();

	return GetNative();
}

const FCameraOutputBarrier* FAGX_CameraOutputBase::GetNative() const
{
	if (!HasNative())
		return nullptr;

	return NativeBarrier.Get();
}

FCameraOutputBarrier* FAGX_CameraOutputBase::GetNative()
{
	if (!HasNative())
		return nullptr;

	return NativeBarrier.Get();
}

bool FAGX_CameraOutputBase::AddTo(UAGX_CameraSensorComponent* Camera)
{
	if (Camera == nullptr)
		return false;

	return Camera->AddOutput(*this);
}

FAGX_CameraOutputBase& FAGX_CameraOutputBase::operator=(const FAGX_CameraOutputBase& Other)
{
	(void) Other;

	// This operator is needed to be able to declare e.g. TArray's containing this struct.
	// It is assumed not to be called during play, therefore no members are copied here.
	NativeBarrier.Reset();
	return *this;
}

bool FAGX_CameraOutputBase::operator==(const FAGX_CameraOutputBase& Other) const
{
	return HasNative() && Other.HasNative() && GetNative() == Other.GetNative();
}
