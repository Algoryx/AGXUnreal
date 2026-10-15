// Copyright 2026, Algoryx Simulation AB.

// AGX Dynamics for Unreal includes.
#include "AGX_CustomVersion.h"
#include "AGX_Simulation.h"
#include "AgxAutomationCommon.h"
#include "Materials/AGX_ContactMaterial.h"
#include "Materials/AGX_ShapeMaterial.h"
#include "Materials/ContactMaterialBarrier.h"
#include "Materials/ShapeMaterialBarrier.h"

// Unreal Engine includes.
#include "Misc/AutomationTest.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "UObject/UObjectGlobals.h"

namespace AGX_MaterialAttenuationSpec_helpers
{
	// Write the old property names without changing reflection data or resaving fixtures.
	struct FLegacyMaterialWriter : FObjectAndNameAsStringProxyArchive
	{
		FLegacyMaterialWriter(FArchive& Inner)
			: FObjectAndNameAsStringProxyArchive(Inner, false)
		{
			SetCustomVersion(
				FAGX_CustomVersion::GUID, FAGX_CustomVersion::DampingToAttenuation - 1,
				TEXT("AGXUnrealVer"));
		}

		virtual FArchive& operator<<(FName& Name) override
		{
			FName LegacyName = Name;
			if (Name == TEXT("Attenuation"))
				LegacyName = TEXT("SpookDamping");
			else if (Name == TEXT("AttenuationStretch"))
				LegacyName = TEXT("SpookDampingStretch");
			else if (Name == TEXT("AttenuationBend"))
				LegacyName = TEXT("SpookDampingBend");
			return FNameAsStringProxyArchive::operator<<(LegacyName);
		}
	};

	TArray<uint8> Save(UObject& Object, bool LegacyNames = false)
	{
		TArray<uint8> Bytes;
		FMemoryWriter Writer(Bytes);
		if (LegacyNames)
		{
			FLegacyMaterialWriter Archive(Writer);
			Object.Serialize(Archive);
		}
		else
		{
			FObjectAndNameAsStringProxyArchive Archive(Writer, false);
			Object.Serialize(Archive);
		}
		return Bytes;
	}

	bool Load(UObject& Object, const TArray<uint8>& Bytes, int32 Version)
	{
		FMemoryReader Reader(Bytes);
		FObjectAndNameAsStringProxyArchive Archive(Reader, false);
		Archive.SetCustomVersion(FAGX_CustomVersion::GUID, Version, TEXT("AGXUnrealVer"));
		Object.Serialize(Archive);
		return !Archive.IsError();
	}
}

BEGIN_DEFINE_SPEC(
	FAGX_MaterialAttenuationSpec, "AGXUnreal.Spec.MaterialAttenuation",
	AgxAutomationCommon::DefaultTestFlags)
END_DEFINE_SPEC(FAGX_MaterialAttenuationSpec)

void FAGX_MaterialAttenuationSpec::Define()
{
	using namespace AGX_MaterialAttenuationSpec_helpers;

	It("matches native material defaults",
	   [this]()
	   {
		   FShapeMaterialBarrier NativeShape;
		   NativeShape.AllocateNative(TEXT("MaterialAttenuationDefaults"));
		   const UAGX_ShapeMaterial* Shape = GetDefault<UAGX_ShapeMaterial>();
		   TestEqual(
			   TEXT("Bulk default"), Shape->Bulk.Attenuation.Value, NativeShape.GetAttenuation());
		   TestEqual(
			   TEXT("Stretch default"), Shape->Wire.AttenuationStretch.Value,
			   NativeShape.GetAttenuationStretch());
		   TestEqual(
			   TEXT("Bend default"), Shape->Wire.AttenuationBend.Value,
			   NativeShape.GetAttenuationBend());
		   TestEqual(
			   TEXT("Surface friction compliance default"), Shape->Surface.FrictionCompliance.Value,
			   NativeShape.GetFrictionCompliance());
		   FContactMaterialBarrier NativeContact;
		   NativeContact.AllocateNative(&NativeShape, &NativeShape);
		   TestEqual(
			   TEXT("Contact default"), GetDefault<UAGX_ContactMaterial>()->Attenuation.Value,
			   NativeContact.GetAttenuation());
	   });

	It("migrates nested shape properties and contact damping only once",
	   [this]()
	   {
		   UAGX_Simulation* Simulation = GetMutableDefault<UAGX_Simulation>();
		   TGuardValue<double> TimeStepGuard(Simulation->TimeStep, 0.02);
		   UAGX_ShapeMaterial* SourceShape = NewObject<UAGX_ShapeMaterial>();
		   UAGX_ContactMaterial* SourceContact = NewObject<UAGX_ContactMaterial>();
		   // Written under legacy names, these values represent damping in seconds.
		   SourceShape->Bulk.Attenuation = 0.06;
		   SourceShape->Wire.AttenuationStretch = 0.08;
		   SourceShape->Wire.AttenuationBend = 0.1;
		   SourceContact->Attenuation = 0.12;

		   UAGX_ShapeMaterial* Shape = NewObject<UAGX_ShapeMaterial>();
		   UAGX_ContactMaterial* Contact = NewObject<UAGX_ContactMaterial>();
		   TestTrue(
			   TEXT("Read legacy shape"),
			   Load(
				   *Shape, Save(*SourceShape, true), FAGX_CustomVersion::DampingToAttenuation - 1));
		   TestTrue(
			   TEXT("Read legacy contact"), Load(
												*Contact, Save(*SourceContact, true),
												FAGX_CustomVersion::DampingToAttenuation - 1));
		   TestEqual(TEXT("Bulk attenuation"), Shape->Bulk.Attenuation.Value, 3.0);
		   TestEqual(TEXT("Stretch attenuation"), Shape->Wire.AttenuationStretch.Value, 4.0);
		   TestEqual(TEXT("Bend attenuation"), Shape->Wire.AttenuationBend.Value, 5.0);
		   TestEqual(TEXT("Contact attenuation"), Contact->Attenuation.Value, 6.0);

		   const TArray<uint8> ShapeBytes = Save(*Shape);
		   const TArray<uint8> ContactBytes = Save(*Contact);
		   Simulation->TimeStep = 0.01;
		   UAGX_ShapeMaterial* ReloadedShape = NewObject<UAGX_ShapeMaterial>();
		   UAGX_ContactMaterial* ReloadedContact = NewObject<UAGX_ContactMaterial>();
		   TestTrue(
			   TEXT("Read resaved shape"),
			   Load(*ReloadedShape, ShapeBytes, FAGX_CustomVersion::LatestVersion));
		   TestTrue(
			   TEXT("Read resaved contact"),
			   Load(*ReloadedContact, ContactBytes, FAGX_CustomVersion::LatestVersion));
		   TestTrue(TEXT("Resaved bulk unchanged"), ReloadedShape->Bulk == Shape->Bulk);
		   TestTrue(TEXT("Resaved wire unchanged"), ReloadedShape->Wire == Shape->Wire);
		   TestEqual(TEXT("Resaved contact unchanged"), ReloadedContact->Attenuation.Value, 6.0);
	   });

	It("converts omitted legacy defaults using the project timestep",
	   [this]()
	   {
		   UAGX_Simulation* Simulation = GetMutableDefault<UAGX_Simulation>();
		   TGuardValue<double> TimeStepGuard(Simulation->TimeStep, 0.01);
		   UAGX_ShapeMaterial* SourceShape = NewObject<UAGX_ShapeMaterial>();
		   UAGX_ContactMaterial* SourceContact = NewObject<UAGX_ContactMaterial>();
		   UAGX_ShapeMaterial* Shape = NewObject<UAGX_ShapeMaterial>();
		   UAGX_ContactMaterial* Contact = NewObject<UAGX_ContactMaterial>();
		   // No legacy names are written, leaving the deprecated fields at their old defaults.
		   TestTrue(
			   TEXT("Read default shape"),
			   Load(*Shape, Save(*SourceShape), FAGX_CustomVersion::DampingToAttenuation - 1));
		   TestTrue(
			   TEXT("Read default contact"),
			   Load(*Contact, Save(*SourceContact), FAGX_CustomVersion::DampingToAttenuation - 1));
		   TestEqual(TEXT("Legacy bulk default"), Shape->Bulk.Attenuation.Value, 7.5);
		   TestEqual(TEXT("Legacy stretch default"), Shape->Wire.AttenuationStretch.Value, 5.0);
		   TestEqual(TEXT("Legacy bend default"), Shape->Wire.AttenuationBend.Value, 10.0);
		   TestEqual(TEXT("Legacy contact default"), Contact->Attenuation.Value, 7.5);
	   });
}
