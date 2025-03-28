// Copyright 2024, Algoryx Simulation AB.

#pragma once

#include "OpenPLX/PLX_ModelRegistry.h"

// Unreal Engine includes.
#include "Engine/World.h"


UPLX_ModelRegistry* UPLX_ModelRegistry::GetFrom(UWorld* World)
{
	if (World == nullptr || !World->IsGameWorld())
		return nullptr;

	return World->GetSubsystem<UPLX_ModelRegistry>();
}

bool UPLX_ModelRegistry::HasNative() const
{
	return Native.HasNative();
}

FPLXModelRegistry* UPLX_ModelRegistry::GetNative()
{
	if (!HasNative())
		return nullptr;

	return &Native;
}

const FPLXModelRegistry* UPLX_ModelRegistry::GetNative() const
{
	if (!HasNative())
		return nullptr;

	return &Native;
}

void UPLX_ModelRegistry::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UPLX_ModelRegistry::Deinitialize()
{
	Native.ReleaseNative();
	Super::Deinitialize();
}
