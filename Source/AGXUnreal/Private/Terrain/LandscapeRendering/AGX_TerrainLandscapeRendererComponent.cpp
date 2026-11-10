// Copyright 2026, Algoryx Simulation AB.

#include "Terrain/LandscapeRendering/AGX_TerrainLandscapeRendererComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Terrain/AGX_Terrain.h"
#include "Utilities/AGX_HeightFieldUtilities.h"
#include "Utilities/AGX_RenderUtilities.h"
#include "Utilities/AGX_StringUtilities.h"

// Unreal Engine includes.
#include "Engine/World.h"
#include "Landscape.h"
#include "LandscapeProxy.h"
#include "LandscapeStreamingProxy.h"
#include "RenderingThread.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/UObjectIterator.h"
#include "UObject/UnrealType.h"

UAGX_TerrainLandscapeRendererComponent::UAGX_TerrainLandscapeRendererComponent()
{
	static ConstructorHelpers::FObjectFinder<UTextureRenderTarget2D> DefaultMap(TEXT(
		"TextureRenderTarget2D'/AGXUnreal/Terrain/Rendering/HeightField/"
		"RT_LandscapeDisplacementMap.RT_LandscapeDisplacementMap'"));
	LandscapeDisplacementMap = DefaultMap.Object;
}

bool UAGX_TerrainLandscapeRendererComponent::InitializeRenderer()
{
	if (AGX_HeightFieldUtilities::IsPartitionedLandscape(*Terrain->SourceLandscape))
	{
		UE_LOG(
			LogAGX, Error,
			TEXT(
				"The render-target Landscape renderer does not support a partitioned Landscape. "
				"Disable it and add an AGX Terrain Partitioned Landscape Renderer."));
		return false;
	}
	if (!FMath::IsNearlyEqual(
			Terrain->GetTerrainVertexSpacing(), Terrain->SourceLandscape->GetActorScale().X))
	{
		UE_LOG(
			LogAGX, Error,
			TEXT(
				"The render-target Landscape renderer requires Terrain resolution to match the "
				"Landscape."));
		return false;
	}
	NumVerticesX = Terrain->NumVerticesX;
	NumVerticesY = Terrain->NumVerticesY;
	if (Terrain->HasNativeTerrainPager())
		CurrentHeights.SetNumZeroed(NumVerticesX * NumVerticesY);
	else
	{
		if (!Terrain->CopyLandscapeSourceHeights(OriginalHeights))
			return false;
		CurrentHeights = OriginalHeights;
	}
	InitializeDisplacementMap();
	if (!DisplacementMapInitialized)
		return false;
	UpdateLandscapeMaterialParameters();
	if (!Terrain->HasNativeTerrainPager())
	{
		Terrain->NativeBarrier.GetHeights(CurrentHeights, false);
		for (int32 Index = 0; Index < CurrentHeights.Num(); ++Index)
			DisplacementData[Index] = FFloat16(CurrentHeights[Index] - OriginalHeights[Index]);
		Upload();
	}
	return true;
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
			LandscapeDisplacementMap->SizeX, LandscapeDisplacementMap->SizeY,
			*GetLabelSafe(Terrain.Get()), NumVerticesX, NumVerticesY);

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
		return;
	}

	DisplacementData.SetNumZeroed(NumVerticesX * NumVerticesY);
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
			*GetLabelSafe(Terrain.Get()));
		return;
	}

	DisplacementMapInitialized = true;
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

	uint8* PixelData = reinterpret_cast<uint8*>(DisplacementData.GetData());
	FAGX_RenderUtilities::UpdateRenderTextureRegions(
		*LandscapeDisplacementMap, 1, DisplacementMapRegions.GetData(),
		NumVerticesX * BytesPerPixel, BytesPerPixel, PixelData, false);
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

	const auto QuadSideSizeX = Terrain->SourceLandscape->GetActorScale().X;
	const auto QuadSideSizeY = Terrain->SourceLandscape->GetActorScale().Y;

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

	auto SetLandscapeMaterialParameters = [=](ALandscapeProxy& Proxy)
	{
		// Parameter for materials supporting only square Landscape.
		Proxy.SetLandscapeMaterialScalarParameterValue(
			"TerrainSize", static_cast<float>(TerrainSizeX));
		// Parameters for materials supporting rectangular Landscape.
		Proxy.SetLandscapeMaterialScalarParameterValue(
			"TerrainSizeX", static_cast<float>(TerrainSizeX));
		Proxy.SetLandscapeMaterialScalarParameterValue(
			"TerrainSizeY", static_cast<float>(TerrainSizeY));
		// Parameters for Landscape position.
		Proxy.SetLandscapeMaterialScalarParameterValue("TerrainPositionX", PositionX);
		Proxy.SetLandscapeMaterialScalarParameterValue("TerrainPositionY", PositionY);
	};

	SetLandscapeMaterialParameters(*Terrain->SourceLandscape);

	if (AGX_HeightFieldUtilities::IsOpenWorldLandscape(*Terrain->SourceLandscape))
	{
		// There might be a better way to get all LandscapeStreamingProxies directly from the
		// Terrain->SourceLandscape, but I have not found any. This is likely slower than any such
		// methods, but this is not extremely time critical since this function is called only once
		// on Play. If a better way if getting them is found in the future, this can be replaced.
		for (TObjectIterator<ALandscapeStreamingProxy> It; It; ++It)
		{
			if (It->GetLandscapeActor() != Terrain->SourceLandscape)
				continue;

			SetLandscapeMaterialParameters(**It);
		}
	}
}

void UAGX_TerrainLandscapeRendererComponent::UpdateRenderer(
	const TArray<FTerrainPagerHeightSample>& Samples)
{
	if (Terrain->HasNativeTerrainPager())
	{
		const FTransform Transform = Terrain->GetNativeTransform();
		const double Spacing = Terrain->GetTerrainVertexSpacing();
		for (const auto& Sample : Samples)
		{
			const FVector Local = Transform.InverseTransformPosition(Sample.Position);
			const int32 X = FMath::RoundToInt(Local.X / Spacing) + NumVerticesX / 2;
			const int32 Y = FMath::RoundToInt(Local.Y / Spacing) + NumVerticesY / 2;
			if (X < 0 || X >= NumVerticesX || Y < 0 || Y >= NumVerticesY)
				continue;
			const int32 Index = X + Y * NumVerticesX;
			float BaseHeight;
			if (Terrain->LandscapeData != nullptr)
			{
				if (!Terrain->GetLandscapeBaseHeight(
						Sample.Position.X, Sample.Position.Y, BaseHeight))
					continue;
				BaseHeight -= static_cast<float>(Transform.GetLocation().Z);
			}
			else
			{
				// The legacy height fetcher publishes baselines as each tile is requested.
				// Read the live physics-owned array rather than an initialization-time copy.
				std::lock_guard<std::mutex> Lock(Terrain->OriginalHeightsMutex);
				BaseHeight = Terrain->OriginalHeights[Index];
			}
			CurrentHeights[Index] = static_cast<float>(Local.Z);
			DisplacementData[Index] = FFloat16(CurrentHeights[Index] - BaseHeight);
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
	Upload();
}

void UAGX_TerrainLandscapeRendererComponent::Upload()
{
	const uint32 BytesPerPixel = sizeof(FFloat16);
	const SIZE_T Bytes = static_cast<SIZE_T>(DisplacementData.Num()) * BytesPerPixel;
	auto* Pixels = static_cast<uint8*>(FMemory::Malloc(Bytes));
	FMemory::Memcpy(Pixels, DisplacementData.GetData(), Bytes);
	auto* Region =
		static_cast<FUpdateTextureRegion2D*>(FMemory::Malloc(sizeof(FUpdateTextureRegion2D)));
	*Region = DisplacementMapRegions[0];
	if (!FAGX_RenderUtilities::UpdateRenderTextureRegions(
			*LandscapeDisplacementMap, 1, Region, NumVerticesX * BytesPerPixel, BytesPerPixel,
			Pixels, true))
	{
		FMemory::Free(Region);
		FMemory::Free(Pixels);
	}
}

void UAGX_TerrainLandscapeRendererComponent::ShutdownRenderer()
{
	ClearDisplacementMap();
	// Render commands reference the upload buffers; complete them before releasing those buffers.
	FlushRenderingCommands();
	OriginalHeights.Reset();
	CurrentHeights.Reset();
	DisplacementData.Reset();
	DisplacementMapRegions.Reset();
	DisplacementMapInitialized = false;
}

#if WITH_EDITOR
bool UAGX_TerrainLandscapeRendererComponent::CanEditChange(const FProperty* Property) const
{
	if (GetWorld() != nullptr && GetWorld()->IsGameWorld() && Property != nullptr &&
		Property->GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, LandscapeDisplacementMap))
		return false;
	return Super::CanEditChange(Property);
}
#endif
