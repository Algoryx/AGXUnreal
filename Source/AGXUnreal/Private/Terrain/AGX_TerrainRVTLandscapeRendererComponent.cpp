// Copyright 2026, Algoryx Simulation AB.

#include "Terrain/AGX_TerrainRVTLandscapeRendererComponent.h"

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
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "Landscape.h"
#include "LandscapeComponent.h"
#include "LandscapeNaniteComponent.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Modules/ModuleManager.h"
#include "RendererInterface.h"
#include "RenderingThread.h"
#include "RHI.h"
#include "UObject/UnrealType.h"
#include "VT/RuntimeVirtualTexture.h"
#include "VT/VirtualTexturePoolConfig.h"

UAGX_TerrainRVTLandscapeRendererComponent::UAGX_TerrainRVTLandscapeRendererComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bTickEvenWhenPaused = true;
}

UAGX_TerrainRVTLandscapeRendererComponent::~UAGX_TerrainRVTLandscapeRendererComponent() = default;

void UAGX_TerrainRVTLandscapeRendererComponent::SetEnabled(bool bInEnabled)
{
	bEnabled = bInEnabled;
	if (!bEnabled)
		Shutdown();
}

void UAGX_TerrainRVTLandscapeRendererComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	Shutdown();
	Super::EndPlay(Reason);
}

void UAGX_TerrainRVTLandscapeRendererComponent::Initialize(AAGX_Terrain& InTerrain)
{
	if (Terrain != nullptr)
		return;
	Terrain = &InTerrain;
	// Keep the failed initialization associated with this enable cycle, avoiding per-frame logs.
	if (!ValidateInitialization() || !CreateVirtualTexture())
		return;
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

bool UAGX_TerrainRVTLandscapeRendererComponent::ValidateInitialization() const
{
	if (!Terrain->HasNativeTerrainPager() && Terrain->GetLandscapeSourceHeightBytes() == 0)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT(
				"Add the RVT Landscape Renderer to '%s' before creating its non-paged native "
				"Terrain."),
			*Terrain->GetName());
		return false;
	}
	if (Terrain->TerrainBounds->bInfiniteBounds)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT(
				"RVT Landscape rendering requires finite Terrain Bounds "
				"on '%s'."),
			*Terrain->GetName());
		return false;
	}
	if (PageSize < 64 || PageSize > 1024 || !FMath::IsPowerOfTwo(PageSize) ||
		!FMath::IsFinite(DisplacementRange) || DisplacementRange <= 0.0 || MaxPagesPerFrame < 1 ||
		MaxPagesPerFrame > 256)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT(
				"Invalid RVT renderer settings on '%s': page size must be a power of two "
				"(64..1024), range positive and finite, pages/frame 1..256."),
			*Terrain->GetName());
		return false;
	}
	for (const TCHAR* Setting : {TEXT("r.VirtualTextures"), TEXT("r.Nanite.Tessellation")})
	{
		const IConsoleVariable* Variable = IConsoleManager::Get().FindConsoleVariable(Setting);
		if (Variable == nullptr || Variable->GetInt() == 0)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT(
					"RVT Landscape rendering requires %s=1. Configure the project and "
					"restart the editor before playing."),
				Setting);
			return false;
		}
	}
	if (!FDataDrivenShaderPlatformInfo::GetSupportsNanite(GMaxRHIShaderPlatform))
	{
		UE_LOG(
			LogAGX, Error,
			TEXT(
				"RVT Landscape rendering requires a Nanite-capable shader platform. Use "
				"DX12/SM6 on Windows."));
		return false;
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
						"Landscape '%s' already has an active RVT Terrain renderer. Use "
						"one renderer per Landscape in a world."),
					*GetNameSafe(Terrain->SourceLandscape));
				return false;
			}
		}
	}
	return true;
}

bool UAGX_TerrainRVTLandscapeRendererComponent::CreateVirtualTexture()
{
	const auto Bounds = Terrain->TerrainBounds->GetUserSetBounds();
	if (!Bounds.IsSet() || Terrain->SourceLandscape == nullptr)
		return false;
	const bool bPaged = Terrain->HasNativeTerrainPager();
	const FTransform Frame =
		bPaged
			? Bounds->Transform
			: FTransform(Terrain->GetNative()->GetRotation(), Terrain->GetNative()->GetPosition());
	const FVector2D HalfExtent = bPaged ? FVector2D(Bounds->HalfExtent.X, Bounds->HalfExtent.Y)
										: Terrain->GetNative()->GetSize() * 0.5;
	const int32 TileCountLog2 = ConfigureVirtualTextureCoverage(Frame, HalfExtent);
	const FVector2D& CoverageMin = Data->CoverageMin;
	const FVector2D& Size = Data->CoverageSize;
	UAGX_TerrainDisplacementVirtualTexture* RuntimeTexture =
		NewObject<UAGX_TerrainDisplacementVirtualTexture>(this);
	RuntimeTexture->Configure(PageSize, TileCountLog2);
	VirtualTexture = RuntimeTexture;
	const FTransform Volume(
		Frame.GetRotation(),
		Frame.TransformPositionNoScale(FVector(CoverageMin.X, CoverageMin.Y, -DisplacementRange)),
		FVector(Size.X, Size.Y, 2.0 * DisplacementRange));
	FVTProducerDescription Description;
	RuntimeTexture->GetProducerDescription(
		Description, URuntimeVirtualTexture::FInitSettings(), Volume);
	// Compensate this producer's tile-count bias without changing project/device settings.
	// The coverage must retain exactly the native-vertex/texel-center lattice on both axes.
	const uint32 RequiredSide = static_cast<uint32>(PageSize) * (1u << TileCountLog2);
	URuntimeVirtualTexture::FInitSettings InitSettings;
	const uint32 EffectiveSide = Description.BlockWidthInTiles * Description.TileSize;
	InitSettings.TileCountBias =
		FMath::RoundToInt(FMath::Log2(static_cast<double>(RequiredSide) / EffectiveSide));
	RuntimeTexture->GetProducerDescription(Description, InitSettings, Volume);
	if (Description.TileSize != static_cast<uint32>(PageSize) ||
		Description.BlockWidthInTiles * Description.TileSize != RequiredSide ||
		Description.BlockHeightInTiles * Description.TileSize != RequiredSide)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT(
				"Terrain '%s' cannot preserve its RVT sampling lattice under "
				"the active device profile. Check the virtual texture tile-size bias, coverage and "
				"Page Size."),
			*Terrain->GetName());
		VirtualTexture = nullptr;
		Data.Reset();
		return false;
	}
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
			.TransformBy(Frame);
	VirtualTexture->Initialize(
		AGX_CreateTerrainVirtualTextureProducer(Data.ToSharedRef(), Description), Description,
		Volume, WorldBounds);
	return true;
}

int32 UAGX_TerrainRVTLandscapeRendererComponent::ConfigureVirtualTextureCoverage(
	const FTransform& Frame, const FVector2D& HalfExtent)
{
	const double PhysicsSpacing = Terrain->HasNativeTerrainPager()
									  ? Terrain->GetSimulationResolution()
									  : Terrain->GetNative()->GetElementSize();
	const double SourceSpacing = FMath::Min(
		Terrain->SourceLandscape->GetActorScale3D().X,
		Terrain->SourceLandscape->GetActorScale3D().Y);
	// Native vertices must fall on RVT texel centers. Resampling onto an unrelated power-of-two
	// lattice smooths sharp cuts and cannot provide accurate visual/physics agreement.
	double TexelSpacing =
		PhysicsSpacing / FMath::CeilToDouble(FMath::Max(1.0, PhysicsSpacing / SourceSpacing));
	const FVector GridOrigin3D = Frame.InverseTransformPositionNoScale(
		Terrain->HasNativeTerrainPager()
			? Terrain->GetNativeTerrainPager()->GetTileGridOrigin()
			: Frame.TransformPositionNoScale(FVector(-HalfExtent.X, -HalfExtent.Y, 0)));
	const FVector2D GridOrigin(GridOrigin3D.X, GridOrigin3D.Y);
	auto FirstVertex = [&GridOrigin, &HalfExtent](double Spacing)
	{
		return GridOrigin + FVector2D(
								FMath::FloorToDouble((-HalfExtent.X - GridOrigin.X) / Spacing),
								FMath::FloorToDouble((-HalfExtent.Y - GridOrigin.Y) / Spacing)) *
								Spacing;
	};
	auto RequiredSamples = [&HalfExtent, &FirstVertex](double Spacing)
	{
		const FVector2D First = FirstVertex(Spacing);
		return FMath::CeilToDouble(
				   FMath::Max(HalfExtent.X - First.X, HalfExtent.Y - First.Y) / Spacing) +
			   1.0;
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
	int32 TileCountLog2 =
		FMath::Clamp(FMath::CeilToInt(FMath::Log2(FMath::Max(1.0, RequiredPages))), 0, MaxLog2);
	const FVector2D Size(static_cast<double>(PageSize) * (1 << TileCountLog2) * TexelSpacing);
	const FVector2D CoverageMin = FirstVertex(TexelSpacing) - FVector2D(TexelSpacing * 0.5);
	Data = MakeShared<FAGX_TerrainVirtualTextureData, ESPMode::ThreadSafe>();
	Data->Frame = Frame;
	Data->CoverageMin = CoverageMin;
	Data->CoverageSize = Size;
	Data->TerrainHalfExtent = HalfExtent;
	Data->MaxPagesPerFrame = MaxPagesPerFrame;
	return TileCountLog2;
}

void UAGX_TerrainRVTLandscapeRendererComponent::OnLevelAdded(ULevel* Level, UWorld* World)
{
	if (World != GetWorld() || Level == nullptr)
		return;
	for (AActor* Actor : Level->Actors)
		if (ALandscapeProxy* Proxy = Cast<ALandscapeProxy>(Actor))
			PendingProxies.AddUnique(Proxy);
}

void UAGX_TerrainRVTLandscapeRendererComponent::OnLevelRemoved(ULevel* Level, UWorld* World)
{
	if (World != GetWorld())
		return;
	RestoreMaterials(Level);
	PendingProxies.RemoveAll(
		[Level](const auto& Proxy)
		{ return !Proxy.IsValid() || Level == nullptr || Proxy->GetLevel() == Level; });
}

void UAGX_TerrainRVTLandscapeRendererComponent::BindProxy(ALandscapeProxy& Proxy)
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
	// Bind the ordinary components first, then share their live MIDs with Nanite sections.
	for (ULandscapeComponent* Component : Proxy.LandscapeComponents)
		if (Component != nullptr)
			BindLandscapeMaterials(*Component);
	for (ULandscapeNaniteComponent* Component : NaniteComponents)
		BindNaniteMaterials(*Component);
}

void UAGX_TerrainRVTLandscapeRendererComponent::BindLandscapeMaterials(
	ULandscapeComponent& Component)
{
	UPrimitiveComponent* Primitive = &Component;
	FAGX_TerrainLandscapeMaterialBinding Binding;
	Binding.Component = Primitive;
	Binding.OriginalShadowCacheInvalidation =
		static_cast<uint8>(Primitive->ShadowCacheInvalidationBehavior);
	Primitive->ShadowCacheInvalidationBehavior = EShadowCacheInvalidationBehavior::Always;
	TArray<UMaterialInterface*> Parents;
	Binding.OriginalPositiveZBoundsExtension = Component.PositiveZBoundsExtension;
	Binding.OriginalNegativeZBoundsExtension = Component.NegativeZBoundsExtension;
	Component.PositiveZBoundsExtension = FMath::Max(
		Component.PositiveZBoundsExtension,
		static_cast<float>(
			DisplacementRange /
			FMath::Max(FMath::Abs(Component.GetComponentScale().Z), SMALL_NUMBER)));
	Component.NegativeZBoundsExtension = FMath::Max(
		Component.NegativeZBoundsExtension,
		static_cast<float>(
			DisplacementRange /
			FMath::Max(FMath::Abs(Component.GetComponentScale().Z), SMALL_NUMBER)));
	Component.UpdateBounds();
	for (UMaterialInstanceDynamic* Original : Component.MaterialInstancesDynamic)
		Binding.OriginalMaterials.Add(Original);
	for (int32 Index = 0; Index < Component.MaterialInstances.Num(); ++Index)
		Parents.Add(
			Component.MaterialInstancesDynamic.IsValidIndex(Index) &&
					Component.MaterialInstancesDynamic[Index] != nullptr
				? static_cast<UMaterialInterface*>(Component.MaterialInstancesDynamic[Index].Get())
				: Component.MaterialInstances[Index].Get());
	for (UMaterialInterface* Parent : Parents)
	{
		UMaterialInstanceDynamic* Material =
			CreateLandscapeMaterial(Parent, FMath::Abs(Component.GetComponentScale().Z));
		if (Material != nullptr)
			Binding.Materials.Add(Material);
	}
	Component.MaterialInstancesDynamic = Binding.Materials;
	Component.MarkRenderStateDirty();
	Bindings.Add(MoveTemp(Binding));
}

UMaterialInstanceDynamic* UAGX_TerrainRVTLandscapeRendererComponent::CreateLandscapeMaterial(
	UMaterialInterface* Parent, double VerticalScale)
{
	if (Parent == nullptr)
		return nullptr;
	UMaterialInstanceDynamic* PreviousDynamic = Cast<UMaterialInstanceDynamic>(Parent);
	while (auto* DynamicParent = Cast<UMaterialInstanceDynamic>(Parent))
		Parent = DynamicParent->Parent;
	if (Parent == nullptr)
		return nullptr;
	UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(Parent, this);
	if (PreviousDynamic != nullptr)
		Material->CopyMaterialUniformParameters(PreviousDynamic);
	Material->SetRuntimeVirtualTextureParameterValue(VirtualTextureParameter, VirtualTexture);
	Material->SetScalarParameterValue(TEXT("AGX_DeformationEnabled"), 1.f);
	Material->SetScalarParameterValue(TEXT("AGX_DisplacementRange"), DisplacementRange);
	const FVector Origin = Data->Frame.GetTranslation();
	const FVector3f OriginHigh(Origin);
	const FVector3f OriginLow(Origin - FVector(OriginHigh));
	Material->SetVectorParameterValue(
		TEXT("AGX_HeightOriginHigh"), FLinearColor(OriginHigh.X, OriginHigh.Y, OriginHigh.Z));
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
	Material->SetVectorParameterValue(
		TEXT("AGX_TerrainHalfExtent"),
		FLinearColor(Data->TerrainHalfExtent.X, Data->TerrainHalfExtent.Y, 0, 0));
	const double SupportedTravel =
		FMath::Min(Scaling.Center, 1.f - Scaling.Center) * Scaling.Magnitude * VerticalScale;
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
	return Material;
}

void UAGX_TerrainRVTLandscapeRendererComponent::BindNaniteMaterials(
	ULandscapeNaniteComponent& Component)
{
	FAGX_TerrainLandscapeMaterialBinding Binding;
	Binding.Component = &Component;
	Binding.OriginalShadowCacheInvalidation =
		static_cast<uint8>(Component.ShadowCacheInvalidationBehavior);
	Component.ShadowCacheInvalidationBehavior = EShadowCacheInvalidationBehavior::Always;
	for (int32 Index = 0; Index < Component.GetNumMaterials(); ++Index)
		Binding.OriginalMaterials.Add(Component.GetMaterial(Index));
	UStaticMesh* Mesh = Component.GetStaticMesh();
	const auto* Metadata =
		Mesh == nullptr ? nullptr
						: Cast<UAGX_TerrainNaniteDisplacementData>(Mesh->GetAssetUserDataOfClass(
							  UAGX_TerrainNaniteDisplacementData::StaticClass()));
	const bool bPrepared = Metadata != nullptr && Metadata->IsCurrent(*Mesh);
	if (!bPrepared && !bWarnedPreparation)
	{
		bWarnedPreparation = true;
		UE_LOG(
			LogAGX, Error,
			TEXT(
				"Landscape Nanite mesh '%s' needs AGX deformation "
				"preparation. Use AGX Terrain mode's Prepare and Save Deformable Nanite "
				"after assigning the deformable material, then save before cooking."),
			*GetNameSafe(Mesh));
	}
	const auto& Sources = Component.GetSourceLandscapeComponents();
	for (int32 Index = 0; Index < Sources.Num(); ++Index)
	{
		ULandscapeComponent* Source = Sources[Index];
		if (Source == nullptr || Source->MaterialInstancesDynamic.IsEmpty())
			continue;
		UMaterialInstanceDynamic* Material = Source->MaterialInstancesDynamic[0];
		if (Material == nullptr)
			continue;
		Material->SetScalarParameterValue(TEXT("AGX_DeformationEnabled"), bPrepared ? 1.f : 0.f);
		Component.SetMaterial(Index, Material);
		Binding.Materials.Add(Material);
	}
	Bindings.Add(MoveTemp(Binding));
}

void UAGX_TerrainRVTLandscapeRendererComponent::Update()
{
	if (!Data.IsValid() || Terrain == nullptr || !Terrain->HasNative())
		return;
	TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("AGXUnreal:RVTLandscapeUpdate"));
	for (auto Proxy : PendingProxies)
		if (Proxy.IsValid())
			BindProxy(*Proxy);
	PendingProxies.Reset();
	Bindings.RemoveAll([](const auto& Binding) { return !Binding.Component.IsValid(); });
	for (auto It = OriginalDynamicMaterialFlags.CreateIterator(); It; ++It)
		if (!It.Key().IsValid())
			It.RemoveCurrent();

	auto Updates = Terrain->GetLandscapeRenderUpdates(KnownTiles);
	TArray<FIntPoint> Removed;
	for (auto It = Tiles.CreateIterator(); It; ++It)
		if (!KnownTiles.Contains(It.Key()))
		{
			Removed.Add(It.Key());
			It.RemoveCurrent();
		}
	TArray<FBox2D> Regions;
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
			for (int32 Index = 0; Index < Update.ModifiedVertices.Num(); ++Index)
			{
				const FIntPoint Vertex = Update.ModifiedVertices[Index];
				Tile.Heights[Vertex.X + Vertex.Y * Tile.Side] = Update.Heights[Index];
			}
		const FVector Local = Data->Frame.InverseTransformPositionNoScale(Tile.Corner);
		const FVector2D Corner(Local.X, Local.Y);
		if (Update.bFullRefresh)
			Regions.Emplace(
				Corner - FVector2D(Tile.Spacing), Corner + FVector2D(Tile.Side * Tile.Spacing));
		else
		{
			// Separate small edits rather than enclosing distant modified vertices in one box.
			TMap<FIntPoint, FBox2D> Blocks;
			for (const FIntPoint& Vertex : Update.ModifiedVertices)
			{
				const FVector2D Position =
					Corner + FVector2D(Vertex.X, Tile.Side - 1 - Vertex.Y) * Tile.Spacing;
				Blocks.FindOrAdd(Vertex / 64, FBox2D(ForceInit)) += Position;
			}
			for (const auto& Block : Blocks)
				Regions.Emplace(
					Block.Value.Min - FVector2D(Tile.Spacing),
					Block.Value.Max + FVector2D(Tile.Spacing));
		}
	}
	for (auto& Entry : Tiles)
	{
		auto& Tile = Entry.Value;
		const bool bNewBaseline = !Tile.Baseline.IsValid();
		if (bNewBaseline)
			Tile.Baseline = Terrain->GetSourceTileSnapshot(Tile.Corner, Tile.Side);
		if (bWarnedClipping || !Tile.Baseline.IsValid())
			continue;
		const auto* Update = Updates.FindByPredicate([&Entry](const auto& Candidate)
													 { return Candidate.Id == Entry.Key; });
		auto Check = [&](int32 Index)
		{
			if (FMath::Abs(Tile.Heights[Index] - Tile.Baseline->Heights[Index]) > DisplacementRange)
				bWarnedClipping = true;
		};
		if (bNewBaseline || (Update != nullptr && Update->bFullRefresh))
			for (int32 Index = 0; Index < Tile.Heights.Num(); ++Index)
				Check(Index);
		else if (Update != nullptr)
			for (const FIntPoint& Vertex : Update->ModifiedVertices)
				Check(Vertex.X + Vertex.Y * Tile.Side);
		if (bWarnedClipping)
			UE_LOG(
				LogAGX, Warning,
				TEXT(
					"Terrain '%s' deformation exceeds Displacement Range "
					"(%.3f cm). Rendering clamps; physics and retained heights are unchanged. "
					"Increase the renderer range and material displacement bounds before playing."),
				*Terrain->GetName(), DisplacementRange);
	}
	if (!Updates.IsEmpty() || !Removed.IsEmpty())
	{
		ENQUEUE_RENDER_COMMAND(AGX_PublishTerrainHeights)(
			[RenderData = Data.ToSharedRef(), Updates = MoveTemp(Updates),
			 Removed = MoveTemp(Removed)](FRHICommandListImmediate&)
			{
				for (const FIntPoint& Id : Removed)
					RenderData->Tiles.Remove(Id);
				for (const FTerrainTileHeights& Update : Updates)
				{
					auto& Tile = RenderData->Tiles.FindOrAdd(Update.Id);
					if (Update.bFullRefresh)
					{
						const FVector Local =
							RenderData->Frame.InverseTransformPositionNoScale(Update.Corner);
						Tile.Id = Update.Id;
						Tile.Corner = FVector2D(Local.X, Local.Y);
						Tile.Spacing = Update.Spacing;
						Tile.Side = Update.SideVertices;
						Tile.Heights = Update.Heights;
						Tile.UpdateMips();
					}
					else
					{
						for (int32 Index = 0; Index < Update.ModifiedVertices.Num(); ++Index)
						{
							const FIntPoint Vertex = Update.ModifiedVertices[Index];
							Tile.Heights[Vertex.X + Vertex.Y * Tile.Side] = Update.Heights[Index];
						}
						Tile.UpdateMips(Update.ModifiedVertices);
					}
				}
				int64 Bytes = 0;
				for (const auto& Entry : RenderData->Tiles)
				{
					Bytes += Entry.Value.Heights.GetAllocatedSize();
					for (const auto& Mip : Entry.Value.Mips)
						Bytes += Mip.Heights.GetAllocatedSize();
				}
				RenderData->RetainedHeightBytes = Bytes;
			});
	}
	// UE expands each invalidation for the physical border at every affected mip.
	for (const FBox2D& Region : Regions)
		Invalidate(Region.Min, Region.Max);
}

void UAGX_TerrainRVTLandscapeRendererComponent::TickComponent(
	float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* TickFunction)
{
	Super::TickComponent(DeltaTime, TickType, TickFunction);
	RequestPages();
}

void UAGX_TerrainRVTLandscapeRendererComponent::RequestPages()
{
	if (!Data.IsValid() || VirtualTexture == nullptr || Tiles.IsEmpty())
		return;
	TArray<FVector> Views;
	for (auto It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		if (It->IsValid() && It->Get()->IsLocalController())
		{
			FVector Position;
			FRotator Rotation;
			It->Get()->GetPlayerViewPoint(Position, Rotation);
			Views.Add(Position);
		}
	struct FRequest
	{
		FVector2D Min;
		FVector2D Max;
		int32 Mip;
	};
	TArray<FRequest> Requests;
	for (const auto& Entry : Tiles)
	{
		const auto& Tile = Entry.Value;
		const double Side = (Tile.Side - 1) * Tile.Spacing;
		const FVector Center =
			Tile.Corner + Data->Frame.TransformVectorNoScale(FVector(Side * 0.5, Side * 0.5, 0));
		double Distance = 0.0;
		if (!Views.IsEmpty())
		{
			Distance = TNumericLimits<double>::Max();
			for (const FVector& View : Views)
				Distance = FMath::Min(Distance, FVector::Distance(View, Center));
		}
		// Conservative near-field requests, independently of material pixel-shader feedback.
		// Include all local players; headless/capture-only worlds request the finest level.
		const int32 Mip =
			FMath::Max(0, FMath::FloorToInt(FMath::Log2(FMath::Max(1.0, Distance / 15000.0))));
		const FVector Local = Data->Frame.InverseTransformPositionNoScale(Tile.Corner);
		const FVector2D Min =
			(FVector2D(Local.X, Local.Y) - Data->CoverageMin) / Data->CoverageSize;
		const FVector2D Max = Min + FVector2D(Side) / Data->CoverageSize;
		Requests.Add(
			{FVector2D(FMath::Clamp(Min.X, 0.0, 1.0), FMath::Clamp(Min.Y, 0.0, 1.0)),
			 FVector2D(FMath::Clamp(Max.X, 0.0, 1.0), FMath::Clamp(Max.Y, 0.0, 1.0)), Mip});
	}
	ENQUEUE_RENDER_COMMAND(AGX_RequestTerrainPages)(
		[Texture = VirtualTexture, Requests = MoveTemp(Requests)](FRHICommandListImmediate&)
		{
			auto* Allocation = Texture->GetAllocatedVirtualTexture();
			if (Allocation == nullptr)
				return;
			auto& Renderer = FModuleManager::GetModuleChecked<IRendererModule>(TEXT("Renderer"));
			for (const auto& Request : Requests)
				Renderer.RequestVirtualTextureTiles(
					Allocation, FVector2D(1024, 1024), FVector2D::ZeroVector, FVector2D(1024, 1024),
					Request.Min, Request.Max, Request.Mip);
		});
}

#if WITH_EDITOR
bool UAGX_TerrainRVTLandscapeRendererComponent::CanEditChange(const FProperty* Property) const
{
	if (GetWorld() != nullptr && GetWorld()->IsGameWorld())
	{
		const FName Name = Property->GetFName();
		if (Name == GET_MEMBER_NAME_CHECKED(ThisClass, PageSize) ||
			Name == GET_MEMBER_NAME_CHECKED(ThisClass, DisplacementRange) ||
			Name == GET_MEMBER_NAME_CHECKED(ThisClass, MaxPagesPerFrame) ||
			Name == GET_MEMBER_NAME_CHECKED(ThisClass, VirtualTextureParameter))
			return false;
	}
	return Super::CanEditChange(Property);
}

void UAGX_TerrainRVTLandscapeRendererComponent::PostEditChangeChainProperty(
	FPropertyChangedChainEvent& Event)
{
	Super::PostEditChangeChainProperty(Event);
	if (Event.GetPropertyName() == GET_MEMBER_NAME_CHECKED(ThisClass, bEnabled))
		SetEnabled(bEnabled);
}
#endif

void UAGX_TerrainRVTLandscapeRendererComponent::Invalidate(
	const FVector2D& Min, const FVector2D& Max)
{
	const FVector2D UVMin = (Min - Data->CoverageMin) / Data->CoverageSize;
	const FVector2D UVMax = (Max - Data->CoverageMin) / Data->CoverageSize;
	URuntimeVirtualTexture* Texture = VirtualTexture;
	ENQUEUE_RENDER_COMMAND(AGX_InvalidateTerrainRVT)(
		[Texture, UVMin, UVMax](FRHICommandListImmediate&)
		{
			if (Texture->GetAllocatedVirtualTexture() == nullptr)
				return;
			FModuleManager::GetModuleChecked<IRendererModule>(TEXT("Renderer"))
				.FlushVirtualTextureCache(
					Texture->GetAllocatedVirtualTexture(), FVector2f(UVMin), FVector2f(UVMax));
		});
}

void UAGX_TerrainRVTLandscapeRendererComponent::RestoreMaterials(ULevel* Level)
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

void UAGX_TerrainRVTLandscapeRendererComponent::Shutdown()
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
	bWarnedClipping = false;
}

int32 UAGX_TerrainRVTLandscapeRendererComponent::GetRetainedTileCount() const
{
	return Tiles.Num();
}

int64 UAGX_TerrainRVTLandscapeRendererComponent::GetRetainedHeightBytes() const
{
	int64 Bytes = 0;
	for (const auto& Entry : Tiles)
	{
		Bytes += Entry.Value.Heights.GetAllocatedSize();
		if (Entry.Value.Baseline.IsValid())
			Bytes += Entry.Value.Baseline->Heights.GetAllocatedSize();
	}
	if (Data.IsValid())
		Bytes += Data->RetainedHeightBytes.load();
	if (Terrain != nullptr)
		Bytes += Terrain->GetLandscapeSourceHeightBytes();
	return Bytes;
}

FVector2D UAGX_TerrainRVTLandscapeRendererComponent::GetActiveDisplacementExtrema() const
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

int64 UAGX_TerrainRVTLandscapeRendererComponent::GetGeneratedPageCount() const
{
	return Data.IsValid() ? Data->ProducedPages.load() : 0;
}

double UAGX_TerrainRVTLandscapeRendererComponent::GetLastPageProductionMilliseconds() const
{
	return Data.IsValid() ? Data->LastProductionMilliseconds.load() : 0.0;
}
