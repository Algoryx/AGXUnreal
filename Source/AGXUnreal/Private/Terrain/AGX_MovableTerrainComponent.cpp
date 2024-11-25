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
#include "Engine/TimerHandle.h"
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
}

void UAGX_MovableTerrainComponent::BeginPlay()
{
	Super::BeginPlay();

	// Create Heights
	InitializeHeights();

	// Create Native
	CreateNative();
	
	// Create Mesh(s) and Tile(s)
	InitializeMesh();

	// Update Mesh each tick
	if (UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this))
	{
		PostStepForwardHandle =
			FAGX_InternalDelegateAccessor::GetOnPostStepForwardInternal(*Simulation)
				.AddLambda(
					[this](double)
					{
						//Update CurrentHeights
						NativeBarrier.GetHeights(CurrentHeights, true);

						//Rebuild mesh if needed
						auto DirtyHeights = NativeBarrier.GetModifiedVertices();
						if (DirtyHeights.Num() > 0)
							UpdateMesh(DirtyHeights);
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

void UAGX_MovableTerrainComponent::InitializeHeights()
{
	FIntVector2 Res = GetTerrainResolution();

	// BedHeights
	BedHeights.Reset();
	BedHeights.SetNumZeroed(Res.X * Res.Y);
	if (GetBedShapes().Num() > 0)
	{
		TArray<UAGX_SimpleMeshComponent*> BedMeshes =
			FAGX_ObjectUtilities::Filter<UAGX_SimpleMeshComponent>(GetBedShapes());
		UAGX_TerrainMeshUtilities::AddBedHeights(
			BedHeights, Res, ElementSize, GetComponentTransform(), BedMeshes);
	}

	// CurrentHeights
	CurrentHeights.Reset();
	CurrentHeights.SetNumZeroed(Res.X * Res.Y);
	if (bInitialNoise)
	{
		UAGX_TerrainMeshUtilities::AddNoiseHeights(
			CurrentHeights, Res, ElementSize, GetComponentTransform(), InitialNoiseParams);
	}
	if (InitialHeight > 0)
	{
		for (float& h : CurrentHeights)
			h += InitialHeight;
	}

	// Put BedHeights in CurrentHeights
	for (int i = 0; i < CurrentHeights.Num(); i++)
		CurrentHeights[i] = FMath::Max(CurrentHeights[i], BedHeights[i]);
}

float UAGX_MovableTerrainComponent::SampleHeights(FVector LocalPos) const
{
	//Normalized/UVCoord,somewhere between: [0.0, 0.0] - [1.0, 1.0]
	FVector2D UvCord = FVector2D(LocalPos.X / Size.X + 0.5, LocalPos.Y / Size.Y + 0.5);
	
	bool IsOnBorder = UvCord.X < SMALL_NUMBER || UvCord.Y < SMALL_NUMBER ||
					  UvCord.X > 1 - SMALL_NUMBER || UvCord.Y > 1 - SMALL_NUMBER;

	// Sample from BedHeights (MinimumHeights) when close to border
	auto& SampleArray = (IsOnBorder && ClampToBorders) ? BedHeights : CurrentHeights;

	return UAGX_TerrainMeshUtilities::SampleHeightArray(
		UvCord, SampleArray, GetTerrainResolution().X, GetTerrainResolution().Y);
}

void UAGX_MovableTerrainComponent::InitializeMesh()
{
	// Height Function
	auto HeightFunction = [&](const FVector& LocalPos) -> float { return SampleHeights(LocalPos); };

	// Create MeshTiles
	MeshTiles.Reset();
	{
		FIntVector2 NrOfTiles;
		FIntVector2 TileRes;
		if (!bEnableTiles)
		{
			// Single tile
			NrOfTiles = FIntVector2(1, 1);
			TileRes = FIntVector2(
				FMath::RoundToInt((GetTerrainResolution().X - 1) * ResolutionScaling),
				FMath::RoundToInt((GetTerrainResolution().Y - 1) * ResolutionScaling));
		}
		else
		{
			// Multiple tiles
			NrOfTiles = FIntVector2(
				FMath::Max(1, FMath::RoundToInt(
						   (GetTerrainResolution().X - 1) * ResolutionScaling / TileResolution)),
				FMath::Max(1, FMath::RoundToInt(
						   (GetTerrainResolution().Y - 1) * ResolutionScaling / TileResolution)));

			TileRes = FIntVector2(TileResolution, TileResolution);
		}

		FVector2D TileSize = FVector2D(Size.X / NrOfTiles.X, Size.Y / NrOfTiles.Y);

		for (int Tx = 0; Tx < NrOfTiles.X; Tx++)
		{
			for (int Ty = 0; Ty < NrOfTiles.Y; Ty++)
			{
				int TileIndex = Tx * NrOfTiles.Y + Ty;
				FVector2D TileCenter =
					TileSize / 2 - Size / 2 + FVector2D(Tx * TileSize.X, Ty * TileSize.Y);
				MeshTiles.Add(TileIndex, MeshTile(TileCenter, TileSize, TileRes));
			}
		}
	}

	// Create MeshSections
	ClearAllMeshSections();
	for (auto& kvp : MeshTiles)
	{
		int TileIndex = kvp.Key;
		auto Tile = kvp.Value;
		FVector TileCenter = FVector(Tile.Center.X, Tile.Center.Y, ZOffset);

		// Create mesh description
		auto MeshDesc = UAGX_TerrainMeshUtilities::CreateMeshDescription(
			TileCenter, Tile.Size, Tile.Resolution, UvScaling, HeightFunction, bTileSkirts && bEnableTiles);

		bool IsCreateUnrealCollision = AdditionalUnrealCollision != ECollisionEnabled::NoCollision;

		// Create mesh section
		CreateMeshSection(
			TileIndex, MeshDesc->Vertices, MeshDesc->Triangles, MeshDesc->Normals, MeshDesc->UV0,
			MeshDesc->Colors, MeshDesc->Tangents, IsCreateUnrealCollision);
		SetMaterial(TileIndex, Material);
		SetMeshSectionVisible(TileIndex, true);
	}
}

void UAGX_MovableTerrainComponent::UpdateMesh(
	const TArray<std::tuple<int32, int32>>& DirtyHeights)
{
	// Height Function
	auto HeightFunction = [&](const FVector& LocalPos) -> float { return SampleHeights(LocalPos); };

	// Update meshes
	for (auto& kvp : MeshTiles)
	{
		int TileIndex = kvp.Key;
		MeshTile Tile = kvp.Value;

		// Check if we need to update Tile
		bool IsTileDirty = false;
		FBox2D TileBox = FBox2D(Tile.Center - Tile.Size / 2, Tile.Center + Tile.Size / 2); 
		for (auto& d : DirtyHeights)
		{
			double x = std::get<0>(d) * ElementSize;
			double y = std::get<1>(d) * ElementSize;
			FVector2D DirtyPos = FVector2D(x, y) - Size / 2;
			if (TileBox.IsInside(DirtyPos))
			{
				IsTileDirty = true;
				break;
			}
		}
		if (!IsTileDirty)
			continue;

		// Create mesh description
		FVector TileCenter = FVector(Tile.Center.X, Tile.Center.Y, ZOffset);
		auto MeshDesc = UAGX_TerrainMeshUtilities::CreateMeshDescription(
			TileCenter, Tile.Size, Tile.Resolution, UvScaling, HeightFunction,
			bTileSkirts && bEnableTiles);

		// Update mesh section
		UpdateMeshSection(
			TileIndex, MeshDesc->Vertices, MeshDesc->Normals, MeshDesc->UV0, MeshDesc->Colors,
			MeshDesc->Tangents);
	}
}

void UAGX_MovableTerrainComponent::UpdateMeshOnPropertyChanged()
{
	//Hacky: This bool is used to force an update of the mesh in-editor
	bRebuildMesh = false;

	UWorld* World = GetWorld();

	if (World == nullptr || !IsValid(World))
		return;

	if (!World->IsGameWorld() && !IsTemplate())
	{
		// In-Editor

		// Postpone the initialization until the next tick
		// because all properties are not copied yet
		World->GetTimerManager().SetTimerForNextTick(
			[this, World]
			{
				if (!IsValid(World))
					return;

				// Recreate Heights
				InitializeHeights();

				// Recreate Mesh
				InitializeMesh();
			});
	}
	else if (World->IsGameWorld())
	{
		// In-Game

		// Recreate Mesh
		InitializeMesh();
	}
}

#if WITH_EDITOR
void UAGX_MovableTerrainComponent::PostEditChangeChainProperty(FPropertyChangedChainEvent& Event)
{
	Super::PostEditChangeChainProperty(Event);

	// TODO: Only UpdateMesh on certain Property changes
	UpdateMeshOnPropertyChanged();
}

void UAGX_MovableTerrainComponent::PostInitProperties()
{
	Super::PostInitProperties();

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

TArray<UAGX_ShapeComponent*> UAGX_MovableTerrainComponent::GetBedShapes() const
{
	TArray<UAGX_ShapeComponent*> Shapes;
	if (GetOwner() != nullptr)
	{
		for (UAGX_ShapeComponent* ShapeComponent :
			 FAGX_ObjectUtilities::Filter<UAGX_ShapeComponent>(GetOwner()->GetComponents()))
		{
			if (BedShapes.Contains(ShapeComponent->GetFName()))
				Shapes.Add(ShapeComponent);
		}
	}

	return Shapes;
}

TArray<FString> UAGX_MovableTerrainComponent::GetBedShapesOptions() const
{
	TArray<FString> Options;
	for (FName Name :
		 FAGX_ObjectUtilities::GetChildComponentNamesOfType<UAGX_ShapeComponent>(GetOuter()))
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

		FShovelBarrier* ShovelBarrier = ShovelComponent->GetOrCreateNative();
		if (ShovelBarrier == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Shovel '%s' in AGX MovableTerrain '%s' could not create AGX Dynamics "
					 "representation. Ignoring this shovel. It will not be able to deform the "
					 "Terrain."),
				*ShovelComponent->GetName(), *GetName());
			continue;
		}
		check(ShovelBarrier->HasNative());

		bool Added = NativeBarrier.AddShovel(*ShovelBarrier);
		if (!Added)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("AGX MovableTerrain '%s' rejected shovel '%s' in '%s'. Reversing edge directions and "
					 "trying again."),
				*GetName(), *ShovelComponent->GetName(),
				*GetLabelSafe(ShovelComponent->GetOwner()));
			ShovelComponent->SwapEdgeDirections();
			Added = NativeBarrier.AddShovel(*ShovelBarrier);
			if (!Added)
			{
				UE_LOG(
					LogAGX, Error,
					TEXT("AGX MovableTerrain '%s' rejected shovel '%s' in '%s' after edge directions flip. "
						 "Abandoning shovel."),
					*GetName(), *GetNameSafe(ShovelComponent),
					*GetLabelSafe(ShovelComponent->GetOwner()));
			}
		}

	}
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
