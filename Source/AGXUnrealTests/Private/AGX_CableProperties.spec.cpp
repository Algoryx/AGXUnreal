// Copyright 2026, Algoryx Simulation AB.

// AGX Dynamics for Unreal includes.
#include "AGX_CustomVersion.h"
#include "AGX_Simulation.h"
#include "AgxAutomationCommon.h"
#include "Cable/AGX_CableProperties.h"

// Unreal Engine includes.
#include "Misc/AutomationTest.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UnrealType.h"

namespace AGX_CablePropertiesSpec_helpers
{
	// Write real property tags with the old names and double values, without changing the
	// reflected class or relying on a binary asset that might be resaved by an editor.
	struct FLegacyCableWriter : FObjectAndNameAsStringProxyArchive
	{
		FLegacyCableWriter(FArchive& Inner)
			: FObjectAndNameAsStringProxyArchive(Inner, false)
		{
			SetCustomVersion(
				FAGX_CustomVersion::GUID, FAGX_CustomVersion::DampingToAttenuation - 1,
				TEXT("AGXUnrealVer"));
		}

		virtual FArchive& operator<<(FName& Name) override
		{
			FName LegacyName = Name;
			if (Name == TEXT("AttenuationBend"))
				LegacyName = TEXT("SpookDampingBend");
			else if (Name == TEXT("AttenuationTwist"))
				LegacyName = TEXT("SpookDampingTwist");
			else if (Name == TEXT("AttenuationStretch"))
				LegacyName = TEXT("SpookDampingStretch");
			return FNameAsStringProxyArchive::operator<<(LegacyName);
		}
	};

	void Load(UAGX_CableProperties& Properties, const TArray<uint8>& Bytes, int32 Version)
	{
		FMemoryReader Reader(Bytes);
		FObjectAndNameAsStringProxyArchive Archive(Reader, false);
		Archive.SetCustomVersion(FAGX_CustomVersion::GUID, Version, TEXT("AGXUnrealVer"));
		Properties.Serialize(Archive);
	}
}

BEGIN_DEFINE_SPEC(
	FAGX_CablePropertiesSpec, "AGXUnreal.Spec.CableProperties",
	AgxAutomationCommon::DefaultTestFlags)
END_DEFINE_SPEC(FAGX_CablePropertiesSpec)

void FAGX_CablePropertiesSpec::Define()
{
	It("loads legacy damping using the configured timestep and migrates only once",
	   [this]()
	   {
		   using namespace AGX_CablePropertiesSpec_helpers;
		   UAGX_Simulation* Simulation = GetMutableDefault<UAGX_Simulation>();
		   TGuardValue<double> TimeStepGuard(Simulation->TimeStep, 0.02);

		   UAGX_CableProperties* Source = NewObject<UAGX_CableProperties>();
		   // These values are written under the old property names, so they represent seconds.
		   Source->AttenuationBend = 0.06;
		   Source->AttenuationTwist = 0.08;
		   Source->AttenuationStretch = 0.1;
		   TArray<uint8> LegacyBytes;
		   {
			   FMemoryWriter Writer(LegacyBytes);
			   FLegacyCableWriter Archive(Writer);
			   Source->Serialize(Archive);
		   }

		   UAGX_CableProperties* Migrated = NewObject<UAGX_CableProperties>();
		   Load(*Migrated, LegacyBytes, FAGX_CustomVersion::DampingToAttenuation - 1);
		   TestEqual(TEXT("Bend attenuation"), Migrated->AttenuationBend, 3.0);
		   TestEqual(TEXT("Twist attenuation"), Migrated->AttenuationTwist, 4.0);
		   TestEqual(TEXT("Stretch attenuation"), Migrated->AttenuationStretch, 5.0);

		   FProperty* LegacyProperty = FindFProperty<FProperty>(
			   UAGX_CableProperties::StaticClass(), TEXT("SpookDampingBend"));
		   TestNotNull(TEXT("Old property name remains readable"), LegacyProperty);
		   if (LegacyProperty != nullptr)
			   TestTrue(
				   TEXT("Old property is deprecated"),
				   LegacyProperty->HasAnyPropertyFlags(CPF_Deprecated));

		   TArray<uint8> CurrentBytes;
		   {
			   FMemoryWriter Writer(CurrentBytes);
			   FObjectAndNameAsStringProxyArchive Archive(Writer, false);
			   Migrated->Serialize(Archive);
		   }
		   Simulation->TimeStep = 0.01;
		   UAGX_CableProperties* Reloaded = NewObject<UAGX_CableProperties>();
		   Load(*Reloaded, CurrentBytes, FAGX_CustomVersion::LatestVersion);
		   TestEqual(TEXT("Resaved bend attenuation"), Reloaded->AttenuationBend, 3.0);
		   TestEqual(TEXT("Resaved twist attenuation"), Reloaded->AttenuationTwist, 4.0);
		   TestEqual(TEXT("Resaved stretch attenuation"), Reloaded->AttenuationStretch, 5.0);
	   });

	It("converts legacy defaults when the archive contains no damping overrides",
	   [this]()
	   {
		   using namespace AGX_CablePropertiesSpec_helpers;
		   UAGX_Simulation* Simulation = GetMutableDefault<UAGX_Simulation>();
		   TGuardValue<double> TimeStepGuard(Simulation->TimeStep, 0.01);
		   UAGX_CableProperties* Source = NewObject<UAGX_CableProperties>();
		   TArray<uint8> Bytes;
		   {
			   // Current defaults are serialized without renaming them to legacy properties, so
			   // loading this as an old archive leaves the deprecated fields at their old defaults.
			   FMemoryWriter Writer(Bytes);
			   FObjectAndNameAsStringProxyArchive Archive(Writer, false);
			   Source->Serialize(Archive);
		   }
		   UAGX_CableProperties* Migrated = NewObject<UAGX_CableProperties>();
		   Load(*Migrated, Bytes, FAGX_CustomVersion::DampingToAttenuation - 1);
		   TestEqual(TEXT("Legacy default bend"), Migrated->AttenuationBend, 3.33);
		   TestEqual(TEXT("Legacy default twist"), Migrated->AttenuationTwist, 3.33);
		   TestEqual(TEXT("Legacy default stretch"), Migrated->AttenuationStretch, 3.33);
	   });
}
