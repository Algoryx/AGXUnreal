// Copyright 2026, Algoryx Simulation AB.

#include "Terrain/AGX_TerrainRVTEditorLibrary.h"

// Unreal Engine includes.
#include "Editor.h"
#include "Engine/World.h"
#include "Landscape.h"
#include "LandscapeInfo.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpression.h"
#include "Materials/MaterialInterface.h"
#include "UObject/UnrealType.h"

ALandscape* UAGX_TerrainRVTEditorLibrary::CreateExperimentLandscape(
	UMaterialInterface* Material, bool bSingleNaniteMesh)
{
	if (GEditor == nullptr || Material == nullptr)
		return nullptr;
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (World == nullptr || World->GetWorldPartition() != nullptr)
		return nullptr;
	ALandscape* Landscape = World->SpawnActor<ALandscape>();
	Landscape->SetActorLabel(TEXT("TerrainRVT_Landscape"));
	Landscape->SetActorScale3D(FVector(200, 200, 100));
	Landscape->LandscapeMaterial = Material;
	Landscape->bUseDynamicMaterialInstance = true;
	Landscape->CollisionMipLevel = 0;
	Landscape->SimpleCollisionMipLevel = 0;
	Landscape->NegativeZBoundsExtension = 1200;
	Landscape->PositiveZBoundsExtension = 1200;
	const int32 ComponentQuads = bSingleNaniteMesh ? 127 : 63;
	const int32 Quads = bSingleNaniteMesh ? 8 * ComponentQuads : 16 * ComponentQuads;
	TArray<uint16> Heights;
	Heights.SetNumUninitialized((Quads + 1) * (Quads + 1));
	for (int32 Y = 0; Y <= Quads; ++Y)
		for (int32 X = 0; X <= Quads; ++X)
		{
			// Flat central workspace; ten percent slope starts 200 m to its east.
			const double Z = FMath::Max(0.0, (X - Quads / 2) * 200.0 - 20000.0) * 0.1;
			Heights[X + Y * (Quads + 1)] =
				static_cast<uint16>(32768 + FMath::RoundToInt(Z * 128.0 / 100.0));
		}
	TMap<FGuid, TArray<uint16>> HeightData;
	HeightData.Add(FGuid(), MoveTemp(Heights));
	TMap<FGuid, TArray<FLandscapeImportLayerInfo>> Layers;
	Layers.Add(FGuid(), {});
	Landscape->Import(
		FGuid::NewGuid(), 0, 0, Quads, Quads, 1, ComponentQuads, HeightData, nullptr, Layers,
		ELandscapeImportAlphamapType::Additive, TArrayView<const FLandscapeLayer>());
	Landscape->CreateLandscapeInfo();
	Landscape->PostEditChange();
	Landscape->MarkPackageDirty();
	return Landscape;
}

void UAGX_TerrainRVTEditorLibrary::BuildNanite(ALandscape* Landscape)
{
	if (Landscape == nullptr)
		return;
	if (FBoolProperty* Property =
			FindFProperty<FBoolProperty>(Landscape->GetClass(), TEXT("bEnableNanite")))
		Property->SetPropertyValue_InContainer(Landscape, true);
	Landscape->PostEditChange();
	Landscape->UpdateNaniteRepresentation(nullptr);
	Landscape->MarkPackageDirty();
}

void UAGX_TerrainRVTEditorLibrary::ConnectNaniteDisplacement(
	UMaterial* Material, UMaterialExpression* Expression)
{
	if (Material == nullptr || Expression == nullptr)
		return;
	Material->bEnableTessellation = true;
	// The fixture uses Landscape Z scale 100. Nanite displaces before object scaling.
	Material->DisplacementScaling.Magnitude = 2000.0f / 100.0f;
	Material->DisplacementScaling.Center = 32768.0f / 65535.0f;
	Material->GetEditorOnlyData()->Displacement.Connect(0, Expression);
	Material->PostEditChange();
	Material->MarkPackageDirty();
}
