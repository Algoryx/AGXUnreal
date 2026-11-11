// Copyright 2026, Algoryx Simulation AB.


using System.IO;
using UnrealBuildTool;

public class AGXUnrealTests : ModuleRules
{
	public AGXUnrealTests(ReadOnlyTargetRules Target) : base(Target)
	{
		// At 4.25 we started getting warnings encouraging us to enable these
		// settings. At or around 4.26 Unreal Engine makes these settings the
		// default.
		// bLegacyPublicIncludePaths adds all subdirectories to the list of
		// include paths passed to the compiler. This makes it too big for many
		// IDEs and compilers. Setting it to false reduces the list but makes
		// it necessary to specify subdirectories in #include statements.
		// PCHUsage has to do with Pre-Compiled Headers and include-what-you-use.
		// See
		// https://docs.unrealengine.com/4.26/en-US/ProductionPipelines/BuildTools/UnrealBuildTool/IWYU/
		bLegacyPublicIncludePaths = false;
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Exercise the private RVT sampler without exposing its implementation as public API.
		PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "../AGXUnreal/Private"));

		PrivateDependencyModuleNames.AddRange(new string[] {
			"Core", "Engine", "UnrealEd", "CoreUObject", "AGXCommon", "AGXUnreal", "AGXUnrealBarrier", "AGXUnrealEditor", "Landscape", "Foliage", "RenderCore", "Renderer", "RHI"
		});
	}
}
