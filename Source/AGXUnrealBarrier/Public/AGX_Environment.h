// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Misc/Optional.h"

class AGXUNREALBARRIER_API FAGX_Environment
{
public:
	~FAGX_Environment();

	static FAGX_Environment& GetInstance();

	/*
	 * Returns true if the AGX Dynamics environment has a valid license. Returns false otherwise. If
	 * the AGX Dynamics environment does not have a valid license, an attempt to unlock is made
	 * searching for a legacy license file in the AGX Dynamics resources bundled with the plugin.
	 */
	bool EnsureAGXDynamicsLicenseValid(FString* OutStatus = nullptr) const;

	bool EnsureEnvironmentSetup() const;

	static FString GetPluginPath();

	static FString GetPluginBinariesPath();

	static FString GetPluginSourcePath();

	static FString GetProjectBinariesPath();

	static FString GetPluginLicenseDirPath();

	static FString GetPluginVersion();

	static FString GetPluginRevision();

	static FString FindAGXEnvironmentResourcePath(const FString& RelativePath);

	static void AddEnvironmentVariableEntry(const FString& EnvVarName, const FString& Entry);

	static void RemoveEnvironmentVariableEntry(const FString& EnvVarName, const FString& Entry);

	static TArray<FString> GetEnvironmentVariableEntries(const FString& EnvVarName);

	static void SetEnvironmentVariableEntries(
		const FString& EnvVarName, const TArray<FString>& Entries);

	void SetNumThreads(uint32 NumThreads);

	static bool IsSetupEnvRun();

	static FString GetAGXDynamicsVersion();

	static void GetAGXDynamicsVersion(
		int32& OutGeneration, int32& OutMajor, int32& OutMinor, int32& OutPatch);

	static FString GetOpenPLXVersion();

	static bool IsAGXDynamicsVersionNewerOrEqualTo(
		int32 InGeneration, int32 InMajor, int32 InMinor, int32 InPatch);

	static FString GetAGXDynamicsResourcesPath();

	bool ActivateAgxDynamicsServiceLicense(int32 LicenseId, const FString& ActivationCode);

	TOptional<FString> GetAGXDynamicsLicenseValue(const FString& Key) const;

	TArray<FString> GetAGXDynamicsEnabledModules() const;

	/**
	 * Returns(optional) path to the final written file on disk if successful.
	 */
	TOptional<FString> GenerateRuntimeActivation(
		int32 LicenseId, const FString& ActivationCode, const FString& ReferenceFilePath,
		const FString& LicenseDir) const;

	/**
	 * Returns (optional) path to the final written file on disk if successful.
	 */
	TOptional<FString> GenerateOfflineActivationRequest(
		int32 LicenseId, const FString& ActivationCode, const FString& OutputFile) const;

	/**
	 * Writes a hardware bound service license to the AGXUnreal license directory if successful.
	 * Returns optional file path to the final written service license file if successful.
	 */
	TOptional<FString> ProcessOfflineActivationResponse(const FString& ResponseFilePath) const;

	bool IsLoadedLicenseOfServiceType() const;

	/**
	 * Will attempt to refresh the service license against the license server. Returns true if the
	 * license could be refreshed, or did not need to be refreshed, false otherwise. The service
	 * license will be loaded after calling this function and it returns true.
	 */
	bool RefreshServiceLicense() const;

	/**
	 * Deactivates the service license against the license server and deletes the service license
	 * from the AGXUnreal/license directory. Returns true if deactivation was successful, false
	 * otherwise.
	 */
	bool DeactivateServiceLicense() const;

	FAGX_Environment(const FAGX_Environment&) = delete;
	FAGX_Environment operator=(const FAGX_Environment&) = delete;

private:
	FAGX_Environment();

	void Init();
	void SetupAGXDynamicsEnvironment();
	void LoadDynamicLibraries();
	bool TryUnlockAGXDynamicsLegacyLicense() const;
	bool TryActivateEncryptedServiceLicense() const;

	TArray<void*> DynamicLibraryHandles;
};
