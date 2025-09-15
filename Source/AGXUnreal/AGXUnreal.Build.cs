// Copyright 2025, Algoryx Simulation AB.

using System; // For Console, Environment.
using System.IO; // For Path.
using System.Diagnostics; // For running processes.
using System.Collections.Generic; // For List.

using UnrealBuildTool;

public class AGXUnreal : ModuleRules
{
	public AGXUnreal(ReadOnlyTargetRules Target) : base(Target)
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

		// TODO: Determine which of these are really need and why.
		PublicDependencyModuleNames.AddRange(new string[] {
			"AGXCommon", "AGXUnrealBarrier", "RHI", "RenderCore", "Core", "CoreUObject", "Engine",
			"InputCore", "Niagara"});


		// TODO: Determine which of these are really needed and why.
		// TODO: Why are some modules listed both here and in Public... above?
		// TODO: These were included before. Don't know why or where they came from. Remove?
		//       "CoreUObject", "Engine", "Slate", "SlateCore"
		PrivateDependencyModuleNames.AddRange(new string[] {
			"RHI", "RenderCore", "Projects", "Json", "Landscape", "Slate",
			"SlateCore", "MeshDescription", "StaticMeshDescription"});

		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.Add("EditorStyle");
			PrivateDependencyModuleNames.Add("UnrealEd");
		}

		UpdateEngineVersionInUPlugin();
		GitInfo GitInfo = CreateGitInfo();
		PluginDescriptor PluginDescriptor = GetPluginDescriptor();
		WriteBuildInfo(PluginDescriptor, GitInfo);
	}

	/**
	 * Overwrite the EngineVersion attribute in AGXUnreal.uplugin with the
	 * Major.Minor version of the currently running Unreal Engine.
	 */
	private void UpdateEngineVersionInUPlugin()
	{
		// I would like to use System.Text.Json, but it seems Unreal Engine 4.26
		// has a too old C#/.Net version. I get the following error message:
		//
		//   error CS0234: The type or namespace name `Json' does not exist in the namespace `System.Text'.
		//   Are you missing an assembly reference?
		//
		// Doing manual line parsing for now.

		string PluginRoot = GetPluginRootPath();
		string UPluginPath = Path.Combine(PluginRoot, "AGXUnreal.uplugin");
		string UPluginTmpPath = Path.Combine(PluginRoot, "AGXUnreal.uplugin.tmp");
		string ErrorPrefix = "While updating Engine Version in AGXUnreal.uplugin:";
		string ErrorSuffix = "AGXUnreal.uplugin is unchanged.";

		if (!File.Exists(UPluginPath))
		{
			Console.Error.WriteLine(
				"{0} Could not find the AGXUnreal.uplugin file. Looked in {1}. {2}",
				ErrorPrefix, UPluginPath, ErrorSuffix);
			return;
		}

		BuildVersion Version;
		if (!BuildVersion.TryRead(BuildVersion.GetDefaultFileName(), out Version))
		{
			Console.Error.WriteLine(
				"{0} Could not parse Unreal Engine version from {1}. {2}",
				ErrorPrefix, BuildVersion.GetDefaultFileName(), ErrorSuffix);
			return;
		}

		List<string> NewUPluginContents = new List<string>();
		foreach (string Line in File.ReadLines(UPluginPath))
		{
			string ToWrite;
			if (Line.Contains("\"EngineVersion\": "))
			{
				ToWrite = String.Format(
					"\t\"EngineVersion\": \"{0}.{1}.0\",",
					Version.MajorVersion, Version.MinorVersion);
			}
			else
			{
				ToWrite = Line;
			}
			NewUPluginContents.Add(ToWrite);
		}

		if (NewUPluginContents.Count == 0)
		{
			Console.Error.WriteLine(
				"{0} Did not get any file contents from {1}. {2}",
				ErrorPrefix, UPluginPath, ErrorSuffix);
			return ;
		}

		File.WriteAllLines(UPluginTmpPath, NewUPluginContents);
		// File.Move overload with Overwrite flag is not available in Unreal Engine 4.26.
		File.Delete(UPluginPath);
		File.Move(UPluginTmpPath, UPluginPath);
	}

	/// Return the path to the AGXUnreal subdirectory in the Plugins directory.
	private string GetPluginRootPath()
	{
		// ModuleDirectory is the full path to Plugins/AGXUnreal/Source/AGXUnreal.
		// This returns the full path to Plugins/AGXUnreal.
		return Path.GetFullPath(Path.Combine(ModuleDirectory, "..", ".."));
	}

	private string GetProjectRootPath()
	{
		// ModuleDirectory is the full path to Plugins/AGXUnreal/Source/AGXUnreal.
		// This returns the full path to the directory that contains the Plugins
		// directory, which typically is an Unreal project root.
		return Path.GetFullPath(Path.Combine(ModuleDirectory, "..", "..", "..", ".."));
	}

	/**
	 * The result of running a blocking external command.
	 */
	private struct ProcessResult
	{
		public bool Success;
		public string Output;
		public string Error;

		public ProcessResult(bool success, string output, string error)
		{
			Success = success;
			Output = output;
			Error = error;
		}

		public bool IsValid()
		{
			return Success && Output.Trim() != "";
		}
	}

	/**
	 * Run an external command and wait for it to finish.
	 */
	private ProcessResult RunProcess(string Executable, string Arguments)
	{
		try
		{
			var Config = new ProcessStartInfo(Executable, Arguments)
			{
				CreateNoWindow = true,
				UseShellExecute = false,
				RedirectStandardOutput = true,
				RedirectStandardError = true
			};
			Process RunningProcess = Process.Start(Config);
			string Output = RunningProcess.StandardOutput.ReadToEnd();
			string Error = RunningProcess.StandardError.ReadToEnd();
			RunningProcess.WaitForExit();
			return new ProcessResult(RunningProcess.ExitCode == 0, Output, Error);
		}
		catch (Exception exception)
		{
			Console.WriteLine("Cannot run process '{0}': {1}", Executable, exception.Message);
			return new ProcessResult(false, "", "");
		}
	}

	private bool IsFileContentNew(List<string> NewContent, string Path)
	{
		if (!File.Exists(Path))
		{
			return true;
		}
		// I tried to make a better variant of this, but I don't know C# well enough.
		List<string> OldContent = new List<string>();
		foreach (string Line in File.ReadLines(Path))
		{
			OldContent.Add(Line);
		}
		if (NewContent.Count != OldContent.Count)
		{
			return true;
		}
		for (int i = 0; i < NewContent.Count; ++i)
		{
			if (NewContent[i] != OldContent[i])
			{
				return true;
			}
		}
		return false;
	}

	/**
	 * Write AGX_BuildInfo.generated.h with information about this particular
	 * build, such as the plugin version and Git branch name.
	 */
	private void WriteBuildInfo(PluginDescriptor PluginDescriptor, GitInfo GitInfo)
	{
		List<string> BuildInfo = new List<string>();

		bool bWroteVersion = false;
		if (PluginDescriptor != null)
		{
			string VersionName = PluginDescriptor.VersionName;
			string[] Versions = VersionName.Split(".");
			int VersionNumber = PluginDescriptor.Version;
			if (Versions.Length == 3)
			{
				BuildInfo.Add("");
				BuildInfo.Add("// Version info:");
				BuildInfo.Add("#define AGXUNREAL_HAS_VERSIONS 1");
				BuildInfo.Add(String.Format("#define AGXUNREAL_VERSION_NAME \"{0}\"", VersionName));
				BuildInfo.Add(String.Format("#define AGXUNREAL_MAJOR_VERSION {0}", Versions[0]));
				BuildInfo.Add(String.Format("#define AGXUNREAL_MINOR_VERSION {0}", Versions[1]));
				BuildInfo.Add(String.Format("#define AGXUNREAL_PATCH_VERSION {0}", Versions[2]));
				BuildInfo.Add(String.Format("#define AGXUNREAL_VERSION {0}", VersionNumber));
				bWroteVersion = true;
			}
			else
			{
				Console.Error.WriteLine(String.Format("Could not split version name string '{0}' into three version numbers. AGX_BuildInfo.generated.h will not contain AGXUNREAL_MAJOR_VERSION, AGXUNREAL_MINOR_VERSION, nor AGXUNREAL_PATCH_VERSION."));
			}
		}
		if (!bWroteVersion)
		{
			BuildInfo.Add("");
			BuildInfo.Add("// Version info:");
			BuildInfo.Add("#define AGXUNREAL_HAS_VERSIONS 0");
			BuildInfo.Add("#define AGXUNREAL_VERSION_NAME \"0.0.0\"");
			BuildInfo.Add("#define AGXUNREAL_MAJOR_VERSION 0");
			BuildInfo.Add("#define AGXUNREAL_MINOR_VERSION 0");
			BuildInfo.Add("#define AGXUNREAL_PATCH_VERSION 0");
			BuildInfo.Add(String.Format("#define AGXUNREAL_VERSION 0"));
		}

		if (GitInfo != null)
		{
			string Hash = GitInfo.CommitHash;
			string Name = GitInfo.BranchName;

			BuildInfo.Add("");
			BuildInfo.Add("// Git info:");
			BuildInfo.Add(String.Format("#define AGXUNREAL_HAS_GIT_HASH {0}", (Hash != "" ? "1" : "0")));
			BuildInfo.Add(String.Format("const TCHAR* const AGXUNREAL_GIT_HASH = TEXT(\"{0}\");", Hash));
			BuildInfo.Add("");
			BuildInfo.Add(String.Format("#define AGXUNREAL_HAS_GIT_NAME {0}", (Name != "" ? "1" : "0")));
			BuildInfo.Add(String.Format("const TCHAR* const AGXUNREAL_GIT_NAME = TEXT(\"{0}\");", Name));
			BuildInfo.Add("");
		}
		else
		{
			BuildInfo.Add("");
			BuildInfo.Add("// Git info:");
			BuildInfo.Add("#define AGXUNREAL_HAS_GIT_HASH 0");
			BuildInfo.Add("const TCHAR* const AGXUNREAL_GIT_HASH = TEXT(\"\");");
			BuildInfo.Add("");
			BuildInfo.Add("#define AGXUNREAL_HAS_GIT_NAME 0");
			BuildInfo.Add("const TCHAR* const AGXUNREAL_GIT_NAME = TEXT(\"\");");
			BuildInfo.Add("");
		}

		string FilePath = Path.Combine(GetPluginRootPath(), "Source", "AGXCommon", "Public", "AGX_BuildInfo.generated.h");
		if (IsFileContentNew(BuildInfo, FilePath))
		{
			File.WriteAllLines(FilePath, BuildInfo);
		}
	}

	private bool IsGitRepository(string Path)
	{
		string Arguments = String.Format("-C {0} remote", Path);
		ProcessResult Result = RunProcess("git", Arguments);
		return Result.Success;
	}

	/// Path to the Git repository, either the plugin's or the project's.
	/// Can be null.
	private string RepositoryPath = null;

	private string FindGitRepository()
	{
		// This used to use GetPluginRootPath instead, but this breaks when
		// using Unreal Build Tool to build the plugin as a stand-alone package,
		// i.e. "RunUAT -BuildPluing"

		// We need to support several different use-cases:
		//
		// Internal Project Build
		//
		// The plugin is being built as part of the AGXUnrealDev project. This
		// is the main development mode for the Algoryx team. In this case the
		// build happens within the Git repository itself and the plugin path is
		// returned. The generated AGX_BuildInfo.generated.h will include the
		// branch name of the plugin working copy of the GitHub repository.
		//
		// Internal Plugin Build
		//
		// When building the plugin stand-alone, for example to produce a self-
		// hosted plugin package, Unreal Build Tool copies the plugin files out
		// of the Unreal project and builds it elsewhere. In this case the copy
		// will contain .git data, it is included with the copy of the plugin
		// files, but that data will not be valid from the new location so
		// IsGitRepository for that location will fail. We will then fall back
		// to the project's location. For builds on our development and GitLab
		// CI machines this will be within the AGXUnrealDev project hosted on
		// our internal GitLab server, so the generated
		// AGX_BuildInfo.generated.h will include the branch name of the project
		// working copy of the GiLab repository. We keep the GitLab and GitHub
		// branches in sync so the name should be the same but the commit hash
		// will be different.
		//
		// External Project Build With Plugin Package
		//
		// This is when a user builds their own projects that includes AGX
		// Dynamics for Unreal installed from a self-hosted plugin package. In
		// this case we have neither the GitHub nor the GitLab repository
		// checked out, but we may be inside a Git repository owned by the user.
		// In this case we will return a non-null path from here but the
		// repository will be rejected later for not having one of the known AGX
		// Dynamics for Unreal URLs and no AGX_BuildInfo.generated.h data will
		// be generated.
		//
		// External Plugin Build With Plugin Clone
		//
		// This is when a user build their own projects that includes AGX
		// Dynamics for Unreal installed by cloning the GitHub repository. This
		// will behave exactly like Internal Project Build, we will find the
		// GitHub working copy and use the branch name from there.
		//
		// External Plugin Build
		//
		// This is when a user builds the plugin stand-alone. This is currently
		// unsupported, meaning that AGX_BuildInfo.generated.h will not contain
		// a branch name. There is, as far as I know, no way to know where
		// Unreal Build Tool copied the plugin files from before the build
		// process was started.
		//
		// External Plugin Build Of Forked GitHub Repository
		//
		// People may chose to fork the GitHub repository to do their own
		// additions and modifications. This is currently not supported, meaning
		// that no branch name will be written to AGX_BuildInfo.generated.h. The
		// reason for this is that we do not want to accidentally pick up a
		// branch name from some unrelated Git repository, so we require that
		// the URL points to a known AGXUnreal or AGXUnrealDev repository. If a
		// user want to have branch name support in their forks then a possible
		// workaround is to modify the URL check in CreateGitInfo.

		string PluginPath = GetPluginRootPath();
		if (IsGitRepository(PluginPath))
		{
			return PluginPath;
		}

		string ProjectPath = GetProjectRootPath();
		if (IsGitRepository(ProjectPath))
		{
			return ProjectPath;
		}

		return null;
	}

	private string GitArgs(string Args)
	{
		return String.Format("-C \"{0}\" {1}", RepositoryPath, Args);
	}

	private string RemovePrefix(string Name, string Prefix)
	{
		return Name.StartsWith(Prefix) ? Name.Substring(Prefix.Length) : Name;
	}

	private class GitInfo
	{
		public string BranchName;
		public string CommitHash;
	}

	private GitInfo CreateGitInfo()
	{
		// Don't write Git info if we can't run 'git'.
		// Not writing empty because if the file already exists then we assume
		// that it came from a plugin package built elsewhere and already
		// contains Git info.
		ProcessResult TestGitResult = RunProcess("git", "--version");
		if (!TestGitResult.Success)
		{
			Console.WriteLine("AGXUnreal: Git not installed, cannot generate Git info.");
			return null;
		}

		RepositoryPath = FindGitRepository();
		if (String.IsNullOrEmpty(RepositoryPath))
		{
			Console.WriteLine("AGXUnreal: Could not find Git repository, cannot generate Git info.");
			return null;
		}

		// Determine if we are in an AGX Dynamics for Unreal working copy by
		// checking the name of the remote. If we aren't then we assume we
		// are in a client repository, or none at all, and assume that Git info
		// has either already been provided by the plugin package or isn't
		// necessary.
		ProcessResult GetRemoteResult = RunProcess("git", GitArgs("remote -v"));
		if (!GetRemoteResult.IsValid())
		{
			Console.WriteLine("AGXUnreal: Could not determine Git remote, cannot generate Git info.");
			return null;
		}
		if (!GetRemoteResult.Output.Contains("github.com/Algoryx/AGXUnreal.git")
			&& !GetRemoteResult.Output.Contains("github.com:Algoryx/AGXUnreal.git")
			&& !GetRemoteResult.Output.Contains("git.algoryx.se:algoryx/unreal/agxunreal.git")
			&& !GetRemoteResult.Output.Contains("git.algoryx.se/algoryx/unreal/agxunreal.git"))
		{
			// Not in an AGX Dynamics for Unreal working copy.
			Console.WriteLine("AGXUnreal: Not in a clone of the AGXUnreal Git repository, cannot generate Git info.");
			Console.WriteLine("  {0}", GetRemoteResult.Output);
			return null;
		}


		// Get Git hash for the current commit.
		string Hash;
		ProcessResult GetHashResult = RunProcess("git", GitArgs("rev-parse HEAD"));
		if (GetHashResult.IsValid())
		{
			Hash = GetHashResult.Output.Trim();
		}
		else
		{
			Console.WriteLine("AGXUnreal: Failed to get Git commit hash:");
			Console.WriteLine(GetHashResult.Error);
			Hash = "";
		}


		// Get the name for this revision. This is either a tag or branch name.
		// Tag names get precedence over branch names, if both are available. A
		// GitLab CI provided name, through CI_COMMIT_REF_NAME, get precendence
		// over everything else since when running in GitLab CI we get bogus
		// branch names from Git itself.
		string Name = "";

		// First check CI_COMMIT_REF_NAME.
		if (String.IsNullOrEmpty(Name))
		{
			Name = Environment.GetEnvironmentVariable("CI_COMMIT_REF_NAME");
		}

		// Then try to read tag name from Git.
		if (String.IsNullOrEmpty(Name))
		{
			// An alternative is to use 'git describe --tags', not sure what the
			// pros and cons are.
			ProcessResult GetTagResult = RunProcess("git", GitArgs("tag --points-at HEAD"));
			if (GetTagResult.IsValid())
			{
				Name = GetTagResult.Output.Trim();
			}
			else
			{
				if (!String.IsNullOrEmpty(GetTagResult.Error))
				{
					Console.WriteLine("AGXUnreal: Failed to get Git tag:");
					Console.WriteLine(GetTagResult.Error);
				}
			}
		}

		// Then try to read branch name from Git with 'rev-parse'.
		if (String.IsNullOrEmpty(Name))
		{
			// Get current Git branch.
			ProcessResult GetBranchResult = RunProcess("git", GitArgs("rev-parse --abbrev-ref HEAD"));
			if (GetBranchResult.IsValid())
			{
				Name = GetBranchResult.Output.Trim();
			}
			else
			{
				Console.Error.WriteLine("AGXUnreal: Failed to get Git branch:");
				Console.WriteLine(GetBranchResult.Error);
			}

			// HEAD is not a valid branch name, we get this if there is no
			// current branch.
			if (Name == "HEAD")
			{
				Name = "";
			}
		}

		// Then try to read the branch or tag name from Git with 'describe`.
		if (String.IsNullOrEmpty(Name))
		{
			ProcessResult DescribeResult = RunProcess("git", GitArgs("describe --all"));
			if (DescribeResult.IsValid())
			{
				// This contains not only the branch or tag name, but also a
				// prefix describing where the name was found, e.g. 'heads/' or
				// 'tags/'. Remove that part. This list may be incomplete.
				Name = DescribeResult.Output.Trim();
				Name = RemovePrefix(Name, "heads/");
				Name = RemovePrefix(Name, "tags/");
			}
			else
			{
				Console.WriteLine("AGXUnreal: Could not branch name using 'git describe':");
				Console.WriteLine(DescribeResult.Error);
			}
		}

		// HEAD is not a valid tag or branch name but can still show up if
		// name detection fails.
		if (Name == "HEAD")
		{
			Name = "";
		}

		if (Name.StartsWith("pipelines/"))
		{
			// When running a GitLab pipeline we get a non-descript branch name
			// which should not be presented to the user. We should not get here,
			// in this case we should use CI_COMMIT_REF_NAME instead.
			Name = "";
		}

		GitInfo Info = new ();
		Info.BranchName = Name;
		Info.CommitHash = Hash;
		return Info;
	}

	private PluginDescriptor GetPluginDescriptor()
	{
		var Plugin = Plugins.GetPlugin("AGXUnreal");
		if (Plugin == null)
		{
			return null;
		}

		return Plugin.Descriptor;
	}
}
