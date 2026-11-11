// Copyright 2026, Algoryx Simulation AB.

#include "Terrain/AGX_TerrainPartitionedLandscapeRendererComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Terrain/AGX_HeightFieldBoundsComponent.h"
#include "Terrain/AGX_Terrain.h"
#include "Terrain/AGX_TerrainDisplacementVirtualTexture.h"
#include "Terrain/AGX_TerrainNaniteDisplacementData.h"
#include "Terrain/AGX_TerrainVirtualTexture.h"

// Unreal Engine includes.
#include "DataDrivenShaderPlatformInfo.h"
#include "Engine/Level.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "HAL/IConsoleManager.h"
#include "Landscape.h"
#include "LandscapeComponent.h"
#include "LandscapeNaniteComponent.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/ScopeRWLock.h"
#include "Modules/ModuleManager.h"
#include "RendererInterface.h"
#include "RenderingThread.h"
#include "RHI.h"
#include "VT/RuntimeVirtualTexture.h"
#include "VT/VirtualTexturePoolConfig.h"

UAGX_TerrainPartitionedLandscapeRendererComponent::
	UAGX_TerrainPartitionedLandscapeRendererComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

UAGX_TerrainPartitionedLandscapeRendererComponent::
	~UAGX_TerrainPartitionedLandscapeRendererComponent() = default;

void UAGX_TerrainPartitionedLandscapeRendererComponent::SetEnabled(bool bInEnabled)
{
	bEnabled = bInEnabled;
	if (!bEnabled)
		Shutdown();
}

void UAGX_TerrainPartitionedLandscapeRendererComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	Shutdown();
	Super::EndPlay(Reason);
}

void UAGX_TerrainPartitionedLandscapeRendererComponent::Initialize(AAGX_Terrain& InTerrain)
{
	if (Terrain != nullptr)
		return;
	Terrain = &InTerrain;
	// Keep the failed initialization associated with this enable cycle, avoiding per-frame logs.
	if (!Terrain->HasNativeTerrainPager() || Terrain->TerrainBounds->bInfiniteBounds)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT(
				"Partitioned Landscape rendering requires Terrain Paging and finite Terrain Bounds "
				"on '%s'."),
			*Terrain->GetName());
		return;
	}
	if (PageSize < 64 || PageSize > 1024 || !FMath::IsPowerOfTwo(PageSize) ||
		!FMath::IsFinite(DisplacementRange) || DisplacementRange <= 0.0 || MaxPagesPerFrame < 1 ||
		MaxPagesPerFrame > 256)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT(
				"Invalid partitioned renderer settings on '%s': page size must be a power of two "
				"(64..1024), range positive and finite, pages/frame 1..256."),
			*Terrain->GetName());
		return;
	}
	for (const TCHAR* Setting : {TEXT("r.VirtualTextures"), TEXT("r.Nanite.Tessellation")})
	{
		const IConsoleVariable* Variable = IConsoleManager::Get().FindConsoleVariable(Setting);
		if (Variable == nullptr || Variable->GetInt() == 0)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT(
					"Partitioned Landscape rendering requires %s=1. Configure the project and "
					"restart the editor before playing."),
				Setting);
			return;
		}
	}
	if (!FDataDrivenShaderPlatformInfo::GetSupportsNanite(GMaxRHIShaderPlatform))
	{
		UE_LOG(
			LogAGX, Error,
			TEXT(
				"Partitioned Landscape rendering requires a Nanite-capable shader platform. Use "
				"DX12/SM6 on Windows."));
		return;
	}
	for (TActorIterator<AAGX_Terrain> It(GetWorld()); It; ++It)
	{
		if (*It == Terrain || It->SourceLandscape != Terrain->SourceLandscape)
			continue;
		TInlineComponentArray<ThisClass*> OtherRenderers(*It);
		for (const ThisClass* Other : OtherRenderers)
		{
			if (Other->GetDisplacementVirtualTexture() != nullptr)
			{
				UE_LOG(
					LogAGX, Error,
					TEXT(
						"Landscape '%s' already has an active partitioned Terrain renderer. Use "
						"one renderer per Landscape in a world."),
					*GetNameSafe(Terrain->SourceLandscape));
				return;
			}
		}
	}
	const auto Bounds = Terrain->TerrainBounds->GetUserSetBounds();
	if (!Bounds.IsSet() || Terrain->SourceLandscape == nullptr)
		return;
	const double PhysicsSpacing = Terrain->GetSimulationResolution();
	const double SourceSpacing = FMath::Min(
		Terrain->SourceLandscape->GetActorScale3D().X,
		Terrain->SourceLandscape->GetActorScale3D().Y);
	// Native vertices must fall on RVT texel centers. Resampling onto an unrelated power-of-two
	// lattice smooths sharp cuts and cannot provide accurate visual/physics agreement.
	double TexelSpacing =
		PhysicsSpacing / FMath::CeilToDouble(FMath::Max(1.0, PhysicsSpacing / SourceSpacing));
	const FVector GridOrigin3D = Bounds->Transform.InverseTransformPositionNoScale(
		Terrain->GetNativeTerrainPager()->GetTileGridOrigin());
	const FVector2D GridOrigin(GridOrigin3D.X, GridOrigin3D.Y);
	const FVector2D Half(Bounds->HalfExtent.X, Bounds->HalfExtent.Y);
	auto FirstVertex = [&GridOrigin, &Half](double Spacing)
	{
		return GridOrigin + FVector2D(
								FMath::FloorToDouble((-Half.X - GridOrigin.X) / Spacing),
								FMath::FloorToDouble((-Half.Y - GridOrigin.Y) / Spacing)) *
								Spacing;
	};
	auto RequiredSamples = [&Half, &FirstVertex](double Spacing)
	{
		const FVector2D First = FirstVertex(Spacing);
		return FMath::CeilToDouble(FMath::Max(Half.X - First.X, Half.Y - First.Y) / Spacing) + 1.0;
	};
	const int32 MaxLog2 = URuntimeVirtualTexture::GetMaxTileCountLog2(false);
	const double MaximumSide = static_cast<double>(PageSize) * (1 << MaxLog2);
	if (RequiredSamples(TexelSpacing) > MaximumSide)
	{
		TexelSpacing *= FMath::Pow(
			2.0, FMath::CeilToDouble(FMath::Log2(RequiredSamples(TexelSpacing) / MaximumSide)));
		UE_LOG(
			LogAGX, Warning,
			TEXT(
				"Terrain '%s' RVT density is clamped by UE's page table limit. Reduce coverage or "
				"increase Page Size for accurate fine geometry."),
			*Terrain->GetName());
	}
	const double RequiredPages = RequiredSamples(TexelSpacing) / PageSize;
	const int32 TileCountLog2 =
		FMath::Clamp(FMath::CeilToInt(FMath::Log2(FMath::Max(1.0, RequiredPages))), 0, MaxLog2);
	const FVector2D Size(static_cast<double>(PageSize) * (1 << TileCountLog2) * TexelSpacing);
	const FVector2D CoverageMin = FirstVertex(TexelSpacing) - FVector2D(TexelSpacing * 0.5);
	Data = MakeShared<FAGX_TerrainVirtualTextureData, ESPMode::ThreadSafe>();
	Data->Frame = Bounds->Transform;
	Data->CoverageMin = CoverageMin;
	Data->CoverageSize = Size;
	Data->MaxPagesPerFrame = MaxPagesPerFrame;
	UAGX_TerrainDisplacementVirtualTexture* RuntimeTexture =
		NewObject<UAGX_TerrainDisplacementVirtualTexture>(this);
	RuntimeTexture->Configure(PageSize, TileCountLog2);
	VirtualTexture = RuntimeTexture;
	const FTransform Volume(
		Bounds->Transform.GetRotation(),
		Bounds->Transform.TransformPositionNoScale(
			FVector(CoverageMin.X, CoverageMin.Y, -DisplacementRange)),
		FVector(Size.X, Size.Y, 2.0 * DisplacementRange));
	FVTProducerDescription Description;
	RuntimeTexture->GetProducerDescription(
		Description, URuntimeVirtualTexture::FInitSettings(), Volume);
	Description.bPersistentHighestMip = true;
	Description.bRequiresSinglePhysicalPool = true;
	// Mask4 has an identity float4 shader unpack. The custom producer stores precise
	// heights/gradients instead of normalized 8-bit masks; no world-sized intermediate.
	Description.LayerFormat[0] = PF_A32B32G32R32F;
	Description.LayerFallbackColor[0] = FLinearColor::Transparent;
	FVirtualTextureSpacePoolConfig PoolConfig;
	const TEnumAsByte<EPixelFormat> Format = PF_A32B32G32R32F;
	VirtualTexturePool::FindPoolConfig(&Format, 1, PageSize + 8, PoolConfig);
	UE_LOG(
		LogAGX, Display,
		TEXT(
			"Terrain '%s' RVT: %u x %u texels, %d interior + 8 border texels/page, shared RGBA32F "
			"pool configuration %d MiB. Physics grid: %.3f cm. Nanite geometry density is "
			"independent."),
		*Terrain->GetName(), Description.BlockWidthInTiles * Description.TileSize,
		Description.BlockHeightInTiles * Description.TileSize, PageSize, PoolConfig.SizeInMegabyte,
		Terrain->GetSimulationResolution());
	if (VirtualTexturePool::GetPoolAutoGrow())
		UE_LOG(
			LogAGX, Display,
			TEXT(
				"UE VT pool auto-growth is enabled. Fixed Pools are not a hard editor cap in stock "
				"UE 5.6; validate a fixed pool in a built application. AGX does not change global "
				"pool settings."));
	const FBox WorldBounds =
		FBox(
			FVector(CoverageMin.X, CoverageMin.Y, -DisplacementRange),
			FVector(CoverageMin.X + Size.X, CoverageMin.Y + Size.Y, DisplacementRange))
			.TransformBy(Bounds->Transform);
	VirtualTexture->Initialize(
		AGX_CreateTerrainVirtualTextureProducer(Data.ToSharedRef(), Description), Description,
		Volume, WorldBounds);
	LevelAddedHandle =
		FWorldDelegates::LevelAddedToWorld.AddUObject(this, &ThisClass::OnLevelAdded);
	LevelRemovedHandle =
		FWorldDelegates::LevelRemovedFromWorld.AddUObject(this, &ThisClass::OnLevelRemoved);
	ActorSpawnedHandle = GetWorld()->AddOnActorSpawnedHandler(
		FOnActorSpawned::FDelegate::CreateWeakLambda(
			this,
			[this](AActor* Actor)
			{
				if (ALandscapeProxy* Proxy = Cast<ALandscapeProxy>(Actor))
					PendingProxies.AddUnique(Proxy);
			}));
	for (TActorIterator<ALandscapeProxy> It(GetWorld()); It; ++It)
		BindProxy(**It);
}

void UAGX_TerrainPartitionedLandscapeRendererComponent::OnLevelAdded(ULevel* Level, UWorld* World)
{
	if (World != GetWorld() || Level == nullptr)
		return;
	for (AActor* Actor : Level->Actors)
		if (ALandscapeProxy* Proxy = Cast<ALandscapeProxy>(Actor))
			PendingProxies.AddUnique(Proxy);
}

void UAGX_TerrainPartitionedLandscapeRendererComponent::OnLevelRemoved(ULevel* Level, UWorld* World)
{
	if (World != GetWorld())
		return;
	RestoreMaterials(Level);
	PendingProxies.RemoveAll(
		[Level](const auto& Proxy)
		{ return !Proxy.IsValid() || Level == nullptr || Proxy->GetLevel() == Level; });
}

void UAGX_TerrainPartitionedLandscapeRendererComponent::BindProxy(ALandscapeProxy& Proxy)
{
	if (!Data.IsValid() || Proxy.GetWorld() != GetWorld() ||
		Proxy.GetLandscapeGuid() != Terrain->SourceLandscape->GetLandscapeGuid() ||
		OriginalDynamicMaterialFlags.Contains(&Proxy))
		return;
	OriginalDynamicMaterialFlags.Add(&Proxy, Proxy.bUseDynamicMaterialInstance);
	Proxy.bUseDynamicMaterialInstance = true;
	TInlineComponentArray<ULandscapeNaniteComponent*> NaniteComponents(&Proxy);
	if (NaniteComponents.IsEmpty() && !Proxy.LandscapeComponents.IsEmpty())
		UE_LOG(
			LogAGX, Warning,
			TEXT(
				"Landscape proxy '%s' has no Nanite rendering components. Enable Nanite and build "
				"Landscape Nanite data before cooking."),
			*Proxy.GetName());
	TInlineComponentArray<UPrimitiveComponent*> Components(&Proxy);
	for (UPrimitiveComponent* Primitive : Components)
	{
		ULandscapeComponent* LandscapeComponent = Cast<ULandscapeComponent>(Primitive);
		if (LandscapeComponent == nullptr && !Primitive->IsA<ULandscapeNaniteComponent>())
			continue;
		bool bPrepared = true;
		if (auto* Nanite = Cast<ULandscapeNaniteComponent>(Primitive))
		{
			UStaticMesh* Mesh = Nanite->GetStaticMesh();
			const auto* Metadata =
				Mesh == nullptr
					? nullptr
					: Cast<UAGX_TerrainNaniteDisplacementData>(Mesh->GetAssetUserDataOfClass(
						  UAGX_TerrainNaniteDisplacementData::StaticClass()));
			bPrepared = Mesh != nullptr && Metadata != nullptr && Metadata->IsCurrent(*Mesh);
			if (!bPrepared && !bWarnedPreparation)
			{
				bWarnedPreparation = true;
				UE_LOG(
					LogAGX, Error,
					TEXT(
						"Landscape Nanite mesh '%s' needs AGX deformation preparation. Use AGX "
						"Terrain mode's Prepare and Save Deformable Nanite after assigning the "
						"deformable "
						"material, then save before cooking."),
					*GetNameSafe(Mesh));
			}
		}
		FAGX_TerrainLandscapeMaterialBinding Binding;
		Binding.Component = Primitive;
		Binding.OriginalShadowCacheInvalidation =
			static_cast<uint8>(Primitive->ShadowCacheInvalidationBehavior);
		Primitive->ShadowCacheInvalidationBehavior = EShadowCacheInvalidationBehavior::Always;
		TArray<UMaterialInterface*> Parents;
		if (LandscapeComponent != nullptr)
		{
			Binding.OriginalPositiveZBoundsExtension = LandscapeComponent->PositiveZBoundsExtension;
			Binding.OriginalNegativeZBoundsExtension = LandscapeComponent->NegativeZBoundsExtension;
			LandscapeComponent->PositiveZBoundsExtension = FMath::Max(
				LandscapeComponent->PositiveZBoundsExtension,
				static_cast<float>(
					DisplacementRange /
					FMath::Max(
						FMath::Abs(LandscapeComponent->GetComponentScale().Z), SMALL_NUMBER)));
			LandscapeComponent->NegativeZBoundsExtension = FMath::Max(
				LandscapeComponent->NegativeZBoundsExtension,
				static_cast<float>(
					DisplacementRange /
					FMath::Max(
						FMath::Abs(LandscapeComponent->GetComponentScale().Z), SMALL_NUMBER)));
			LandscapeComponent->UpdateBounds();
			for (UMaterialInstanceDynamic* Original : LandscapeComponent->MaterialInstancesDynamic)
				Binding.OriginalMaterials.Add(Original);
			for (UMaterialInterface* Material : LandscapeComponent->MaterialInstances)
				Parents.Add(Material);
		}
		else
		{
			for (int32 Index = 0; Index < Primitive->GetNumMaterials(); ++Index)
			{
				UMaterialInterface* Original = Primitive->GetMaterial(Index);
				Binding.OriginalMaterials.Add(Original);
				Parents.Add(Original);
			}
		}
		for (int32 Index = 0; Index < Parents.Num(); ++Index)
		{
			UMaterialInterface* Parent = Parents[Index];
			if (Parent == nullptr)
				continue;
			UMaterialInstanceDynamic* PreviousDynamic = Cast<UMaterialInstanceDynamic>(Parent);
			while (auto* DynamicParent = Cast<UMaterialInstanceDynamic>(Parent))
				Parent = DynamicParent->Parent;
			if (Parent == nullptr)
				continue;
			UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(Parent, this);
			if (PreviousDynamic != nullptr)
				Material->CopyInterpParameters(PreviousDynamic);
			Material->SetRuntimeVirtualTextureParameterValue(
				VirtualTextureParameter, VirtualTexture);
			Material->SetScalarParameterValue(
				TEXT("AGX_DeformationEnabled"), bPrepared ? 1.f : 0.f);
			Material->SetScalarParameterValue(TEXT("AGX_DisplacementRange"), DisplacementRange);
			const FVector Origin = Data->Frame.GetTranslation();
			const FVector3f OriginHigh(Origin);
			const FVector3f OriginLow(Origin - FVector(OriginHigh));
			Material->SetVectorParameterValue(
				TEXT("AGX_HeightOriginHigh"),
				FLinearColor(OriginHigh.X, OriginHigh.Y, OriginHigh.Z));
			Material->SetVectorParameterValue(
				TEXT("AGX_HeightOriginLow"), FLinearColor(OriginLow.X, OriginLow.Y, OriginLow.Z));
			const FDisplacementScaling Scaling = Parent->GetDisplacementScaling();
			Material->SetScalarParameterValue(
				TEXT("AGX_DisplacementMagnitude"), FMath::Max(Scaling.Magnitude, SMALL_NUMBER));
			Material->SetScalarParameterValue(TEXT("AGX_DisplacementCenter"), Scaling.Center);
			const FVector AxisX = Data->Frame.GetUnitAxis(EAxis::X);
			const FVector AxisY = Data->Frame.GetUnitAxis(EAxis::Y);
			Material->SetVectorParameterValue(
				TEXT("AGX_LandscapeAxisX"), FLinearColor(AxisX.X, AxisX.Y, AxisX.Z));
			Material->SetVectorParameterValue(
				TEXT("AGX_LandscapeAxisY"), FLinearColor(AxisY.X, AxisY.Y, AxisY.Z));
			const double MinimumScale = FMath::Abs(Primitive->GetComponentScale().Z);
			const double SupportedTravel =
				FMath::Min(Scaling.Center, 1.f - Scaling.Center) * Scaling.Magnitude * MinimumScale;
			if (SupportedTravel < DisplacementRange && !bWarnedMaterial)
			{
				bWarnedMaterial = true;
				UE_LOG(
					LogAGX, Warning,
					TEXT(
						"Landscape material '%s' displacement bounds permit %.3f cm in both "
						"directions at the component's vertical scale; the renderer requests %.3f "
						"cm. Increase the material magnitude or reduce Displacement Range. "
						"Out-of-bounds material output clamps."),
					*Parent->GetName(), SupportedTravel, DisplacementRange);
			}
			if (!Parent->IsTessellationEnabled() && !bWarnedMaterial)
			{
				bWarnedMaterial = true;
				UE_LOG(
					LogAGX, Warning,
					TEXT(
						"Landscape material '%s' needs Nanite tessellation enabled, a signed "
						"displacement range and the AGX deformation material function."),
					*Parent->GetName());
			}
			Binding.Materials.Add(Material);
			if (LandscapeComponent == nullptr)
				Primitive->SetMaterial(Index, Material);
		}
		if (LandscapeComponent != nullptr)
		{
			LandscapeComponent->MaterialInstancesDynamic = Binding.Materials;
			LandscapeComponent->MarkRenderStateDirty();
		}
		Bindings.Add(MoveTemp(Binding));
	}
}

void UAGX_TerrainPartitionedLandscapeRendererComponent::Update()
{
	if (!Data.IsValid() || Terrain == nullptr || !Terrain->HasNativeTerrainPager())
		return;
	TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("AGXUnreal:PartitionedLandscapeUpdate"));
	for (auto Proxy : PendingProxies)
		if (Proxy.IsValid())
			BindProxy(*Proxy);
	PendingProxies.Reset();
	Bindings.RemoveAll([](const auto& Binding) { return !Binding.Component.IsValid(); });
	for (auto It = OriginalDynamicMaterialFlags.CreateIterator(); It; ++It)
		if (!It.Key().IsValid())
			It.RemoveCurrent();
	auto Updates = Terrain->GetNativeTerrainPager()->GetTileHeightUpdates(KnownTiles);
	{
		FWriteScopeLock Lock(Data->Lock);
		for (auto It = Tiles.CreateIterator(); It; ++It)
		{
			if (!KnownTiles.Contains(It.Key()))
			{
				// Deliberately preserve already produced GPU pages; AGX owns terrain history.
				Data->Tiles.Remove(It.Key());
				It.RemoveCurrent();
			}
		}
	}
	for (const FTerrainTileHeights& Update : Updates)
	{
		FTileState& Tile = Tiles.FindOrAdd(Update.Id);
		if (Update.bFullRefresh)
		{
			Tile = FTileState();
			Tile.Heights = Update.Heights;
			Tile.Corner = Update.Corner;
			Tile.Side = Update.SideVertices;
			Tile.Spacing = Update.Spacing;
		}
		else
		{
			for (int32 Index = 0; Index < Update.ModifiedVertices.Num(); ++Index)
			{
				const FIntPoint Vertex = Update.ModifiedVertices[Index];
				Tile.Heights[Vertex.X + Vertex.Y * Tile.Side] = Update.Heights[Index];
			}
		}
	}
	for (auto& Entry : Tiles)
	{
		FTileState& Tile = Entry.Value;
		const FTerrainTileHeights* Update = Updates.FindByPredicate(
			[&Entry](const auto& Candidate) { return Candidate.Id == Entry.Key; });
		// Heights are absolute in the pager frame. A source baseline is optional diagnostic
		// data, so a restored cached tile can render even before its source proxy arrives.
		if (!Tile.Baseline.IsValid())
			Tile.Baseline = Terrain->GetSourceTileSnapshot(Tile.Corner, Tile.Side);
		if (Update == nullptr && Data->Tiles.Contains(Entry.Key))
			continue;
		const FVector LocalCorner = Data->Frame.InverseTransformPositionNoScale(Tile.Corner);
		FVector2D DirtyMin(LocalCorner.X, LocalCorner.Y);
		FVector2D DirtyMax = DirtyMin + FVector2D((Tile.Side - 1) * Tile.Spacing);
		{
			FWriteScopeLock Lock(Data->Lock);
			const bool bFull =
				!Data->Tiles.Contains(Entry.Key) || (Update != nullptr && Update->bFullRefresh);
			auto& RenderTile = Data->Tiles.FindOrAdd(Entry.Key);
			RenderTile.Id = Entry.Key;
			RenderTile.Corner = FVector2D(LocalCorner.X, LocalCorner.Y);
			RenderTile.Spacing = Tile.Spacing;
			RenderTile.Side = Tile.Side;
			if (bFull)
			{
				RenderTile.Heights.SetNumUninitialized(Tile.Heights.Num());
				for (int32 Index = 0; Index < Tile.Heights.Num(); ++Index)
					RenderTile.Heights[Index] = Tile.Heights[Index];
			}
			else
			{
				DirtyMin = FVector2D(TNumericLimits<double>::Max());
				DirtyMax = -DirtyMin;
				for (const FIntPoint& Vertex : Update->ModifiedVertices)
				{
					const int32 Index = Vertex.X + Vertex.Y * Tile.Side;
					RenderTile.Heights[Index] = Tile.Heights[Index];
					const FVector2D Position =
						RenderTile.Corner +
						FVector2D(Vertex.X, Tile.Side - 1 - Vertex.Y) * Tile.Spacing;
					DirtyMin = FVector2D(
						FMath::Min(DirtyMin.X, Position.X), FMath::Min(DirtyMin.Y, Position.Y));
					DirtyMax = FVector2D(
						FMath::Max(DirtyMax.X, Position.X), FMath::Max(DirtyMax.Y, Position.Y));
				}
			}
		}
		// Include interpolation neighbors and the physical border. UE invalidates all affected
		// mips.
		const double Padding = FMath::Max(
			Tile.Spacing, FMath::Max(Data->CoverageSize.X, Data->CoverageSize.Y) /
							  VirtualTexture->GetSize() * 5.0);
		Invalidate(DirtyMin - FVector2D(Padding), DirtyMax + FVector2D(Padding));
	}
}

void UAGX_TerrainPartitionedLandscapeRendererComponent::Invalidate(
	const FVector2D& Min, const FVector2D& Max)
{
	const FVector2D UVMin = (Min - Data->CoverageMin) / Data->CoverageSize;
	const FVector2D UVMax = (Max - Data->CoverageMin) / Data->CoverageSize;
	URuntimeVirtualTexture* Texture = VirtualTexture;
	ENQUEUE_RENDER_COMMAND(AGX_InvalidateTerrainRVT)(
		[Texture, UVMin, UVMax](FRHICommandListImmediate&)
		{
			FModuleManager::GetModuleChecked<IRendererModule>(TEXT("Renderer"))
				.FlushVirtualTextureCache(
					Texture->GetAllocatedVirtualTexture(), FVector2f(UVMin), FVector2f(UVMax));
		});
}

void UAGX_TerrainPartitionedLandscapeRendererComponent::RestoreMaterials(ULevel* Level)
{
	Bindings.RemoveAll(
		[Level](auto& Binding)
		{
			UPrimitiveComponent* Component = Binding.Component.Get();
			if (Component == nullptr)
				return true;
			if (Level != nullptr && Component->GetOwner() != nullptr &&
				Component->GetOwner()->GetLevel() != Level)
				return false;
			Component->ShadowCacheInvalidationBehavior =
				static_cast<EShadowCacheInvalidationBehavior>(
					Binding.OriginalShadowCacheInvalidation);
			if (ULandscapeComponent* Landscape = Cast<ULandscapeComponent>(Component))
			{
				Landscape->PositiveZBoundsExtension = Binding.OriginalPositiveZBoundsExtension;
				Landscape->NegativeZBoundsExtension = Binding.OriginalNegativeZBoundsExtension;
				Landscape->UpdateBounds();
				Landscape->MaterialInstancesDynamic.Reset();
				for (UMaterialInterface* Material : Binding.OriginalMaterials)
					Landscape->MaterialInstancesDynamic.Add(
						Cast<UMaterialInstanceDynamic>(Material));
				Landscape->MarkRenderStateDirty();
			}
			else
				for (int32 Index = 0; Index < Binding.OriginalMaterials.Num(); ++Index)
					Component->SetMaterial(Index, Binding.OriginalMaterials[Index]);
			return true;
		});
	for (auto It = OriginalDynamicMaterialFlags.CreateIterator(); It; ++It)
	{
		ALandscapeProxy* Proxy = It.Key().Get();
		if (Proxy == nullptr || Level == nullptr || Proxy->GetLevel() == Level)
		{
			if (Proxy != nullptr)
				Proxy->bUseDynamicMaterialInstance = It.Value();
			It.RemoveCurrent();
		}
	}
}

void UAGX_TerrainPartitionedLandscapeRendererComponent::Shutdown()
{
	FWorldDelegates::LevelAddedToWorld.Remove(LevelAddedHandle);
	FWorldDelegates::LevelRemovedFromWorld.Remove(LevelRemovedHandle);
	if (GetWorld() != nullptr)
		GetWorld()->RemoveOnActorSpawnedHandler(ActorSpawnedHandle);
	RestoreMaterials();
	PendingProxies.Empty();
	if (VirtualTexture != nullptr)
		VirtualTexture->Release();
	VirtualTexture = nullptr;
	Data.Reset();
	Tiles.Empty();
	KnownTiles.Empty();
	Terrain = nullptr;
	bWarnedMaterial = false;
	bWarnedPreparation = false;
}

int32 UAGX_TerrainPartitionedLandscapeRendererComponent::GetRetainedTileCount() const
{
	return Tiles.Num();
}

int64 UAGX_TerrainPartitionedLandscapeRendererComponent::GetRetainedHeightBytes() const
{
	int64 Bytes = 0;
	for (const auto& Entry : Tiles)
	{
		Bytes += Entry.Value.Heights.GetAllocatedSize();
		if (Entry.Value.Baseline.IsValid())
			Bytes += Entry.Value.Baseline->Heights.GetAllocatedSize();
	}
	if (Data.IsValid())
	{
		FReadScopeLock Lock(Data->Lock);
		for (const auto& Entry : Data->Tiles)
			Bytes += Entry.Value.Heights.GetAllocatedSize();
	}
	return Bytes;
}

FVector2D UAGX_TerrainPartitionedLandscapeRendererComponent::GetActiveDisplacementExtrema() const
{
	FVector2D Extrema = FVector2D::ZeroVector;
	for (const auto& Entry : Tiles)
		if (Entry.Value.Baseline.IsValid())
			for (int32 Index = 0; Index < Entry.Value.Heights.Num(); ++Index)
			{
				const double Value =
					Entry.Value.Heights[Index] - Entry.Value.Baseline->Heights[Index];
				Extrema.X = FMath::Min(Extrema.X, Value);
				Extrema.Y = FMath::Max(Extrema.Y, Value);
			}
	return Extrema;
}

int64 UAGX_TerrainPartitionedLandscapeRendererComponent::GetGeneratedPageCount() const
{
	return Data.IsValid() ? Data->ProducedPages.load() : 0;
}

double UAGX_TerrainPartitionedLandscapeRendererComponent::GetLastPageProductionMilliseconds() const
{
	return Data.IsValid() ? Data->LastProductionMilliseconds.load() : 0.0;
}
