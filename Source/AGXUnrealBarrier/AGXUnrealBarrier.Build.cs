// Copyright 2025, Algoryx Simulation AB.


using System;
using UnrealBuildTool;

public class AGXUnrealBarrier : ModuleRules
{
	/// Set to true to enable runtime checks that we only want in our testing
	/// and continuous integration pipelines. Any public release should have
	/// this set to false. Can be overridden by setting the AGXUNREAL_CHECK
	/// environment variable to 'true' or 'false'.
	bool bEnableAGXCheck = false;

	public AGXUnrealBarrier(ReadOnlyTargetRules Target) : base(Target)
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

		// These settings are needed to make this module compiler-compatible
		// with AGX Dynamics. As a side effect, this also makes it incompatible
		// with many Unreal Engine modules. Therefore, source files in this
		// module should minimize the number of Unreal Engine headers they
		// include. In particular, inheritance from any UObject class is
		// prohibited.
		// bUseRTTI because the AGX Dynamics type system uses typeid and dynamic_cast.
		// bEnableExceptions because AGX Dynamics uses exceptions.
		bUseRTTI = true;
		bEnableExceptions = true;

		PrecompileForTargets = PrecompileTargetsType.Any;

		string EnableAGXCheckEnv = Environment.GetEnvironmentVariable("AGXUNREAL_CHECK");
		if (!String.IsNullOrEmpty(EnableAGXCheckEnv))
		{
			bEnableAGXCheck = EnableAGXCheckEnv.Equals("true", StringComparison.OrdinalIgnoreCase);
		}
		if (bEnableAGXCheck)
		{
			PublicDefinitions.Add("AGXUNREAL_CHECK=1");
		}


		// TODO: Determine which of these are really need and why.
		PublicDependencyModuleNames.AddRange(new string[] {
			"AGXCommon", "RHI", "RenderCore", "Core", "CoreUObject", "Engine", "InputCore"});

		// TODO: Determine which of these are really needed and why.
		// TODO: Why are some modules listed both here and in Public... above?
		PrivateDependencyModuleNames.AddRange(new string[] {
			"RHI", "RenderCore", "Projects", "Json",
			"AGXDynamicsLibrary", "Landscape"});
	}
}
