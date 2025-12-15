// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "OpenPLX/OpenPLXModelRegistry.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "LevelInstance/LevelInstanceSubsystem.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "OpenPLX_ModelRegistry.generated.h"

/**
 * The OpenPLX Model Registry is responsible for keeping track of runtime OpenPLX data generated
 * from a specific OpenPLX model, which can be shared across all instances of that model in a
 * Simulation.
 */
UCLASS(ClassGroup = "OpenPLX", Category = "OpenPLX")
class AGXUNREAL_API UOpenPLX_ModelRegistry : public ULevelInstanceSubsystem
{
	GENERATED_BODY()

public:
	static UOpenPLX_ModelRegistry* GetFrom(UWorld* World);

	bool HasNative() const;
	FOpenPLXModelRegistry* GetNative();
	const FOpenPLXModelRegistry* GetNative() const;

private:
	// ~Begin USubsystem interface.
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	// ~End USubsystem interface.

	FOpenPLXModelRegistry Native;
};
