// Copyright 2025, Algoryx Simulation AB.

#include "AGX_CustomVersion.h"

// Unreal Engine includes.
#include "Serialization/Archive.h"
#include "Serialization/CustomVersion.h"

// Important: Do not change the value of this GUID. Doing so will break any backward compatibility
// for previous AGX Dynamics for Unreal releases. There is an exception: if a backward
// compatibility "reset" is wanted, for example when moving to a new major release of the plugin,
// this GUID must be changed to a new and randomly generated GUID. When doing such a "reset", the
// enum literals in FAGX_CustomVersion::Type should be removed also. See the comment in
// AGX_CusomVersion.h for more details.
const FGuid FAGX_CustomVersion::GUID(0x9A157FFA, 0x909A4B69, 0xBE1B12A0, 0x51B6F233);

// Register the custom version with core
FCustomVersionRegistration GRegisterAGXUnrealCustomVersion(
	FAGX_CustomVersion::GUID, FAGX_CustomVersion::LatestVersion, TEXT("AGXUnrealVer"));

bool ShouldUpgradeTo(const FArchive& Archive, FAGX_CustomVersion::Type Version)
{
	return Archive.IsLoading() && Archive.CustomVer(FAGX_CustomVersion::GUID) < Version;
}
