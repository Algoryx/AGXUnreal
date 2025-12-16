// Copyright 2025, Algoryx Simulation AB.

using UnrealBuildTool;

public class AGXUnrealShaders : ModuleRules
{
    public AGXUnrealShaders(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bLegacyPublicIncludePaths = false;

        PrecompileForTargets = PrecompileTargetsType.Any;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "RenderCore",
            "RHI",
            "Projects"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Niagara"
        });
    }
}
