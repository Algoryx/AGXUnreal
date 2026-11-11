// Copyright 2026, Algoryx Simulation AB.

#include "Terrain/AGX_TerrainLandscapeRendererComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_CustomVersion.h"
#include "AGX_LogCategory.h"
#include "Terrain/AGX_Terrain.h"
#include "Utilities/AGX_HeightFieldUtilities.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_RenderUtilities.h"

// Unreal Engine includes.
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/World.h"
#include "Landscape.h"
#include "UObject/ConstructorHelpers.h"

UAGX_TerrainLandscapeRendererComponent::UAGX_TerrainLandscapeRendererComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	static ConstructorHelpers::FObjectFinder<UTextureRenderTarget2D> Target(
		TEXT("/AGXUnreal/Terrain/Rendering/HeightField/RT_LandscapeDisplacementMap"));
	LandscapeDisplacementMap = Target.Object;
}

void UAGX_TerrainLandscapeRendererComponent::Serialize(FArchive& Archive)
{
	Super::Serialize(Archive);
	// Blueprint CDOs bypass actor Serialize. Persist the migration marker with their
	// new default subobject so subsequent loads preserve edited component defaults.
	Archive.UsingCustomVersion(FAGX_CustomVersion::GUID);
}

void UAGX_TerrainLandscapeRendererComponent::SetEnabled(bool bInEnabled)
{
	bEnabled = bInEnabled;
	if (!bEnabled)
		Shutdown();
	else if (HasBegunPlay())
		if (const auto* Owner = Cast<AAGX_Terrain>(GetOwner()))
			ValidateSource(*Owner);
}

void UAGX_TerrainLandscapeRendererComponent::BeginPlay()
{
	Super::BeginPlay();
	if (bEnabled)
		if (const auto* Owner = Cast<AAGX_Terrain>(GetOwner()))
			ValidateSource(*Owner);
}

bool UAGX_TerrainLandscapeRendererComponent::ValidateSource(const AAGX_Terrain& InTerrain)
{
	const ALandscape* Landscape = InTerrain.SourceLandscape;
	if (Landscape == nullptr)
		return false;
	const UWorld* World = Landscape->GetWorld();
	if ((World == nullptr || !World->IsPartitionedWorld()) &&
		!AGX_HeightFieldUtilities::IsOpenWorldLandscape(*Landscape))
		return true;
	if (!bUnsupportedSourceReported)
	{
		bUnsupportedSourceReported = true;
		const FString Message = FString::Printf(
			TEXT(
				"AGX Terrain Landscape Renderer on '%s' supports only non-partitioned Landscapes. "
				"Disable this component and use AGX Terrain Partitioned Landscape Renderer for "
				"Landscape '%s'. Terrain physics and paging are unaffected by this rendering "
				"restriction."),
			*InTerrain.GetName(), *Landscape->GetName());
		UE_LOG(LogAGX, Warning, TEXT("%s"), *Message);
		FAGX_NotificationUtilities::ShowNotification(Message, SNotificationItem::CS_None);
	}
	return false;
}

void UAGX_TerrainLandscapeRendererComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	Shutdown();
	Super::EndPlay(Reason);
}

void UAGX_TerrainLandscapeRendererComponent::Initialize(AAGX_Terrain& InTerrain)
{
	if (Terrain != nullptr || !ValidateSource(InTerrain) || !InTerrain.HasNative())
		return;
	Terrain = &InTerrain;
	NumVerticesX = Terrain->NumVerticesX;
	NumVerticesY = Terrain->NumVerticesY;
	if (NumVerticesX <= 0 || NumVerticesY <= 0 || NumVerticesX > GMaxTextureDimensions ||
		NumVerticesY > GMaxTextureDimensions ||
		static_cast<int64>(NumVerticesX) * NumVerticesY > MAX_int32)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT(
				"Legacy Landscape rendering exceeds the dense array limit. Use the partitioned "
				"renderer."));
		return;
	}
	if (!Terrain->bEnableTerrainPaging)
	{
		const FTransform Frame = Terrain->GetNativeTransform();
		const double Spacing = Terrain->GetSimulationResolution();
		const FVector Size(
			(NumVerticesX - 1) * Spacing,
			(NumVerticesY - 1) * Terrain->SourceLandscape->GetActorScale3D().Y, 0);
		const TArray<float> Source = AGX_HeightFieldUtilities::GetHeights(
			*Terrain->SourceLandscape, Frame.TransformPositionNoScale(-Size / 2.0), Size.X, Size.Y);
		if (Source.Num() != NumVerticesX * NumVerticesY)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Legacy Landscape rendering could not read its source baseline."));
			return;
		}
		OriginalHeights.SetNumUninitialized(Source.Num());
		for (int32 Y = 0; Y < NumVerticesY; ++Y)
			for (int32 X = 0; X < NumVerticesX; ++X)
				OriginalHeights[X + Y * NumVerticesX] =
					Source[X + (NumVerticesY - 1 - Y) * NumVerticesX];
		Terrain->NativeBarrier.GetHeights(CurrentHeights, false);
	}
	InitializeDisplacementMap();
	UpdateLandscapeMaterialParameters();
}

void UAGX_TerrainLandscapeRendererComponent::Update()
{
	UpdateDisplacementMap();
}

void UAGX_TerrainLandscapeRendererComponent::Shutdown()
{
	ClearDisplacementMap();
	DisplacementMapInitialized = false;
	DisplacementData.Empty();
	DisplacementMapRegions.Empty();
	CurrentHeights.Empty();
	KnownTiles.Empty();
	OriginalHeights.Empty();
	Terrain = nullptr;
	bUnsupportedSourceReported = false;
}

void UAGX_TerrainLandscapeRendererComponent::InitializeDisplacementMap()
{
	if (LandscapeDisplacementMap == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT(
				"No landscape displacement map configured for terrain '%s'. Terrain rendering "
				"will not include height updates."),
			*GetName());
		return;
	}

	if (LandscapeDisplacementMap->GetFormat() != EPixelFormat::PF_R16F)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("The displacement map pixel format for the terrain '%s' must be R16F."),
			*GetName());
		return;
	}

	if (LandscapeDisplacementMap->SizeX != NumVerticesX ||
		LandscapeDisplacementMap->SizeY != NumVerticesY)
	{
		UE_LOG(
			LogAGX, Log,
			TEXT(
				"The size of the Displacement Map render target (%dx%d) for "
				"AGX Terrain '%s' does not match the vertices in the Terrain (%dx%d). "
				"Resizing the displacement map."),
			LandscapeDisplacementMap->SizeX, LandscapeDisplacementMap->SizeY, *GetNameSafe(this),
			NumVerticesX, NumVerticesY);

		LandscapeDisplacementMap->ResizeTarget(NumVerticesX, NumVerticesY);
	}
	if (LandscapeDisplacementMap->SizeX != NumVerticesX ||
		LandscapeDisplacementMap->SizeY != NumVerticesY)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT(
				"Landscape displacement map for terrain '%s' could not be resized. "
				"Expected size is %dx%d, current size is %dx%d. There may be rendering issues."),
			*GetName(), NumVerticesX, NumVerticesY, LandscapeDisplacementMap->SizeX,
			LandscapeDisplacementMap->SizeY);
	}

	DisplacementData.SetNum(NumVerticesX * NumVerticesY);
	if (!Terrain->bEnableTerrainPaging)
		for (int32 Index = 0; Index < DisplacementData.Num(); ++Index)
			DisplacementData[Index] = FFloat16(CurrentHeights[Index] - OriginalHeights[Index]);
	DisplacementMapRegions.Add(FUpdateTextureRegion2D(0, 0, 0, 0, NumVerticesX, NumVerticesY));

	/// \todo I'm not sure why we need this. Does the texture sampler "fudge the
	/// values" when using non-linear gamma?
	LandscapeDisplacementMap->bForceLinearGamma = true;

	if (LandscapeDisplacementMap->GetResource() == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT(
				"Could not allocate resource for Landscape Displacement Map for AGX Terrain '%s'. "
				"There may be rendering issues."),
			*GetNameSafe(this));
		return;
	}

	DisplacementMapInitialized = true;
}

void UAGX_TerrainLandscapeRendererComponent::UpdateDisplacementMap()
{
	if (!DisplacementMapInitialized)
	{
		return;
	}
	if (LandscapeDisplacementMap == nullptr)
	{
		return;
	}
	if (!Terrain->HasNative())
	{
		return;
	}

	TRACE_CPUPROFILER_EVENT_SCOPE(
		TEXT("AGXUnreal:UAGX_TerrainLandscapeRendererComponent::UpdateDisplacementMap"));

	if (Terrain->bEnableTerrainPaging)
	{
		const auto Updates = Terrain->NativeTerrainPagerBarrier.GetTileHeightUpdates(KnownTiles);
		const FTransform Frame = Terrain->GetNativeTransform();
		const double Spacing = Terrain->GetSimulationResolution();
		for (const FTerrainTileHeights& Tile : Updates)
		{
			auto Baseline = Terrain->GetSourceTileSnapshot(Tile.Corner, Tile.SideVertices);
			if (!Baseline.IsValid())
			{
				KnownTiles.Remove(
					Tile.Id); // Retry complete authoritative state when source arrives.
				continue;
			}
			const FVector Corner = Frame.InverseTransformPositionNoScale(Tile.Corner);
			const int32 StartX = FMath::RoundToInt(Corner.X / Spacing) + NumVerticesX / 2;
			const int32 StartY = FMath::RoundToInt(Corner.Y / Spacing) + NumVerticesY / 2;
			auto SetHeight = [&](int32 X, int32 Y, float Height)
			{
				const int32 WorldX = StartX + X;
				const int32 WorldY = StartY + Tile.SideVertices - 1 - Y;
				if (WorldX >= 0 && WorldY >= 0 && WorldX < NumVerticesX && WorldY < NumVerticesY)
					DisplacementData[WorldX + WorldY * NumVerticesX] =
						FFloat16(Height - Baseline->Heights[X + Y * Tile.SideVertices]);
			};
			if (Tile.bFullRefresh)
				for (int32 Y = 0; Y < Tile.SideVertices; ++Y)
					for (int32 X = 0; X < Tile.SideVertices; ++X)
						SetHeight(X, Y, Tile.Heights[X + Y * Tile.SideVertices]);
			else
				for (int32 Index = 0; Index < Tile.ModifiedVertices.Num(); ++Index)
					SetHeight(
						Tile.ModifiedVertices[Index].X, Tile.ModifiedVertices[Index].Y,
						Tile.Heights[Index]);
		}
	}
	else
	{
		Terrain->NativeBarrier.GetHeights(CurrentHeights, true);
		for (const auto& Vertex : Terrain->NativeBarrier.GetModifiedVertices())
		{
			const int32 Index = std::get<0>(Vertex) + std::get<1>(Vertex) * NumVerticesX;
			DisplacementData[Index] = FFloat16(CurrentHeights[Index] - OriginalHeights[Index]);
		}
	}

	const uint32 BytesPerPixel = sizeof(FFloat16);
	uint8* PixelData = static_cast<uint8*>(FMemory::Malloc(DisplacementData.Num() * BytesPerPixel));
	FMemory::Memcpy(PixelData, DisplacementData.GetData(), DisplacementData.Num() * BytesPerPixel);
	auto* Region =
		static_cast<FUpdateTextureRegion2D*>(FMemory::Malloc(sizeof(FUpdateTextureRegion2D)));
	*Region = DisplacementMapRegions[0];
	FAGX_RenderUtilities::UpdateRenderTextureRegions(
		*LandscapeDisplacementMap, 1, Region, NumVerticesX * BytesPerPixel, BytesPerPixel,
		PixelData, true);
}

void UAGX_TerrainLandscapeRendererComponent::ClearDisplacementMap()
{
	if (!DisplacementMapInitialized)
	{
		return;
	}
	if (LandscapeDisplacementMap == nullptr)
	{
		return;
	}
	if (!Terrain->HasNative())
	{
		return;
	}
	if (DisplacementMapRegions.Num() == 0)
	{
		return;
	}

	const uint32 BytesPerPixel = sizeof(FFloat16);
	for (FFloat16& Displacement : DisplacementData)
	{
		Displacement = FFloat16();
	}

	uint8* PixelData = static_cast<uint8*>(FMemory::Malloc(DisplacementData.Num() * BytesPerPixel));
	FMemory::Memcpy(PixelData, DisplacementData.GetData(), DisplacementData.Num() * BytesPerPixel);
	auto* Region =
		static_cast<FUpdateTextureRegion2D*>(FMemory::Malloc(sizeof(FUpdateTextureRegion2D)));
	*Region = DisplacementMapRegions[0];
	FAGX_RenderUtilities::UpdateRenderTextureRegions(
		*LandscapeDisplacementMap, 1, Region, NumVerticesX * BytesPerPixel, BytesPerPixel,
		PixelData, true);
}

void UAGX_TerrainLandscapeRendererComponent::UpdateLandscapeMaterialParameters()
{
	if (!IsValid(Terrain->SourceLandscape) || GetWorld() == nullptr || !GetWorld()->IsGameWorld())
	{
		return;
	}

	// Set scalar material parameters for Landscape size and position.
	// It is the Landscape material's responsibility to declare and implement displacement map
	// sampling and passing on to World Position Offset.

	const double QuadSideSizeX = Terrain->GetSimulationResolution();
	const double QuadSideSizeY =
		Terrain->bEnableTerrainPaging ? QuadSideSizeX : Terrain->SourceLandscape->GetActorScale().Y;

	// This assumes that the Terrain and Landscape resolution (quad size) is the same.
	const double TerrainSizeX = static_cast<double>(NumVerticesX - 1) * QuadSideSizeX;
	const double TerrainSizeY = static_cast<double>(NumVerticesY - 1) * QuadSideSizeY;

	const FVector TerrainCenterGlobal = Terrain->NativeBarrier.GetPosition();

	const FVector TerrainCenterLocal =
		Terrain->SourceLandscape->GetActorTransform().InverseTransformPositionNoScale(
			TerrainCenterGlobal);
	const FVector TerrainCornerLocal =
		TerrainCenterLocal - FVector(TerrainSizeX / 2.0, TerrainSizeY / 2.0, 0.0);
	const FVector TerrainCornerGlobal =
		Terrain->SourceLandscape->GetActorTransform().TransformPositionNoScale(TerrainCornerLocal);

	const double PositionX = TerrainCornerGlobal.X;
	const double PositionY = TerrainCornerGlobal.Y;

	ALandscape& Landscape = *Terrain->SourceLandscape;
	// Parameters belong only to this ordinary source Landscape.
	Landscape.SetLandscapeMaterialScalarParameterValue(
		"TerrainSize", static_cast<float>(TerrainSizeX));
	Landscape.SetLandscapeMaterialScalarParameterValue(
		"TerrainSizeX", static_cast<float>(TerrainSizeX));
	Landscape.SetLandscapeMaterialScalarParameterValue(
		"TerrainSizeY", static_cast<float>(TerrainSizeY));
	Landscape.SetLandscapeMaterialScalarParameterValue("TerrainPositionX", PositionX);
	Landscape.SetLandscapeMaterialScalarParameterValue("TerrainPositionY", PositionY);
}
