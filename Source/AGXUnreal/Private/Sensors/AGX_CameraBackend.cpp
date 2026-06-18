// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/AGX_CameraBackend.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/CameraBackendBarrier.h"

UAGX_CameraBackend::~UAGX_CameraBackend() = default;

FCameraBackendBarrier* UAGX_CameraBackend::GetOrCreateNative()
{
	if (NativeBarrier == nullptr)
	{
		NativeBarrier.Reset(new FCameraBackendBarrier());
	}

	if (!NativeBarrier->HasNative())
	{
		NativeBarrier->AllocateNative();
	}

	return NativeBarrier.Get();
}

void UAGX_CameraBackend::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UAGX_CameraBackend::Deinitialize()
{
	NativeBarrier.Reset();
	Super::Deinitialize();
}
