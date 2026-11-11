// Copyright 2026, Algoryx Simulation AB.

#include "Terrain/AGX_TerrainVirtualTexture.h"

// Unreal Engine includes.
#include "HAL/PlatformTime.h"
#include "Math/UnrealMathUtility.h"
#include "Misc/EngineVersionComparison.h"
#include "RenderGraphBuilder.h"
#include "RenderTargetPool.h"
#include "RHICommandList.h"
#include "ShaderParameterStruct.h"

void FAGX_TerrainVirtualTextureData::FTile::UpdateMips(TConstArrayView<FIntPoint> Vertices)
{
	Mips.Reserve(FMath::CeilLogTwo(Side));
	TSet<FIntPoint> Dirty;
	Dirty.Append(Vertices);
	const TArray<float>* Source = &Heights;
	int32 SourceSide = Side;
	int32 Level = 0;
	while (SourceSide > 1)
	{
		const int32 TargetSide = FMath::DivideAndRoundUp(SourceSide, 2);
		if (!Mips.IsValidIndex(Level))
			Mips.AddDefaulted();
		auto& Target = Mips[Level];
		const bool bFull = Vertices.IsEmpty() || Target.Side != TargetSide;
		Target.Side = TargetSide;
		Target.Heights.SetNumUninitialized(TargetSide * TargetSide);
		TSet<FIntPoint> Parents;
		if (!bFull)
			for (const FIntPoint& Vertex : Dirty)
				Parents.Add(Vertex / 2);
		auto Filter = [&](int32 X, int32 Y)
		{
			float Sum = 0.f;
			for (int32 DY = 0; DY < 2; ++DY)
				for (int32 DX = 0; DX < 2; ++DX)
					Sum += (*Source)
						[FMath::Min(2 * X + DX, SourceSide - 1) +
						 FMath::Min(2 * Y + DY, SourceSide - 1) * SourceSide];
			Target.Heights[X + Y * TargetSide] = Sum * 0.25f;
		};
		if (bFull)
			for (int32 Y = 0; Y < TargetSide; ++Y)
				for (int32 X = 0; X < TargetSide; ++X)
					Filter(X, Y);
		else
			for (const FIntPoint& Parent : Parents)
				Filter(Parent.X, Parent.Y);
		Dirty = MoveTemp(Parents);
		Source = &Target.Heights;
		SourceSide = TargetSide;
		++Level;
	}
}

FVector4f FAGX_TerrainVirtualTextureData::Sample(
	const FVector2D& Position, TConstArrayView<const FTile*> Candidates, double Footprint) const
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
	const int32 Level = FMath::Clamp(
		FMath::FloorToInt(FMath::Log2(FMath::Max(1.0, Footprint / Best->Spacing))), 0,
		Best->Mips.Num());
	const double Scale = static_cast<double>(1u << Level);
	const int32 Side = Level == 0 ? Best->Side : Best->Mips[Level - 1].Side;
	const auto& Heights = Level == 0 ? Best->Heights : Best->Mips[Level - 1].Heights;
	// Parent texels are centered on their children's box footprint, in native (reversed Y) order.
	Coordinate =
		(FVector2D(Coordinate.X, Best->Side - 1 - Coordinate.Y) - FVector2D((Scale - 1.0) * 0.5)) /
		Scale;
	Coordinate.X = FMath::Clamp(Coordinate.X, 0.0, static_cast<double>(Side - 1));
	Coordinate.Y = FMath::Clamp(Coordinate.Y, 0.0, static_cast<double>(Side - 1));
	Coordinate.Y = Side - 1 - Coordinate.Y;
	if (Side == 1)
		return FVector4f(Heights[0], 0, 0, 1);
	const int32 X = FMath::Min(FMath::FloorToInt(Coordinate.X), Side - 2);
	const int32 Y = FMath::Min(FMath::FloorToInt(Coordinate.Y), Side - 2);
	auto Height = [&Heights, Side](int32 Column, int32 Row)
	{ return Heights[Column + (Side - 1 - Row) * Side]; };
	const float H00 = Height(X, Y);
	const float H10 = Height(X + 1, Y);
	const float H01 = Height(X, Y + 1);
	const float H11 = Height(X + 1, Y + 1);
	const float U = static_cast<float>(Coordinate.X - X);
	const float V = static_cast<float>(Coordinate.Y - Y);
	return FVector4f(
		FMath::Lerp(FMath::Lerp(H00, H10, U), FMath::Lerp(H01, H11, U), V),
		FMath::Lerp(H10 - H00, H11 - H01, V) / (Best->Spacing * Scale),
		FMath::Lerp(H01 - H00, H11 - H10, U) / (Best->Spacing * Scale), 1.f);
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
						Pixels[Column + Row * Physical] = Data->Sample(
							Position, Candidates, FMath::Max(TexelSize.X, TexelSize.Y));
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
