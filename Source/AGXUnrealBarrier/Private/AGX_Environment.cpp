// Copyright 2024, Algoryx Simulation AB.

#include "AGX_Environment.h"

// AGX Dynamics for Unreal includes.
#include "AGX_BuildInfo.h"
#include "AGX_LogCategory.h"
#include "TypeConversions.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agx/Runtime.h>
#include <agx/version.h>
#include <agxUtil/agxUtil.h>
#include "EndAGXIncludes.h"

// Unreal Engine includes.
#include "Misc/EngineVersionComparison.h"
#include "GenericPlatform/GenericPlatformProcess.h"
#include "HAL/PlatformFileManager.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/EngineVersionComparison.h"
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

namespace AGX_Environment_helpers
{
	// Returns true if the directory could be created or already exists. Returns false otherwise.
	bool CreateDirectoryIfNonExistent(const FString& Path)
	{
		if (FPaths::DirectoryExists(Path))
		{
			return true;
		}

		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		if (!PlatformFile.CreateDirectory(*Path))
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Unable to create directory: '%s'. The Output Log may contain more "
					 "information."),
				*Path);
			return false;
		}

		return true;
	}

	const FString& GetLegacyLicenseFileEnding()
	{
		static const FString s = ".lic";
		return s;
	}

	const FString& GetServiceLicenseFileEnding()
	{
		static const FString s = ".lfx";
		return s;
	}

	const FString& GetEncryptedRuntimeServiceLicenseFileEnding()
	{
		static const FString s = ".rtlfx";
		return s;
	}

	agx::Runtime* GetAgxRuntime()
	{
		agx::Runtime* AgxRuntime = agx::Runtime::instance();
		if (AgxRuntime == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Unexpected error: agx::Runtime::instance() returned nullptr. "
					 "Please contact the Algoryx support."));
			return nullptr;
		}

		return AgxRuntime;
	}
}

FAGX_Environment::FAGX_Environment()
{
	Init();
}

FAGX_Environment::~FAGX_Environment()
{
}

void FAGX_Environment::Init()
{
	const FString AgxDynamicsResoucePath = GetAGXDynamicsResourcesPath();
	if (FAGX_Environment::IsSetupEnvRun())
	{
		UE_LOG(LogAGX, Log, TEXT("Using AGX Dynamics resources from: %s"), *AgxDynamicsResoucePath);
		// Only setup AGX Dynamics environment if setup_env has not been called.
		return;
	}

	UE_LOG(LogAGX, Log, TEXT("Using AGX Dynamics resources from: %s"), *AgxDynamicsResoucePath);

	LoadDynamicLibraries();
	SetupAGXDynamicsEnvironment();
}

// All AGX Dynamics dlls are loaded by using GetDllHandle on Windows. On Linux, most AGX Dynamics
// libraries are found by the OS without the need for GetDllHandle.
void FAGX_Environment::LoadDynamicLibraries()
{
	check(DynamicLibraryHandles.Num() == 0);
	check(IsSetupEnvRun() == false);
	const FString AgxResourcesPath = GetAGXDynamicsResourcesPath();
	TArray<FString> AGXDynamicsDependencyFileNames;

#if defined(_WIN64)
	const FString LibraryNameList = PREPROCESSOR_TO_STRING(AGXUNREAL_DELAY_LOAD_LIBRARY_NAMES);
	LibraryNameList.ParseIntoArray(AGXDynamicsDependencyFileNames, TEXT(" "), false);
	const FString DependecyDir =
		FPaths::Combine(AgxResourcesPath, FString("bin"), FString("Win64"));
	if (!IsAGXDynamicsVersionNewerOrEqualTo(2, 32, 0, 0))
	{
		// vdbgrid must always be loaded to be found by agxTerrain during runtime.
		AGXDynamicsDependencyFileNames.Add("vdbgrid.dll");
	}

#elif defined(__linux__)
	const FString DependecyDir =
		FPaths::Combine(AgxResourcesPath, FString("lib"), FString("Linux"));

	if (!IsAGXDynamicsVersionNewerOrEqualTo(2, 32, 0, 0))
	{
		// vdbgrid must always be loaded to be found by agxTerrain during runtime.
		// On Linux, we give GetDllHandle the full path, because otherwise it seems to look in the
		//  wrong place.
		AGXDynamicsDependencyFileNames.Add(FPaths::Combine(DependecyDir, TEXT("libvdbgrid.so")));
	}

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
	const FString AgxResourcesPath = GetAGXDynamicsResourcesPath();
#if defined(_WIN64)
	const FString DependecyDir =
		FPaths::Combine(AgxResourcesPath, FString("bin"), FString("Win64"));
#elif defined(__linux__)
	const FString DependecyDir =
		FPaths::Combine(AgxResourcesPath, FString("lib"), FString("Linux"));
#else
	// Unsupported platform.
	static_assert(false);
#endif

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
		!FPaths::DirectoryExists(AgxPluginsPath) || !FPaths::DirectoryExists(DependecyDir))
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
		.getFilePath(agxIO::Environment::RUNTIME_PATH)
		.pushbackPath(Convert(DependecyDir)); // Used by AGPU to find AlgoryxGPUSensorsImpl lib.

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
	if (AGX_Environment_helpers::CreateDirectoryIfNonExistent(AgxLicensePath))
	{
		AGX_ENVIRONMENT()
			.getFilePath(agxIO::Environment::RESOURCE_PATH)
			.pushbackPath(Convert(AgxLicensePath));
	}
	else
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Unable to create license directory '%s' while setting up AGX Dynamics "
				 "environment. License file detection may not work as expected. Try to create this "
				 "directory manually."),
			*AgxLicensePath);
	}
}

FAGX_Environment& FAGX_Environment::GetInstance()
{
	static FAGX_Environment Instance;
	return Instance;
}

bool FAGX_Environment::EnsureEnvironmentSetup() const
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

FString FAGX_Environment::GetPluginSourcePath()
{
	const FString PluginPath = GetPluginPath();
	const FString PluginSrcPath = FPaths::Combine(PluginPath, FString("Source"));

	return PluginSrcPath;
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

FString FAGX_Environment::GetPluginRevision()
{
	const FString Hash = []()
	{
		if (AGXUNREAL_HAS_GIT_HASH)
			return FString(AGXUNREAL_GIT_HASH).Left(8);
		else
			return FString(TEXT(""));
	}();

	const FString Name = []()
	{
		if (AGXUNREAL_HAS_GIT_NAME)
			return FString::Printf(TEXT(" (%s)"), AGXUNREAL_GIT_NAME);
		else
			return FString(TEXT(""));
	}();

	return FString::Printf(TEXT("%s%s"), *Hash, *Name);
}

FString FAGX_Environment::FindAGXEnvironmentResourcePath(const FString& RelativePath)
{
	const agx::String RelativePathAGX = Convert(RelativePath);
	FString FullPath;
	{
		agx::String FullPathAGX =
			AGX_ENVIRONMENT().getFilePath(agxIO::Environment::RESOURCE_PATH).find(RelativePathAGX);
		FullPath = Convert(FullPathAGX);

		// Must be called to avoid crash due to different allocators used by AGX Dynamics and
		// Unreal Engine.
		agxUtil::freeContainerMemory(FullPathAGX);
	}

	return FullPath;
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
	const FString EnvVarVal = FCurrentPlatformMisc::GetEnvironmentVariable(*EnvVarName);
	if (EnvVarVal.IsEmpty())
		return TArray<FString>();

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

void FAGX_Environment::SetNumThreads(uint32 NumThreads)
{
	if (NumThreads == agx::getNumThreads())
	{
		return;
	}

	UE_LOG(
		LogAGX, Log, TEXT("Setting number of AGX threads to %i (was previously %i)."), NumThreads,
		agx::getNumThreads());

	agx::setNumThreads(NumThreads);
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

FString FAGX_Environment::GetAGXDynamicsResourcesPath()
{
	if (IsSetupEnvRun())
	{
		const TArray<FString> AgxDirEntries =
			FAGX_Environment::GetEnvironmentVariableEntries("AGX_DIR");
		if (AgxDirEntries.Num() <= 0)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("FAGX_Environment::GetAGXDynamicsResourcesPath environment variable "
					 "AGX_DIR not set when expecting setup_env to have be called. Returning empty "
					 "string."));
			return FString("");
		}

		return AgxDirEntries[0];
	}
	else
	{
		// Get and return path to AGX Dynamics resources when bundled with the plugin.
		const FString SourcePath = FAGX_Environment::GetPluginSourcePath();
		const FString AGXResourcesPath =
			FPaths::Combine(SourcePath, FString("ThirdParty"), FString("agx"));

		return AGXResourcesPath;
	}
}

bool FAGX_Environment::ActivateAgxDynamicsServiceLicense(
	int32 LicenseId, const FString& ActivationCode)
{
	using namespace AGX_Environment_helpers;
	agx::Runtime* AgxRuntime = GetAgxRuntime();
	if (AgxRuntime == nullptr)
	{
		return false; // Logging done in GetAgxRuntime.
	}
	const FString LicenseDir = GetPluginLicenseDirPath();
	if (!AGX_Environment_helpers::CreateDirectoryIfNonExistent(LicenseDir))
	{
		return false;
	}

	const FString OutputFilePath =
		FPaths::Combine(LicenseDir, FString("agx") + GetServiceLicenseFileEnding());
	return AgxRuntime->activateAgxLicense(
		LicenseId, Convert(ActivationCode), Convert(OutputFilePath));
}

TOptional<FString> FAGX_Environment::GetAGXDynamicsLicenseValue(const FString& Key) const
{
	using namespace AGX_Environment_helpers;
	agx::Runtime* AgxRuntime = GetAgxRuntime();
	if (AgxRuntime == nullptr)
	{
		return TOptional<FString>(); // Logging done in GetAgxRuntime.
	}

	const agx::String KeyAGX = Convert(Key);
	if (!AgxRuntime->hasKey(KeyAGX.c_str()))
	{
		return TOptional<FString>();
	}

	return Convert(AgxRuntime->readValue(KeyAGX.c_str()));
}

TArray<FString> FAGX_Environment::GetAGXDynamicsEnabledModules() const
{
	using namespace AGX_Environment_helpers;
	TArray<FString> Modules;
	agx::Runtime* AgxRuntime = GetAgxRuntime();
	if (AgxRuntime == nullptr)
	{
		return Modules; // Logging done in GetAgxRuntime.
	}

	for (const auto& Module : AgxRuntime->getEnabledModules())
	{
		Modules.Add(Convert(Module));
	}

	return Modules;
}

bool FAGX_Environment::EnsureAGXDynamicsLicenseValid(FString* OutStatus) const
{
	using namespace AGX_Environment_helpers;
	agx::Runtime* AgxRuntime = GetAgxRuntime();
	if (AgxRuntime == nullptr)
	{
		return false; // Logging done in GetAgxRuntime.
	}

	if (AgxRuntime->isValid())
	{
		if (OutStatus)
		{
			*OutStatus = Convert(AgxRuntime->getStatus());
		}
		return true;
	}

	// License is not valid. Attempt to unlock using a legacy license file (.lic) in the plugin's
	// license directory that might have been put there recently by the user.
	if (!TryUnlockAGXDynamicsLegacyLicense())
	{
#if !WITH_EDITOR
		// For built executables, try to find and activate runtime activation (.rtlfx).
		TryActivateEncryptedServiceLicense();
#endif
	}

	const bool LicenseValid = AgxRuntime->isValid();
	if (OutStatus)
	{
		*OutStatus = TEXT("\n  Service License: ") + Convert(AgxRuntime->getExtendedStatus());
		*OutStatus += TEXT("\n  Legacy license: ") + Convert(AgxRuntime->getStatus());
	}

	return LicenseValid;
}

bool FAGX_Environment::TryUnlockAGXDynamicsLegacyLicense() const
{
	using namespace AGX_Environment_helpers;
	agx::Runtime* AgxRuntime = GetAgxRuntime();
	if (AgxRuntime == nullptr)
	{
		return false; // Logging done in GetAgxRuntime.
	}

	const FString AgxLicensePath =
		FPaths::Combine(GetPluginLicenseDirPath(), FString("agx") + GetLegacyLicenseFileEnding());
	if (!FPaths::FileExists(AgxLicensePath))
	{
		return false;
	}

	FString License;
	FFileHelper::LoadFileToString(License, *AgxLicensePath);
	if (License.IsEmpty())
	{
		return false;
	}

	if (!AgxRuntime->unlock(Convert(License)))
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Could not unlock using legacy license '%s'. The Output Log may contain "
				 "more information."),
			*AgxLicensePath);
		return false;
	}

	UE_LOG(
		LogAGX, Log,
		TEXT("Successfully unlocked AGX Dynamics license using legacy license file located at: %s"),
		*AgxLicensePath);
	return true;
}

bool FAGX_Environment::TryActivateEncryptedServiceLicense() const
{
	using namespace AGX_Environment_helpers;
	agx::Runtime* AgxRuntime = GetAgxRuntime();
	if (AgxRuntime == nullptr)
	{
		return false; // Logging done in GetAgxRuntime.
	}

	const FString LicenseDir = GetPluginLicenseDirPath();
	const FString EncryptedServiceLicensePath =
		FPaths::Combine(LicenseDir, FString("agx") + GetEncryptedRuntimeServiceLicenseFileEnding());
	if (!FPaths::FileExists(EncryptedServiceLicensePath))
	{
		return false;
	}

	FString LicenseContent;
	FFileHelper::LoadFileToString(LicenseContent, *EncryptedServiceLicensePath);

	const FString FinalOutputPath =
		FPaths::Combine(LicenseDir, FString("agx") + GetServiceLicenseFileEnding());

	if (!AgxRuntime->activateEncryptedRuntime(Convert(LicenseContent), Convert(FinalOutputPath)))
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Could not activate encrypted service license '%s'. The Output Log may contain "
				 "more information."),
			*EncryptedServiceLicensePath);
		return false;
	}

	// The activation was successful, delete the encrypted service license that was used to perform
	// the activation.
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	PlatformFile.DeleteFile(*EncryptedServiceLicensePath);

	UE_LOG(
		LogAGX, Log,
		TEXT("Successfully activated encrypted service license using file located at: %s"),
		*EncryptedServiceLicensePath);
	return true;
}

TOptional<FString> FAGX_Environment::GenerateRuntimeActivation(
	int32 LicenseId, const FString& ActivationCode, const FString& ReferenceFilePath,
	const FString& LicenseDir) const
{
	using namespace AGX_Environment_helpers;
	agx::Runtime* AgxRuntime = GetAgxRuntime();
	if (AgxRuntime == nullptr)
	{
		return TOptional<FString>(); // Logging done in GetAgxRuntime.
	}

	if (!CreateDirectoryIfNonExistent(LicenseDir))
	{
		return TOptional<FString>();
	}

	if (!FPaths::FileExists(ReferenceFilePath))
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Error during runtime activation generation. File %s does not exists."),
			*ReferenceFilePath);
		return TOptional<FString>();
	}

	const FString ReferenceFileDirectory = FPaths::GetPath(ReferenceFilePath);

	// The ReferenceFile must be inside a directory known to agxIO::Environment.
	AGX_ENVIRONMENT()
		.getFilePath(agxIO::Environment::RESOURCE_PATH)
		.pushbackPath(Convert(ReferenceFileDirectory));

	agx::String ContentAGX = AgxRuntime->encryptRuntimeActivation(
		LicenseId, Convert(ActivationCode), Convert(ReferenceFilePath));
	const FString Content = Convert(ContentAGX);

	// Must be called to avoid crash due to different allocators used by AGX Dynamics and
	// Unreal Engine.
	agxUtil::freeContainerMemory(ContentAGX);

	// Restore the agxIO::Environment's list of RESOURCE_PATHs.
	AGX_ENVIRONMENT().getFilePath(agxIO::Environment::RESOURCE_PATH).getFilePathList().pop_back();

	if (Content.IsEmpty())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Unable to generate runtime activation. The Output Log may contain more "
				 "information."));
		return TOptional<FString>();
	}

	const FString FinalOutputPath =
		FPaths::Combine(LicenseDir, FString("agx") + GetEncryptedRuntimeServiceLicenseFileEnding());

	if (!FFileHelper::SaveStringToFile(Content, *FinalOutputPath))
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Unable to write to file %s. The Output Log may contain more "
				 "information."),
			*FinalOutputPath);
		return TOptional<FString>();
	}

	return FinalOutputPath;
}

TOptional<FString> FAGX_Environment::GenerateOfflineActivationRequest(
	int32 LicenseId, const FString& ActivationCode, const FString& OutputFile) const
{
	using namespace AGX_Environment_helpers;
	agx::Runtime* AgxRuntime = GetAgxRuntime();
	if (AgxRuntime == nullptr)
	{
		return TOptional<FString>(); // Logging done in GetAgxRuntime.
	}

	FString Content = "";
	{
		agx::String ContentAGX =
			AgxRuntime->generateOfflineActivationRequest(LicenseId, Convert(ActivationCode));
		Content = Convert(ContentAGX);

		// Must be called to avoid crash due to different allocators used by AGX Dynamics
		// and Unreal Engine.
		agxUtil::freeContainerMemory(ContentAGX);
	}

	if (Content.IsEmpty())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Unable to generate offline activation request. The Output Log may contain more "
				 "information."));
		return TOptional<FString>();
	}

	if (!FFileHelper::SaveStringToFile(Content, *OutputFile))
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Unable to write to file %s. The Output Log may contain more "
				 "information."),
			*OutputFile);
		return TOptional<FString>();
	}

	return OutputFile;
}

TOptional<FString> FAGX_Environment::ProcessOfflineActivationResponse(
	const FString& ResponseFilePath) const
{
	using namespace AGX_Environment_helpers;
	agx::Runtime* AgxRuntime = GetAgxRuntime();
	if (AgxRuntime == nullptr)
	{
		return TOptional<FString>(); // Logging done in GetAgxRuntime.
	}

	if (!FPaths::FileExists(ResponseFilePath))
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("ProcessOfflineActivationResponse failed, file '%s' does not exist."),
			*ResponseFilePath);
		return TOptional<FString>();
	}

	FString ResponseContent;
	FFileHelper::LoadFileToString(ResponseContent, *ResponseFilePath);
	if (!AgxRuntime->processOfflineActivationRequest(Convert(ResponseContent)))
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Unable to process offline activation response using '%s'. "
				 "Please ensure the file is valid."),
			*ResponseFilePath);
		return TOptional<FString>();
	}

	FString LicenseContent = "";
	{
		agx::String LicenseContentAgx = AgxRuntime->readEncryptedLicense();
		LicenseContent = Convert(LicenseContentAgx);

		// Must be called to avoid crash due to different allocators used by AGX Dynamics
		// and Unreal Engine.
		agxUtil::freeContainerMemory(LicenseContentAgx);
	}

	if (LicenseContent.IsEmpty())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Unable to read license content from the AGX Dynamics runtime. Please ensure "
				 "that the offline acivation response file is valid."));
		return TOptional<FString>();
	}

	const FString OutputFile =
		FPaths::Combine(GetPluginLicenseDirPath(), FString("agx") + GetServiceLicenseFileEnding());
	if (!FFileHelper::SaveStringToFile(LicenseContent, *OutputFile))
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Unable to write to file %s. The Output Log may contain more "
				 "information."),
			*OutputFile);
		return TOptional<FString>();
	}

	return OutputFile;
}

bool FAGX_Environment::IsLoadedLicenseOfServiceType() const
{
	// Only service licenses has this key set. The legacy license key equivalence is "License".
	return GetAGXDynamicsLicenseValue("InstallationID").IsSet();
}

bool FAGX_Environment::RefreshServiceLicense() const
{
	using namespace AGX_Environment_helpers;
	if (!IsLoadedLicenseOfServiceType())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Unable to refresh service license. The service license must be loaded."));
		return false;
	}

	if (IsSetupEnvRun())
	{
		// If setup_env is used, we have no way of knowing the license file location on disk.
		// This is because in that situation, the default AGXUnreal license directory will be
		// unknown to AGX Dynamics, and AGX Dynamics is responsible for finding any license
		// file it can, and it might be inside an installation of AGX Dynamics. So for now,
		// refreshing a service license is only possible when running without setup_env, which
		// is the "normal" use case anyway.
		UE_LOG(
			LogAGX, Error,
			TEXT("Cannot refresh service license with AGX Dynamics setup_env active."));
		return false;
	}

	agx::Runtime* AgxRuntime = GetAgxRuntime();
	if (AgxRuntime == nullptr)
	{
		return false; // Logging done in GetAgxRuntime.
	}

	const FString LicenseDir = GetPluginLicenseDirPath();
	const FString ServiceLicenseFilePath =
		FPaths::Combine(LicenseDir, FString("agx") + GetServiceLicenseFileEnding());
	if (!FPaths::FileExists(ServiceLicenseFilePath))
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Error during refresh of service license. File %s does not exists."),
			*ServiceLicenseFilePath);
		return false;
	}

	return AgxRuntime->loadLicenseFile(Convert(ServiceLicenseFilePath), true);
}

bool FAGX_Environment::DeactivateServiceLicense() const
{
	using namespace AGX_Environment_helpers;
	if (!IsLoadedLicenseOfServiceType())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Unable to deactivate service license. The service license must be loaded."));
		return false;
	}

	if (IsSetupEnvRun())
	{
		// If setup_env is used, we have no way of knowing the license file location on disk.
		// This is because in that situation, the default AGXUnreal license directory will be
		// unknown to AGX Dynamics, and AGX Dynamics is responsible for finding any license
		// file it can, and it might be inside an installation of AGX Dynamics. So for now,
		// deactivating a service license is only possible when running without setup_env, which
		// is the "normal" use case anyway.
		UE_LOG(
			LogAGX, Error,
			TEXT("Cannot deactivate service license with AGX Dynamics setup_env active."));
		return false;
	}

	agx::Runtime* AgxRuntime = GetAgxRuntime();
	if (AgxRuntime == nullptr)
	{
		return false; // Logging done in GetAgxRuntime.
	}

	const FString LicenseDir = GetPluginLicenseDirPath();
	const FString ServiceLicenseFilePath =
		FPaths::Combine(LicenseDir, FString("agx") + GetServiceLicenseFileEnding());
	if (!FPaths::FileExists(ServiceLicenseFilePath))
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Error during deactivation of service license. File %s does not exists."),
			*ServiceLicenseFilePath);
		return false;
	}

	if (!AgxRuntime->deactivateAgxLicense())
	{
		return false;
	}

	// The deactivation was successful, delete the service license from disk.
	// Note that we cannot (easily) be 100% sure the license file on disk is the license currently
	// loaded by the AGX Dynamics Runtime. We do know that the currently loaded license is of
	// service license type, and that setup_env has not been called, but still it is not 100%
	// certain. For example, the user could in theory have replaced the license file on disk since
	// the time that the editor was started, and in that case we will delete the wrong file.
	// A possible check for this would be to store the License ID of the currently loaded license,
	// load the license on disk, and then compare its License ID to the one originally loaded. That
	// method of checking has its own side-effects and it was rejected as a solution for this case,
	// so instead we assume that the license file is the correct one, which it will be in the
	// normal case.
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	PlatformFile.DeleteFile(*ServiceLicenseFilePath);
	return true;
}

#undef LOCTEXT_NAMESPACE
