// Copyright 2026, Algoryx Simulation AB.

#include "Terrain/AGX_TerrainRVTComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Terrain/AGX_TerrainRuntimeVirtualTexture.h"
#include "Terrain/TerrainPagerBarrier.h"

// Unreal Engine includes.
#include "Engine/World.h"
#include "EngineModule.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Landscape.h"
#include "LandscapeComponent.h"
#include "LandscapeHeightfieldCollisionComponent.h"
#include "LandscapeNaniteComponent.h"
#include "LandscapeProxy.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Misc/ScopeRWLock.h"
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
		int32 Nonzero = 0;
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

	void Write(FBlocks& Blocks, FIntPoint Index, float Value)
	{
		const FIntPoint Key = BlockKey(Index);
		auto* Existing = Blocks.Find(Key);
		if (Existing == nullptr && Value == 0.0f)
			return;
		auto& Block = Blocks.FindOrAdd(Key);
		if (!Block.IsValid())
			Block = MakeShared<FBlock, ESPMode::ThreadSafe>();
		float& Old = Block->Values[BlockOffset(Index, Key)];
		Block->Nonzero += (Value != 0.0f ? 1 : 0) - (Old != 0.0f ? 1 : 0);
		Old = Value;
		if (Block->Nonzero == 0)
			Blocks.Remove(Key);
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
		FBlocks Levels[NumLevels]; // Rendering owned; UE may produce pages on its VT worker.
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
	struct FActiveTile
	{
		FBox2D Bounds {ForceInit};
		TSet<FIntPoint> NonzeroIndices;
		TMap<FIntPoint, FVector> Pending;
	};
	TMap<FIntPoint, FActiveTile> Tiles;
	TSet<FIntPoint> LoadedSourceKeys;
	struct FSourceChunk
	{
		int32 Quads = 0;
		TArray<float> Heights;
	};
	// Published on the game thread. The native cache worker holds a read lock per fetch.
	mutable FRWLock SourceLock;
	TMap<FIntPoint, TSharedPtr<const FSourceChunk>> SourceChunks;
	FVector2D LandscapeOrigin;
	int32 ComponentQuads = 0;
	std::atomic<int64> MissingSourceRequests {0};
	int64 LastReportedMissingRequests = 0;
	double LastMissingWarningTime = -5.0;
	TSet<TWeakObjectPtr<UMaterialInstanceDynamic>> BoundMaterials;
	using FBlocks = AGX_TerrainRVT_helpers::FBlocks;
	FVector Min;
	FVector Size;
	FVector Origin;
	FVector2D FetchMin;
	FVector2D FetchMax;
	FVector2D LandscapeSpacing;
	double Spacing = 0.0;
	FBlocks Levels[AGX_TerrainRVT_helpers::NumLevels];
	TSharedRef<AGX_TerrainRVT_helpers::FRenderData, ESPMode::ThreadSafe> RenderData =
		MakeShared<AGX_TerrainRVT_helpers::FRenderData, ESPMode::ThreadSafe>();
	TWeakObjectPtr<ALandscape> Landscape;
	bool bReportedClipping = false;

	// Caller must hold SourceLock when running outside the game thread.
	bool TryBaseHeight(double X, double Y, float& Height) const
	{
		const double GX = (X - LandscapeOrigin.X) / LandscapeSpacing.X;
		const double GY = (Y - LandscapeOrigin.Y) / LandscapeSpacing.Y;
		const FIntPoint Key(
			FMath::FloorToInt(GX / ComponentQuads), FMath::FloorToInt(GY / ComponentQuads));
		// Shared edges may be supplied by either adjacent component, including outer edges.
		for (int32 DY = 0; DY >= -1; --DY)
			for (int32 DX = 0; DX >= -1; --DX)
			{
				const FIntPoint Candidate = Key + FIntPoint(DX, DY);
				const double LX = GX - Candidate.X * ComponentQuads;
				const double LY = GY - Candidate.Y * ComponentQuads;
				if (LX < -0.00001 || LY < -0.00001 || LX > ComponentQuads + 0.00001 ||
					LY > ComponentQuads + 0.00001)
					continue;
				const auto* Chunk = SourceChunks.Find(Candidate);
				if (Chunk == nullptr)
					continue;
				const int32 Quads = (*Chunk)->Quads;
				const double CX = FMath::Clamp(LX / ComponentQuads, 0.0, 1.0) * Quads;
				const double CY = FMath::Clamp(LY / ComponentQuads, 0.0, 1.0) * Quads;
				const int32 IX = FMath::Min(FMath::FloorToInt(CX), Quads - 1);
				const int32 IY = FMath::Min(FMath::FloorToInt(CY), Quads - 1);
				const int32 Side = Quads + 1;
				const auto& H = (*Chunk)->Heights;
				const float H00 = H[IX + IY * Side];
				const float H10 = H[IX + 1 + IY * Side];
				const float H01 = H[IX + (IY + 1) * Side];
				const float H11 = H[IX + 1 + (IY + 1) * Side];
				const double FX = CX - IX;
				const double FY = CY - IY;
				Height = FX > FY ? H00 + FX * (H10 - H00) + FY * (H11 - H10)
								 : H00 + FY * (H01 - H00) + FX * (H11 - H01);
				return true;
			}
		return false;
	}
};

UAGX_TerrainRuntimeVirtualTexture::UAGX_TerrainRuntimeVirtualTexture(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Configure(5);
}

void UAGX_TerrainRuntimeVirtualTexture::Configure(int32 TileCountLog2, int32 PageSize)
{
	TileCount = TileCountLog2;
	TileSize = FMath::FloorLog2(PageSize) - 6;
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
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
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
			 GET_MEMBER_NAME_CHECKED(UAGX_TerrainRVTComponent, DisplacementRange) ||
		 Property->GetFName() == GET_MEMBER_NAME_CHECKED(UAGX_TerrainRVTComponent, PageSize)))
		return false;
	return Super::CanEditChange(Property);
}
#endif

bool UAGX_TerrainRVTComponent::Initialize(ALandscape& Landscape, const FBox& Coverage)
{
	using namespace AGX_TerrainRVT_helpers;
	check(IsInGameThread());
	if (!Coverage.IsValid || Coverage.Min.ContainsNaN() || Coverage.Max.ContainsNaN() ||
		Coverage.Max.X <= Coverage.Min.X || Coverage.Max.Y <= Coverage.Min.Y ||
		Landscape.ComponentSizeQuads <= 0 || (PageSize != 64 && PageSize != 128 && PageSize != 256))
	{
		UE_LOG(
			LogAGX, Error,
			TEXT(
				"Terrain RVT needs valid finite Terrain Bounds and a page size of 64, 128 or "
				"256."));
		return false;
	}
	if (GetWorld()->IsPartitionedWorld() && !Landscape.IsNaniteEnabled())
	{
		UE_LOG(LogAGX, Error, TEXT("Terrain RVT World Partition requires Nanite Landscape."));
		return false;
	}
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
		!FMath::IsNearlyEqual(Landscape.GetActorScale().X, Landscape.GetActorScale().Y))
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
	State->LandscapeOrigin = FVector2D(Landscape.LandscapeActorToWorld().GetLocation());
	State->ComponentQuads = Landscape.ComponentSizeQuads;
	State->Min = FVector(Coverage.Min.X, Coverage.Min.Y, Landscape.GetActorLocation().Z);
	State->Size = FVector(Coverage.Max.X - Coverage.Min.X, Coverage.Max.Y - Coverage.Min.Y, 1.0);
	const double DesiredSpacing = FMath::Min(TerrainVertexSpacing, State->LandscapeSpacing.X);
	const double RequiredResolution = FMath::Max(State->Size.X, State->Size.Y) / DesiredSpacing;
	int32 TileLog2 = FMath::Max(
		0, FMath::CeilToInt(FMath::Log2(FMath::Max(1.0, RequiredResolution / PageSize))));
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
	Texture->Configure(TileLog2, PageSize);
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
		Texture->Configure(TileLog2, PageSize);
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
	RefreshLandscapeProxies();
	SetComponentTickEnabled(true);
	UE_LOG(
		LogAGX, Display,
		TEXT(
			"Terrain RVT initialized: %dx%d virtual texels, native spacing %.3f cm, "
			"source snapshots %d, G16 range +/-%.1f cm."),
		VirtualResolution, VirtualResolution, TerrainVertexSpacing, SourceSnapshotCount,
		DisplacementRange);
	return true;
}

FBox UAGX_TerrainRVTComponent::GetSourceBounds() const
{
	return State != nullptr ? FBox(State->Min, State->Min + State->Size) : FBox(ForceInit);
}

void UAGX_TerrainRVTComponent::RefreshLandscapeProxies()
{
	check(IsInGameThread());
	if (State == nullptr || !State->Landscape.IsValid() || Texture == nullptr)
		return;
	const FGuid Guid = State->Landscape->GetLandscapeGuid();
	State->LoadedSourceKeys.Reset();
	for (auto It = State->BoundMaterials.CreateIterator(); It; ++It)
		if (!It->IsValid())
			It.RemoveCurrent();
	// Only loaded actors are inspected. No loading is requested here or by the native worker.
	for (TActorIterator<ALandscapeProxy> It(GetWorld()); It; ++It)
	{
		ALandscapeProxy& Proxy = **It;
		if (Proxy.GetLandscapeGuid() != Guid || Proxy.IsActorBeingDestroyed())
			continue;
		for (ULandscapeComponent* Component : Proxy.LandscapeComponents)
		{
			if (!IsValid(Component) || !Component->IsRegistered())
				continue;
			if (Component->ComponentSizeQuads == State->ComponentQuads)
			{
				const FIntPoint Key = Component->GetSectionBase() / State->ComponentQuads;
				const FVector2D Min =
					State->LandscapeOrigin +
					FVector2D(Component->GetSectionBase()) * State->LandscapeSpacing;
				const FVector2D Max =
					Min + FVector2D(State->ComponentQuads) * State->LandscapeSpacing;
				const bool InCoverage = Max.X >= State->Min.X && Max.Y >= State->Min.Y &&
										Min.X <= State->Min.X + State->Size.X &&
										Min.Y <= State->Min.Y + State->Size.Y;
				if (InCoverage)
					State->LoadedSourceKeys.Add(Key);
				auto* Collision = Component->GetCollisionComponent();
				if (InCoverage && !State->SourceChunks.Contains(Key) && IsValid(Collision) &&
					Collision->IsRegistered())
				{
					const int32 Quads = Collision->CollisionSizeQuads;
					if (Quads <= 0)
						continue;
					const int32 Side = Quads + 1;
					auto Chunk = MakeShared<FAGX_TerrainRVTState::FSourceChunk>();
					Chunk->Quads = Quads;
					Chunk->Heights.SetNumUninitialized(Side * Side);
					// Runtime API reads the live complex collision heightfield, including exact
					// edge vertices. It also handles nonzero collision mip levels without querying
					// an adjacent, possibly unloaded proxy at shared edges.
					if (Collision->FillHeightTile(Chunk->Heights, 0, Side) &&
						!Chunk->Heights.ContainsByPredicate([](float Height)
															{ return !FMath::IsFinite(Height); }))
					{
						FWriteScopeLock Lock(State->SourceLock);
						State->SourceChunks.Add(Key, MoveTemp(Chunk));
					}
				}
			}
			// Runtime MID creation and texture binding; no Landscape heightmap edits or editor
			// APIs.
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
				if (State->BoundMaterials.Contains(Material))
					continue;
				Material->SetRuntimeVirtualTextureParameterValue(TEXT("AGX_Deformation"), Texture);
				Material->SetScalarParameterValue(TEXT("AGX_DisplacementRange"), DisplacementRange);
				Material->SetVectorParameterValue(
					TEXT("AGX_CoverageMin"), FLinearColor(State->Min.X, State->Min.Y, 0.0f));
				Material->SetVectorParameterValue(
					TEXT("AGX_CoverageMax"),
					FLinearColor(State->Min.X + State->Size.X, State->Min.Y + State->Size.Y, 0.0f));
				Material->SetScalarParameterValue(TEXT("AGX_RVTReady"), 1.0f);
				State->BoundMaterials.Add(Material);
			}
		}
		// Nanite's static mesh can retain shared editor/cooked material references. Override the
		// component slots with this world's source MIDs; never mutate the shared static mesh asset.
		TArray<ULandscapeNaniteComponent*> NaniteComponents;
		Proxy.GetComponents(NaniteComponents);
		for (auto* NaniteComponent : NaniteComponents)
		{
			const auto& Sources = NaniteComponent->GetSourceLandscapeComponents();
			for (int32 Index = 0; Index < Sources.Num(); ++Index)
			{
				ULandscapeComponent* Source = Sources[Index];
				if (Source != nullptr && Source->MaterialInstancesDynamic.IsValidIndex(0) &&
					NaniteComponent->GetMaterial(Index) != Source->MaterialInstancesDynamic[0])
					NaniteComponent->SetMaterial(Index, Source->MaterialInstancesDynamic[0]);
			}
		}
	}
	PruneSourceSnapshots();
}

void UAGX_TerrainRVTComponent::PruneSourceSnapshots()
{
	TSet<FIntPoint> Needed = State->LoadedSourceKeys;
	const FVector2D ComponentSize = State->LandscapeSpacing * State->ComponentQuads;
	for (const auto& Entry : State->Tiles)
	{
		const FBox2D& Bounds = Entry.Value.Bounds;
		if (!Bounds.bIsValid)
			continue;
		const FVector2D Min = (Bounds.Min - State->LandscapeOrigin) / ComponentSize;
		const FVector2D Max = (Bounds.Max - State->LandscapeOrigin) / ComponentSize;
		// Include shared-edge candidates used by TryBaseHeight.
		for (int32 Y = FMath::FloorToInt(Min.Y) - 1; Y <= FMath::FloorToInt(Max.Y); ++Y)
			for (int32 X = FMath::FloorToInt(Min.X) - 1; X <= FMath::FloorToInt(Max.X); ++X)
				Needed.Add(FIntPoint(X, Y));
	}
	FWriteScopeLock Lock(State->SourceLock);
	for (auto It = State->SourceChunks.CreateIterator(); It; ++It)
		if (!Needed.Contains(It.Key()))
			It.RemoveCurrent();
	SourceSnapshotCount = State->SourceChunks.Num();
}

void UAGX_TerrainRVTComponent::TickComponent(
	float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	RefreshLandscapeProxies();
	if (State == nullptr)
		return;
	UnavailableHeightRequests = State->MissingSourceRequests.load();
	const double Now = GetWorld()->GetTimeSeconds();
	if (UnavailableHeightRequests > State->LastReportedMissingRequests &&
		Now - State->LastMissingWarningTime >= 5.0)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT(
				"Terrain RVT '%s' rejected %lld source height requests because "
				"Landscape collision snapshots are not available. AGX will retry; simulation may "
				"continue without terrain collision. Load Landscape proxies before tracked objects "
				"arrive."),
			*GetOwner()->GetName(), UnavailableHeightRequests - State->LastReportedMissingRequests);
		State->LastReportedMissingRequests = UnavailableHeightRequests;
		State->LastMissingWarningTime = Now;
	}
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
	// Called on the AGX worker. Never wait for proxy loading or game-thread work.
	Heights.Reset();
	if (State == nullptr || Start.X < State->FetchMin.X - 0.01 ||
		Start.Y < State->FetchMin.Y - 0.01 ||
		Start.X + (VertsX - 1) * State->Spacing > State->FetchMax.X + 0.01 ||
		Start.Y + (VertsY - 1) * State->Spacing > State->FetchMax.Y + 0.01)
		return false;
	FReadScopeLock Lock(State->SourceLock);
	Heights.Reserve(VertsX * VertsY);
	for (int32 Y = VertsY - 1; Y >= 0; --Y)
		for (int32 X = 0; X < VertsX; ++X)
		{
			float Height;
			if (!State->TryBaseHeight(
					Start.X + X * State->Spacing, Start.Y + Y * State->Spacing, Height))
			{
				++State->MissingSourceRequests;
				Heights.Reset();
				return false;
			}
			Heights.Add(Height - State->Origin.Z);
		}
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
	const TArray<FIntPoint> ActiveIds = Pager.GetActiveHeightCaptureTileIds();
	TSet<FIntPoint> Active;
	Active.Append(ActiveIds);
	TSet<FIntPoint> Dirty[NumLevels];
	const auto SetDelta = [&](FIntPoint Index, float Delta)
	{
		for (int32 Level = 0; Level < NumLevels; ++Level)
		{
			Dirty[Level].Add(BlockKey(Index));
			Write(State->Levels[Level], Index, Delta);
			Index = FIntPoint(FloorDivide(Index.X, 2), FloorDivide(Index.Y, 2));
			Delta = 0.25f * (Read(State->Levels[Level], Index * 2) +
							 Read(State->Levels[Level], Index * 2 + FIntPoint(1, 0)) +
							 Read(State->Levels[Level], Index * 2 + FIntPoint(0, 1)) +
							 Read(State->Levels[Level], Index * 2 + FIntPoint(1, 1)));
		}
	};
	for (auto It = State->Tiles.CreateIterator(); It; ++It)
	{
		if (Active.Contains(It.Key()))
			continue;
		for (const FIntPoint Index : It.Value().NonzeroIndices)
			SetDelta(Index, 0.0f);
		It.RemoveCurrent();
		// Deliberately keep already-produced GPU pages until normal VT eviction. No CPU
		// history is retained for unloaded tiles; a future page request sees neutral terrain.
	}
	TArray<FTerrainPagerHeightSample> Samples;
	for (auto& Tile : State->Tiles)
	{
		for (const auto& Pending : Tile.Value.Pending)
			Samples.Add({Pending.Value, Tile.Key, true});
		Tile.Value.Pending.Reset();
	}
	Samples.Append(Pager.ConsumeHeightChanges());
	// Separate edits remain separate invalidations. Group nearby samples without expanding
	// every update to a whole block or to the box enclosing all tracked objects.
	TMap<FIntPoint, FBox2D> Regions;
	for (const auto& Sample : Samples)
	{
		if (!Active.Contains(Sample.TileId))
			continue;
		auto& Tile = State->Tiles.FindOrAdd(Sample.TileId);
		Tile.Bounds += FVector2D(Sample.Position);
		const FVector Grid = (Sample.Position - State->Origin) / State->Spacing;
		const FIntPoint Index(FMath::RoundToInt(Grid.X), FMath::RoundToInt(Grid.Y));
		float BaseHeight;
		if (!State->TryBaseHeight(Sample.Position.X, Sample.Position.Y, BaseHeight))
		{
			// Cached AGX tiles can return before their original Landscape proxy. Pending
			// absolute heights are deduplicated per active tile and discarded on unload.
			Tile.Pending.Add(Index, Sample.Position);
			continue;
		}
		float Delta = Sample.Position.Z - BaseHeight;
		if (FMath::Abs(Delta) < 0.005f)
			Delta = 0.0f;
		const bool Changed = !FMath::IsNearlyEqual(Read(State->Levels[0], Index), Delta, 0.0001f);
		if (!Changed && !Sample.bTileSnapshot)
			continue;
		auto& Region = Regions.FindOrAdd(BlockKey(Index), FBox2D(ForceInit));
		Region += FVector2D(Sample.Position);
		if (!Changed)
			continue;
		++ChangedSamples;
		if (Delta == 0.0f)
			Tile.NonzeroIndices.Remove(Index);
		else
			Tile.NonzeroIndices.Add(Index);
		if (FMath::Abs(Delta) > DisplacementRange && !State->bReportedClipping)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT(
					"Terrain RVT displacement exceeded its +/-%.1f cm encoding range. "
					"Active CPU values are retained but rendering is clipped."),
				DisplacementRange);
			State->bReportedClipping = true;
		}
		SetDelta(Index, Delta);
	}
	struct FChangedBlock
	{
		int32 Level;
		FIntPoint Key;
		TSharedPtr<FBlock, ESPMode::ThreadSafe> Block;
	};
	TArray<FChangedBlock> Updates;
	for (int32 Level = 0; Level < NumLevels; ++Level)
		for (const FIntPoint Key : Dirty[Level])
		{
			TSharedPtr<FBlock, ESPMode::ThreadSafe> Copy;
			if (const auto* Block = State->Levels[Level].Find(Key))
			{
				Copy = MakeShared<FBlock, ESPMode::ThreadSafe>();
				*Copy = **Block;
			}
			Updates.Add({Level, Key, MoveTemp(Copy)});
		}
	ResidentCPUBlocks = 0;
	for (const auto& Level : State->Levels)
		ResidentCPUBlocks += Level.Num();
	ResidentCPUTiles = State->Tiles.Num();
	PendingHeightSamples = 0;
	for (const auto& Tile : State->Tiles)
		PendingHeightSamples += Tile.Value.Pending.Num();
	PruneSourceSnapshots();
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
	TArray<FRequest> Invalidations;
	const auto MakeRequest = [&](FVector2D WorldMin, FVector2D WorldMax)
	{
		const double Distance = FVector2D::Distance(FVector2D(Camera), (WorldMin + WorldMax) * 0.5);
		const int32 Mip =
			FMath::Max(0, FMath::FloorToInt(FMath::Log2(FMath::Max(1.0, Distance / 15000.0))));
		const FVector2D Min = (WorldMin - FVector2D(State->Min)) / FVector2D(State->Size);
		const FVector2D Max = (WorldMax - FVector2D(State->Min)) / FVector2D(State->Size);
		return FRequest {
			{FMath::Clamp(Min.X, 0.0, 1.0), FMath::Clamp(Min.Y, 0.0, 1.0)},
			{FMath::Clamp(Max.X, 0.0, 1.0), FMath::Clamp(Max.Y, 0.0, 1.0)},
			Mip};
	};
	// Vertex displacement sampling has feedback disabled. Request only active deformation;
	// distant unloaded areas are best-effort GPU cache contents, not persistent visual storage.
	for (const auto& Entry : State->Levels[0])
	{
		const FVector2D Min =
			FVector2D(State->Origin) + FVector2D(Entry.Key) * (BlockSide * State->Spacing);
		Requests.Add(MakeRequest(Min, Min + FVector2D(BlockSide * State->Spacing)));
	}
	for (const auto& Region : Regions)
	{
		// One native cell covers interpolation support. UE expands each mip's invalidation
		// by its physical page border, including the coarser box-filtered samples.
		const FRequest Request = MakeRequest(
			Region.Value.Min - FVector2D(State->Spacing),
			Region.Value.Max + FVector2D(State->Spacing));
		if (Request.UV1.X > Request.UV0.X && Request.UV1.Y > Request.UV0.Y)
		{
			Invalidations.Add(Request);
			// Also refresh zero-valued tile-load samples, clearing stale cached GPU heights.
			Requests.Add(Request);
		}
	}
	auto Data = State->RenderData;
	auto* RuntimeTexture = Texture.Get();
	ENQUEUE_RENDER_COMMAND(AGXTerrainRVTUpdate)(
		[Data, RuntimeTexture, Updates = MoveTemp(Updates), Requests = MoveTemp(Requests),
		 Invalidations = MoveTemp(Invalidations)](FRHICommandListImmediate&) mutable
		{
			for (const auto& Update : Updates)
				if (Update.Block.IsValid())
					Data->Levels[Update.Level].Add(Update.Key, Update.Block);
				else
					Data->Levels[Update.Level].Remove(Update.Key);
			if (auto* Allocation = RuntimeTexture->GetAllocatedVirtualTexture())
			{
				for (const auto& Region : Invalidations)
					GetRendererModule().FlushVirtualTextureCache(
						Allocation, FVector2f(Region.UV0), FVector2f(Region.UV1));
				for (const auto& Request : Requests)
					GetRendererModule().RequestVirtualTextureTiles(
						Allocation, FVector2D(1024, 1024), FVector2D::ZeroVector,
						FVector2D(1024, 1024), Request.UV0, Request.UV1, Request.Mip);
			}
		});
}

void UAGX_TerrainRVTComponent::Shutdown()
{
	SetComponentTickEnabled(false);
	if (State != nullptr)
	{
		for (auto Material : State->BoundMaterials)
			if (Material.IsValid())
			{
				Material->SetScalarParameterValue(TEXT("AGX_RVTReady"), 0.0f);
				Material->SetRuntimeVirtualTextureParameterValue(TEXT("AGX_Deformation"), nullptr);
			}
	}
	if (Texture != nullptr)
		Texture->Release();
	// All commands capturing the texture UObject must finish before its reference is dropped.
	FlushRenderingCommands();
	Texture = nullptr;
	State.reset();
}
