using System;
using System.IO;
using System.Collections.Generic;
using UnrealBuildTool;


/// The AGXDynamicsLibrary is the portal from AGXUnrealBarrier to AGX Dynamics.
/// This is where we list the include paths and linker requirements to build an
/// Unreal Engine plugin that uses AGX Dynamics.
public class AGXDynamicsLibrary : ModuleRules
{

	/// Information about how AGX Dynamics is bundled and used on the current
	/// platform.
	private AGXResourcesInfo BundledAGXResources;

	// CAUTION: Setting bCopyLicenseFileToTarget to 'true' means the AGX Dynamics license file will
	// be copied to the build target location. This is true even for cooked builds. An exception is
	// when doing shipping builds, for those cases the license file is never copied to the target.
	// Use with care, make sure the license file is never distributed.
	bool bCopyLicenseFileToTarget = false;

	/// The various dependency sources we have. Each come with an include path,
	/// a linker path and a runtime path. The include path contains the header
	/// files (.h) needed to compile source files using the dependency. The
	/// linker path contains libraries (.lib/.so) needed link applications and
	/// libraries using the dependency. The runtime path contains other files
	/// (.dll/.so/.agxEntity) needed to run applications using the dependency.
	///
	/// LibSourceInfo instances are responsible for keeping the LibSource ->
	/// Paths associations.
	///
	/// Some LibSourceInfo instances provide only a subset of the paths. In
	/// particular, non-library dependencies only provide the runtime path.
	private enum LibSource
	{
		/// The default AGX Dynamics locations, provides the main AGX Dynamics
		/// libraries.
		AGX,

		/// The configuration part of AGX Dynamics. Contains generated header
		/// files for things such as version and CMake settings. Does not
		/// contain any libraries or runtime files.
		Config,

		/// A non-library dependency which points to the Components directory,
		/// the one that contains the entities, kernels, shaderes, and such.
		Components,

		/// The AGX Dynamics dependencies. Provides libraries that AGX Dynamics
		/// depend on.
		Dependencies,

		/// The AGX Terrain depdendencies. Provides libraries that AGX Terrain
		/// depend on.
		TerrainDependencies,

		/// A non-library dependency which points to the 'cfg' directory
		/// within AGX Dynamics.
		Cfg,

		/// A non-library dependency which points to the terrain material
		/// library within AGX Dynamics.
		TerrainMaterialLibrary
	};

	/// A carrier for the paths associated with a LibSource.
	///
	/// Instance of LibSourceInfo are help by AGXResourcesInfo in a
	/// LibSource -> LibSourceInfo dictionary.
	private class LibSourceInfo
	{
		/// Path to a directory containing header files used during compilation.
		public string IncludePath;

		/// Path to a directory containing build-time library files.
		public string LinkLibrariesPath;

		/// Path to a directory containing run-time library files.
		public string RuntimeLibrariesPath;

		public LibSourceInfo(string InIncludePath, string InLinkLibrariesPath, string InRuntimeLibrariesPath)
		{
			IncludePath = InIncludePath;
			LinkLibrariesPath = InLinkLibrariesPath;
			RuntimeLibrariesPath = InRuntimeLibrariesPath;
		}
	}

	/// Whether an AGXResourcesInfo references into a separate AGX Dynamics
	/// installation or an AGX Dynamics bundled into the plugin.
	/// AGXResourcesInfo use this to determine where relative to the base path
	/// and/or the setup_env environment variables that various parts of the
	/// AGX Dynamics installation are stored.
	private enum AGXResourcesLocation
	{
		/// A local build of AGX Dynamics. Must be setup_env'd and should
		/// contain separate source- and build directories.
		LocalBuildAGX,

		/// An AGX Dynamics installation made either by the AGX Dynamics install
		/// build target or by an AGX Dynamics installer.
		InstalledAGX,

		/// An AGX Dynamics installation that has been bundled into the plugin.
		BundledAGX
	}

	/// All needed AGX Dynamics runtime and build-time resources are located in
	/// the directory AGXUnreal/Binaries/Thirdparty/agx within this plugin. When
	/// compiling/packaging the AGX Dynamics for Unreal plugin, the AGX Dynamics
	/// resources found in AGXUnreal/Binaries/Thirdparty/agx are used. This
	/// directory also contain all needed AGX Dynamics runtime files. That means
	/// this plugin can be built and used without the need to call AGX Dynamic's
	/// setup_env as long as these resources are available.
	///
	/// If the AGX Dynamics resources are not available in the directory
	/// AGXUnreal/Binaries/Thirdparty/agx, then they are automatically copied
	/// from the AGX Dynamics installation in which setup_env has been called as
	/// part of the build process. Note that this means that if the AGX Dynamics
	/// resources are not available in AGXUnreal/Binaries/Thirdparty/agx at
	/// build-time, setup_env must have been called prior to performing the
	/// build. The recommended procedure is to build once within a setup_env'd
	/// environment leave the setup_env'd environment after that.
	public AGXDynamicsLibrary(ReadOnlyTargetRules Target) : base(Target)
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

		// Because the AGX Dynamics type system uses typeid and dynamic_cast.
		bUseRTTI = true;

		// Because AGX Dynamics uses exceptions.
		bEnableExceptions = true;

		// This marks this module as an external library, which means that the
		// library binary already exists. There are no source files in this
		// module.
		Type = ModuleType.External;

		string BundledAGXResourcesPath = GetBundledAGXResourcesPath();
		BundledAGXResources =
			new AGXResourcesInfo(Target, AGXResourcesLocation.BundledAGX, BundledAGXResourcesPath);

		// The AGX Dynamics version we are currently building against.
		AGXVersion TargetAGXVersion = GetAGXVersion();

		// List of run-time libraries that we need. These will be added to the
		// Unreal Engine RuntimeDependencies list. See
		// https://docs.unrealengine.com/en-US/ProductionPipelines/BuildTools/UnrealBuildTool/ThirdPartyLibraries/index.html
		Dictionary<string, LibSource> RuntimeLibFiles = new Dictionary<string, LibSource>();
		RuntimeLibFiles.Add("agxPhysics", LibSource.AGX);
		RuntimeLibFiles.Add("agxCore", LibSource.AGX);
		RuntimeLibFiles.Add("agxSabre", LibSource.AGX);
		RuntimeLibFiles.Add("agxTerrain", LibSource.AGX);
		RuntimeLibFiles.Add("agxCable", LibSource.AGX);
		RuntimeLibFiles.Add("agxModel", LibSource.AGX);
		RuntimeLibFiles.Add("vdbgrid", LibSource.AGX);
		RuntimeLibFiles.Add("colamd", LibSource.AGX);
		RuntimeLibFiles.Add("Half", LibSource.TerrainDependencies);
		RuntimeLibFiles.Add("Iex-2_2", LibSource.TerrainDependencies);
		RuntimeLibFiles.Add("Imath-2_2", LibSource.TerrainDependencies);
		RuntimeLibFiles.Add("IlmImf-2_2", LibSource.TerrainDependencies);
		RuntimeLibFiles.Add("IlmThread-2_2", LibSource.TerrainDependencies);
		RuntimeLibFiles.Add("openvdb", LibSource.TerrainDependencies);
		RuntimeLibFiles.Add("tbb", LibSource.TerrainDependencies);

		// List of link-time libraries that we need. These will be added to the
		// Unreal Engine PublicAdditionalLibraries list. See
		// https://docs.unrealengine.com/en-US/ProductionPipelines/BuildTools/UnrealBuildTool/ModuleFiles/index.html
		Dictionary<string, LibSource> LinkLibFiles = new Dictionary<string, LibSource>();
		LinkLibFiles.Add("agxPhysics", LibSource.AGX);
		LinkLibFiles.Add("agxCore", LibSource.AGX);
		LinkLibFiles.Add("agxSabre", LibSource.AGX);
		LinkLibFiles.Add("agxTerrain", LibSource.AGX);
		LinkLibFiles.Add("agxCable", LibSource.AGX);
		LinkLibFiles.Add("agxModel", LibSource.AGX);

		// List of the include directories that we need. These will be added to
		// the Unreal Engine PublicIncludePaths.
		List<LibSource> IncludePaths = new List<LibSource>();
		IncludePaths.Add(LibSource.AGX);

		// OS specific dependencies.
		if (Target.Platform == UnrealTargetPlatform.Linux)
		{
			IncludePaths.Add(LibSource.Components);
			IncludePaths.Add(LibSource.Config);
			IncludePaths.Add(LibSource.Dependencies);
			IncludePaths.Add(LibSource.TerrainDependencies);
		}
		else if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			RuntimeLibFiles.Add("msvcp140", LibSource.AGX);
			RuntimeLibFiles.Add("vcruntime140", LibSource.AGX);
			if (TargetAGXVersion.IsNewerOrEqualTo(2, 30, 0, 0) && TargetAGXVersion.IsOlderThan(2, 31, 0, 0))
			{
				RuntimeLibFiles.Add("agx-assimp-vc*-mt", LibSource.AGX);
			}

			RuntimeLibFiles.Add("zlib", LibSource.Dependencies);
			RuntimeLibFiles.Add("websockets", LibSource.Dependencies);
			RuntimeLibFiles.Add("libpng", LibSource.Dependencies);
			RuntimeLibFiles.Add("ot2?-OpenThreads", LibSource.Dependencies);
			if (TargetAGXVersion.IsOlderThan(2, 31, 0, 0))
			{
				RuntimeLibFiles.Add("glew", LibSource.Dependencies);
			}
		}

		// Bundle AGX Dynamics resources in plugin if no bundled resources exists.
		if (!IsAGXResourcesBundled())
		{
			BundleAGXResources(Target, RuntimeLibFiles, LinkLibFiles, IncludePaths);
		}
		else
		{
			Console.WriteLine("Skipping packaging of AGX Dynamics resources, bundled "
				+ "resources already exists in: {0}", BundledAGXResourcesPath);
		}

		// Create a license directory at the right place if non exists.
		EnsureLicenseDirCreated();

		string MisplacedLicensePath;
		if (MisplacedLicenseFileExists(out MisplacedLicensePath))
		{
			Console.Error.WriteLine("Error: Found misplaced AGX Dynamics license file at: {0} Please "
				+ "remove the license file and start the build process again.", MisplacedLicensePath);
			return;
		}

		foreach (var LinkLibFile in LinkLibFiles)
		{
			AddLinkLibrary(LinkLibFile.Key, LinkLibFile.Value);
		}

		foreach (var HeaderPath in IncludePaths)
		{
			AddIncludePath(HeaderPath);
		}

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			Dictionary<string, LibSource> DelayLoadLibraries = new Dictionary<string, LibSource>();
			DelayLoadLibraries.Add("agxPhysics", LibSource.AGX);
			DelayLoadLibraries.Add("agxCore", LibSource.AGX);
			DelayLoadLibraries.Add("agxSabre", LibSource.AGX);
			DelayLoadLibraries.Add("agxTerrain", LibSource.AGX);
			DelayLoadLibraries.Add("agxCable", LibSource.AGX);
			DelayLoadLibraries.Add("agxModel", LibSource.AGX);

			AddDelayLoadDependencies(DelayLoadLibraries);
		}

		// Ensure all runtime dependencies are copied to target.
		RuntimeDependencies.Add(Path.Combine(BundledAGXResourcesPath, "bin", "*"));
		RuntimeDependencies.Add(Path.Combine(BundledAGXResourcesPath, "data", "*"));
		RuntimeDependencies.Add(Path.Combine(BundledAGXResourcesPath, "plugins", "*"));
		RuntimeDependencies.Add(Path.Combine(BundledAGXResourcesPath, "include", "*"));
		RuntimeDependencies.Add(Path.Combine(BundledAGXResourcesPath, "lib", "*"));
		SetLicenseForCopySafe(Target);

		// This is a work-around for Linux which ensures the .so files are always copied to the target's
		// binaries directory (next to the executable if this is a cooked build). The reason why
		// this is needed is that RPATH of the module's .so files points to the wrong location otherwise.
		// Really, we would like those .so files' RPATH to point to the ThirdParty/agx/Lib/Linux directory
		// but we have not managed to find a way to do that yet.
		if (Target.Platform == UnrealTargetPlatform.Linux)
		{
			foreach (var RuntimeLibFile in RuntimeLibFiles)
			{
				CopyLinuxSoFromBundleToPluginBinaries();
				AddRuntimeDependencyCopyToBinariesDirectory(RuntimeLibFile.Key, RuntimeLibFile.Value, Target);
			}
		}
	}

	private void AddDelayLoadDependencies(Dictionary<string, LibSource> DelayLoadLibraries)
	{
		string PreprocessorDynamicLibraries = "";
		foreach (var Library in DelayLoadLibraries)
		{
			string FileName = BundledAGXResources.RuntimeLibraryFileName(Library.Key);
			PublicDelayLoadDLLs.Add(FileName);
			PreprocessorDynamicLibraries += FileName + " ";
		}

		// Add the list of library names as a preprocessor definition so that it can be used at runtime
		// to find and load the dynamic libraries.
		PublicDefinitions.Add("AGXUNREAL_DELAY_LOAD_LIBRARY_NAMES=" + PreprocessorDynamicLibraries);
	}

	private void AddLinkLibrary(string Name, LibSource Src)
	{
		string Dir = BundledAGXResources.LinkLibraryDirectory(Src);
		string FileName = BundledAGXResources.LinkLibraryFileName(Name);

		// File name and/or extension may include search patterns such as '*' or '?'. Resolve all these.
		string[] FilesToAdd = Directory.GetFiles(Dir, FileName);

		if (FilesToAdd.Length == 0)
		{
			Console.Error.WriteLine(
				"Error: File {0} did not match any file in {1}. The library will not be added in the build.",
				FileName, Dir);
			return;
		}

		foreach (string FilePath in FilesToAdd)
		{
			PublicAdditionalLibraries.Add(FilePath);
		}
	}

	private void AddIncludePath(LibSource Src)
	{
		PublicIncludePaths.Add(BundledAGXResources.IncludePath(Src));
	}

	private void CopyLinuxSoFromBundleToPluginBinaries()
	{
		string SoFilesDirSource = Path.Combine(GetBundledAGXResourcesPath(), "lib", "Linux");
		string[] FilesToCopy = Directory.GetFiles(SoFilesDirSource);
		string DestDir = Path.Combine(GetPluginBinariesPath(), "Linux");
		foreach (string FileToCopy in FilesToCopy)
		{
			string DestFilePath = Path.Combine(DestDir, Path.GetFileName(FileToCopy));
			if (!File.Exists(DestFilePath))
			{
				CopyFile(FileToCopy, DestFilePath);
			}
		}
	}

	private void AddRuntimeDependencyCopyToBinariesDirectory(string Name, LibSource Src, ReadOnlyTargetRules Target)
	{
		string Dir = BundledAGXResources.RuntimeLibraryDirectory(Src);
		string FileName = BundledAGXResources.RuntimeLibraryFileName(Name);

		// File name and/or extension may include search patterns such as '*' or
		// '?'. Resolve all these.
		string[] FilesToAdd = Directory.GetFiles(Dir, FileName);

		if (FilesToAdd.Length == 0)
		{
			Console.Error.WriteLine("Error: File {0} did not match any file in {1}. The dependency " +
				"will not be added in the build.", FileName, Dir);
			return;
		}

		foreach (string FilePath in FilesToAdd)
		{
			string Dest = Path.Combine("$(BinaryOutputDir)", Path.GetFileName(FilePath));
			RuntimeDependencies.Add(Dest, FilePath);
		}
	}

	private void SetLicenseForCopySafe(ReadOnlyTargetRules Target)
	{
		// License copying is only allowed for Development and Debug builds.
		bool bAllowedConfiguration = Target.Configuration == UnrealTargetConfiguration.Development ||
			Target.Configuration == UnrealTargetConfiguration.Debug ||
			Target.Configuration == UnrealTargetConfiguration.DebugGame;

		string LicenseCopyEnvVariableVal =
			Environment.GetEnvironmentVariable("AGXUNREAL_COPY_LICENSE_FILE_TO_TARGET");
		bool bLicenseCopyEnvVariableSet = !String.IsNullOrEmpty(LicenseCopyEnvVariableVal) &&
			LicenseCopyEnvVariableVal.Equals("true", StringComparison.OrdinalIgnoreCase);
		string LicenseDir = GetPluginLicensePath();

		if (bAllowedConfiguration && (bCopyLicenseFileToTarget || bLicenseCopyEnvVariableSet))
		{
			Console.WriteLine("AGX Dynamics license file will be copied to the build target with "
				+ "bCopyLicenseFileToTarget = {0} and bLicenseCopyEnvVariableSet = {1}",
				bCopyLicenseFileToTarget, bLicenseCopyEnvVariableSet);

			RuntimeDependencies.Add(Path.Combine(LicenseDir, "*"));
		}
		else
		{
			// Note the lack of '*'here. We copy only the README file, not all files.
			Console.WriteLine("AGX Dynamics license file will not be copied to build target.");
			RuntimeDependencies.Add(Path.Combine(LicenseDir, "README.md"));
		}
	}

	AGXVersion GetAGXVersion()
	{
		string VersionHeaderPath = GetAgxVersionHeaderPath();
		if (String.IsNullOrEmpty(VersionHeaderPath))
		{
			// Logging done in GetAgxVersionHeaderPath.
			return new AGXVersion();
		}

		string[] Lines;

		try
		{
			Lines = File.ReadAllLines(VersionHeaderPath);
		}
		catch (Exception e)
		{
			Console.Error.WriteLine("Error: GetAGXVersion failed. " +
				"Unable to read file {0}. Exception: {1}", VersionHeaderPath, e.Message);
			return new AGXVersion();
		}

		int? GenerationVer = ParseDefineDirectiveValue(Lines, "AGX_GENERATION_VERSION");
		int? MajorVer = ParseDefineDirectiveValue(Lines, "AGX_MAJOR_VERSION");
		int? MinorVer = ParseDefineDirectiveValue(Lines, "AGX_MINOR_VERSION");
		int? PatchVer = ParseDefineDirectiveValue(Lines, "AGX_PATCH_VERSION");

		if (!GenerationVer.HasValue || !MajorVer.HasValue || !MinorVer.HasValue || !PatchVer.HasValue)
		{
			Console.Error.WriteLine("Error: GetAGXVersion failed. " +
				"Unable to parse define directives in {0}", VersionHeaderPath);
			return new AGXVersion();
		}

		return new AGXVersion(GenerationVer.Value, MajorVer.Value, MinorVer.Value, PatchVer.Value);
	}

	private string GetBundledAGXResourcesPath()
	{
		return Path.Combine(GetPluginBinariesPath(), "ThirdParty", "agx");
	}

	private string GetPluginBinariesPath()
	{
		return Path.Combine(GetPluginRootPath(), "Binaries");
	}

	private string GetPluginLicensePath()
	{
		return Path.Combine(GetPluginRootPath(), "license");
	}

	private string GetPluginRootPath()
	{
		return Path.GetFullPath(Path.Combine(ModuleDirectory, "..", "..", ".."));
	}

	/// Returns true if AGX Dynamics resources are currently bundled with the plugin.
	/// Returns false otherwise.
	private bool IsAGXResourcesBundled()
	{
		return Directory.Exists(GetBundledAGXResourcesPath());
	}


	private void BundleAGXResources(ReadOnlyTargetRules Target, Dictionary<string, LibSource> RuntimeLibFiles,
		Dictionary<string, LibSource> LinkLibFiles, List<LibSource> IncludePaths)
	{
		if (!Heuristics.IsAGXSetupEnvCalled())
		{
			Console.Error.WriteLine("Error: Could not bundle AGX Dynamics resources because no AGX Dynamics installation "
				+ "was found. Please ensure that setup_env has been called.");
			return;
		}

		Console.WriteLine("Packaging AGX Dynamics resources starting...");
		AGXResourcesInfo InstalledAGXResources = new AGXResourcesInfo(Target, AGXResourcesLocation.InstalledAGX);

		// Copy AGX Dynamics runtime library files.
		foreach (var RuntimeLibFile in RuntimeLibFiles)
		{
			string Dir = InstalledAGXResources.RuntimeLibraryDirectory(RuntimeLibFile.Value);
			string FileName = InstalledAGXResources.RuntimeLibraryFileName(RuntimeLibFile.Key);

			// File name and/or extension may include search patterns such as '*' or '?'. Resolve all these.
			string[] FilesToCopy = Directory.GetFiles(Dir, FileName);
			if (FilesToCopy.Length == 0)
			{
				Console.Error.WriteLine("Error: File {0} did not match any file in {1}. Packaging " +
					"of AGX Dynamics resources failed.", FileName, Dir);
				CleanBundledAGXDynamicsResources();
				return;
			}

			foreach (string FilePath in FilesToCopy)
			{
				// Note: the BundledAGXResources.RuntimeLibraryPath() function cannot be used here since
				// the file name may have an added prefix that would then be added once again.
				string Dest = Path.Combine(
					BundledAGXResources.RuntimeLibraryDirectory(RuntimeLibFile.Value), Path.GetFileName(FilePath));
				if (!CopyFile(FilePath, Dest))
				{
					CleanBundledAGXDynamicsResources();
					return;
				}
			}
		}

		// Copy AGX Dynamics link library files.
		foreach (var LinkLibFile in LinkLibFiles)
		{
			string Dir = InstalledAGXResources.LinkLibraryDirectory(LinkLibFile.Value);
			string FileName = InstalledAGXResources.LinkLibraryFileName(LinkLibFile.Key);

			// File name and/or extension may include search patterns such as '*' or '?'. Resolve all these.
			string[] FilesToCopy = Directory.GetFiles(Dir, FileName);
			if (FilesToCopy.Length == 0)
			{
				Console.Error.WriteLine("Error: File {0} did not match any file in {1}. Packaging " +
					"of AGX Dynamics resources failed.", FileName, Dir);
				CleanBundledAGXDynamicsResources();
				return;
			}

			foreach (string FilePath in FilesToCopy)
			{
				// Note: the BundledAGXResources.LinkLibraryPath() function cannot be used here since
				// the file name may have an added prefix that would then be added once again.
				string Dest = Path.Combine(
					BundledAGXResources.LinkLibraryDirectory(LinkLibFile.Value), Path.GetFileName(FilePath));
				if (!CopyFile(FilePath, Dest))
				{
					CleanBundledAGXDynamicsResources();
					return;
				}
			}
		}

		// Copy AGX Dynamics header files.
		foreach (var IncludePath in IncludePaths)
		{
			string Source = InstalledAGXResources.IncludePath(IncludePath);
			string Dest = BundledAGXResources.IncludePath(IncludePath);

			// The three ____Win32 files are ignored because they are located in a directory with name
			// Win32. That directory name is not allowed by UAT during the staging phase and gives build
			// errors during cook builds.
			List<string> FilesToIgnore = new List<string>
				{ "GraphicsHandleWin32", "GraphicsWindowWin32", "PixelBufferWin32" };
			if(!CopyDirectoryRecursively(Source, Dest, FilesToIgnore))
			{
				CleanBundledAGXDynamicsResources();
				return;
			}
		}

		// Copy AGX Dynamics cfg directory.
		{
			string Source = InstalledAGXResources.RuntimeLibraryPath(string.Empty, LibSource.Cfg, true);
			string Dest = BundledAGXResources.RuntimeLibraryPath(string.Empty, LibSource.Cfg, true);
			if (!CopyDirectoryRecursively(Source, Dest))
			{
				CleanBundledAGXDynamicsResources();
				return;
			}
		}

		// Copy Terrain Material Library.
		{
			string Source = InstalledAGXResources.RuntimeLibraryPath(string.Empty, LibSource.TerrainMaterialLibrary, true);
			string Dest = BundledAGXResources.RuntimeLibraryPath(string.Empty, LibSource.TerrainMaterialLibrary, true);

			// We don't yet include the Terrain Material Library in the Docker images.
			// Remove this check once the images has been rebuilt.
			if (Directory.Exists(Source))
			{
				if (!CopyDirectoryRecursively(Source, Dest))
				{
					CleanBundledAGXDynamicsResources();
					return;
				}
			}
		}

		// Copy AGX Dynamics Components/agx/Physics directory and Components/agx/Referenced.agxEntity file.
		{
			string ComponentsDirSource = InstalledAGXResources.RuntimeLibraryPath(string.Empty, LibSource.Components, true);
			string ComponentsDirDest = BundledAGXResources.RuntimeLibraryPath(string.Empty, LibSource.Components, true);
			string PhysicsDirSource = Path.Combine(ComponentsDirSource, "agx", "Physics");
			string PhysicsDirDest = Path.Combine(ComponentsDirDest, "agx", "Physics");

			// The two .agxKernel files are not used and have given issues because of too long file paths on
			// Windows since they are located in a very deep directory tree branch.
			List<string> FilesToIgnore = new List<string>
				{ "GenerateLinesFromJacobians.agxKernel", "RenderJacobians.agxTask" };
			if (!CopyDirectoryRecursively(PhysicsDirSource, PhysicsDirDest, FilesToIgnore))
			{
				CleanBundledAGXDynamicsResources();
				return;
			}

			// Copy all single files in the Components/agx directory.
			if (!CopyFilesNonRecursive(Path.Combine(ComponentsDirSource, "agx"), Path.Combine(ComponentsDirDest, "agx")))
			{
				CleanBundledAGXDynamicsResources();
				return;
			}
		}

		Console.WriteLine("Packaging AGX Dynamics resources complete.");
	}

	private bool CopyFile(string Source, string Dest)
	{
		try
		{
			string DestDir = Path.GetDirectoryName(Dest);
			if (!Directory.Exists(DestDir))
			{
				Directory.CreateDirectory(DestDir);
			}

			File.Copy(Source, Dest, true);
		}
		catch (Exception e)
		{
			Console.Error.WriteLine("Error: Unable to copy file {0} to {1}. Exception: {2}", Source, Dest, e.Message);
			return false;
		}

		return true;
	}

	private bool CopyDirectoryRecursively(string SourceDir, string DestDir, List<string> FilesToIgnore = null)
	{
		foreach (string FilePath in Directory.GetFiles(SourceDir, "*", SearchOption.AllDirectories))
		{
			if (FilesToIgnore != null && FilesToIgnore.Contains(Path.GetFileName(FilePath)))
			{
				continue;
			}

			// Do not copy license files.
			if (Path.GetExtension(FilePath).Equals(".lic"))
			{
				continue;
			}

			string DestFilePath = Path.GetFullPath(FilePath).Replace(
				Path.GetFullPath(SourceDir), Path.GetFullPath(DestDir));
			if (!CopyFile(FilePath, DestFilePath))
			{
				return false;
			}
		}

		return true;
	}

	private bool CopyFilesNonRecursive(string SourceDir, string DestDir)
	{
		string[] FilesPaths = Directory.GetFiles(SourceDir);

		foreach (string FilePath in FilesPaths)
		{
			string FileName = Path.GetFileName(FilePath);
			if (!CopyFile(FilePath, Path.Combine(DestDir, FileName)))
			{
				return false;
			}
		}

		return true;
	}

	private void CleanBundledAGXDynamicsResources()
	{
		Console.WriteLine("Cleaning bundled AGX Dynamics resources started...");
		string BundledAGXResourcesPath = GetBundledAGXResourcesPath();
		try
		{
			if (Directory.Exists(BundledAGXResourcesPath))
			{
				Directory.Delete(BundledAGXResourcesPath, true);
			}
		}
		catch (Exception e)
		{
			Console.Error.WriteLine("Error: Unable to delete directory {0}. Exception: {1}",
				BundledAGXResourcesPath, e.Message);
			return;
		}
		Console.WriteLine("Cleaning bundled AGX Dynamics resources complete.");
	}

	private void EnsureLicenseDirCreated()
	{
		string LicenseDir = GetPluginLicensePath();
		if (!Directory.Exists(LicenseDir))
		{
			Directory.CreateDirectory(LicenseDir);
		}

		string ReadMePath = Path.Combine(LicenseDir, "README.md");
		if (!File.Exists(ReadMePath))
		{
			string ReadMeContent = "The AGX Dynamics license file should be placed in this directory.\n"
			+ "This directory should never be manually removed.";
			File.WriteAllText(ReadMePath, ReadMeContent);
		}
	}

	// Within the plugin, an AGX Dynamics license files may only exist within the specified 'license'
	// directory; in AGXUnreal/license.
	private bool MisplacedLicenseFileExists(out string MisplacedLicensePath)
	{
		MisplacedLicensePath = String.Empty;
		string RootDir = GetPluginRootPath();
		string licenseDirName = new DirectoryInfo(GetPluginLicensePath()).Name;
		foreach (string DirPath in Directory.GetDirectories(RootDir, "*", SearchOption.TopDirectoryOnly))
		{
			DirectoryInfo DirInfo = new DirectoryInfo(DirPath);
			if (DirInfo.Name.Equals(licenseDirName))
			{
				continue;
			}

			foreach (string FilePath in Directory.GetFiles(DirPath, "*", SearchOption.AllDirectories))
			{
				if (Path.GetExtension(FilePath).Equals(".lic"))
				{
					MisplacedLicensePath = FilePath;
					return true;
				}
			}
		}

		return false;
	}

	private int? ParseDefineDirectiveValue(string[] HeaderFileLines, string Identifier)
	{
		foreach (var Line in HeaderFileLines)
		{
			string[] Words = Line.Split(' ');
			if (Words.Length == 3 && Words[0].Equals("#define") && Words[1].Equals(Identifier))
			{
				int Val = 0;
				if (Int32.TryParse(Words[2], out Val))
				{
					return Val;
				}
			}
		}

		return null;
	}

	private string GetAgxVersionHeaderPath()
	{
		if (IsAGXResourcesBundled())
		{
			return Path.Combine(BundledAGXResources.IncludePath(LibSource.AGX), "agx", "agx_version.h");
		}

		// If the AGX Dynamics resources has not yet been bundled with the plugin, an AGX Dynamics
		// environment must be set up, so we can get the header file from there.
		if (!Heuristics.IsAGXSetupEnvCalled())
		{
			Console.Error.WriteLine("Error: GetAgxVersionHeaderPath failed. AGX Dynamics resources are not " +
			"bundled with the plugin and no AGX Dynamics environment has been setup. Please ensure that " +
			"setup_env has been called.");
			return string.Empty;
		}

		AGXResourcesInfo InstalledAGXResources = new AGXResourcesInfo(Target, AGXResourcesLocation.InstalledAGX);
		return Path.Combine(InstalledAGXResources.IncludePath(LibSource.AGX), "agx", "agx_version.h");
	}

	private class Heuristics
	{
		public static bool IsAGXSetupEnvCalled()
		{
			return Environment.GetEnvironmentVariable("AGX_DEPENDENCIES_DIR") != null;
		}
	}

	private class AGXVersion
	{
		public int GenerationVersion;
		public int MajorVersion;
		public int MinorVersion;
		public int PatchVersion;
		public bool IsInitialized = false;

		public AGXVersion(int Generation, int Major, int Minor, int Patch)
		{
			GenerationVersion = Generation;
			MajorVersion = Major;
			MinorVersion = Minor;
			PatchVersion = Patch;
			IsInitialized = true;
		}

		public AGXVersion()
		{
			IsInitialized = false;
		}

		public bool IsOlderThan(AGXVersion Other)
		{
			if (!IsInitialized || !Other.IsInitialized)
			{
				Console.Error.WriteLine("Error: IsOlderThan called on or with uninitialized AGXVersion object.");
				return false;
			}

			List<int> Ver = ToList();
			List<int> OtherVer = Other.ToList();

			for (int I = 0; I < Ver.Count; I++)
			{
				if (Ver[I] < OtherVer[I])
				{
					return true;
				}

				if (Ver[I] > OtherVer[I])
				{
					return false;
				}
			}

			// Both versions are identical.
			return false;
		}

		public bool IsNewerOrEqualTo(int Generation, int Major, int Minor, int Patch)
		{
			if (!IsInitialized)
			{
				Console.Error.WriteLine("Error: IsNewerOrEqualTo called on uninitialized AGXVersion object.");
				return false;
			}

			return !IsOlderThan(Generation, Major, Minor, Patch);
		}

		public bool IsOlderThan(int Generation, int Major, int Minor, int Patch)
		{
			return IsOlderThan(new AGXVersion(Generation, Major, Minor, Patch));
		}

		public List<int> ToList()
		{
			return new List<int> { GenerationVersion, MajorVersion, MinorVersion, PatchVersion };
		}
	}

	///
	private class AGXResourcesInfo
	{
		public string LinkLibraryPrefix;
		public string LinkLibraryPostfix;

		public string RuntimeLibraryPrefix;
		public string RuntimeLibraryPostfix;

		Dictionary<LibSource, LibSourceInfo> LibSources;

		public string LinkLibraryFileName(string LibraryName)
		{
			return LinkLibraryPrefix + LibraryName + LinkLibraryPostfix;
		}

		public string RuntimeLibraryFileName(string LibraryName)
		{
			return RuntimeLibraryPrefix + LibraryName + RuntimeLibraryPostfix;
		}

		public string IncludePath(LibSource Src)
		{
			LibSourceInfo Info = LibSources[Src];
			if (Info.IncludePath == null)
			{
				Console.Error.WriteLine("Error: No include path for '{0}'.", Src);
				return null;
			}
			return Info.IncludePath;
		}

		public string LinkLibraryPath(string LibraryName, LibSource Src)
		{
			LibSourceInfo Info = LibSources[Src];
			if (Info.LinkLibrariesPath == null)
			{
				Console.Error.WriteLine("Error: No LinkLibraryPath for '{0}', '{1}' cannot be found.", Src, LibraryName);
				return LibraryName;
			}
			return Path.Combine(Info.LinkLibrariesPath, LinkLibraryFileName(LibraryName));
		}

		public string LinkLibraryDirectory(LibSource Src)
		{
			LibSourceInfo Info = LibSources[Src];
			if (Info.LinkLibrariesPath == null)
			{
				Console.Error.WriteLine("Error: No LinkLibraryPath for '{0}'.", Src);
				return string.Empty;
			}
			return Info.LinkLibrariesPath;
		}

		public string RuntimeLibraryPath(string LibraryName, LibSource Src,
			bool IsDirectory = false)
		{
			LibSourceInfo Info = LibSources[Src];
			if (Info.RuntimeLibrariesPath == null)
			{
				Console.Error.WriteLine("Error: No RuntimeLibraryPath for '{0}', '{1}' cannot be found.", Src,
					LibraryName);
				return LibraryName;
			}
			if (IsDirectory)
			{
				return Path.Combine(Info.RuntimeLibrariesPath, LibraryName);
			}
			else
			{
				return Path.Combine(Info.RuntimeLibrariesPath, RuntimeLibraryFileName(LibraryName));
			}
		}

		public string RuntimeLibraryDirectory(LibSource Src)
		{
			LibSourceInfo Info = LibSources[Src];
			if (Info.RuntimeLibrariesPath == null)
			{
				Console.Error.WriteLine("Error: No RuntimeLibraryDirectory for '{0}'.", Src);
				return string.Empty;
			}
			return Info.RuntimeLibrariesPath;
		}


		private void InitializeLinuxLocalBuildAGX()
		{
			string SourceDir = Environment.GetEnvironmentVariable("AGX_DIR");
			string BuildDir = Environment.GetEnvironmentVariable("AGX_BUILD_DIR");
			string DependenciesDir = Environment.GetEnvironmentVariable("AGX_DEPENDENCIES_DIR");
			string TerrainDependenciesDir = Environment.GetEnvironmentVariable("AGXTERRAIN_DEPENDENCIES_DIR");

			LibSources.Add(LibSource.AGX, new LibSourceInfo(
				Path.Combine(SourceDir, "include"),
				Path.Combine(BuildDir, "lib"),
				Path.Combine(BuildDir, "lib")
			));
			LibSources.Add(LibSource.Config, new LibSourceInfo(
				Path.Combine(BuildDir, "include"),
				null, null
			));
			LibSources.Add(LibSource.Components, new LibSourceInfo(
				Path.Combine(SourceDir, "Components"),
				null,
				Path.Combine(SourceDir, "Components")
			));
			LibSources.Add(LibSource.Dependencies, new LibSourceInfo(
				Path.Combine(DependenciesDir, "include"),
				Path.Combine(DependenciesDir, "lib"),
				Path.Combine(DependenciesDir, "lib")
			));
			LibSources.Add(LibSource.TerrainDependencies, new LibSourceInfo(
				Path.Combine(TerrainDependenciesDir, "include"),
				Path.Combine(TerrainDependenciesDir, "lib"),
				Path.Combine(TerrainDependenciesDir, "lib")
			));
			LibSources.Add(LibSource.Cfg, new LibSourceInfo(
				null, null,
				Path.Combine(SourceDir, "data", "cfg")
			));
			LibSources.Add(LibSource.TerrainMaterialLibrary, new LibSourceInfo(
				null, null,
				Path.Combine(SourceDir, "data", "TerrainMaterials")
			));
		}


		private void InitializeLinuxInstalledAGX()
		{
			string BaseDir = Environment.GetEnvironmentVariable("AGX_DIR");

			LibSources.Add(LibSource.AGX, new LibSourceInfo(
				Path.Combine(BaseDir, "include"),
				Path.Combine(BaseDir, "lib"),
				Path.Combine(BaseDir, "lib")
			));
			LibSources.Add(LibSource.Config, new LibSourceInfo(
				Path.Combine(BaseDir, "include"),
				null, null
			));
			LibSources.Add(LibSource.Components, new LibSourceInfo(
				Path.Combine(BaseDir, "include"),
				null,
				Path.Combine(BaseDir, "bin", "plugins", "Components")
			));
			LibSources.Add(LibSource.Dependencies, new LibSourceInfo(
				Path.Combine(BaseDir, "include"),
				Path.Combine(BaseDir, "lib"),
				Path.Combine(BaseDir, "lib")
			));
			LibSources.Add(LibSource.TerrainDependencies, new LibSourceInfo(
				Path.Combine(BaseDir, "include"),
				Path.Combine(BaseDir, "lib"),
				Path.Combine(BaseDir, "lib")
			));
			LibSources.Add(LibSource.Cfg, new LibSourceInfo(
				null, null,
				Path.Combine(BaseDir, "data", "cfg")
			));
			LibSources.Add(LibSource.TerrainMaterialLibrary, new LibSourceInfo(
				null, null,
				Path.Combine(BaseDir, "data", "TerrainMaterials")
			));
		}



		private void InitializeLinuxBundledAGX(string BundledAGXResourcesPath)
		{
			string BaseDir = BundledAGXResourcesPath;

			LibSources.Add(LibSource.AGX, new LibSourceInfo(
				Path.Combine(BaseDir, "include"),
				Path.Combine(BaseDir, "lib", "Linux"),
				Path.Combine(BaseDir, "lib", "Linux")
			));
			LibSources.Add(LibSource.Config, new LibSourceInfo(
				Path.Combine(BaseDir, "include"),
				null, null
			));
			LibSources.Add(LibSource.Components, new LibSourceInfo(
				Path.Combine(BaseDir, "include"),
				null,
				Path.Combine(BaseDir, "bin", "plugins", "Components")
			));
			LibSources.Add(LibSource.Dependencies, new LibSourceInfo(
				Path.Combine(BaseDir, "include"),
				Path.Combine(BaseDir, "lib", "Linux"),
				Path.Combine(BaseDir, "lib", "Linux")
			));
			LibSources.Add(LibSource.TerrainDependencies, new LibSourceInfo(
				Path.Combine(BaseDir, "include"),
				Path.Combine(BaseDir, "lib", "Linux"),
				Path.Combine(BaseDir, "lib", "Linux")
			));
			LibSources.Add(LibSource.Cfg, new LibSourceInfo(
				null, null,
				Path.Combine(BaseDir, "data", "cfg")
			));
			LibSources.Add(LibSource.TerrainMaterialLibrary, new LibSourceInfo(
				null, null,
				Path.Combine(BaseDir, "data", "TerrainMaterials")
			));
		}


		private void InitializeWindowsInstalledAGX()
		{
			string BaseDir = Environment.GetEnvironmentVariable("AGX_DIR");
			string PluginDir = Environment.GetEnvironmentVariable("AGX_PLUGIN_PATH");
			string DataDir = Environment.GetEnvironmentVariable("AGX_DATA_DIR");

			LibSources.Add(LibSource.AGX, new LibSourceInfo(
				Path.Combine(BaseDir, "include"),
				Path.Combine(BaseDir, "lib", "x64"),
				Path.Combine(BaseDir, "bin", "x64")
			));
			LibSources.Add(LibSource.Config, new LibSourceInfo(
				null, null, null
			));
			LibSources.Add(LibSource.Components, new LibSourceInfo(
				null, null,
				Path.Combine(PluginDir, "Components")
			));
			LibSources.Add(LibSource.Dependencies, new LibSourceInfo(
				null,
				Path.Combine(BaseDir, "lib", "x64"),
				Path.Combine(BaseDir, "bin", "x64")
			));
			LibSources.Add(LibSource.TerrainDependencies, new LibSourceInfo(
				null,
				Path.Combine(BaseDir, "lib", "x64"),
				Path.Combine(BaseDir, "bin", "x64")
			));
			LibSources.Add(LibSource.Cfg, new LibSourceInfo(
				null, null,
				Path.Combine(DataDir, "cfg")
			));
			LibSources.Add(LibSource.TerrainMaterialLibrary, new LibSourceInfo(
				null, null,
				Path.Combine(DataDir, "TerrainMaterials")
			));
		}


		private void InitializeWindowsBundledAGX(string BundledAGXResourcesPath)
		{
			string BaseDir = BundledAGXResourcesPath;

			LibSources.Add(LibSource.AGX, new LibSourceInfo(
				Path.Combine(BaseDir, "include"),
				Path.Combine(BaseDir, "lib", "Win64"),
				Path.Combine(BaseDir, "bin", "Win64")
			));
			LibSources.Add(LibSource.Config, new LibSourceInfo(
				null, null, null
			));
			LibSources.Add(LibSource.Components, new LibSourceInfo(
				null, null,
				Path.Combine(BaseDir, "plugins", "Components")
			));
			LibSources.Add(LibSource.Dependencies, new LibSourceInfo(
				null,
				Path.Combine(BaseDir, "lib", "Win64"),
				Path.Combine(BaseDir, "bin", "Win64")
			));
			LibSources.Add(LibSource.TerrainDependencies, new LibSourceInfo(
				null,
				Path.Combine(BaseDir, "lib", "Win64"),
				Path.Combine(BaseDir, "bin", "Win64")
			));
			LibSources.Add(LibSource.Cfg, new LibSourceInfo(
				null, null,
				Path.Combine(BaseDir, "data", "cfg")
			));
			LibSources.Add(LibSource.TerrainMaterialLibrary, new LibSourceInfo(
				null, null,
				Path.Combine(BaseDir, "data", "TerrainMaterials")
			));
		}

		public AGXResourcesInfo(
			ReadOnlyTargetRules Target, AGXResourcesLocation AGXLocation, string BundledAGXResourcesPath = "")
		{
			LibSources = new Dictionary<LibSource, LibSourceInfo>();
			if (Target.Platform == UnrealTargetPlatform.Linux)
			{
				// On Linux there is only one library file type and it is used
				// both for linking and at runtime. All libraries are named
				// following the pattern libNAME.so[.VERSION]. By convention
				// library files are stored in the 'lib' directory. Each library
				// file should be passed on the linker command line and copied
				// to the runtime directory.
				LinkLibraryPrefix = "lib";
				LinkLibraryPostfix = ".so";
				RuntimeLibraryPrefix = "lib";
				RuntimeLibraryPostfix = ".so*";

				switch (AGXLocation)
				{
					case AGXResourcesLocation.LocalBuildAGX:
					{
						InitializeLinuxLocalBuildAGX();
						break;
					}
					case AGXResourcesLocation.InstalledAGX:
					{
						InitializeLinuxInstalledAGX();
						break;
					}
					case AGXResourcesLocation.BundledAGX:
					{
						InitializeLinuxBundledAGX(BundledAGXResourcesPath);
						break;
					}
				}
			}
			else if (Target.Platform == UnrealTargetPlatform.Win64)
			{
				// On Windows there is separate file types for linking and at
				// runtime. At link time .lib files in the lib directory is
				// used, and  at run time .dll files in the bin directory is
				// used.
				LinkLibraryPrefix = "";
				LinkLibraryPostfix = ".lib";
				RuntimeLibraryPrefix = "";
				RuntimeLibraryPostfix = ".dll";

				switch (AGXLocation)
				{
					case AGXResourcesLocation.LocalBuildAGX:
					{
						throw new InvalidOperationException("Local AGX Dynamics build not yet supported on Windows.");
					}
					case AGXResourcesLocation.InstalledAGX:
					{
						InitializeWindowsInstalledAGX();
						break;
					}
					case AGXResourcesLocation.BundledAGX:
					{
						InitializeWindowsBundledAGX(BundledAGXResourcesPath);
						break;
					}
				}
			}

			if (Target.Configuration == UnrealTargetConfiguration.Debug)
			{
				// Always building against the release AGX Dynamics on Linux for
				// now. Only because it is difficult to switch between release
				// and debug on Linux.
				if (Target.Platform != UnrealTargetPlatform.Linux) {
					string DebugSuffix = "d";
					LinkLibraryPostfix = DebugSuffix + LinkLibraryPostfix;
					RuntimeLibraryPostfix = DebugSuffix + RuntimeLibraryPostfix;
				}
			}
		}
	}
}