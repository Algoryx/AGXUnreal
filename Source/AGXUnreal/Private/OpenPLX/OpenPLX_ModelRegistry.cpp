// Copyright 2025, Algoryx Simulation AB.

#include "OpenPLX/OpenPLX_ModelRegistry.h"

// Unreal Engine includes.
#include "Engine/World.h"


UOpenPLX_ModelRegistry* UOpenPLX_ModelRegistry::GetFrom(UWorld* World)
{
	if (World == nullptr || !World->IsGameWorld())
		return nullptr;

	return World->GetSubsystem<UOpenPLX_ModelRegistry>();
}

bool UOpenPLX_ModelRegistry::HasNative() const
{
	return Native.HasNative();
}

FOpenPLXModelRegistry* UOpenPLX_ModelRegistry::GetNative()
{
	if (!HasNative())
		return nullptr;

	return &Native;
}

const FOpenPLXModelRegistry* UOpenPLX_ModelRegistry::GetNative() const
{
	if (!HasNative())
		return nullptr;

	return &Native;
}

void UOpenPLX_ModelRegistry::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UOpenPLX_ModelRegistry::Deinitialize()
{
	Native.ReleaseNative();
	Super::Deinitialize();
}
