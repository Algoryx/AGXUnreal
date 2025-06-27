#include "Terrain/TerrainPagerBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGXBarrierFactories.h"
#include "BarrierOnly/AGXRefs.h"
#include "AGX_Check.h"
#include "RigidBodyBarrier.h"
#include "Terrain/ShovelBarrier.h"
#include "Terrain/TerrainBarrier.h"
#include "Terrain/TerrainDataSource.h"
#include "Terrain/TerrainHeightFetcherBase.h"
#include "TypeConversions.h"
#include "Utilities/TerrainUtilities.h"

FTerrainPagerBarrier::FTerrainPagerBarrier()
	: NativeRef {new FTerrainPagerRef}
{
}

FTerrainPagerBarrier::FTerrainPagerBarrier(std::unique_ptr<FTerrainPagerRef> InNativeRef)
	: NativeRef {std::move(InNativeRef)}
{
}

FTerrainPagerBarrier::FTerrainPagerBarrier(FTerrainPagerBarrier&& Other)
	: NativeRef {std::move(Other.NativeRef)}
{
}

FTerrainPagerBarrier::~FTerrainPagerBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the
	// std::unique_ptr NativeRef's destructor must be able to see the definition,
	// not just the forward declaration, of FTerrainPagerRef.
}

namespace TerrainPagerBarrier_helpers
{
	bool DoesExistModifiedHeights(
		const agxTerrain::TerrainPager::TileAttachmentPtrVector& ActiveTiles)
	{
		for (agxTerrain::TerrainPager::TileAttachments* Tile : ActiveTiles)
		{
			if (Tile == nullptr || Tile->m_terrainTile == nullptr)
				continue;

			// getModifiedVertices simply returns a reference to a member of AGX Terrain, i.e. is
			// fast.
			if (Tile->m_terrainTile->getModifiedVertices().size() > 0)
				return true;
		}

		return false;
	}

	agxTerrain::Terrain* GetFirstValidTerrainFrom(
		const agxTerrain::TerrainPager::TileAttachmentPtrVector& ActiveTiles)
	{
		for (agxTerrain::TerrainPager::TileAttachments* Tile : ActiveTiles)
		{
			if (Tile != nullptr && Tile->m_terrainTile != nullptr)
				return Tile->m_terrainTile;
		}

		return nullptr;
	}

	void SetCanCollide(
		const agxTerrain::TerrainPager::TileAttachmentPtrVector& ActiveTiles, bool bCanCollide)
	{
		for (agxTerrain::TerrainPager::TileAttachments* Tile : ActiveTiles)
		{
			if (Tile == nullptr || Tile->m_terrainTile == nullptr)
				continue;

			if (agxCollide::Geometry* Geom = Tile->m_terrainTile->getGeometry())
				Geom->setEnableCollisions(bCanCollide);
		}
	}
}

bool FTerrainPagerBarrier::HasNative() const
{
	return NativeRef->Native != nullptr;
}

void FTerrainPagerBarrier::AllocateNative(
	FTerrainHeightFetcherBase* HeightFetcher, FTerrainBarrier& TerrainBarrier,
	int32 TileSideVertices, int32 TileOverlapVerties, double ElementSize, double MaxDepth)
{
	check(TerrainBarrier.HasNative());

	if (HeightFetcher == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("TerrainPager got nullptr HeightFetcher when allocating native. AGX Dynamics will "
				 "not be able to fetch heights from the Landscape."));
	}

	// Create a TerrainDataSource and assign the HeightFetcher to it. This HeightFetcher is owned by
	// the UAGX_Terrain and is a way for us to call UAGX_Terrain::FetchHeights from the Barrier
	// module.
	DataSourceRef = std::make_unique<FTerrainDataSourceRef>();
	{
		FTerrainDataSource* DataSource = new FTerrainDataSource();
		DataSource->SetTerrainHeightFetcher(HeightFetcher);
		DataSourceRef->Native = DataSource;
	}

	// Use the same position/rotation as the given Terrain.
	const agx::Vec3 Position = ConvertDisplacement(TerrainBarrier.GetPosition());
	const agx::Quat Rotation = Convert(TerrainBarrier.GetRotation());
	const agx::Real ElementSizeAGX = ConvertDistanceToAGX(ElementSize);
	const agx::Real MaxDepthAGX = ConvertDistanceToAGX(MaxDepth);

	NativeRef->Native = new agxTerrain::TerrainPager(
		TileSideVertices, TileOverlapVerties, ElementSizeAGX, MaxDepthAGX, Position, Rotation,
		TerrainBarrier.GetNative()->Native);

	NativeRef->Native->setTerrainDataSource(DataSourceRef->Native);
}

FTerrainPagerRef* FTerrainPagerBarrier::GetNative()
{
	check(HasNative());
	return NativeRef.get();
}

const FTerrainPagerRef* FTerrainPagerBarrier::GetNative() const
{
	check(HasNative());
	return NativeRef.get();
}

void FTerrainPagerBarrier::ReleaseNative()
{
	check(HasNative());
	NativeRef->Native = nullptr;
}

void FTerrainPagerBarrier::SetCanCollide(bool bCanCollide)
{
	// AGX Dynamics does not provide a clean way to disable collisions when using the Terrain pager,
	// instead we need to do it "manually". The approach here is that if bCanCollide is false, we
	// first stop the Terrain pager from paging in new Terrain tiles. Then, we loop over any
	// existing Terrain tile and disable collision on it. And conversely, if bCanCollide is true, we
	// enable collision on all existing Terrain tile, and then enable the Terrain pager tiling.
	// The disabling and enabling of the Terrain pager tiling when enabling/disabling collision is
	// to simplify state handling. If we do not do this, we could disable collision for a tile that
	// is tiled-out and serialized to disk, and then that could be read back long in the future with
	// collision disabled, even when the user might have long since re-enabled collision on the
	// Terrain pager.
	//
	// In the future, we could consider using AGX Terrain Pager's TileLoadEvent to get a callback
	// here in the Barrier, and then, similarly to how we get data using the HeightFetcher, we
	// could have a properties fetcher (or something like that) to get properties from the Terrain
	// Actor that could then be applied to the tile when the TileLoadEvent is called.
	check(HasNative());
	if (bCanCollide)
	{
		TerrainPagerBarrier_helpers::SetCanCollide(
			NativeRef->Native->getActiveTileAttachments(), true);
		NativeRef->Native->setEnable(true);
	}
	else
	{
		NativeRef->Native->setEnable(false);
		TerrainPagerBarrier_helpers::SetCanCollide(
			NativeRef->Native->getActiveTileAttachments(), false);
	}
}

bool FTerrainPagerBarrier::AddShovel(
	FShovelBarrier& Shovel, double RequiredRadius, double PreloadRadius)
{
	check(HasNative());
	check(Shovel.HasNative());

	return NativeRef->Native->add(
		Shovel.GetNative()->Native, ConvertDistanceToAGX(RequiredRadius),
		ConvertDistanceToAGX(PreloadRadius));
}

bool FTerrainPagerBarrier::AddRigidBody(
	FRigidBodyBarrier& Body, double RequiredRadius, double PreloadRadius)
{
	check(HasNative());
	check(Body.HasNative());

	return NativeRef->Native->add(
		Body.GetNative()->Native, ConvertDistanceToAGX(RequiredRadius),
		ConvertDistanceToAGX(PreloadRadius));
}

bool FTerrainPagerBarrier::SetTileLoadRadii(
	FRigidBodyBarrier& Body, double RequiredRadius, double PreloadRadius)
{
	check(HasNative());
	check(Body.HasNative());
	return NativeRef->Native->setTileLoadRadiuses(
		Body.GetNative()->Native, ConvertDistanceToAGX(RequiredRadius),
		ConvertDistanceToAGX(PreloadRadius));
}

FParticleData FTerrainPagerBarrier::GetParticleData() const
{
	using namespace agxTerrain;
	using namespace TerrainPagerBarrier_helpers;
	check(HasNative());

	FParticleData ParticleData;
	const size_t NumParticles = GetNumParticles();
	ParticleData.Positions.Reserve(NumParticles);
	ParticleData.Radii.Reserve(NumParticles);
	ParticleData.Rotations.Reserve(NumParticles);

	const TerrainPager::TileAttachmentPtrVector ActiveTiles =
		NativeRef->Native->getActiveTileAttachments();
	if (Terrain* Terrain = GetFirstValidTerrainFrom(ActiveTiles))
	{
		const FTerrainBarrier TerrainBarrier = AGXBarrierFactories::CreateTerrainBarrier(Terrain);
		EParticleDataFlags ToInclude = EParticleDataFlags::Positions |
									   EParticleDataFlags::Rotations | EParticleDataFlags::Radii |
									   EParticleDataFlags::Velocities;
		FTerrainUtilities::AppendParticleData(TerrainBarrier, ParticleData, ToInclude);
	}

	return ParticleData;
}

FParticleDataById FTerrainPagerBarrier::GetParticleDataById(EParticleDataFlags ToInclude) const
{
	using namespace agxTerrain;
	using namespace TerrainPagerBarrier_helpers;
	check(HasNative());

	FParticleDataById ParticleData;
	const TerrainPager::TileAttachmentPtrVector ActiveTiles =
		NativeRef->Native->getActiveTileAttachments();
	if (Terrain* Terrain = GetFirstValidTerrainFrom(ActiveTiles))
	{
		const FTerrainBarrier TerrainBarrier = AGXBarrierFactories::CreateTerrainBarrier(Terrain);

		FTerrainUtilities::GetParticleExistsById(TerrainBarrier, ParticleData.Exists);
		if (ToInclude & EParticleDataFlags::Positions)
		{
			FTerrainUtilities::GetParticlePositionsById(TerrainBarrier, ParticleData.Positions);
		}
		if (ToInclude & EParticleDataFlags::Velocities)
		{
			FTerrainUtilities::GetParticleVelocitiesById(TerrainBarrier, ParticleData.Velocities);
		}
		if (ToInclude & EParticleDataFlags::Rotations)
		{
			FTerrainUtilities::GetParticleRotationsById(TerrainBarrier, ParticleData.Rotations);
		}
		if (ToInclude & EParticleDataFlags::Radii)
		{
			FTerrainUtilities::GetParticleRadiiById(TerrainBarrier, ParticleData.Radii);
		}
		if (ToInclude & EParticleDataFlags::Masses)
		{
			FTerrainUtilities::GetParticleMassesById(TerrainBarrier, ParticleData.Masses);
		}
	}

	return ParticleData;
}

size_t FTerrainPagerBarrier::GetNumParticles() const
{
	check(HasNative());
	using namespace TerrainPagerBarrier_helpers;

	const agxTerrain::TerrainPager::TileAttachmentPtrVector ActiveTiles =
		NativeRef->Native->getActiveTileAttachments();

	if (agxTerrain::Terrain* Terrain = GetFirstValidTerrainFrom(ActiveTiles))
		return Terrain->getSoilSimulationInterface()->getNumSoilParticles();
	else
		return 0;
}

TArray<std::tuple<int32, int32>> FTerrainPagerBarrier::GetModifiedHeights(
	TArray<float>& OutHeights, int32 BoundVertsX, int32 BoundVertsY) const
{
	using namespace TerrainPagerBarrier_helpers;
	check(HasNative());

	TArray<std::tuple<int32, int32>> ModifiedVertices;
	const agxTerrain::TerrainPager::TileAttachmentPtrVector ActiveTiles =
		NativeRef->Native->getActiveTileAttachments();

	if (!DoesExistModifiedHeights(ActiveTiles))
		return ModifiedVertices;

	const int32 BoundsCornerToCenterOffsX = BoundVertsX / 2;
	const int32 BoundsCornerToCenterOffsY = BoundVertsY / 2;

	const agxTerrain::TileSpecification& TileSpec = NativeRef->Native->getTileSpecification();
	const int32 NumVertsPerTile = static_cast<int32>(TileSpec.getTileResolution());
	const int32 TileOverlap = static_cast<int32>(TileSpec.getTileMarginSize());

	agx::FrameRef TPFrame = new agx::Frame();
	TPFrame->setRotate(NativeRef->Native->getTileSpecification().getReferenceRotation());
	TPFrame->setTranslate(NativeRef->Native->getTileSpecification().getReferencePoint());

	for (agxTerrain::TerrainPager::TileAttachments* Tile : ActiveTiles)
	{
		if (Tile == nullptr || Tile->m_terrainTile == nullptr)
			continue;

		agxTerrain::TileId Id =
			TileSpec.convertWorldCoordinateToTileId(Tile->m_terrainTile->getPosition());

		const agx::Vec3 TileLocalPos =
			TPFrame->transformPointToLocal(Tile->m_terrainTile->getPosition());

		const int32 CenterToTileOffsetX = Id.x() * ((NumVertsPerTile - 1) - TileOverlap);
		const int32 CenterToTileOffsetY =
			-Id.y() * ((NumVertsPerTile - 1) - TileOverlap); // Flip y axis.

		const auto& ModifiedVerticesAGX = Tile->m_terrainTile->getModifiedVertices();
		for (const auto& Index2d : ModifiedVerticesAGX)
		{
			const int32 X = BoundsCornerToCenterOffsX + CenterToTileOffsetX + Index2d.x();
			const int32 Y = BoundsCornerToCenterOffsY + CenterToTileOffsetY - Index2d.y();

			const agx::Real TileHeightOffs = TileLocalPos.z();
			const agx::Real LocalHeight =
				Tile->m_terrainTile->getHeightField()->getHeight(Index2d.x(), Index2d.y());
			ModifiedVertices.Add(std::tuple<int32, int32>(X, Y));
			OutHeights[X + Y * BoundVertsX] =
				ConvertDistanceToUnreal<float>(LocalHeight + TileHeightOffs);
		}
	}

	return ModifiedVertices;
}

FVector FTerrainPagerBarrier::GetReferencePoint() const
{
	check(HasNative());
	return ConvertDisplacement(NativeRef->Native->getTileSpecification().getReferencePoint());
}

FQuat FTerrainPagerBarrier::GetReferenceRotation() const
{
	check(HasNative());
	return Convert(NativeRef->Native->getTileSpecification().getReferenceRotation());
}

TArray<FTransform> FTerrainPagerBarrier::GetActiveTileTransforms() const
{
	TArray<FTransform> TileTransforms;

	if (!HasNative())
		return TileTransforms;

	const agxTerrain::TerrainPager::TileAttachmentPtrVector ActiveTiles =
		NativeRef->Native->getActiveTileAttachments();

	TileTransforms.Reserve(ActiveTiles.size());

	for (agxTerrain::TerrainPager::TileAttachments* Tile : ActiveTiles)
	{
		if (Tile == nullptr || Tile->m_terrainTile == nullptr)
			continue;

		TileTransforms.Add(Convert(Tile->m_terrainTile->getTransform()));
	}

	return TileTransforms;
}

void FTerrainPagerBarrier::OnTemplateTerrainChanged() const
{
	check(HasNative());
	NativeRef->Native->applyChangesToTemplateTerrain();
}
