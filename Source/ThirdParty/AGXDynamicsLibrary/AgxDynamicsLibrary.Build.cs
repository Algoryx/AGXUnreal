using System.IO;
using System;
using System.Collections.Generic;
using UnrealBuildTool;


/// The AGXDynamicsLibrary is the portal from AGXUnrealBarrier to AGX Dynamics.
/// This is where we list the include paths and linker requirements to build an
/// Unreal Engine plugin that uses AGX Dynamics.
public class AGXDynamicsLibrary : ModuleRules
{

	/// Information about how AGX Dynamics is packaged and used on the current
	/// platform.
	private AGXResourcesInfo PackagedAGXResources;

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

		/// The AGX Terrain depdendencies. Procides libraries that AGX Terrain
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
	/// installation or an AGX Dynamics packaged into the plugin.
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

		/// An AGX Dynamics installation that has been packaged into the plugin.
		PackagedAGX
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
	///
	/// Important note: Currently no reliable way of copying the AGX Dynamics
	/// resources to the correct location when building an executable from a
	/// project using this plugin has been found. It is therefore recommended to
	/// manually copy the 'agx' directory located in
	/// AGXUnreal/Binaries/Thirdparty and place it in the same directory as the
	/// executable file. Also, all files inside
	/// AGXUnreal/Binaries/Thirdparty/agx/bin/PLATFORM should be manually
	/// copied to the same directory as the executable file.
	public AGXDynamicsLibrary(ReadOnlyTargetRules Target) : base(Target)
	{
		string PackagedAGXResoucesPath = GetPackagedAGXResourcesPath();
		PackagedAGXResources =
			new AGXResourcesInfo(Target, AGXResourcesLocation.PackagedAGX, PackagedAGXResoucesPath);
		Type = ModuleType.External;

		// Because the AGX Dynamics type system uses typeid and dynamic_cast.
		bUseRTTI = true;

		// Because AGX Dynamics uses exceptions.
		bEnableExceptions = true;

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
		RuntimeLibFiles.Add("zlib", LibSource.Dependencies);
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
			RuntimeLibFiles.Add("png", LibSource.Dependencies);

			IncludePaths.Add(LibSource.Components);
			IncludePaths.Add(LibSource.Config);
			IncludePaths.Add(LibSource.Dependencies);
			IncludePaths.Add(LibSource.TerrainDependencies);
		}
		else if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			RuntimeLibFiles.Add("msvcp140", LibSource.AGX);
			RuntimeLibFiles.Add("vcruntime140", LibSource.AGX);
            RuntimeLibFiles.Add("agx-assimp-vc*-mt", LibSource.AGX);

			RuntimeLibFiles.Add("websockets", LibSource.Dependencies);
			RuntimeLibFiles.Add("libpng", LibSource.Dependencies);
			RuntimeLibFiles.Add("ot2?-OpenThreads", LibSource.Dependencies);
			RuntimeLibFiles.Add("glew", LibSource.Dependencies);
		}

		// Package AGX Dynamics resources in plugin if no packaged resources exists.
		if (!IsAGXResourcesPackaged())
		{
			PackageAGXResources(Target, RuntimeLibFiles, LinkLibFiles, IncludePaths);
		}
		else
		{
			Console.WriteLine("Skipping packaging of AGX Dynamics resources, packaged "
				+ "resources already exists in: {0}", PackagedAGXResoucesPath);
		}

		foreach (var RuntimeLibFile in RuntimeLibFiles)
		{
			AddRuntimeDependency(RuntimeLibFile.Key, RuntimeLibFile.Value, Target);
		}

		foreach (var LinkLibFile in LinkLibFiles)
		{
			AddLinkLibrary(LinkLibFile.Key, LinkLibFile.Value);
		}

		foreach (var HeaderPath in IncludePaths)
		{
			AddIncludePath(HeaderPath);
		}
	}

	// The runtime dependency file is copied to the target binaries directory.
	private void AddRuntimeDependency(string Name, LibSource Src, ReadOnlyTargetRules Target)
	{
		string Dir = PackagedAGXResources.RuntimeLibraryDirectory(Src);
		string FileName = PackagedAGXResources.RuntimeLibraryFileName(Name);

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
			// This is a temporary work-around to fix the issue where dll/so
			// files are copied to the Binaries directory of any project that
			// are built that uses this plugin, which should not happen. See
			// internal issue 282.
			//
			// @todo Figure out how to properly copy the dll/so files for all
			// build scenarios.
			if (ShouldCopyBinFiles(Target))
			{
				string Dest = Path.Combine("$(BinaryOutputDir)", Path.GetFileName(FilePath));
				RuntimeDependencies.Add(Dest, FilePath);
			}
			else
			{
				RuntimeDependencies.Add(FilePath);
			}
		}
	}

	private void AddLinkLibrary(string Name, LibSource Src)
	{
		string Dir = PackagedAGXResources.LinkLibraryDirectory(Src);
		string FileName = PackagedAGXResources.LinkLibraryFileName(Name);

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
		PublicIncludePaths.Add(PackagedAGXResources.IncludePath(Src));
	}


	private string GetPackagedAGXResourcesPath()
	{
		return Path.GetFullPath(Path.Combine(ModuleDirectory, "..", "..", "..", "Binaries", "ThirdParty", "agx"));
	}


	/// Returns true if AGX Dynamics resources are currently packaged with the plugin.
	/// Returns false otherwise.
	private bool IsAGXResourcesPackaged()
	{
		return Directory.Exists(GetPackagedAGXResourcesPath());
	}


	private void PackageAGXResources(ReadOnlyTargetRules Target, Dictionary<string, LibSource> RuntimeLibFiles,
		Dictionary<string, LibSource> LinkLibFiles, List<LibSource> IncludePaths)
	{
		if (!Heuristics.IsAGXSetupEnvCalled())
		{
			Console.Error.WriteLine("Error: Could not package AGX Dynamics resources because no AGX Dynamics installation "
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
				CleanPackagedAGXDynamicsResources();
				return;
			}

			foreach (string FilePath in FilesToCopy)
			{
				// Note: the PackagedAGXResources.RuntimeLibraryPath() function cannot be used here since
				// the file name may have an added prefix that would then be added once again.
				string Dest = Path.Combine(
					PackagedAGXResources.RuntimeLibraryDirectory(RuntimeLibFile.Value), Path.GetFileName(FilePath));
				if (!CopyFile(FilePath, Dest))
				{
					CleanPackagedAGXDynamicsResources();
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
				CleanPackagedAGXDynamicsResources();
				return;
			}

			foreach (string FilePath in FilesToCopy)
			{
				// Note: the PackagedAGXResources.LinkLibraryPath() function cannot be used here since
				// the file name may have an added prefix that would then be added once again.
				string Dest = Path.Combine(
					PackagedAGXResources.LinkLibraryDirectory(LinkLibFile.Value), Path.GetFileName(FilePath));
				if (!CopyFile(FilePath, Dest))
				{
					CleanPackagedAGXDynamicsResources();
					return;
				}
			}
		}

		// Copy AGX Dynamics header files.
		foreach (var IncludePath in IncludePaths)
		{
			string Source = InstalledAGXResources.IncludePath(IncludePath);
			string Dest = PackagedAGXResources.IncludePath(IncludePath);
			if(!CopyDirectoryRecursively(Source, Dest))
			{
				CleanPackagedAGXDynamicsResources();
				return;
			}
		}

		// Copy AGX Dynamics cfg directory.
		{
			string Source = InstalledAGXResources.RuntimeLibraryPath(string.Empty, LibSource.Cfg, true);
			string Dest = PackagedAGXResources.RuntimeLibraryPath(string.Empty, LibSource.Cfg, true);
			if (!CopyDirectoryRecursively(Source, Dest))
			{
				CleanPackagedAGXDynamicsResources();
				return;
			}
		}

		// Copy Terrain Material Library.
		{
			string Source = InstalledAGXResources.RuntimeLibraryPath(string.Empty, LibSource.TerrainMaterialLibrary, true);
			string Dest = PackagedAGXResources.RuntimeLibraryPath(string.Empty, LibSource.TerrainMaterialLibrary, true);

			// We don't yet include the Terrain Material Library in the Docker images.
			// Remove this check once the images has been rebuilt.
			if (Directory.Exists(Source))
			{
				if (!CopyDirectoryRecursively(Source, Dest))
				{
					CleanPackagedAGXDynamicsResources();
					return;
				}
			}
		}

		// Copy AGX Dynamics Components/agx/Physics directory and Components/agx/Referenced.agxEntity file.
		{
			string ComponentsDirSource = InstalledAGXResources.RuntimeLibraryPath(string.Empty, LibSource.Components, true);
			string ComponentsDirDest = PackagedAGXResources.RuntimeLibraryPath(string.Empty, LibSource.Components, true);
			string PhysicsDirSource = Path.Combine(ComponentsDirSource, "agx", "Physics");
			string PhysicsDirDest = Path.Combine(ComponentsDirDest, "agx", "Physics");

			if (!CopyDirectoryRecursively(PhysicsDirSource, PhysicsDirDest))
			{
				CleanPackagedAGXDynamicsResources();
				return;
			}

			// Copy all single files in the Components/agx directory.
			if (!CopyFilesNonRecursive(Path.Combine(ComponentsDirSource, "agx"), Path.Combine(ComponentsDirDest, "agx")))
			{
				CleanPackagedAGXDynamicsResources();
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

	private bool CopyDirectoryRecursively(string SourceDir, string DestDir)
	{
		foreach (string DirPath in Directory.GetDirectories(SourceDir, "*", SearchOption.AllDirectories))
		{
			Directory.CreateDirectory(DirPath.Replace(SourceDir, DestDir));
		}

		foreach (string FilePath in Directory.GetFiles(SourceDir, "*", SearchOption.AllDirectories))
		{
			// Do not copy license files.
			if (Path.GetExtension(FilePath).Equals(".lic"))
			{
				continue;
			}

			if (!CopyFile(FilePath, FilePath.Replace(SourceDir, DestDir)))
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

	private void CleanPackagedAGXDynamicsResources()
	{
		Console.WriteLine("Cleaning packaged AGX Dynamics resources started...");
		string PackagedAGXResourcesPath = GetPackagedAGXResourcesPath();
		try
		{
			if (Directory.Exists(PackagedAGXResourcesPath))
			{
				Directory.Delete(PackagedAGXResourcesPath, true);
			}
		}
		catch (Exception e)
		{
			Console.Error.WriteLine("Error: Unable to delete directory {0}. Exception: {1}",
				PackagedAGXResourcesPath, e.Message);
			return;
		}
		Console.WriteLine("Cleaning packaged AGX Dynamics resources complete.");
	}

	// This is a temporary work-around to fix the issue where dll/so files are copied to the
	// Binaries directory of any project that are built that uses this plugin, which should not happen.
	// See internal issue 282.
	// @todo Figure out how to properly copy the dll/so files for all build scenarios.
	private bool ShouldCopyBinFiles(ReadOnlyTargetRules Target)
	{
		string ProjectName = Path.GetFileNameWithoutExtension(Target.ProjectFile.ToString());

		// When building an executable, the project name will be set to 'HostProject'.
		return ProjectName.Equals("AGXUnrealDev") || ProjectName.Equals("HostProject");
	}

	private class Heuristics
	{
		public static bool IsAGXSetupEnvCalled()
		{
			return Environment.GetEnvironmentVariable("AGX_DEPENDENCIES_DIR") != null;
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



		private void InitializeLinuxPackagedAGX(string PackagedAGXResourcesPath)
		{
			string BaseDir = PackagedAGXResourcesPath;

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


		private void InitializeWindowsPackagedAGX(string PackagedAGXResourcesPath)
		{
			string BaseDir = PackagedAGXResourcesPath;

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
			ReadOnlyTargetRules Target, AGXResourcesLocation AGXLocation, string PackagedAGXResourcesPath = "")
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
					case AGXResourcesLocation.PackagedAGX:
					{
						InitializeLinuxPackagedAGX(PackagedAGXResourcesPath);
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
					case AGXResourcesLocation.PackagedAGX:
					{
						InitializeWindowsPackagedAGX(PackagedAGXResourcesPath);
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
