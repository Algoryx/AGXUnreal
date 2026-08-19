// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/AGX_CameraBackend.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/CameraBackendBarrier.h"

// Unreal Engine includes.
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

UAGX_CameraBackend::~UAGX_CameraBackend() = default;

FCameraBackendBarrier* UAGX_CameraBackend::GetOrCreateNative()
{
	return &FCameraBackendBarrier::GetInstance();
}

UAGX_CameraBackend* UAGX_CameraBackend::GetFrom(const UActorComponent* Component)
{
	if (!Component)
		return nullptr;

	return GetFrom(Component->GetOwner());
}

UAGX_CameraBackend* UAGX_CameraBackend::GetFrom(const AActor* Actor)
{
	if (!Actor)
		return nullptr;

	return GetFrom(Actor->GetWorld());
}

UAGX_CameraBackend* UAGX_CameraBackend::GetFrom(const UWorld* World)
{
	if (World == nullptr || !World->IsGameWorld())
		return nullptr;

	return const_cast<UWorld*>(World)->GetSubsystem<UAGX_CameraBackend>();
}

void UAGX_CameraBackend::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UAGX_CameraBackend::Deinitialize()
{
	FCameraBackendBarrier::GetInstance().ClearCameras();
	Super::Deinitialize();
}
