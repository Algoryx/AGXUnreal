#include "AGX_Environment.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "TypeConversions.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agx/Runtime.h>
#include <agx/version.h>
#include "EndAGXIncludes.h"

// Unreal Engine includes.
#include "GenericPlatform/GenericPlatformProcess.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

#define LOCTEXT_NAMESPACE "FAGX_Environment"

// Create a current-platform-specific version of the OS utilities.
/// \note Something like this should be built into Unreal. Find it.
#if defined(_WIN64)
#include "Windows/WindowsPlatformMisc.h"
struct FCurrentPlatformMisc : public FWindowsPlatformMisc
{
};
#elif defined(__linux__)
#include "Linux/LinuxPlatformMisc.h"
struct FCurrentPlatformMisc : public FLinuxPlatformMisc
{
};
#else
// Unsupported platform.
static_assert(false);
#endif

FAGX_Environment::FAGX_Environment()
{
	Init();
}

FAGX_Environment::~FAGX_Environment()
{
}

void FAGX_Environment::Init()
{
	const FString AgxDynamicsResoucePath = GetAgxDynamicsResourcesPath();
	if (FAGX_Environment::IsSetupEnvRun())
	{
		UE_LOG(
			LogAGX, Log, TEXT("AGX Dynamics installation was detected. Using resources from: %s"),
			*AgxDynamicsResoucePath);
		// Only setup AGX Dynamics environment if setup_env has not been called.
		return;
	}

	UE_LOG(
		LogAGX, Log,
		TEXT("No installation of AGX Dynamics detected. Using AGX Dynamics resources from the "
			 "AGXUnreal plugin at: %s"),
		*AgxDynamicsResoucePath);

	LoadDynamicLibraries();
	SetupAGXDynamicsEnvironment();
}

// All AGX Dynamics dlls are loaded by using GetDllHandle on Windows. On Linux, most AGX Dynamics
// libraries are found by the OS without the need for GetDllHandle.
void FAGX_Environment::LoadDynamicLibraries()
{
	check(DynamicLibraryHandles.Num() == 0);
	check(IsSetupEnvRun() == false);
	const FString AgxResourcesPath = GetAgxDynamicsResourcesPath();
	TArray<FString> AGXDynamicsDependencyFileNames;

#if defined(_WIN64)
	const FString LibraryNameList = PREPROCESSOR_TO_STRING(AGXUNREAL_DELAY_LOAD_LIBRARY_NAMES);
	LibraryNameList.ParseIntoArray(AGXDynamicsDependencyFileNames, TEXT(" "), false);
	const FString DependecyDir =
		FPaths::Combine(AgxResourcesPath, FString("bin"), FString("Win64"));

	// vdbgrid must always be loaded to be found by agxTerrain during runtime.
	AGXDynamicsDependencyFileNames.Add("vdbgrid.dll");
#elif defined(__linux__)
	const FString DependecyDir =
		FPaths::Combine(AgxResourcesPath, FString("lib"), FString("Linux"));

	// vdbgrid must always be loaded to be found by agxTerrain during runtime.
	// On Linux, we give GetDllHandle the full path, because otherwise it seems to look in the wrong
	// place.
	AGXDynamicsDependencyFileNames.Add(FPaths::Combine(DependecyDir, TEXT("libvdbgrid.so")));
#else
	// Unsupported platform.
	static_assert(false);
#endif

	UE_LOG(LogAGX, Log, TEXT("About to load dynamic libraries."));

#if defined(_WIN64)
	FPlatformProcess::PushDllDirectory(*DependecyDir);
#endif

	for (const FString& FileName : AGXDynamicsDependencyFileNames)
	{
		const FString FullFilePath = FPaths::Combine(DependecyDir, FileName);
		void* Handle = FPlatformProcess::GetDllHandle(*FileName);
		if (Handle == nullptr)
		{
#if defined(_WIN64)
			UE_LOG(
				LogAGX, Error,
				TEXT("Tried to dynamically load '%s' but the loading failed. Some AGX "
					 "Dynamics for Unreal features might not be available."),
				*FullFilePath);
#endif
#if defined(__linux__)
			UE_LOG(
				LogAGX, Error,
				TEXT("Tried to dynamically load '%s' but the loading failed. Some AGX "
					 "Dynamics for Unreal features might not be available."),
				*FileName);
#endif
			continue;
		}

		DynamicLibraryHandles.Add(Handle);
	}

#if defined(_WIN64)
	FPlatformProcess::PopDllDirectory(*DependecyDir);
#endif

	if (AGXDynamicsDependencyFileNames.Num() == DynamicLibraryHandles.Num())
	{
		UE_LOG(LogAGX, Log, TEXT("Successfully loaded all dynamic libraries."));
	}
	else
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("At least one dynamic library failed to load. The AGX Dynamics for Unreal "
				 "plugin will likely not function as expected."));
	}
}

void FAGX_Environment::SetupAGXDynamicsEnvironment()
{
	const FString AgxResourcesPath = GetAgxDynamicsResourcesPath();
	const FString AgxBinPath = FPaths::Combine(AgxResourcesPath, FString("bin"));
	const FString AgxDataPath = FPaths::Combine(AgxResourcesPath, FString("data"));
	const FString AgxCfgPath = FPaths::Combine(AgxDataPath, FString("cfg"));
	FString AgxPluginsPath = FPaths::Combine(AgxResourcesPath, FString("plugins"));

	if (!FPaths::DirectoryExists(AgxPluginsPath))
	{
		AgxPluginsPath = FPaths::Combine(AgxBinPath, FString("plugins"));
	}

	// Ensure that the necessary AGX Dynamics resources are packed with the plugin.
	if (!FPaths::DirectoryExists(AgxResourcesPath) || !FPaths::DirectoryExists(AgxBinPath) ||
		!FPaths::DirectoryExists(AgxDataPath) || !FPaths::DirectoryExists(AgxCfgPath) ||
		!FPaths::DirectoryExists(AgxPluginsPath))
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("AGX Dynamics resources are not bundled with the AGXUnreal plugin. The "
				 "plugin will not be able to load AGX Dynamics. The resources where expected "
				 "to be at: %s"),
			*AgxResourcesPath);

		// This will likely result in a runtime error since the needed AGX Dynamics resources
		// are nowhere to be found.
		return;
	}

	// If AGX Dynamics is installed on this computer, agxIO.Environment.instance() will
	// read data from the registry and add runtime and resource paths to
	// the installed version (even if setup_env has not been called). Clear all, from registry
	// added paths since we will use the AGX Dynamics resources packed with the plugin only.
	for (int i = 0; i < (int) agxIO::Environment::Type::NUM_TYPES; i++)
	{
		AGX_ENVIRONMENT().getFilePath((agxIO::Environment::Type) i).clear();
	}

	// Point the AGX environment to the resources packed with the plugin.
	AGX_ENVIRONMENT()
		.getFilePath(agxIO::Environment::RUNTIME_PATH)
		.pushbackPath(Convert(AgxPluginsPath));

	AGX_ENVIRONMENT()
		.getFilePath(agxIO::Environment::RESOURCE_PATH)
		.pushbackPath(Convert(AgxResourcesPath));

	AGX_ENVIRONMENT()
		.getFilePath(agxIO::Environment::RESOURCE_PATH)
		.pushbackPath(Convert(AgxDataPath));

	AGX_ENVIRONMENT()
		.getFilePath(agxIO::Environment::RESOURCE_PATH)
		.pushbackPath(Convert(AgxCfgPath));

	const FString AgxLicensePath = GetPluginLicenseDirPath();
	if (FPaths::DirectoryExists(AgxLicensePath))
	{
		AGX_ENVIRONMENT()
			.getFilePath(agxIO::Environment::RESOURCE_PATH)
			.pushbackPath(Convert(AgxLicensePath));
	}
}

FAGX_Environment& FAGX_Environment::GetInstance()
{
	static FAGX_Environment Instance;
	return Instance;
}

bool FAGX_Environment::EnsureEnvironmentSetup()
{
	// Environment setup is done from the constructor, i.e. at this point it has already been done.
	return true;
}

// May return empty FString if plugin path is not found.
FString FAGX_Environment::GetPluginPath()
{
	constexpr TCHAR PLUGIN_NAME[] = TEXT("AGXUnreal");

	FString AgxPluginPath;
	if (auto Plugin = IPluginManager::Get().FindPlugin(PLUGIN_NAME))
	{
		AgxPluginPath = FPaths::ConvertRelativePathToFull(Plugin->GetBaseDir());
	}
	else
	{
		UE_LOG(LogAGX, Error, TEXT("FAGX_Environment::GetPluginPath unable to get plugin path."));
	}

	return AgxPluginPath;
}

FString FAGX_Environment::GetPluginBinariesPath()
{
	const FString PluginPath = GetPluginPath();
	const FString PluginBinPath = FPaths::Combine(PluginPath, FString("Binaries"));

	return PluginBinPath;
}

FString FAGX_Environment::GetPluginLicenseDirPath()
{
	const FString PluginPath = GetPluginPath();
	return FPaths::Combine(PluginPath, FString("license"));
}

FString FAGX_Environment::GetProjectBinariesPath()
{
	const FString ProjectPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
	const FString ProjectBinPath = FPaths::Combine(ProjectPath, FString("Binaries"));

	return ProjectBinPath;
}

FString FAGX_Environment::GetPluginVersion()
{
	constexpr TCHAR PLUGIN_NAME[] = TEXT("AGXUnreal");

	if (auto Plugin = IPluginManager::Get().FindPlugin(PLUGIN_NAME))
	{
		return Plugin->GetDescriptor().VersionName;
	}
	else
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("FAGX_Environment::GetPluginVersion unable to get plugin version."));
		return FString();
	}
}

void FAGX_Environment::AddEnvironmentVariableEntry(const FString& EnvVarName, const FString& Entry)
{
	if (Entry.IsEmpty())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("FAGX_Environment::AddEnvironmentVariableEntry parameter Entry was "
				 "empty."));
		return;
	}

	TArray<FString> EnvVarValArray = GetEnvironmentVariableEntries(EnvVarName);

	// Only append Entry if it is not already present.
	if (EnvVarValArray.Find(Entry) != -1)
	{
		return;
	}

	EnvVarValArray.Add(Entry);
	SetEnvironmentVariableEntries(EnvVarName, EnvVarValArray);
}

void FAGX_Environment::RemoveEnvironmentVariableEntry(
	const FString& EnvVarName, const FString& Entry)
{
	if (Entry.IsEmpty())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("FAGX_Environment::RemoveEnvironmentVariableEntry parameter Entry was "
				 "empty."));
		return;
	}

	TArray<FString> EnvVarValArray = GetEnvironmentVariableEntries(EnvVarName);
	EnvVarValArray.Remove(Entry);
	SetEnvironmentVariableEntries(EnvVarName, EnvVarValArray);
}

TArray<FString> FAGX_Environment::GetEnvironmentVariableEntries(const FString& EnvVarName)
{
	FString EnvVarVal = FCurrentPlatformMisc::GetEnvironmentVariable(*EnvVarName);
	TArray<FString> EnvVarValArray;
	EnvVarVal.ParseIntoArray(EnvVarValArray, TEXT(";"), false);
	return EnvVarValArray;
}

void FAGX_Environment::SetEnvironmentVariableEntries(
	const FString& EnvVarName, const TArray<FString>& Entries)
{
	FString EnvVarVal = FString::Join(Entries, TEXT(";"));
	FCurrentPlatformMisc::SetEnvironmentVar(*EnvVarName, *EnvVarVal);
}

bool FAGX_Environment::IsSetupEnvRun()
{
	const TArray<FString> AgxDepDirEntries =
		FAGX_Environment::GetEnvironmentVariableEntries("AGX_DEPENDENCIES_DIR");

	const TArray<FString> AgxDirEntries =
		FAGX_Environment::GetEnvironmentVariableEntries("AGX_DIR");

	return AgxDepDirEntries.Num() > 0 && AgxDirEntries.Num() > 0;
}

FString FAGX_Environment::GetAGXDynamicsVersion()
{
	return FString(agxGetVersion(false));
}

void FAGX_Environment::GetAGXDynamicsVersion(
	int32& OutGeneration, int32& OutMajor, int32& OutMinor, int32& OutPatch)
{
	OutGeneration = AGX_GENERATION_VERSION;
	OutMajor = AGX_MAJOR_VERSION;
	OutMinor = AGX_MINOR_VERSION;
	OutPatch = AGX_PATCH_VERSION;
}

bool FAGX_Environment::IsAGXDynamicsVersionNewerOrEqualTo(
	int32 InGeneration, int32 InMajor, int32 InMinor, int32 InPatch)
{
	int32 Generation, Major, Minor, Patch;
	GetAGXDynamicsVersion(Generation, Major, Minor, Patch);

	const TArray<int32> InVer {InGeneration, InMajor, InMinor, InPatch};
	const TArray<int32> AGXVer {Generation, Major, Minor, Patch};

	for (int I = 0; I < InVer.Num(); I++)
	{
		if (InVer[I] < AGXVer[I])
		{
			return true;
		}

		if (InVer[I] > AGXVer[I])
		{
			return false;
		}
	}

	// Both versions are identical.
	return true;
}

FString FAGX_Environment::GetAgxDynamicsResourcesPath()
{
	if (IsSetupEnvRun())
	{
		const TArray<FString> AgxDirEntries =
			FAGX_Environment::GetEnvironmentVariableEntries("AGX_DIR");
		if (AgxDirEntries.Num() <= 0)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("FAGX_Environment::GetAgxDynamicsResourcesPath environment variable "
					 "AGX_DIR not set when expecting setup_env to have be called. Returning empty "
					 "string."));
			return FString("");
		}

		return AgxDirEntries[0];
	}
	else
	{
		// Get and return path to AGX Dynamics resources when bundled with the plugin.
		const FString BinariesPath = FAGX_Environment::GetPluginBinariesPath();
		const FString AgxResourcesPath =
			FPaths::Combine(BinariesPath, FString("ThirdParty"), FString("agx"));

		return AgxResourcesPath;
	}
}

bool FAGX_Environment::EnsureAgxDynamicsLicenseValid(FString* OutStatus)
{
	agx::Runtime* AgxRuntime = agx::Runtime::instance();
	if (AgxRuntime == nullptr)
	{
		return false;
	}

	if (AgxRuntime->isValid())
	{
		if (OutStatus)
		{
			*OutStatus = Convert(AgxRuntime->getStatus());
		}
		return true;
	}

	// License is not valid. Attempt to unlock using a license file in the plugin's
	// license directory that might have been put there recently by the user.
	TryUnlockAgxDynamicsLicense();

	const bool LicenseValid = AgxRuntime->isValid();
	if (OutStatus)
	{
		*OutStatus = Convert(AgxRuntime->getStatus());
	}

	return LicenseValid;
}

void FAGX_Environment::TryUnlockAgxDynamicsLicense()
{
	agx::Runtime* AgxRuntime = agx::Runtime::instance();
	if (AgxRuntime == nullptr)
	{
		return;
	}

	const FString AgxLicensePath =
		FPaths::Combine(GetPluginLicenseDirPath(), FString("agx.lic"));
	if (!FPaths::FileExists(AgxLicensePath))
	{
		return;
	}

	FString License;
	FFileHelper::LoadFileToString(License, *AgxLicensePath);
	if (License.IsEmpty())
	{
		return;
	}

	if (AgxRuntime->unlock(Convert(License)))
	{
		UE_LOG(
			LogAGX, Log,
			TEXT("Successfully unlocked AGX Dynamics license using license file located at: %s"),
			*AgxLicensePath);
	}
}

#undef LOCTEXT_NAMESPACE