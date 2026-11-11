// Copyright 2026, Algoryx Simulation AB.

#include "Terrain/AGX_TerrainVirtualTexture.h"

// Unreal Engine includes.
#include "HAL/PlatformTime.h"
#include "Math/UnrealMathUtility.h"
#include "Misc/EngineVersionComparison.h"
#include "Misc/ScopeRWLock.h"
#include "RenderGraphBuilder.h"
#include "RenderTargetPool.h"
#include "RHICommandList.h"
#include "ShaderParameterStruct.h"

FVector4f FAGX_TerrainVirtualTextureData::Sample(
	const FVector2D& Position, TConstArrayView<const FTile*> Candidates) const
{
	// Stable ownership of overlapping samples: select the tile with the deepest interior.
	// This keeps border interpolation from switching unpredictably with TMap iteration order.
	const FTile* Best = nullptr;
	double BestMargin = -1.0;
	FIntPoint BestKey(MAX_int32, MAX_int32);
	FVector2D Coordinate = FVector2D::ZeroVector;
	for (const FTile* Candidate : Candidates)
	{
		const FTile& Tile = *Candidate;
		const FVector2D Local = (Position - Tile.Corner) / Tile.Spacing;
		const double Margin = FMath::Min(
			FMath::Min(Local.X, Local.Y),
			FMath::Min(Tile.Side - 1 - Local.X, Tile.Side - 1 - Local.Y));
		if (Margin < 0.0)
			continue;
		const bool bEarlier =
			Tile.Id.X < BestKey.X || (Tile.Id.X == BestKey.X && Tile.Id.Y < BestKey.Y);
		if (Margin > BestMargin || (Margin == BestMargin && bEarlier))
		{
			Best = &Tile;
			BestKey = Tile.Id;
			BestMargin = Margin;
			Coordinate = Local;
		}
	}
	if (Best == nullptr)
		return FVector4f(0.f, 0.f, 0.f, 0.f);
	const int32 X = FMath::Min(FMath::FloorToInt(Coordinate.X), Best->Side - 2);
	const int32 Y = FMath::Min(FMath::FloorToInt(Coordinate.Y), Best->Side - 2);
	auto Height = [Best](int32 Column, int32 Row)
	{
		// Heights from AGX are stored with Y reversed relative to Unreal.
		return Best->Heights[Column + (Best->Side - 1 - Row) * Best->Side];
	};
	const float H00 = Height(X, Y);
	const float H10 = Height(X + 1, Y);
	const float H01 = Height(X, Y + 1);
	const float H11 = Height(X + 1, Y + 1);
	const float U = static_cast<float>(Coordinate.X - X);
	const float V = static_cast<float>(Coordinate.Y - Y);
	return FVector4f(
		FMath::Lerp(FMath::Lerp(H00, H10, U), FMath::Lerp(H01, H11, U), V),
		FMath::Lerp(H10 - H00, H11 - H01, V) / Best->Spacing,
		FMath::Lerp(H01 - H00, H11 - H10, U) / Best->Spacing, 1.f);
}

namespace
{
	BEGIN_SHADER_PARAMETER_STRUCT(FAGX_TerrainPageUploadParameters, )
	RDG_TEXTURE_ACCESS(Texture, ERHIAccess::CopyDest)
	END_SHADER_PARAMETER_STRUCT()

	class FTerrainVirtualTextureProducer final : public IVirtualTexture,
												 public IVirtualTextureFinalizer
	{
	public:
		FTerrainVirtualTextureProducer(
			TSharedRef<FAGX_TerrainVirtualTextureData, ESPMode::ThreadSafe> InData,
			const FVTProducerDescription& InDescription)
			: Data(InData)
			, Description(InDescription)
		{
		}

		virtual bool IsPageStreamed(uint8, uint32) const override
		{
			return false;
		}

		virtual FVTRequestPageResult RequestPageData(
			FRHICommandList&, const FVirtualTextureProducerHandle&, uint8, uint8 Level,
			uint64 Address, EVTRequestPagePriority) override
		{
			if (Frame != GFrameNumberRenderThread)
			{
				Frame = GFrameNumberRenderThread;
				Requests = 0;
			}
			if (Level > Description.MaxLevel)
				return FVTRequestPageResult(EVTRequestPageStatus::Invalid);
			if (++Requests > Data->MaxPagesPerFrame)
				return FVTRequestPageResult(EVTRequestPageStatus::Saturated);
			return FVTRequestPageResult(EVTRequestPageStatus::Available);
		}

		virtual IVirtualTextureFinalizer* ProducePageData(
			FRHICommandList&, ERHIFeatureLevel::Type, EVTProducePageFlags,
			const FVirtualTextureProducerHandle&, uint8, uint8 Level, uint64 Address, uint64,
			const FVTProduceTargetLayer* Targets) override
		{
			FPage& Page = Pages.AddDefaulted_GetRef();
			Page.Target = Targets[0].PooledRenderTarget;
			Page.Location = Targets[0].pPageLocation;
			Page.Level = Level;
			Page.Address = Address;
			return this;
		}

		virtual void Finalize(FRDGBuilder& GraphBuilder) override
		{
			TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("AGXUnreal:TerrainRVTPageProduction"));
			const double Start = FPlatformTime::Seconds();
			const int32 Interior = Description.TileSize;
			const int32 Border = Description.TileBorderSize;
			const int32 Physical = Interior + 2 * Border;
			FReadScopeLock Lock(Data->Lock);
			for (const FPage& Page : Pages)
			{
				const uint32 X = FMath::ReverseMortonCode2(static_cast<uint32>(Page.Address));
				const uint32 Y = FMath::ReverseMortonCode2(static_cast<uint32>(Page.Address >> 1));
				const FVector2D TexelSize = Data->CoverageSize *
											static_cast<double>(1u << Page.Level) /
											FVector2D(
												Description.BlockWidthInTiles * Interior,
												Description.BlockHeightInTiles * Interior);
				TArray<const FAGX_TerrainVirtualTextureData::FTile*, TInlineAllocator<8>>
					Candidates;
				const FVector2D PageMin =
					Data->CoverageMin + TexelSize * FVector2D(
														static_cast<double>(X) * Interior - Border,
														static_cast<double>(Y) * Interior - Border);
				const FVector2D PageMax = PageMin + TexelSize * static_cast<double>(Physical);
				for (const auto& Entry : Data->Tiles)
				{
					const auto& Tile = Entry.Value;
					const FVector2D TileMax =
						Tile.Corner + FVector2D((Tile.Side - 1) * Tile.Spacing);
					if (Tile.Corner.X <= PageMax.X && Tile.Corner.Y <= PageMax.Y &&
						TileMax.X >= PageMin.X && TileMax.Y >= PageMin.Y)
						Candidates.Add(&Tile);
				}
				TArray<FVector4f> Pixels;
				Pixels.SetNumUninitialized(Physical * Physical);
				for (int32 Row = 0; Row < Physical; ++Row)
				{
					for (int32 Column = 0; Column < Physical; ++Column)
					{
						const FVector2D Position =
							Data->CoverageMin +
							TexelSize *
								FVector2D(
									static_cast<double>(X) * Interior + Column - Border + 0.5,
									static_cast<double>(Y) * Interior + Row - Border + 0.5);
						Pixels[Column + Row * Physical] = Data->Sample(Position, Candidates);
					}
				}
				FRDGTextureRef Texture = GraphBuilder.RegisterExternalTexture(Page.Target);
				auto* Parameters = GraphBuilder.AllocParameters<FAGX_TerrainPageUploadParameters>();
				Parameters->Texture = Texture;
				const FUpdateTextureRegion2D Region(
					Page.Location.X * Physical, Page.Location.Y * Physical, 0, 0, Physical,
					Physical);
				GraphBuilder.AddPass(
					RDG_EVENT_NAME("AGX Terrain RVT upload"), Parameters,
					ERDGPassFlags::Copy | ERDGPassFlags::NeverCull,
					[Texture, Region, Physical,
					 Pixels = MoveTemp(Pixels)](FRHICommandListImmediate& RHICmdList)
					{
#if UE_VERSION_OLDER_THAN(5, 8, 0)
						RHIUpdateTexture2D(
							Texture->GetRHI(), 0, Region, Physical * sizeof(FVector4f),
							reinterpret_cast<const uint8*>(Pixels.GetData()));
#else
						RHICmdList.UpdateTexture2D(
							Texture->GetRHI(), 0, Region, Physical * sizeof(FVector4f),
							reinterpret_cast<const uint8*>(Pixels.GetData()));
#endif
					});
			}
			Data->ProducedPages.fetch_add(Pages.Num());
			Data->LastProductionMilliseconds = (FPlatformTime::Seconds() - Start) * 1000.0;
			Pages.Reset();
		}

	private:
		struct FPage
		{
			TRefCountPtr<IPooledRenderTarget> Target;
			FIntVector Location;
			uint8 Level;
			uint64 Address;
		};
		TSharedRef<FAGX_TerrainVirtualTextureData, ESPMode::ThreadSafe> Data;
		FVTProducerDescription Description;
		TArray<FPage> Pages;
		uint32 Frame = MAX_uint32;
		int32 Requests = 0;
	};
}

IVirtualTexture* AGX_CreateTerrainVirtualTextureProducer(
	TSharedRef<FAGX_TerrainVirtualTextureData, ESPMode::ThreadSafe> Data,
	const FVTProducerDescription& Description)
{
	return new FTerrainVirtualTextureProducer(Data, Description);
}
