// Copyright 2025, Algoryx Simulation AB.

#include "Terrain/TerrainBarrier.h"

// AGX Dynamics for Unreal includes.
#include "BarrierOnly/AGXRefs.h"
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "Materials/TerrainMaterialBarrier.h"
#include "Materials/ShapeMaterialBarrier.h"
#include "Shapes/HeightFieldShapeBarrier.h"
#include "Shapes/ShapeBarrierImpl.h"
#include "Terrain/ShovelBarrier.h"
#include "TypeConversions.h"
#include "Utilities/TerrainUtilities.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxCollide/HeightField.h>
#include <agx/Physics/GranularBodySystem.h>
#include "EndAGXIncludes.h"

FTerrainBarrier::FTerrainBarrier()
	: NativeRef {new FTerrainRef}
{
}

FTerrainBarrier::FTerrainBarrier(std::unique_ptr<FTerrainRef> InNativeRef)
	: NativeRef {std::move(InNativeRef)}
{
}

FTerrainBarrier::FTerrainBarrier(FTerrainBarrier&& Other)
	: NativeRef {std::move(Other.NativeRef)}
{
}

FTerrainBarrier::~FTerrainBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the
	// std::unique_ptr NativeRef's destructor must be able to see the definition,
	// not just the forward declaration, of FTerrainRef.
}

bool FTerrainBarrier::HasNative() const
{
	return NativeRef->Native != nullptr;
}

void FTerrainBarrier::AllocateNative(FHeightFieldShapeBarrier& SourceHeightField, double MaxDepth)
{
	check(!HasNative());
	const agx::Real MaxDepthAGX = ConvertDistanceToAGX(MaxDepth);
	agxCollide::HeightField* HeightFieldAGX =
		SourceHeightField.GetNativeShape<agxCollide::HeightField>();
	NativeRef->Native = agxTerrain::Terrain::createFromHeightField(HeightFieldAGX, MaxDepthAGX);
}

FTerrainRef* FTerrainBarrier::GetNative()
{
	check(HasNative());
	return NativeRef.get();
}

const FTerrainRef* FTerrainBarrier::GetNative() const
{
	check(HasNative());
	return NativeRef.get();
}

void FTerrainBarrier::ReleaseNative()
{
	check(HasNative());
	NativeRef->Native = nullptr;
}

void FTerrainBarrier::SetCanCollide(bool bCanCollide)
{
	check(HasNative());
	if (agxCollide::Geometry* Geom = NativeRef->Native->getGeometry())
		Geom->setEnableCollisions(bCanCollide);
}

bool FTerrainBarrier::GetCanCollide() const
{
	check(HasNative());
	if (agxCollide::Geometry* Geom = NativeRef->Native->getGeometry())
		return Geom->getEnableCollisions();

	return false;
}

void FTerrainBarrier::SetPosition(const FVector& PositionUnreal)
{
	check(HasNative());
	agx::Vec3 PositionAGX = ConvertDisplacement(PositionUnreal);
	NativeRef->Native->setPosition(PositionAGX);
}

FVector FTerrainBarrier::GetPosition() const
{
	check(HasNative());
	agx::Vec3 PositionAGX = NativeRef->Native->getPosition();
	FVector PositionUnreal = ConvertDisplacement(PositionAGX);
	return PositionUnreal;
}

void FTerrainBarrier::SetRotation(const FQuat& RotationUnreal)
{
	check(HasNative());
	agx::Quat RotationAGX = Convert(RotationUnreal);
	NativeRef->Native->setRotation(RotationAGX);
}

FQuat FTerrainBarrier::GetRotation() const
{
	check(HasNative());
	agx::Quat RotationAGX = NativeRef->Native->getRotation();
	FQuat RotationUnreal = Convert(RotationAGX);
	return RotationUnreal;
}

void FTerrainBarrier::SetCreateParticles(bool CreateParticles)
{
	check(HasNative());
	NativeRef->Native->getProperties()->setCreateParticles(CreateParticles);
}

bool FTerrainBarrier::GetCreateParticles() const
{
	check(HasNative());
	return NativeRef->Native->getProperties()->getCreateParticles();
}

void FTerrainBarrier::SetDeleteParticlesOutsideBounds(bool DeleteParticlesOutsideBounds)
{
	check(HasNative());
	NativeRef->Native->getProperties()->setDeleteSoilParticlesOutsideBounds(
		DeleteParticlesOutsideBounds);
}

bool FTerrainBarrier::GetDeleteParticlesOutsideBounds() const
{
	check(HasNative());
	return NativeRef->Native->getProperties()->getDeleteSoilParticlesOutsideBounds();
}

void FTerrainBarrier::SetPenetrationForceVelocityScaling(double PenetrationForceVelocityScaling)
{
	check(HasNative());
	NativeRef->Native->getProperties()->setPenetrationForceVelocityScaling(
		PenetrationForceVelocityScaling);
}

double FTerrainBarrier::GetPenetrationForceVelocityScaling() const
{
	check(HasNative());
	return NativeRef->Native->getProperties()->getPenetrationForceVelocityScaling();
}

void FTerrainBarrier::SetMaximumParticleActivationVolume(double MaximumParticleActivationVolume)
{
	check(HasNative());
	NativeRef->Native->getProperties()->setMaximumParticleActivationVolume(
		ConvertVolumeToAGX(MaximumParticleActivationVolume));
}

double FTerrainBarrier::GetMaximumParticleActivationVolume() const
{
	check(HasNative());
	return ConvertVolumeToUnreal<double>(
		NativeRef->Native->getProperties()->getMaximumParticleActivationVolume());
}

bool FTerrainBarrier::AddShovel(FShovelBarrier& Shovel)
{
	check(HasNative());
	check(Shovel.HasNative());
	return NativeRef->Native->add(Shovel.GetNative()->Native);
}

void FTerrainBarrier::SetShapeMaterial(const FShapeMaterialBarrier& Material)
{
	check(HasNative());
	check(Material.HasNative());
	NativeRef->Native->setMaterial(Material.GetNative()->Native);
}

void FTerrainBarrier::SetTerrainMaterial(const FTerrainMaterialBarrier& TerrainMaterial)
{
	check(HasNative());
	check(TerrainMaterial.HasNative());
	NativeRef->Native->setTerrainMaterial(TerrainMaterial.GetNative()->Native);
}

void FTerrainBarrier::ClearTerrainMaterial()
{
	check(HasNative());
	NativeRef->Native->setTerrainMaterial(nullptr);
}

void FTerrainBarrier::ClearShapeMaterial()
{
	check(HasNative());
	NativeRef->Native->setMaterial(nullptr);
}

void FTerrainBarrier::AddCollisionGroup(const FName& GroupName)
{
	check(HasNative());

	// Add collision group as (hashed) unsigned int.  Making changes to the Terrain's Geometry is
	// not generally a good idea, but this case (adding collision groups) is ok.
	NativeRef->Native->getGeometry()->addGroup(StringTo32BitFnvHash(GroupName.ToString()));
}

void FTerrainBarrier::AddCollisionGroups(const TArray<FName>& GroupNames)
{
	for (auto& GroupName : GroupNames)
	{
		AddCollisionGroup(GroupName);
	}
}

void FTerrainBarrier::RemoveCollisionGroup(const FName& GroupName)
{
	check(HasNative());

	// Remove collision group as (hashed) unsigned int. Making changes to the Terrain's Geometry is
	// not generally a good idea, but this case (removing collision groups) is ok.
	NativeRef->Native->getGeometry()->removeGroup(StringTo32BitFnvHash(GroupName.ToString()));
}

TArray<FName> FTerrainBarrier::GetCollisionGroups() const
{
	check(HasNative());
	const agxCollide::GroupIdCollection Groups =
		NativeRef->Native->getGeometry()->findGroupIdCollection();

	TArray<FName> Result;
	for (const agx::Name& Name : Groups.getNames())
	{
		Result.Add(FName(*Convert(Name)));
	}

	for (const agx::UInt32 Id : Groups.getIds())
	{
		Result.Add(FName(*FString::FromInt(Id)));
	}

	return Result;
}

int32 FTerrainBarrier::GetGridSizeX() const
{
	check(HasNative());
	size_t GridSize = NativeRef->Native->getResolutionX();
	check(GridSize < static_cast<size_t>(std::numeric_limits<int32>::max()));
	return static_cast<int32>(GridSize);
}

int32 FTerrainBarrier::GetGridSizeY() const
{
	check(HasNative());
	size_t GridSize = NativeRef->Native->getResolutionY();
	check(GridSize < static_cast<size_t>(std::numeric_limits<int32>::max()));
	return static_cast<int32>(GridSize);
}

TArray<std::tuple<int32, int32>> FTerrainBarrier::GetModifiedVertices() const
{
	check(HasNative());

	const agxCollide::HeightField* HeightField = NativeRef->Native->getHeightField();
	const size_t SizeXAGX = HeightField->getResolutionX();
	const size_t SizeYAGX = HeightField->getResolutionY();
	const int32 SizeX = static_cast<int32>(SizeXAGX);
	const int32 SizeY = static_cast<int32>(SizeYAGX);

	const auto& ModifiedVerticesAGX = NativeRef->Native->getModifiedVertices();
	TArray<std::tuple<int32, int32>> ModifiedVertices;
	ModifiedVertices.Reserve(ModifiedVerticesAGX.size());
	for (const auto& Index2d : ModifiedVerticesAGX)
	{
		ModifiedVertices.Add(std::make_tuple<int32, int32>(
			static_cast<int32>(Index2d.x()), SizeY - 1 - static_cast<int32>(Index2d.y())));
	}

	return ModifiedVertices;
}

void FTerrainBarrier::GetHeights(TArray<float>& Heights, bool bChangesOnly) const
{
	check(HasNative());
	const agxCollide::HeightField* HeightField = NativeRef->Native->getHeightField();
	const size_t SizeXAGX = HeightField->getResolutionX();
	const size_t SizeYAGX = HeightField->getResolutionY();

	if (SizeXAGX == 0 || SizeYAGX == 0)
	{
		/// \todo Unclear if this really should be a warning or not. When would
		/// zero-sized terrains be used?
		UE_LOG(LogAGX, Warning, TEXT("Cannot get heights from terrain with zero size."));
		return;
	}
	if (SizeXAGX * SizeYAGX > static_cast<size_t>(std::numeric_limits<int32>::max()))
	{
		UE_LOG(LogAGX, Error, TEXT("Cannot get heights, terrain has too many vertices."));
		return;
	}

	const int32 SizeX = static_cast<int32>(SizeXAGX);
	const int32 SizeY = static_cast<int32>(SizeYAGX);

	// AGX Dynamics and Unreal have different coordinate systems, so we must
	// flip the Y axis for the vertex locations.
	//
	// AGX Dynamics has [0,0] in the bottom left:
	//
	// [0,n-1] [1,n-1] ...      This is
	// ...                      what we
	// [0,2] [1,2] [3,2] ...    read from.
	// [0,1] [1,1] [2,1] ...
	// [0,0] [1,0] [2,0] ...
	//
	//
	// Unreal has [0,0] in the top left:
	//
	// [0,0] [1,0] [2,0] ...    This is
	// [0,1] [1,1] [2,1] ...    what we
	// [0,2] [1,2] [2,2] ...    write to.
	// ...
	// [0,n-1] [1,n-1] ...
	//
	// When looking at the Landscape in Unreal Editor, the user sees the axis
	// widget aligned with the Unreal section above, with the red/green/blue
	// lines at the [0,0] vertex.
	//
	// We want the ordering of the returned array to be row major according to
	// the Unreal figure above. Thus, we start by reading the last/top row of
	// increasing X coordinates, i.e., the row with Y=n-1, from AGX Dynamics'
	// point of view.
	if (bChangesOnly)
	{
		const auto& ModifiedVerticesAGX = NativeRef->Native->getModifiedVertices();
		for (const auto& Index2d : ModifiedVerticesAGX)
		{
			int32 I = Index2d.x() + (SizeY - 1 - Index2d.y()) * SizeX;
			AGX_CHECK(Heights.Num() > I);
			Heights[I] =
				ConvertDistanceToUnreal<float>(HeightField->getHeight(Index2d.x(), Index2d.y()));
		}
	}
	else
	{
		Heights.Reset(SizeX * SizeY);
		for (int32 Y = SizeY - 1; Y >= 0; Y--)
		{
			for (int32 X = 0; X < SizeX; ++X)
			{
				Heights.Add(ConvertDistanceToUnreal<float>(HeightField->getHeight(X, Y)));
			}
		}
	}
}

TArray<FVector> FTerrainBarrier::GetParticlePositions() const
{
	check(HasNative());
	const size_t NumParticles = FTerrainUtilities::GetNumParticles(*this);
	TArray<FVector> Positions;
	Positions.Reserve(NumParticles);

	FTerrainUtilities::AppendParticlePositions(*this, Positions);
	return Positions;
}

TArray<float> FTerrainBarrier::GetParticleRadii() const
{
	check(HasNative());
	const size_t NumParticles = FTerrainUtilities::GetNumParticles(*this);
	TArray<float> Radii;
	Radii.Reserve(NumParticles);

	FTerrainUtilities::AppendParticleRadii(*this, Radii);
	return Radii;
}

TArray<FQuat> FTerrainBarrier::GetParticleRotations() const
{
	check(HasNative());
	const size_t NumParticles = FTerrainUtilities::GetNumParticles(*this);
	TArray<FQuat> Rotations;
	Rotations.Reserve(NumParticles);

	FTerrainUtilities::AppendParticleRotations(*this, Rotations);
	return Rotations;
}

TArray<float> FTerrainBarrier::GetParticleMasses() const
{
	check(HasNative());
	const size_t NumParticles = FTerrainUtilities::GetNumParticles(*this);
	TArray<float> Masses;
	Masses.Reserve(NumParticles);

	FTerrainUtilities::AppendParticleMasses(*this, Masses);
	return Masses;
}

FParticleData FTerrainBarrier::GetParticleData(const EParticleDataFlags ToInclude) const
{
	const size_t NumParticles = FTerrainUtilities::GetNumParticles(*this);
	FParticleData ParticleData;
	ParticleData.Positions.Reserve(NumParticles);
	ParticleData.Velocities.Reserve(NumParticles);
	ParticleData.Radii.Reserve(NumParticles);
	ParticleData.Rotations.Reserve(NumParticles);
	ParticleData.Masses.Reserve(NumParticles);

	FTerrainUtilities::AppendParticleData(*this, ParticleData, ToInclude);

	return ParticleData;
}

FParticleDataById FTerrainBarrier::GetParticleDataById(EParticleDataFlags ToInclude) const
{
	FParticleDataById ParticleData;
	FTerrainUtilities::GetParticleExistsById(*this, ParticleData.Exists);
	if (ToInclude & EParticleDataFlags::Positions)
	{
		FTerrainUtilities::GetParticlePositionsById(*this, ParticleData.Positions);
	}
	if (ToInclude & EParticleDataFlags::Velocities)
	{
		FTerrainUtilities::GetParticleVelocitiesById(*this, ParticleData.Velocities);
	}
	if (ToInclude & EParticleDataFlags::Rotations)
	{
		FTerrainUtilities::GetParticleRotationsById(*this, ParticleData.Rotations);
	}
	if (ToInclude & EParticleDataFlags::Radii)
	{
		FTerrainUtilities::GetParticleRadiiById(*this, ParticleData.Radii);
	}
	if (ToInclude & EParticleDataFlags::Masses)
	{
		FTerrainUtilities::GetParticleMassesById(*this, ParticleData.Masses);
	}

	return ParticleData;
}

size_t FTerrainBarrier::GetNumParticles() const
{
	check(HasNative());
	return FTerrainUtilities::GetNumParticles(*this);
}
