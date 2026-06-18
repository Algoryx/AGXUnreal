// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/CameraBackendBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "LevelInstance/LevelInstanceSubsystem.h"

#include "AGX_CameraBackend.generated.h"

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

private:
	// ~Begin USubsystem interface.
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	// ~End USubsystem interface.

	TUniquePtr<FCameraBackendBarrier> NativeBarrier;
};
