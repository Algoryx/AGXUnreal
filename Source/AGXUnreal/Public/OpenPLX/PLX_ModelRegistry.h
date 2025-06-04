// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "OpenPLX/PLXModelRegistry.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "LevelInstance/LevelInstanceSubsystem.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "PLX_ModelRegistry.generated.h"

/**
 * The PLX Model Registry is responsible for keeping track of runtime OpenPLX data generated from a
 * specific OpenPLX model, which can be shared across all instances of that model in a
 * Simulation.
 */
UCLASS(ClassGroup = "PLX", Category = "PLX")
class AGXUNREAL_API UPLX_ModelRegistry : public ULevelInstanceSubsystem
{
	GENERATED_BODY()

public:
	static UPLX_ModelRegistry* GetFrom(UWorld* World);

	bool HasNative() const;
	FPLXModelRegistry* GetNative();
	const FPLXModelRegistry* GetNative() const;

private:
	// ~Begin USubsystem interface.
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	// ~End USubsystem interface.

	FPLXModelRegistry Native;
};
