// Copyright 2026, Algoryx Simulation AB.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * The AGXUnrealShaders module is responsible for registering custom shader directories
 * and exposing Custom Shaders to the rest of the plugin.
 */
class AGXUNREALSHADERS_API FAGXUnrealShadersModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
