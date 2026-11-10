// Copyright 2026, Algoryx Simulation AB.

// AGX Dynamics for Unreal includes.
#include "AGX_CustomVersion.h"
#include "AgxAutomationCommon.h"
#include "Terrain/AGX_Terrain.h"
#include "Terrain/LandscapeRendering/AGX_TerrainLandscapeRendererComponent.h"
#include "Terrain/LandscapeRendering/AGX_TerrainPartitionedLandscapeRendererComponent.h"

// Unreal Engine includes.
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "UObject/UnrealType.h"

namespace AGX_TerrainLandscapeRendererSpec_helpers
{
	// Write the released actor property layout, including deprecated fields. Exclude the
	// newly introduced renderer pointer so it cannot alias the source actor's component.
	class FLegacyRenderingArchive : public FObjectAndNameAsStringProxyArchive
	{
	public:
		explicit FLegacyRenderingArchive(FArchive& Inner, bool bReleasedLayout = true)
			: FObjectAndNameAsStringProxyArchive(Inner, false)
		{
			if (bReleasedLayout)
				SetPortFlags(PPF_UseDeprecatedProperties);
			SetCustomVersion(
				FAGX_CustomVersion::GUID,
				bReleasedLayout ? FAGX_CustomVersion::LandscapeRenderingByRenderingComponents - 1
								: FAGX_CustomVersion::LatestVersion,
				TEXT("AGXUnrealVer"));
		}

		virtual bool ShouldSkipProperty(const FProperty* Property) const override
		{
			const FName Name = Property->GetFName();
			return Name != TEXT("bEnableDisplacementRendering") &&
				   Name != TEXT("LandscapeDisplacementMap") &&
				   Name != TEXT("bEnableActiveZoneRendering");
		}
	};
}

BEGIN_DEFINE_SPEC(
	FAGX_TerrainLandscapeRendererSpec, "AGXUnreal.Spec.TerrainLandscapeRenderer",
	AgxAutomationCommon::DefaultTestFlags)
END_DEFINE_SPEC(FAGX_TerrainLandscapeRendererSpec)

void FAGX_TerrainLandscapeRendererSpec::Define()
{
	using namespace AGX_TerrainLandscapeRendererSpec_helpers;
	Describe(
		"Released Terrain rendering properties",
		[this]()
		{
			for (int32 Case = 0; Case < 3; ++Case)
			{
				const FString Name =
					Case == 0 ? TEXT("Preserves omitted defaults")
							  : (Case == 1 ? TEXT("Migrates explicit disabled/null settings")
										   : TEXT("Migrates a custom displacement map"));
				It(Name,
				   [this, Case]()
				   {
					   UWorld* World = UWorld::CreateWorld(EWorldType::Editor, false);
					   AAGX_Terrain* Source = World->SpawnActor<AAGX_Terrain>();
					   AAGX_Terrain* Destination = World->SpawnActor<AAGX_Terrain>();
					   UTextureRenderTarget2D* DefaultMap =
						   Source->DefaultLandscapeRenderer->LandscapeDisplacementMap;
					   UTextureRenderTarget2D* ExpectedMap =
						   Case == 0 ? DefaultMap
									 : (Case == 1 ? nullptr : NewObject<UTextureRenderTarget2D>());
					   const bool ExplicitValues = Case != 0;
					   if (ExplicitValues)
					   {
						   FindFProperty<FBoolProperty>(
							   AAGX_Terrain::StaticClass(), TEXT("bEnableDisplacementRendering"))
							   ->SetPropertyValue_InContainer(Source, false);
						   FindFProperty<FObjectPropertyBase>(
							   AAGX_Terrain::StaticClass(), TEXT("LandscapeDisplacementMap"))
							   ->SetObjectPropertyValue_InContainer(Source, ExpectedMap);
						   FindFProperty<FBoolProperty>(
							   AAGX_Terrain::StaticClass(), TEXT("bEnableActiveZoneRendering"))
							   ->SetPropertyValue_InContainer(Source, true);
					   }
					   TArray<uint8> Bytes;
					   FMemoryWriter MemoryWriter(Bytes, true);
					   FLegacyRenderingArchive Writer(MemoryWriter);
					   static_cast<UObject*>(Source)->Serialize(Writer);
					   FMemoryReader MemoryReader(Bytes, true);
					   FLegacyRenderingArchive Reader(MemoryReader);
					   static_cast<UObject*>(Destination)->Serialize(Reader);
					   TestNotEqual(
						   TEXT("Renderer belongs to the destination"),
						   Destination->DefaultLandscapeRenderer, Source->DefaultLandscapeRenderer);
					   TestEqual(
						   TEXT("Enable setting"), Destination->DefaultLandscapeRenderer->bEnabled,
						   !ExplicitValues);
					   TestEqual(
						   TEXT("Displacement map"),
						   Destination->DefaultLandscapeRenderer->LandscapeDisplacementMap,
						   ExpectedMap);
					   TestEqual(
						   TEXT("Active-zone setting"),
						   Destination->DefaultLandscapeRenderer->bEnableActiveZoneRendering,
						   ExplicitValues);
					   World->DestroyWorld(false);
				   });
			}
		});

	It("Keeps component settings when reading a resaved Terrain without migrating again",
	   [this]()
	   {
		   UWorld* World = UWorld::CreateWorld(EWorldType::Editor, false);
		   AAGX_Terrain* Source = World->SpawnActor<AAGX_Terrain>();
		   AAGX_Terrain* Destination = World->SpawnActor<AAGX_Terrain>();
		   Source->DefaultLandscapeRenderer->bEnabled = false;
		   Source->DefaultLandscapeRenderer->LandscapeDisplacementMap = nullptr;
		   TArray<uint8> Bytes;
		   FMemoryWriter MemoryWriter(Bytes, true);
		   FObjectAndNameAsStringProxyArchive ComponentWriter(MemoryWriter, false);
		   Source->DefaultLandscapeRenderer->Serialize(ComponentWriter);
		   FLegacyRenderingArchive ActorWriter(MemoryWriter, false);
		   static_cast<UObject*>(Source)->Serialize(ActorWriter);
		   FMemoryReader MemoryReader(Bytes, true);
		   FObjectAndNameAsStringProxyArchive ComponentReader(MemoryReader, false);
		   ComponentReader.SetCustomVersions(ComponentWriter.GetCustomVersions());
		   Destination->DefaultLandscapeRenderer->Serialize(ComponentReader);
		   FLegacyRenderingArchive ActorReader(MemoryReader, false);
		   static_cast<UObject*>(Destination)->Serialize(ActorReader);
		   TestFalse(
			   TEXT("Saved disabled setting"), Destination->DefaultLandscapeRenderer->bEnabled);
		   TestNull(
			   TEXT("Saved null map"),
			   Destination->DefaultLandscapeRenderer->LandscapeDisplacementMap);
		   World->DestroyWorld(false);
	   });

	It("Rejects simultaneous enabled renderers and selects the user's replacement",
	   [this]()
	   {
		   UWorld* World = UWorld::CreateWorld(EWorldType::Editor, false);
		   AAGX_Terrain* Terrain = World->SpawnActor<AAGX_Terrain>();
		   TestEqual(
			   TEXT("Default renderer"), Terrain->GetLandscapeRenderer(),
			   static_cast<UAGX_TerrainLandscapeRendererBaseComponent*>(
				   Terrain->DefaultLandscapeRenderer));
		   auto* RVT = NewObject<UAGX_TerrainPartitionedLandscapeRendererComponent>(Terrain);
		   Terrain->AddInstanceComponent(RVT);
		   TestNull(TEXT("Conflicting renderers"), Terrain->GetLandscapeRenderer());
		   Terrain->DefaultLandscapeRenderer->SetEnabled(false);
		   TestEqual(
			   TEXT("Selected replacement"), Terrain->GetLandscapeRenderer(),
			   static_cast<UAGX_TerrainLandscapeRendererBaseComponent*>(RVT));
		   RVT->SetEnabled(false);
		   TestNull(TEXT("No rendering requested"), Terrain->GetLandscapeRenderer());
		   World->DestroyWorld(false);
	   });
}
