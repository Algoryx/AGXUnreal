// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "LevelInstance/LevelInstanceSubsystem.h"

#include "AGX_CameraBackend.generated.h"

class AActor;
class UActorComponent;
class UWorld;
struct FCameraBackendBarrier;

/**
 * Todo: add API comment.
 */
UCLASS(ClassGroup = "AGX_Sensor", Category = "AGX")
class AGXUNREAL_API UAGX_CameraBackend : public ULevelInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual ~UAGX_CameraBackend() override;

	FCameraBackendBarrier* GetOrCreateNative();

	static UAGX_CameraBackend* GetFrom(const UActorComponent* Component);
	static UAGX_CameraBackend* GetFrom(const AActor* Actor);
	static UAGX_CameraBackend* GetFrom(const UWorld* World);

private:
	// ~Begin USubsystem interface.
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	// ~End USubsystem interface.
};
