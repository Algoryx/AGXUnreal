// Copyright 2025, Algoryx Simulation AB.

#include "AGXUnrealShaders.h"

// Unreal Engine includes.
#include "Interfaces/IPluginManager.h"
#include "ShaderCore.h"
#include "Misc/Paths.h"
#include "Logging/LogMacros.h"

#define LOCTEXT_NAMESPACE "FAGXUnrealShadersModule"

void FAGXUnrealShadersModule::StartupModule()
{
    // Register the directory containing AGX Unreal shader source files.
    FString PluginShaderDir = FPaths::Combine(
        IPluginManager::Get().FindPlugin(TEXT("AGXUnreal"))->GetBaseDir(), TEXT("Shaders"));

    AddShaderSourceDirectoryMapping(TEXT("/AGXUnrealShaders"), PluginShaderDir);
}

void FAGXUnrealShadersModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAGXUnrealShadersModule, AGXUnrealShaders)
