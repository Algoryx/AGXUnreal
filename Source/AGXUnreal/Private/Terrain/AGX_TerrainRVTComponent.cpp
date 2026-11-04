// Copyright 2026, Algoryx Simulation AB.

#include "Terrain/AGX_TerrainRVTComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Terrain/AGX_TerrainRuntimeVirtualTexture.h"
#include "Terrain/TerrainPagerBarrier.h"

// Unreal Engine includes.
#include "Engine/World.h"
#include "EngineModule.h"
#include "GameFramework/PlayerController.h"
#include "Landscape.h"
#include "LandscapeComponent.h"
#include "LandscapeNaniteComponent.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "RendererInterface.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "RenderingThread.h"
#include "RenderTargetPool.h"
#include "RenderUtils.h"
#include "RHICommandList.h"
#include "UObject/UnrealType.h"
#include "VirtualTexturing.h"

// Standard library includes.
#include <atomic>

namespace AGX_TerrainRVT_helpers
{
	constexpr int32 BlockSide = 64;
	constexpr int32 NumLevels = 24;
	struct FBlock
	{
		float Values[BlockSide * BlockSide] = {};
	};
	using FBlocks = TMap<FIntPoint, TSharedPtr<FBlock, ESPMode::ThreadSafe>>;

	int32 FloorDivide(int32 Value, int32 Divisor)
	{
		return FMath::FloorToInt(static_cast<double>(Value) / Divisor);
	}

	FIntPoint BlockKey(FIntPoint Index)
	{
		return FIntPoint(FloorDivide(Index.X, BlockSide), FloorDivide(Index.Y, BlockSide));
	}

	int32 BlockOffset(FIntPoint Index, FIntPoint Key)
	{
		return Index.X - Key.X * BlockSide + (Index.Y - Key.Y * BlockSide) * BlockSide;
	}

	float Read(const FBlocks& Blocks, FIntPoint Index)
	{
		const FIntPoint Key = BlockKey(Index);
		const auto* Block = Blocks.Find(Key);
		return Block != nullptr ? (*Block)->Values[BlockOffset(Index, Key)] : 0.0f;
	}

	float Interpolate(const FBlocks& Blocks, double X, double Y)
	{
		const int32 IX = FMath::FloorToInt(X);
		const int32 IY = FMath::FloorToInt(Y);
		return FMath::Lerp(
			FMath::Lerp(Read(Blocks, {IX, IY}), Read(Blocks, {IX + 1, IY}), X - IX),
			FMath::Lerp(Read(Blocks, {IX, IY + 1}), Read(Blocks, {IX + 1, IY + 1}), X - IX),
			Y - IY);
	}

	struct FRenderData
	{
		FBlocks Levels[NumLevels]; // Only accessed on the render thread.
		FVector2D GridOrigin;
		FVector2D Min;
		FVector2D Size;
		double Spacing = 0.0;
		double Range = 0.0;
		std::atomic<int64> ProducedPages {0};
	};

	BEGIN_SHADER_PARAMETER_STRUCT(FUploadParameters, )
	RDG_TEXTURE_ACCESS(Texture, ERHIAccess::CopyDest)
	END_SHADER_PARAMETER_STRUCT()

	class FProducer final : public IVirtualTexture, public IVirtualTextureFinalizer
	{
	public:
		FProducer(
			TSharedRef<FRenderData, ESPMode::ThreadSafe> InData,
			const FVTProducerDescription& InDesc)
			: Data(InData)
			, Desc(InDesc)
		{
		}

		virtual bool IsPageStreamed(uint8, uint32) const override
		{
			return false;
		}
		virtual FVTRequestPageResult RequestPageData(
			FRHICommandList&, const FVirtualTextureProducerHandle&, uint8, uint8, uint64,
			EVTRequestPagePriority) override
		{
			return FVTRequestPageResult(EVTRequestPageStatus::Available);
		}

		virtual IVirtualTextureFinalizer* ProducePageData(
			FRHICommandList&, ERHIFeatureLevel::Type, EVTProducePageFlags,
			const FVirtualTextureProducerHandle&, uint8, uint8 Level, uint64 Address, uint64,
			const FVTProduceTargetLayer* Targets) override
		{
			FPage Page;
			Page.Target = Targets[0];
			const int32 Side = static_cast<int32>(Desc.TileSize + 2 * Desc.TileBorderSize);
			Page.Values.SetNumUninitialized(Side * Side);
			const uint32 PageX = FMath::ReverseMortonCode2(static_cast<uint32>(Address));
			const uint32 PageY = FMath::ReverseMortonCode2(static_cast<uint32>(Address >> 1));
			const double Width = static_cast<double>(Desc.BlockWidthInTiles * Desc.TileSize);
			const double Height = static_cast<double>(Desc.BlockHeightInTiles * Desc.TileSize);
			const double Scale = static_cast<double>(1u << Level);
			const double Footprint =
				FMath::Max(Data->Size.X / Width, Data->Size.Y / Height) * Scale;
			const int32 GridLevel = FMath::Clamp(
				FMath::FloorToInt(FMath::Log2(FMath::Max(1.0, Footprint / Data->Spacing))), 0,
				NumLevels - 1);
			const double GridScale = static_cast<double>(1u << GridLevel);
			for (int32 Y = 0; Y < Side; ++Y)
			{
				for (int32 X = 0; X < Side; ++X)
				{
					// Include borders in the same world mapping as interior texels, at every mip.
					const double U = ((PageX * Desc.TileSize + X -
									   static_cast<double>(Desc.TileBorderSize) + 0.5) *
									  Scale) /
									 Width;
					const double V = ((PageY * Desc.TileSize + Y -
									   static_cast<double>(Desc.TileBorderSize) + 0.5) *
									  Scale) /
									 Height;
					const FVector2D World =
						Data->Min +
						FVector2D(FMath::Clamp(U, 0.0, 1.0), FMath::Clamp(V, 0.0, 1.0)) *
							Data->Size;
					const FVector2D Grid =
						(World - Data->GridOrigin) / (Data->Spacing * GridScale) -
						FVector2D(0.5 - 0.5 / GridScale);
					const float Delta = Interpolate(Data->Levels[GridLevel], Grid.X, Grid.Y);
					const double Encoded = 32768.0 + Delta / (2.0 * Data->Range) * 65535.0;
					Page.Values[X + Y * Side] =
						static_cast<uint16>(FMath::Clamp(FMath::RoundToInt(Encoded), 0, 65535));
				}
			}
			Pages.Add(MoveTemp(Page));
			++Data->ProducedPages;
			return this;
		}

		virtual void Finalize(FRDGBuilder& GraphBuilder) override
		{
			for (FPage& Page : Pages)
			{
				FRDGTextureRef Target =
					GraphBuilder.RegisterExternalTexture(Page.Target.PooledRenderTarget);
				auto* Parameters = GraphBuilder.AllocParameters<FUploadParameters>();
				Parameters->Texture = Target;
				const int32 Side = static_cast<int32>(Desc.TileSize + 2 * Desc.TileBorderSize);
				const FUpdateTextureRegion2D Region(
					Page.Target.pPageLocation.X * Side, Page.Target.pPageLocation.Y * Side, 0, 0,
					Side, Side);
				// RDG owns the upload data until the pass executes. UpdateTexture2D copies it into
				// the RHI command's upload allocation before returning.
				uint16* Values = GraphBuilder.AllocPODArray<uint16>(Page.Values.Num());
				FMemory::Memcpy(Values, Page.Values.GetData(), Page.Values.Num() * sizeof(uint16));
				GraphBuilder.AddPass(
					RDG_EVENT_NAME("AGX Terrain RVT Upload"), Parameters, ERDGPassFlags::Copy,
					[Target, Region, Side, Values](FRHICommandList& RHICmdList)
					{
						RHICmdList.UpdateTexture2D(
							Target->GetRHI(), 0, Region, Side * sizeof(uint16),
							reinterpret_cast<const uint8*>(Values));
					});
			}
			Pages.Reset();
		}

	private:
		struct FPage
		{
			FVTProduceTargetLayer Target;
			TArray<uint16> Values;
		};
		TSharedRef<FRenderData, ESPMode::ThreadSafe> Data;
		FVTProducerDescription Desc;
		TArray<FPage> Pages;
	};
}

struct FAGX_TerrainRVTState
{
	using FBlocks = AGX_TerrainRVT_helpers::FBlocks;
	FVector Min;
	FVector Size;
	FVector Origin;
	FVector2D FetchMin;
	FVector2D FetchMax;
	FVector2D LandscapeSpacing;
	FIntPoint BaselineSize;
	TArray<float> Baseline; // Immutable coarse source. Safe for the native cache worker to read.
	double Spacing = 0.0;
	FBlocks Levels[AGX_TerrainRVT_helpers::NumLevels];
	TSharedRef<AGX_TerrainRVT_helpers::FRenderData, ESPMode::ThreadSafe> RenderData =
		MakeShared<AGX_TerrainRVT_helpers::FRenderData, ESPMode::ThreadSafe>();
	TWeakObjectPtr<ALandscape> Landscape;
	bool bReportedClipping = false;

	float BaseHeight(double X, double Y) const
	{
		const double GX = FMath::Clamp(
			(X - Min.X) / LandscapeSpacing.X, 0.0, static_cast<double>(BaselineSize.X - 1));
		const double GY = FMath::Clamp(
			(Y - Min.Y) / LandscapeSpacing.Y, 0.0, static_cast<double>(BaselineSize.Y - 1));
		const int32 IX = FMath::Min(FMath::FloorToInt(GX), BaselineSize.X - 2);
		const int32 IY = FMath::Min(FMath::FloorToInt(GY), BaselineSize.Y - 2);
		const double FX = GX - IX;
		const double FY = GY - IY;
		const float H00 = Baseline[IX + IY * BaselineSize.X];
		const float H10 = Baseline[IX + 1 + IY * BaselineSize.X];
		const float H01 = Baseline[IX + (IY + 1) * BaselineSize.X];
		const float H11 = Baseline[IX + 1 + (IY + 1) * BaselineSize.X];
		// Landscape/Chaos quads use a diagonal, so retain piecewise planar interpolation.
		return FX > FY ? H00 + FX * (H10 - H00) + FY * (H11 - H10)
					   : H00 + FY * (H01 - H00) + FX * (H11 - H01);
	}
};

UAGX_TerrainRuntimeVirtualTexture::UAGX_TerrainRuntimeVirtualTexture(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Configure(5);
}

void UAGX_TerrainRuntimeVirtualTexture::Configure(int32 TileCountLog2)
{
	TileCount = TileCountLog2;
	TileSize = 2; // 256 interior texels.
	TileBorderSize = 2; // Four border texels.
	MaterialType = ERuntimeVirtualTextureMaterialType::Displacement;
	bCompressTextures = false;
	bSinglePhysicalSpace = true;
	bPrivateSpace = true;
	bAdaptive = false;
	bContinuousUpdate = false;
}

UAGX_TerrainRVTComponent::UAGX_TerrainRVTComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

UAGX_TerrainRVTComponent::~UAGX_TerrainRVTComponent() = default;

#if WITH_EDITOR
bool UAGX_TerrainRVTComponent::CanEditChange(const FProperty* Property) const
{
	if (GetWorld() != nullptr && GetWorld()->IsGameWorld() && Property != nullptr &&
		(Property->GetFName() == GET_MEMBER_NAME_CHECKED(UAGX_TerrainRVTComponent, bEnabled) ||
		 Property->GetFName() ==
			 GET_MEMBER_NAME_CHECKED(UAGX_TerrainRVTComponent, TerrainVertexSpacing) ||
		 Property->GetFName() ==
			 GET_MEMBER_NAME_CHECKED(UAGX_TerrainRVTComponent, DisplacementRange)))
		return false;
	return Super::CanEditChange(Property);
}
#endif

bool UAGX_TerrainRVTComponent::Initialize(ALandscape& Landscape)
{
	using namespace AGX_TerrainRVT_helpers;
	check(IsInGameThread());
	if (!UseVirtualTexturing(GMaxRHIShaderPlatform) || !Landscape.bUseDynamicMaterialInstance)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT(
				"Terrain RVT requires project Virtual Texture Support and Use Dynamic Material "
				"Instance on the Landscape before play."));
		return false;
	}
	if (TerrainVertexSpacing < 1.0 || !FMath::IsFinite(TerrainVertexSpacing) ||
		DisplacementRange <= 0.0 || !FMath::IsFinite(DisplacementRange) ||
		!Landscape.GetActorRotation().IsNearlyZero() || Landscape.GetActorScale().X <= 0.0 ||
		Landscape.GetActorScale().Z <= 0.0 ||
		!FMath::IsNearlyEqual(Landscape.GetActorScale().X, Landscape.GetActorScale().Y) ||
		Landscape.LandscapeComponents.IsEmpty())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT(
				"Terrain RVT requires positive finite spacing/range and a loaded, unrotated "
				"Landscape with square cells."));
		return false;
	}
	if (Landscape.IsNaniteEnabled() && Landscape.LandscapeMaterial != nullptr)
	{
		const double ExpectedMagnitude = 2.0 * DisplacementRange / Landscape.GetActorScale().Z;
		const double Magnitude = Landscape.LandscapeMaterial->GetDisplacementScaling().Magnitude;
		if (!FMath::IsNearlyEqual(Magnitude, ExpectedMagnitude, 0.001))
		{
			UE_LOG(
				LogAGX, Error,
				TEXT(
					"Terrain RVT Nanite material magnitude is %.3f, expected %.3f (2 * "
					"Displacement Range / Landscape Z scale)."),
				Magnitude, ExpectedMagnitude);
			return false;
		}
	}
	State = std::make_shared<FAGX_TerrainRVTState>();
	State->Landscape = &Landscape;
	State->Spacing = TerrainVertexSpacing;
	State->LandscapeSpacing = FVector2D(Landscape.GetActorScale());
	FIntPoint Min(MAX_int32, MAX_int32), Max(MIN_int32, MIN_int32);
	for (const ULandscapeComponent* Component : Landscape.LandscapeComponents)
	{
		Min.X = FMath::Min(Min.X, Component->SectionBaseX);
		Min.Y = FMath::Min(Min.Y, Component->SectionBaseY);
		Max.X = FMath::Max(Max.X, Component->SectionBaseX + Component->ComponentSizeQuads);
		Max.Y = FMath::Max(Max.Y, Component->SectionBaseY + Component->ComponentSizeQuads);
	}
	State->Min = Landscape.GetTransform().TransformPosition(FVector(Min.X, Min.Y, 0));
	State->Size = FVector(
		(Max.X - Min.X) * State->LandscapeSpacing.X, (Max.Y - Min.Y) * State->LandscapeSpacing.Y,
		1.0);
	State->BaselineSize = Max - Min + FIntPoint(1, 1);
	State->Baseline.SetNumUninitialized(State->BaselineSize.X * State->BaselineSize.Y);
	for (int32 Y = 0; Y < State->BaselineSize.Y; ++Y)
	{
		for (int32 X = 0; X < State->BaselineSize.X; ++X)
		{
			FVector Position =
				State->Min +
				FVector(X * State->LandscapeSpacing.X, Y * State->LandscapeSpacing.Y, 0);
			// At the far edge the collision component map otherwise selects a nonexistent neighbor.
			Position.X = FMath::Clamp(
				Position.X, State->Min.X + 0.001, State->Min.X + State->Size.X - 0.001);
			Position.Y = FMath::Clamp(
				Position.Y, State->Min.Y + 0.001, State->Min.Y + State->Size.Y - 0.001);
			const auto Height = Landscape.GetHeightAtLocation(Position);
			if (!Height.IsSet())
			{
				UE_LOG(
					LogAGX, Error,
					TEXT(
						"Terrain RVT baseline is incomplete at %s. Collision must be loaded before "
						"Terrain initialization."),
					*Position.ToString());
				State.reset();
				return false;
			}
			State->Baseline[X + Y * State->BaselineSize.X] = *Height;
		}
	}
	const double DesiredSpacing = FMath::Min(TerrainVertexSpacing, State->LandscapeSpacing.X);
	const double RequiredResolution = FMath::Max(State->Size.X, State->Size.Y) / DesiredSpacing;
	int32 TileLog2 =
		FMath::Max(0, FMath::CeilToInt(FMath::Log2(FMath::Max(1.0, RequiredResolution / 256.0))));
	if (TileLog2 > 12)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT(
				"Terrain RVT requested resolution exceeds ordinary page-table limits. Adaptive "
				"page tables are not implemented in this experiment."));
		State.reset();
		return false;
	}
	Texture = NewObject<UAGX_TerrainRuntimeVirtualTexture>(this, NAME_None, RF_Transient);
	Texture->Configure(TileLog2);
	FTransform Volume(FQuat::Identity, State->Min, State->Size);
	FVTProducerDescription Desc;
	Texture->GetProducerDescription(Desc, URuntimeVirtualTexture::FInitSettings(), Volume);
	// Aspect-ratio rounding and device-profile biases can lower density on either axis.
	while (State->Size.X / (Desc.BlockWidthInTiles * Desc.TileSize) > DesiredSpacing ||
		   State->Size.Y / (Desc.BlockHeightInTiles * Desc.TileSize) > DesiredSpacing)
	{
		if (++TileLog2 > 12)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT(
					"Terrain RVT cannot meet the requested sample density with this device profile "
					"and page table."));
			Texture = nullptr;
			State.reset();
			return false;
		}
		Texture->Configure(TileLog2);
		Texture->GetProducerDescription(Desc, URuntimeVirtualTexture::FInitSettings(), Volume);
	}
	Desc.LayerFallbackColor[0] = FLinearColor(32768.0f / 65535.0f, 0, 0, 0);
	State->RenderData->Min = FVector2D(State->Min);
	State->RenderData->Size = FVector2D(State->Size);
	State->RenderData->Spacing = TerrainVertexSpacing;
	State->RenderData->Range = DisplacementRange;
	Texture->Initialize(
		new FProducer(State->RenderData, Desc), Desc, Volume,
		FBox(State->Min, State->Min + State->Size));
	VirtualResolution = static_cast<int32>(
		FMath::Max(Desc.BlockWidthInTiles, Desc.BlockHeightInTiles) * Desc.TileSize);
	// Runtime MID creation and texture binding; no Landscape heightmap edits or editor APIs.
	for (ULandscapeComponent* Component : Landscape.LandscapeComponents)
	{
		if (Component->MaterialInstancesDynamic.IsEmpty())
		{
			for (UMaterialInstanceConstant* Parent : Component->MaterialInstances)
				Component->MaterialInstancesDynamic.Add(
					UMaterialInstanceDynamic::Create(Parent, Component));
			Component->MarkRenderStateDirty();
		}
		for (UMaterialInstanceDynamic* Material : Component->MaterialInstancesDynamic)
		{
			if (Material == nullptr)
				continue;
			Material->SetRuntimeVirtualTextureParameterValue(TEXT("AGX_Deformation"), Texture);
			Material->SetScalarParameterValue(TEXT("AGX_DisplacementRange"), DisplacementRange);
			Material->SetScalarParameterValue(TEXT("AGX_RVTReady"), 1.0f);
		}
	}
	// Nanite's static mesh can retain shared editor/cooked material references. Override the
	// component slots with this world's source MIDs; never mutate the shared static mesh asset.
	TArray<ULandscapeNaniteComponent*> NaniteComponents;
	Landscape.GetComponents(NaniteComponents);
	for (auto* NaniteComponent : NaniteComponents)
	{
		const auto& Sources = NaniteComponent->GetSourceLandscapeComponents();
		for (int32 Index = 0; Index < Sources.Num(); ++Index)
		{
			ULandscapeComponent* Source = Sources[Index];
			if (Source != nullptr && Source->MaterialInstancesDynamic.IsValidIndex(0))
				NaniteComponent->SetMaterial(Index, Source->MaterialInstancesDynamic[0]);
		}
	}
	UE_LOG(
		LogAGX, Display,
		TEXT(
			"Terrain RVT initialized: %dx%d virtual texels, native spacing %.3f cm, baseline "
			"%dx%d, G16 range +/-%.1f cm."),
		VirtualResolution, VirtualResolution, TerrainVertexSpacing, State->BaselineSize.X,
		State->BaselineSize.Y, DisplacementRange);
	return true;
}

void UAGX_TerrainRVTComponent::SetTerrainBounds(const FVector& Origin, const FVector& HalfExtent)
{
	check(State != nullptr);
	State->Origin = Origin;
	State->FetchMin = FVector2D(
		FMath::Max(State->Min.X, Origin.X - HalfExtent.X),
		FMath::Max(State->Min.Y, Origin.Y - HalfExtent.Y));
	State->FetchMax = FVector2D(
		FMath::Min(State->Min.X + State->Size.X, Origin.X + HalfExtent.X),
		FMath::Min(State->Min.Y + State->Size.Y, Origin.Y + HalfExtent.Y));
	// Before the first step/texture request. Render-thread data is otherwise updated by commands.
	auto Data = State->RenderData;
	ENQUEUE_RENDER_COMMAND(AGXTerrainRVTOrigin)([Data, Origin](FRHICommandListImmediate&)
												{ Data->GridOrigin = FVector2D(Origin); });
}

bool UAGX_TerrainRVTComponent::FetchHeights(
	const FVector& Start, int32 VertsX, int32 VertsY, TArray<float>& Heights) const
{
	// Called on the AGX worker. Only immutable numeric data is accessed here.
	if (State == nullptr || Start.X < State->FetchMin.X - 0.01 ||
		Start.Y < State->FetchMin.Y - 0.01 ||
		Start.X + (VertsX - 1) * State->Spacing > State->FetchMax.X + 0.01 ||
		Start.Y + (VertsY - 1) * State->Spacing > State->FetchMax.Y + 0.01)
		return false;
	Heights.Reserve(VertsX * VertsY);
	for (int32 Y = VertsY - 1; Y >= 0; --Y)
		for (int32 X = 0; X < VertsX; ++X)
			Heights.Add(
				State->BaseHeight(Start.X + X * State->Spacing, Start.Y + Y * State->Spacing) -
				State->Origin.Z);
	return true;
}

double UAGX_TerrainRVTComponent::GetDisplacementAtLocation(FVector Position) const
{
	if (State == nullptr)
		return 0.0;
	const FVector Grid = (Position - State->Origin) / State->Spacing;
	return AGX_TerrainRVT_helpers::Interpolate(State->Levels[0], Grid.X, Grid.Y);
}

UAGX_TerrainRuntimeVirtualTexture* UAGX_TerrainRVTComponent::GetRuntimeVirtualTexture() const
{
	return Texture;
}

void UAGX_TerrainRVTComponent::Update(FTerrainPagerBarrier& Pager)
{
	using namespace AGX_TerrainRVT_helpers;
	if (State == nullptr || Texture == nullptr)
		return;
	const FIntVector Statistics = Pager.GetHeightCaptureStatistics();
	TileLoads = Statistics.X;
	TileUnloads = Statistics.Y;
	ActiveTiles = Statistics.Z;
	TSet<FIntPoint> Dirty[NumLevels];
	FBox2D Bounds(ForceInit);
	for (const auto& Sample : Pager.ConsumeHeightChanges())
	{
		const FVector Grid = (Sample.Position - State->Origin) / State->Spacing;
		FIntPoint Index(FMath::RoundToInt(Grid.X), FMath::RoundToInt(Grid.Y));
		float Delta = Sample.Position.Z - State->BaseHeight(Sample.Position.X, Sample.Position.Y);
		if (FMath::Abs(Delta) < 0.005f)
			Delta = 0.0f;
		if (FMath::IsNearlyEqual(Read(State->Levels[0], Index), Delta, 0.0001f))
			continue;
		++ChangedSamples;
		Bounds += FVector2D(Sample.Position);
		if (FMath::Abs(Delta) > DisplacementRange && !State->bReportedClipping)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT(
					"Terrain RVT displacement exceeded its +/-%.1f cm encoding range. CPU values "
					"are retained but rendering is clipped."),
				DisplacementRange);
			State->bReportedClipping = true;
		}
		for (int32 Level = 0; Level < NumLevels; ++Level)
		{
			const FIntPoint Key = BlockKey(Index);
			auto& Block = State->Levels[Level].FindOrAdd(Key);
			if (!Block.IsValid())
				Block = MakeShared<FBlock, ESPMode::ThreadSafe>();
			Block->Values[BlockOffset(Index, Key)] = Delta;
			Dirty[Level].Add(Key);
			Index = FIntPoint(FloorDivide(Index.X, 2), FloorDivide(Index.Y, 2));
			// Sparse box-filtered mip hierarchy preserves contributions at distant mips.
			Delta = 0.25f * (Read(State->Levels[Level], Index * 2) +
							 Read(State->Levels[Level], Index * 2 + FIntPoint(1, 0)) +
							 Read(State->Levels[Level], Index * 2 + FIntPoint(0, 1)) +
							 Read(State->Levels[Level], Index * 2 + FIntPoint(1, 1)));
		}
	}
	struct FChangedBlock
	{
		int32 Level;
		FIntPoint Key;
		TSharedPtr<FBlock, ESPMode::ThreadSafe> Block;
	};
	TArray<FChangedBlock> Updates;
	for (int32 Level = 0; Level < NumLevels; ++Level)
		for (FIntPoint Key : Dirty[Level])
		{
			auto Copy = MakeShared<FBlock, ESPMode::ThreadSafe>();
			*Copy = *State->Levels[Level][Key];
			Updates.Add({Level, Key, Copy});
		}
	ResidentCPUBlocks = 0;
	for (const auto& Level : State->Levels)
		ResidentCPUBlocks += Level.Num();
	ProducedPages = State->RenderData->ProducedPages.load();
	FVector Camera = State->Origin;
	if (const auto* Player = GetWorld()->GetFirstPlayerController())
	{
		FRotator Rotation;
		Player->GetPlayerViewPoint(Camera, Rotation);
	}
	struct FRequest
	{
		FVector2D UV0;
		FVector2D UV1;
		int32 Mip;
	};
	TArray<FRequest> Requests;
	// WPO generates no VT feedback. Explicitly request retained deformations, with distance
	// based mips; this also restores visible deformations whose native tiles are no longer active.
	for (const auto& Entry : State->Levels[0])
	{
		const FVector2D WorldMin = FVector2D(State->Origin) + FVector2D(Entry.Key.X, Entry.Key.Y) *
																  (BlockSide * State->Spacing);
		const FVector2D WorldMax = WorldMin + FVector2D(BlockSide * State->Spacing);
		const double Distance = FVector2D::Distance(FVector2D(Camera), (WorldMin + WorldMax) * 0.5);
		const int32 Mip =
			FMath::Max(0, FMath::FloorToInt(FMath::Log2(FMath::Max(1.0, Distance / 15000.0))));
		Requests.Add(
			{(WorldMin - FVector2D(State->Min)) / FVector2D(State->Size),
			 (WorldMax - FVector2D(State->Min)) / FVector2D(State->Size), Mip});
	}
	auto Data = State->RenderData;
	auto* RuntimeTexture = Texture.Get();
	const double Margin = State->Spacing * BlockSide;
	FVector2f UV0(0, 0), UV1(1, 1);
	if (Bounds.bIsValid)
	{
		UV0 = FVector2f(
			(Bounds.Min - FVector2D(Margin) - FVector2D(State->Min)) / FVector2D(State->Size));
		UV1 = FVector2f(
			(Bounds.Max + FVector2D(Margin) - FVector2D(State->Min)) / FVector2D(State->Size));
	}
	ENQUEUE_RENDER_COMMAND(AGXTerrainRVTUpdate)(
		[Data, RuntimeTexture, Updates = MoveTemp(Updates), Requests = MoveTemp(Requests), UV0,
		 UV1](FRHICommandListImmediate&) mutable
		{
			for (const auto& Update : Updates)
				Data->Levels[Update.Level].Add(Update.Key, Update.Block);
			if (auto* Allocation = RuntimeTexture->GetAllocatedVirtualTexture())
			{
				if (!Updates.IsEmpty())
					GetRendererModule().FlushVirtualTextureCache(Allocation, UV0, UV1);
				for (const auto& Request : Requests)
					GetRendererModule().RequestVirtualTextureTiles(
						Allocation, FVector2D(1024, 1024), FVector2D::ZeroVector,
						FVector2D(1024, 1024), Request.UV0, Request.UV1, Request.Mip);
			}
		});
}

void UAGX_TerrainRVTComponent::Shutdown()
{
	if (State != nullptr && State->Landscape.IsValid())
	{
		for (ULandscapeComponent* Component : State->Landscape->LandscapeComponents)
			for (UMaterialInstanceDynamic* Material : Component->MaterialInstancesDynamic)
				if (Material != nullptr)
				{
					Material->SetScalarParameterValue(TEXT("AGX_RVTReady"), 0.0f);
					Material->SetRuntimeVirtualTextureParameterValue(
						TEXT("AGX_Deformation"), nullptr);
				}
	}
	if (Texture != nullptr)
		Texture->Release();
	// All commands capturing the texture UObject must finish before its reference is dropped.
	FlushRenderingCommands();
	Texture = nullptr;
	State.reset();
}
