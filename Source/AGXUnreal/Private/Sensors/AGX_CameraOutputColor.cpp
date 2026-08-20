// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/AGX_CameraOutputColor.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/CameraOutputColorBarrier.h"

TUniquePtr<FCameraOutputBarrier> FAGX_CameraOutputColor::CreateNativeBarrier() const
{
	return MakeUnique<FCameraOutputColorBarrier>();
}

FAGX_CameraOutputColor& FAGX_CameraOutputColor::operator=(const FAGX_CameraOutputColor& Other)
{
	FAGX_CameraOutputBase::operator=(Other);
	return *this;
}

bool FAGX_CameraOutputColor::operator==(const FAGX_CameraOutputColor& Other) const
{
	return FAGX_CameraOutputBase::operator==(Other);
}
