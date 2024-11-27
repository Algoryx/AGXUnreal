// Copyright 2024, Algoryx Simulation AB.


using UnrealBuildTool;

public class AGXUnrealEditor : ModuleRules
{
	public AGXUnrealEditor(ReadOnlyTargetRules Target) : base(Target)
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

		PrecompileForTargets = PrecompileTargetsType.Any;

		/// \todo Copied from the prototype plugin. Not sure if all of these are
		///       required.
		PublicDependencyModuleNames.AddRange(new string[]{
			"AGXCommon", "AGXUnrealBarrier", "AGXUnreal", "ComponentVisualizers", "Core", "CoreUObject", "Engine",
			"InputCore", "RawMesh", "RHI", "RenderCore"
		});

		/// \todo Copied from the prototype plugin. Not sure if all of these are
		///       required.
		PrivateDependencyModuleNames.AddRange(new string[] {
			"AGXDynamicsLibrary", "AssetTools", "CoreUObject", "DesktopPlatform", "EditorStyle", "Engine", "InputCore",
			"Json", "LevelEditor", "PlacementMode", "Projects", "PropertyEditor", "PropertyPath", "RenderCore", "RHI",
			"SceneOutliner", "Slate", "SlateCore", "SubobjectEditor", "UnrealEd", "MeshDescription"
		});

#if UE_5_0_OR_LATER
		PrivateDependencyModuleNames.Add("EditorFramework");
#endif
	}
}
