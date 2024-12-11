#include "Terrain/AGX_MovableTerrainComponent.h"
#include "Terrain/AGX_ShovelComponent.h"
#include "Shapes/HeightFieldShapeBarrier.h"
#include "Shapes/AGX_ShapeComponent.h"
#include "Materials/AGX_ShapeMaterial.h"
#include "Materials/AGX_TerrainMaterial.h"
#include "Utilities/AGX_StringUtilities.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "AGX_InternalDelegateAccessor.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_LogCategory.h"
#include "AGX_Simulation.h"
#include "TimerManager.h"
#include "Engine/SimpleConstructionScript.h"
#include "Engine/SCS_Node.h"
#include "NiagaraComponent.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"
#include "NiagaraFunctionLibrary.h"

UAGX_MovableTerrainComponent::UAGX_MovableTerrainComponent(
	const FObjectInitializer& ObjectInitializer)
	: UProceduralMeshComponent(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	SetCanEverAffectNavigation(false);
}

void UAGX_MovableTerrainComponent::BeginPlay()
{
	Super::BeginPlay();

	// Create Native
	CreateNative();
	
	// Create Mesh
	RecreateMesh();

	// Update Mesh each tick
	if (UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this))
	{

		PostStepForwardHandle =
			FAGX_InternalDelegateAccessor::GetOnPostStepForwardInternal(*Simulation)
				.AddLambda(
					[this](double)
					{
						// Get ModifiedHeights
						auto ModifiedHeights = NativeBarrier.GetModifiedVertices();

						// Update CurrentHeights
						if (ModifiedHeights.Num() > 0)
							NativeBarrier.GetHeights(CurrentHeights, true);

						// Update all MeshTiles that contains any ModifiedHeights (TODO: Optimize..)
						for (auto& MeshTile : MeshTiles)
						{
							for (auto& j : ModifiedHeights)
							{
								double x = std::get<0>(j);
								double y = std::get<1>(j);
								FVector2D ModifiedPos =
									FVector2D(x, y) * ElementSize;
								if (MeshTile.Box.IsInside(ModifiedPos))
								{
									UpdateMeshTileSection(MeshTile);
									break;
								}
							}
						}
					});
	}

	// Init particles
	if (bEnableParticleRendering)
	{
		if (!InitializeParticles())
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("InitializeParticles returned false in AGX_MovableTerrainComponent '%s'. "
					 "Ensure the selected Niagara System is valid."),
				*GetName());
		}
	}

	// Unreal Collision
	this->SetCollisionEnabled(AdditionalUnrealCollision);
}

void UAGX_MovableTerrainComponent::CreateNative()
{
	// Make sure OwningRigidBody is created, if there is one
	UAGX_RigidBodyComponent* OwningRigidBody =
		FAGX_ObjectUtilities::FindFirstAncestorOfType<UAGX_RigidBodyComponent>(*this);
	if (OwningRigidBody)
		OwningRigidBody->GetOrCreateNative();

	// Resolution
	FIntVector2 TerrainResolution = GetTerrainResolution();

	// Create Heights
	InitializeHeights();

	// Allocate Native
	NativeBarrier.AllocateNative(
		TerrainResolution.X, TerrainResolution.Y, ElementSize, CurrentHeights, BedHeights);

	// Attach to RigidBody
	if (OwningRigidBody)
		OwningRigidBody->GetNative()->AddTerrain(&NativeBarrier);

	// Set transform
	NativeBarrier.SetRotation(this->GetComponentQuat());
	NativeBarrier.SetPosition(this->GetComponentLocation());

	// Set Native Properties
	NativeBarrier.SetCanCollide(bCanCollide);
	NativeBarrier.AddCollisionGroups(CollisionGroups);
	NativeBarrier.SetCreateParticles(bCreateParticles);
	NativeBarrier.SetDeleteParticlesOutsideBounds(bDeleteParticlesOutsideBounds);
	NativeBarrier.SetPenetrationForceVelocityScaling(PenetrationForceVelocityScaling);
	NativeBarrier.SetMaximumParticleActivationVolume(MaximumParticleActivationVolume);

	// Create/Add Shovels
	CreateNativeShovels();

	if (!UpdateNativeTerrainMaterial())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("UpdateNativeTerrainMaterial returned false in AGX_MovableTerrainComponent '%s'. "
				 "Ensure the selected Terrain Material is valid."),
			*GetName());
	}

	if (!UpdateNativeShapeMaterial())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("UpdateNativeShapeMaterial returned false in AGX_MovableTerrainComponent '%s'. "
				 "Ensure the selected Shape Material is valid."),
			*GetName());
	}

	// Add Native
	UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this);
	Simulation->Add(*this);
}

float UAGX_MovableTerrainComponent::CalcInitialHeight(FVector LocalPos, float BedHeight) const
{
	float NoiseHeight = bUseInitialNoise ? UAGX_TerrainMeshUtilities::GetNoiseHeight(
											   LocalPos, GetComponentTransform(), InitialNoise)
										 : 0.0f;
	float InitialHeight = FMath::Max(BaseHeight, BedHeight) + PaddedHeight + NoiseHeight;

	InitialHeight =
		FMath::Lerp(BedHeight, InitialHeight, FMath::Clamp(FMath::Pow(DistFromEdge(LocalPos) / 50.0, 0.5f), 0.0f, 1.0f));

	return FMath::Max(InitialHeight, BedHeight);
}

float UAGX_MovableTerrainComponent::CalcInitialBedHeight(FVector LocalPos) const
{
	return !bUseBedShapes || GetBedShapes().Num() == 0
			   ? 0.0f
			   : UAGX_TerrainMeshUtilities::GetBedHeight(
					 LocalPos, GetComponentTransform(), GetBedShapes()) +
					 BedZOffset;
}

float UAGX_MovableTerrainComponent::GetCurrentHeight(FVector LocalPos) const
{
	return UAGX_TerrainMeshUtilities::SampleHeightArray(
		ToUvCord(LocalPos), CurrentHeights, GetTerrainResolution().X, GetTerrainResolution().Y);
}

float UAGX_MovableTerrainComponent::GetBedHeight(FVector LocalPos) const
{
	return UAGX_TerrainMeshUtilities::SampleHeightArray(
		ToUvCord(LocalPos), BedHeights, GetTerrainResolution().X, GetTerrainResolution().Y);
}

void UAGX_MovableTerrainComponent::InitializeHeights()
{
	FIntVector2 Res = GetTerrainResolution();
	CurrentHeights.Reset();
	CurrentHeights.SetNumZeroed(Res.X * Res.Y);

	BedHeights.Reset();
	BedHeights.SetNumZeroed(Res.X * Res.Y);

	FVector Center = FVector(ElementSize * (1 - Res.X) / 2.0, ElementSize * (1 - Res.Y) / 2.0, 0.0);
	for (int y = 0; y < Res.Y; y++)
	{
		for (int x = 0; x < Res.X; x++)
		{
			FVector LocalPos = Center + FVector(x * ElementSize, y * ElementSize, 0);
			float BedHeight = CalcInitialBedHeight(LocalPos);
			BedHeights[y * Res.X + x] = BedHeight; 

			float InitialHeight = CalcInitialHeight(LocalPos, BedHeight);
			CurrentHeights[y * Res.X + x] = InitialHeight;
		}
	}
}

float UAGX_MovableTerrainComponent::GetMeshHeight(FVector LocalPos) const
{
	float BedHeight = 0.0f;
	float CurrentHeight = 0.0f;
	if (HasNative())
	{
		//In-Game: Sample CurrentHeights and BedHeights
		BedHeight = GetBedHeight(LocalPos);
		CurrentHeight = GetCurrentHeight(LocalPos);
	}
	else
	{
		//In-Editor: Sample InitializeHeights
		BedHeight = CalcInitialBedHeight(LocalPos);
		CurrentHeight = CalcInitialHeight(LocalPos, BedHeight);
	}

	FVector2D Uv = ToUvCord(LocalPos);
	bool IsOnBorder = bClampMeshEdges && (Uv.X < SMALL_NUMBER || Uv.Y < SMALL_NUMBER ||
										Uv.X > 1 - SMALL_NUMBER || Uv.Y > 1 - SMALL_NUMBER);
	return IsOnBorder ? BedHeight : CurrentHeight;

}

TArray<MeshTile> UAGX_MovableTerrainComponent::GenerateMeshTiles()
{
	FIntVector2 MeshResolution = FIntVector2(GetTerrainResolution().X - 1, GetTerrainResolution().Y - 1);
	float LodScaling = 1.0f / (FMath::Pow(2.0f, MeshLevelOfDetail));
	TArray<MeshTile> Tiles;

	int MeshIndex = 0;
	// Create MeshTiles
	if (!bMeshTiles)
	{
		// One single tile
		FIntVector2 TileRes = FIntVector2(
			FMath::Max(1, FMath::RoundToInt(MeshResolution.X * LodScaling)),
			FMath::Max(1, FMath::RoundToInt(MeshResolution.Y * LodScaling)));
		FVector2D TileSize = Size;
		FVector2D TileCenter = FVector2D::ZeroVector;

		Tiles.Add(MeshTile(MeshIndex++, TileCenter, TileSize, TileRes));
	}
	else
	{
		// Multiple tiles in a grid
		FIntVector2 NrOfTiles = FIntVector2(
			FMath::Max(1, FMath::RoundToInt(MeshResolution.X * LodScaling / MeshTileResolution)),
			FMath::Max(1, FMath::RoundToInt(MeshResolution.Y * LodScaling / MeshTileResolution)));
		FIntVector2 TileRes = FIntVector2(MeshTileResolution, MeshTileResolution);
		FVector2D TileSize = FVector2D(Size.X / NrOfTiles.X, Size.Y / NrOfTiles.Y);

		for (int Tx = 0; Tx < NrOfTiles.X; Tx++)
		{
			for (int Ty = 0; Ty < NrOfTiles.Y; Ty++)
			{
				FVector2D TileCenter =
					TileSize / 2 - Size / 2 + FVector2D(Tx * TileSize.X, Ty * TileSize.Y);
				Tiles.Add(MeshTile(MeshIndex++, TileCenter, TileSize, TileRes));
			}
		}
	}

	return Tiles;
}

void UAGX_MovableTerrainComponent::CreateMeshTileSection(const MeshTile& Tile)
{
	if (IsMeshSectionVisible(Tile.MeshIndex))
		return;

	// Callbacks to modify vertex positions, uvs...
	auto HeightFunction = [&](const FVector& LocalPos) -> float { return GetMeshHeight(LocalPos); };
	auto UvFunction = [&](const FVector& LocalPos) -> FVector2D
	{ return bWorldSpaceUVs ? ToUvCentimeters(LocalPos) : ToUvCord(LocalPos); };

	int MeshIndex = Tile.MeshIndex;

	// Create mesh description 
	MeshSectionDescriptions.Add(MeshIndex, 
		UAGX_TerrainMeshUtilities::CreateMeshDescription(
			FVector(Tile.Center.X, Tile.Center.Y, 0.0f), Tile.Size, Tile.Resolution,
			HeightFunction, UvFunction, bMeshTileSkirts && bMeshTiles ? 1.0f : 0.0f));

	bool IsCreateUnrealCollision = AdditionalUnrealCollision != ECollisionEnabled::NoCollision;

	// Create mesh section
	if (auto* MeshDesc = MeshSectionDescriptions[MeshIndex].Get())
	{
		CreateMeshSection(
			MeshIndex, MeshDesc->Vertices, MeshDesc->Triangles, MeshDesc->Normals, MeshDesc->UV0,
			MeshDesc->Colors, MeshDesc->Tangents, IsCreateUnrealCollision);
		SetMaterial(MeshIndex, Material);
		SetMeshSectionVisible(MeshIndex, true);
	}
}

void UAGX_MovableTerrainComponent::UpdateMeshTileSection(const MeshTile& Tile)
{
	// Callbacks to modify vertex positions, uvs...
	auto HeightFunction = [&](const FVector& LocalPos) -> float { return GetMeshHeight(LocalPos); };
	auto UvFunction = [&](const FVector& LocalPos) -> FVector2D
	{ return bWorldSpaceUVs ? ToUvCentimeters(LocalPos) : ToUvCord(LocalPos); };

	int MeshIndex = Tile.MeshIndex;

	// Update mesh description
	if (auto* MeshDesc = MeshSectionDescriptions[MeshIndex].Get())
	{
		UAGX_TerrainMeshUtilities::UpdateMeshDescription(
			*MeshDesc, FVector(Tile.Center.X, Tile.Center.Y, 0.0f), Tile.Size,
			HeightFunction, UvFunction, bMeshTileSkirts && bMeshTiles ? 1.0f : 0.0f);

		// Update mesh section
		UpdateMeshSection(
			MeshIndex, MeshDesc->Vertices, MeshDesc->Normals, MeshDesc->UV0, MeshDesc->Colors,
			MeshDesc->Tangents);
	}

}

void UAGX_MovableTerrainComponent::RecreateMesh()
{
	// Create MeshTiles
	MeshTiles = GenerateMeshTiles();


	// Create MeshSections
	ClearAllMeshSections();
	MeshSectionDescriptions.Reset();
	for (auto& Tile : MeshTiles)
	{
		CreateMeshTileSection(Tile);
	}
}


void UAGX_MovableTerrainComponent::UpdateMeshOnPropertyChanged()
{
	// Hacky: This bool is used to force an update of the mesh in-editor
	bRebuildMesh = false;

	UWorld* World = GetWorld();
	// In-Editor
	if (IsValid(World) && !World->IsGameWorld() && !IsTemplate())
	{
		// Postpone to next tick because all properties are not copied
		World->GetTimerManager().SetTimerForNextTick(
			[this, World]
			{
				if (!IsValid(World))
					return;


				// Recreate Mesh
				RecreateMesh();
			});
	}
	// In-Game
	else if (IsValid(World) && World->IsGameWorld() && !IsTemplate())
	{
		if (!HasNative())
			return;

		// Copy CurrentHeights from Native
		NativeBarrier.GetHeights(CurrentHeights, false);

		// Recreate Mesh
		RecreateMesh();
	}
}

#if WITH_EDITOR
void UAGX_MovableTerrainComponent::PostEditChangeChainProperty(FPropertyChangedChainEvent& Event)
{
	Super::PostEditChangeChainProperty(Event);
	UE_LOG(LogAGX, Warning, TEXT("PostEditChangeChainProperty"));

	// TODO: Only UpdateMesh on certain Property changes
	UpdateMeshOnPropertyChanged();
}

void UAGX_MovableTerrainComponent::PostInitProperties()
{
	Super::PostInitProperties();
	UE_LOG(LogAGX, Warning, TEXT("PostInitProperties"));
	// TODO: Only UpdateMesh on certain Property changes
	UpdateMeshOnPropertyChanged();
}
#endif

void UAGX_MovableTerrainComponent::TickComponent(
	float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("AGXUnreal:AAGX_MovableTerrain::Tick"));
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (bEnableParticleRendering)
	{
		UpdateParticles();
	}
}

void UAGX_MovableTerrainComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);
	if (HasNative() && Reason != EEndPlayReason::EndPlayInEditor &&
		Reason != EEndPlayReason::Quit && Reason != EEndPlayReason::LevelTransition)
	{
		if (UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this))
		{
			Simulation->Remove(*this);

			FAGX_InternalDelegateAccessor::GetOnPostStepForwardInternal(*Simulation)
				.Remove(PostStepForwardHandle);
		}
	}
}

TArray<UMeshComponent*> UAGX_MovableTerrainComponent::GetBedShapes() const
{
	TArray<UMeshComponent*> Shapes;
	if (GetOwner() != nullptr)
	{
		for (UMeshComponent* ShapeComponent :
			 FAGX_ObjectUtilities::Filter<UMeshComponent>(GetOwner()->GetComponents()))
		{
			if (BedShapes.Contains(ShapeComponent->GetFName()))
				Shapes.Add(ShapeComponent);
		}
	}

	Shapes.Append(BedShapeComponents);

	return Shapes;
}

TArray<FString> UAGX_MovableTerrainComponent::GetBedShapesOptions() const
{
	TArray<FString> Options;
	for (FName Name :
		 FAGX_ObjectUtilities::GetChildComponentNamesOfType<UMeshComponent>(GetOuter()))
	{
		if (!BedShapes.Contains(Name) && this->GetName() != Name.ToString())
			Options.Add(Name.ToString());
	}
	return Options;
}

/*
	--- AGX_Terrain Implementation
	------------------------------
*/

bool UAGX_MovableTerrainComponent::HasNative() const
{
	return NativeBarrier.HasNative();
}

FTerrainBarrier* UAGX_MovableTerrainComponent::GetNative()
{
	if (!NativeBarrier.HasNative())
	{
		return nullptr;
	}

	return &NativeBarrier;
}

const FTerrainBarrier* UAGX_MovableTerrainComponent::GetNative() const
{
	if (!NativeBarrier.HasNative())
	{
		return nullptr;
	}

	return &NativeBarrier;
}

void UAGX_MovableTerrainComponent::SetCanCollide(bool bInCanCollide)
{
	if (HasNative())
	{
		NativeBarrier.SetCanCollide(bInCanCollide);
	}

	bCanCollide = bInCanCollide;
}

bool UAGX_MovableTerrainComponent::GetCanCollide() const
{
	if (HasNative())
	{
		return NativeBarrier.GetCanCollide();
	}

	return bCanCollide;
}

void UAGX_MovableTerrainComponent::SetCreateParticles(bool CreateParticles)
{
	if (HasNative())
	{
		NativeBarrier.SetCreateParticles(CreateParticles);
	}

	bCreateParticles = CreateParticles;
}

bool UAGX_MovableTerrainComponent::GetCreateParticles() const
{
	if (HasNative())
	{
		return NativeBarrier.GetCreateParticles();
	}

	return bCreateParticles;
}

void UAGX_MovableTerrainComponent::SetDeleteParticlesOutsideBounds(
	bool DeleteParticlesOutsideBounds)
{
	if (HasNative())
	{
		NativeBarrier.SetDeleteParticlesOutsideBounds(DeleteParticlesOutsideBounds);
	}

	bDeleteParticlesOutsideBounds = DeleteParticlesOutsideBounds;
}

bool UAGX_MovableTerrainComponent::GetDeleteParticlesOutsideBounds() const
{
	if (HasNative())
	{
		return NativeBarrier.GetDeleteParticlesOutsideBounds();
	}

	return bDeleteParticlesOutsideBounds;
}

void UAGX_MovableTerrainComponent::SetPenetrationForceVelocityScaling(
	double InPenetrationForceVelocityScaling)
{
	if (HasNative())
	{
		NativeBarrier.SetPenetrationForceVelocityScaling(InPenetrationForceVelocityScaling);
	}

	PenetrationForceVelocityScaling = InPenetrationForceVelocityScaling;
}

double UAGX_MovableTerrainComponent::GetPenetrationForceVelocityScaling() const
{
	if (HasNative())
	{
		return NativeBarrier.GetPenetrationForceVelocityScaling();
	}

	return PenetrationForceVelocityScaling;
}

void UAGX_MovableTerrainComponent::SetMaximumParticleActivationVolume(
	double InMaximumParticleActivationVolume)
{
	if (HasNative())
	{
		NativeBarrier.SetMaximumParticleActivationVolume(InMaximumParticleActivationVolume);
	}

	MaximumParticleActivationVolume = InMaximumParticleActivationVolume;
}

double UAGX_MovableTerrainComponent::GetMaximumParticleActivationVolume() const
{
	if (HasNative())
	{
		return NativeBarrier.GetMaximumParticleActivationVolume();
	}

	return MaximumParticleActivationVolume;
}

bool UAGX_MovableTerrainComponent::SetTerrainMaterial(UAGX_TerrainMaterial* InTerrainMaterial)
{
	UAGX_TerrainMaterial* TerrainMaterialOrig = TerrainMaterial;
	TerrainMaterial = InTerrainMaterial;

	if (!HasNative())
	{
		// Not in play, we are done.
		return true;
	}

	// UpdateNativeTerrainMaterial is responsible to create an instance if none exists and do the
	// asset/instance swap.
	if (!UpdateNativeTerrainMaterial())
	{
		// Something went wrong, restore original TerrainMaterial.
		TerrainMaterial = TerrainMaterialOrig;
		return false;
	}

	return true;
}

bool UAGX_MovableTerrainComponent::SetShapeMaterial(UAGX_ShapeMaterial* InShapeMaterial)
{
	UAGX_ShapeMaterial* ShapeMaterialOrig = ShapeMaterial;
	ShapeMaterial = InShapeMaterial;

	if (!HasNative())
	{
		// Not in play, we are done.
		return true;
	}

	// UpdateNativeShapeMaterial is responsible to create an instance if none exists and do the
	// asset/instance swap.
	if (!UpdateNativeShapeMaterial())
	{
		// Something went wrong, restore original ShapeMaterial.
		ShapeMaterial = ShapeMaterialOrig;
		return false;
	}

	return true;
}

bool UAGX_MovableTerrainComponent::UpdateNativeTerrainMaterial()
{
	if (!HasNative())
		return false;

	if (TerrainMaterial == nullptr)
	{
		GetNative()->ClearTerrainMaterial();
		return true;
	}

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Cannot update native Terrain material because don't have a world to create "
				 "the material instance in."));
		return false;
	}

	UAGX_TerrainMaterial* Instance = TerrainMaterial->GetOrCreateInstance(World);
	check(Instance);

	if (TerrainMaterial != Instance)
		TerrainMaterial = Instance;

	FTerrainMaterialBarrier* TerrainMaterialBarrier =
		Instance->GetOrCreateTerrainMaterialNative(World);
	check(TerrainMaterialBarrier);

	GetNative()->SetTerrainMaterial(*TerrainMaterialBarrier);

	return true;
}

bool UAGX_MovableTerrainComponent::UpdateNativeShapeMaterial()
{
	if (!HasNative())
		return false;

	if (ShapeMaterial == nullptr)
	{
		GetNative()->ClearShapeMaterial();
		return true;
	}

	UAGX_ShapeMaterial* Instance =
		static_cast<UAGX_ShapeMaterial*>(ShapeMaterial->GetOrCreateInstance(GetWorld()));
	check(Instance);

	if (ShapeMaterial != Instance)
		ShapeMaterial = Instance;

	FShapeMaterialBarrier* MaterialBarrier = Instance->GetOrCreateShapeMaterialNative(GetWorld());
	check(MaterialBarrier);

	GetNative()->SetShapeMaterial(*MaterialBarrier);
	return true;
}

void UAGX_MovableTerrainComponent::AddCollisionGroup(FName GroupName)
{
	if (GroupName.IsNone())
	{
		return;
	}

	if (CollisionGroups.Contains(GroupName))
		return;

	CollisionGroups.Add(GroupName);
	if (HasNative())
		NativeBarrier.AddCollisionGroup(GroupName);
}

void UAGX_MovableTerrainComponent::RemoveCollisionGroupIfExists(FName GroupName)
{
	if (GroupName.IsNone())
		return;

	auto Index = CollisionGroups.IndexOfByKey(GroupName);
	if (Index == INDEX_NONE)
		return;

	CollisionGroups.RemoveAt(Index);
	if (HasNative())
		NativeBarrier.RemoveCollisionGroup(GroupName);
}

void UAGX_MovableTerrainComponent::CreateNativeShovels()
{
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("CreateNativeShovels called on AGX MovableTerrain '%s' which doesn't have a native "
				 "representation."),
			*GetName());
	}

	for (FAGX_ShovelReference& ShovelRef : ShovelComponents)
	{
		UAGX_ShovelComponent* ShovelComponent = ShovelRef.GetShovelComponent();
		if (ShovelComponent == nullptr)
		{
			const FString Message = FString::Printf(
				TEXT("AGX MovableTerrain '%s' have a Shovel reference to '%s' in '%s' that does not "
					 "reference a valid Shovel."),
				*GetName(), *ShovelRef.Name.ToString(),
				*GetLabelSafe(ShovelRef.OwningActor));
			FAGX_NotificationUtilities::ShowNotification(Message, SNotificationItem::CS_Fail);
			continue;
		}

		AddNativeShovel(ShovelComponent);
	}
}

void UAGX_MovableTerrainComponent::AddShovel(UAGX_ShovelComponent* ShovelComponent)
{
	FAGX_ShovelReference ShovelRef;
	ShovelRef.SetComponent(ShovelComponent);
	ShovelComponents.Add(ShovelRef);

	if (HasNative())
	{
		AddNativeShovel(ShovelComponent);
	}
}


bool UAGX_MovableTerrainComponent::AddNativeShovel(UAGX_ShovelComponent* ShovelComponent)
{
	FShovelBarrier* ShovelBarrier = ShovelComponent->GetOrCreateNative();
	if (ShovelBarrier == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Shovel '%s' in AGX MovableTerrain '%s' could not create AGX Dynamics "
				 "representation. Ignoring this shovel. It will not be able to deform the "
				 "Terrain."),
			*ShovelComponent->GetName(), *GetName());
		return false;
	}
	check(ShovelBarrier->HasNative());

	bool Added = NativeBarrier.AddShovel(*ShovelBarrier);
	if (!Added)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("AGX MovableTerrain '%s' rejected shovel '%s' in '%s'. Reversing edge directions "
				 "and "
				 "trying again."),
			*GetName(), *ShovelComponent->GetName(), *GetLabelSafe(ShovelComponent->GetOwner()));
		ShovelComponent->SwapEdgeDirections();
		Added = NativeBarrier.AddShovel(*ShovelBarrier);
		if (!Added)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("AGX MovableTerrain '%s' rejected shovel '%s' in '%s' after edge directions "
					 "flip. "
					 "Abandoning shovel."),
				*GetName(), *GetNameSafe(ShovelComponent),
				*GetLabelSafe(ShovelComponent->GetOwner()));
		}
	}

	return Added;
}

void UAGX_MovableTerrainComponent::ConvertToDynamicMassInShape(UAGX_ShapeComponent* Shape)
{
	if (HasNative() && Shape->HasNative())
		NativeBarrier.ConvertToDynamicMassInShape(Shape->GetNative());
}

void UAGX_MovableTerrainComponent::SetIsNoMerge(bool IsNoMerge)
{
	if (HasNative())
		NativeBarrier.SetNoMerge(IsNoMerge);
}

bool UAGX_MovableTerrainComponent::GetIsNoMerge() const
{
	if (HasNative())
		return NativeBarrier.GetNoMerge();
	return false;
}

UNiagaraComponent* UAGX_MovableTerrainComponent::GetSpawnedParticleSystemComponent()
{
	return ParticleSystemComponent;
}

int32 UAGX_MovableTerrainComponent::GetNumParticles() const
{
	if (!HasNative())
		return 0;

	check(HasNative());
	return static_cast<int32>(NativeBarrier.GetNumParticles());
}

bool UAGX_MovableTerrainComponent::InitializeParticles()
{
	if (!ParticleSystemAsset)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("MovableTerrainComponent '%s' does not have a particle system, cannot render particles"),
			*GetName());
		return false;
	}

	// It is important that we attach the ParticleSystemComponent using "KeepRelativeOffset" so that
	// it's world position becomes the same as the Terrain's. Otherwise it will be spawned at
	// the world origin which in turn may result in particles being culled and not rendered if the
	// terrain is located far away from the world origin (see Fixed Bounds in the Particle System).
	ParticleSystemComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
		ParticleSystemAsset, this, NAME_None, FVector::ZeroVector, FRotator::ZeroRotator,
		FVector::OneVector, EAttachLocation::Type::KeepRelativeOffset, false,
#if UE_VERSION_OLDER_THAN(4, 24, 0)
		EPSCPoolMethod::None
#else
		ENCPoolMethod::None
#endif
	);
#if WITH_EDITORONLY_DATA
	// Must check for nullptr here because no particle system component is created with running
	// as a unit test without graphics, i.e. with our run_unit_tests script in GitLab CI.
	if (ParticleSystemComponent != nullptr)
	{
		ParticleSystemComponent->bVisualizeComponent = true;
	}
#endif

	return ParticleSystemComponent != nullptr;
}

void UAGX_MovableTerrainComponent::UpdateParticles()
{
	if (!NativeBarrier.HasNative())
	{
		return;
	}
	if (ParticleSystemComponent == nullptr)
	{
		return;
	}

	// Copy data with holes.
	EParticleDataFlags ToInclude = EParticleDataFlags::Positions | EParticleDataFlags::Rotations |
								   EParticleDataFlags::Radii | EParticleDataFlags::Velocities;
	const FParticleDataById ParticleData = NativeBarrier.GetParticleDataById(ToInclude);

	const TArray<FVector>& Positions = ParticleData.Positions;
	const TArray<FQuat>& Rotations = ParticleData.Rotations;
	const TArray<float>& Radii = ParticleData.Radii;
	const TArray<bool>& Exists = ParticleData.Exists;
	const TArray<FVector>& Velocities = ParticleData.Velocities;

#if UE_VERSION_OLDER_THAN(5, 3, 0)
	ParticleSystemComponent->SetNiagaraVariableInt("User.Target Particle Count", Exists.Num());
#else
	ParticleSystemComponent->SetVariableInt(FName("User.Target Particle Count"), Exists.Num());
#endif

	const int32 NumParticles = Positions.Num();

	TArray<FVector4> PositionsAndScale;
	PositionsAndScale.SetNum(NumParticles);
	TArray<FVector4> Orientations;
	Orientations.SetNum(NumParticles);

	for (int32 I = 0; I < NumParticles; ++I)
	{
		// The particle size slot in the PositionAndScale buffer is a scale and not the
		// actual size. The scale is relative to a SI unit cube, meaning that a
		// scale of 1.0 should render a particle that is 1x1x1 m large, or
		// 100x100x100 Unreal units. We multiply by 2.0 to convert from radius
		// to full width.
		float UnitCubeScale = (Radii[I] * 2.0f) / 100.0f;
		PositionsAndScale[I] = FVector4(Positions[I], UnitCubeScale);
		Orientations[I] = FVector4(Rotations[I].X, Rotations[I].Y, Rotations[I].Z, Rotations[I].W);
	}

	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector4(
		ParticleSystemComponent, "Positions And Scales", PositionsAndScale);
	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector4(
		ParticleSystemComponent, "Orientations", Orientations);
	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayBool(
		ParticleSystemComponent, "Exists", Exists);
	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(
		ParticleSystemComponent, TEXT("Velocities"), Velocities);
}
