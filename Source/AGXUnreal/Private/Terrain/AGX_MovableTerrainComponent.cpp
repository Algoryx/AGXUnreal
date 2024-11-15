#include "Terrain/AGX_MovableTerrainComponent.h"
#include "Terrain/AGX_TerrainMeshUtilities.h"
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

	//Create Native
	CreateNative();
	
	// TODO: Copy BedHeights and CurrentHeights from Native and remove call to SetupHeights
	// CurrentHeights.Reserve(TerrainResolution.X * TerrainResolution.Y);
	// NativeBarrier.GetHeights(CurrentHeights, false);
	// BedHeights.Reserve(TerrainResolution.X * TerrainResolution.Y);
	// NativeBarrier.GetMinimumHeights(BedHeights);
	SetupHeights(CurrentHeights, BedHeights, GetTerrainResolution(), false);

	// Create Mesh(s) and Tile(s)
	InitializeMesh();

	//Check to update mesh each tick
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

// Called every frame
void UAGX_MovableTerrainComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{

	TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("AGXUnreal:AAGX_MovableTerrain::Tick"));
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (bEnableParticleRendering)
	{
		UpdateParticles();
	}
}

void UAGX_MovableTerrainComponent::CreateNative()
{
	UAGX_RigidBodyComponent* OwningRigidBody =
		FAGX_ObjectUtilities::FindFirstAncestorOfType<UAGX_RigidBodyComponent>(*this);

	if (OwningRigidBody)
		OwningRigidBody->GetOrCreateNative();

	// Resolution
	FIntVector2 TerrainResolution = GetTerrainResolution();
	
	// Create heightfields
	TArray<float> InitialHeights;
	TArray<float> MinimumHeights;
	SetupHeights(InitialHeights, MinimumHeights, TerrainResolution, true);

	// Create native
	NativeBarrier.AllocateNative(
		TerrainResolution.X, TerrainResolution.Y, ElementSize, InitialHeights, MinimumHeights);


	// Make sure Native Resoltion and ElementSize are correct
	ensureMsgf(
		FMath::IsNearlyEqual(ElementSize, NativeBarrier.GetElementSize(), KINDA_SMALL_NUMBER),
		TEXT("ElementSize and NativeBarrier.GetElementSize() are not nearly equal. ElementSize: "
			 "%f, NativeBarrier.GetElementSize(): %f"),
		ElementSize, NativeBarrier.GetElementSize());

	ensureMsgf(
		FMath::IsNearlyEqual(
			TerrainResolution.X, NativeBarrier.GetGridSizeX(), KINDA_SMALL_NUMBER) &&
			FMath::IsNearlyEqual(
				TerrainResolution.Y, NativeBarrier.GetGridSizeY(), KINDA_SMALL_NUMBER),
		TEXT("TerrainResolution (X: %f, Y: %f) and NativeBarrier grid size (X: %f, Y: %f) are not "
			 "nearly equal."),
		TerrainResolution.X, TerrainResolution.Y, NativeBarrier.GetGridSizeX(),
		NativeBarrier.GetGridSizeY());

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

void UAGX_MovableTerrainComponent::UpdateInEditorMesh()
{
	if (UWorld* World = GetWorld(); IsValid(World) && !World->IsGameWorld() && !IsTemplate())
	{
		// In-Editor: Postpone the initialization for the next tick because all properties are not
		// copied yet
		World->GetTimerManager().SetTimerForNextTick(
			[this, World]
			{
				if (!IsValid(World))
					return;

				SetupHeights(CurrentHeights, BedHeights, GetTerrainResolution(), false);
				InitializeMesh();
			});
	}
}


float UAGX_MovableTerrainComponent::SampleHeight(FVector LocalPos) const
{
	FVector2D UvCord = FVector2D(LocalPos.X / Size.X + 0.5, LocalPos.Y / Size.Y + 0.5);

	float Epsilon = 1e-6;
	bool IsOnBorder = UvCord.X < Epsilon || UvCord.Y < Epsilon || UvCord.X > 1 - Epsilon ||
					  UvCord.Y > 1 - Epsilon;

	// Sample from MinimumHeights when close to border
	auto& SampleArray = ClampToBorders && IsOnBorder ? BedHeights : CurrentHeights;

	return UAGX_TerrainMeshUtilities::SampleHeightArray(
		UvCord, SampleArray, GetTerrainResolution().X, GetTerrainResolution().Y);
}

void UAGX_MovableTerrainComponent::InitializeMesh()
{
	// Height Function
	auto HeightFunction = [&](const FVector& LocalPos) -> float { return SampleHeight(LocalPos); };

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
	auto HeightFunction = [&](const FVector& LocalPos) -> float { return SampleHeight(LocalPos); };

	// Update meshes
	for (auto& kvp : MeshTiles)
	{
		int TileIndex = kvp.Key;
		MeshTile Tile = kvp.Value;

		//Check if we need to update this Tile
		bool IsTileDirty = false;
		FBox2D TileBox = FBox2D(Tile.Center - Tile.Size / 2, Tile.Center + Tile.Size / 2); 
		for (auto d : DirtyHeights)
		{
			float x = std::get<0>(d) * ElementSize;
			float y = std::get<1>(d) * ElementSize;
			FVector2D HeightPos = FVector2D(x, y) - Size / 2;
			if (TileBox.IsInside(HeightPos))
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

void UAGX_MovableTerrainComponent::SetupHeights(
	TArray<float>& InitialHeights, TArray<float>& MinimumHeights, const FIntVector2& Res,
	bool FlipYAxis) const
{
	//Setup MinimumHeights
	MinimumHeights.Reset();
	MinimumHeights.SetNumZeroed(Res.X * Res.Y);
	if (GetBedShapes().Num() != 0)
		AddBedHeights(MinimumHeights, Res, FlipYAxis);
	
	// Setup InitialHeights
	InitialHeights.Reset();
	InitialHeights.SetNumZeroed(Res.X * Res.Y);
	if (bEnableNoise)
		AddNoiseHeights(InitialHeights, Res, FlipYAxis);
	for (float& h : InitialHeights)
		h += StartHeight;

	// Put MinimumHeights in InitialHeights
	for (int i = 0; i < InitialHeights.Num(); i++)
		InitialHeights[i] = FMath::Max(InitialHeights[i], MinimumHeights[i]);
}

void UAGX_MovableTerrainComponent::AddBedHeights(
	TArray<float>& Heights, const FIntVector2& Res, bool FlipYAxis) const
{
	auto Shapes = GetBedShapes();
	float SignY = FlipYAxis ? -1.0 : 1.0;
	FVector Up = GetComponentQuat().GetUpVector();
	FVector Center =
		FVector(ElementSize * (1 - Res.X) / 2.0, ElementSize * SignY * (1 - Res.Y) / 2.0, 0.0);

	for (int y = 0; y < Res.Y; y++)
	{
		for (int x = 0; x < Res.X; x++)
		{
			FVector Pos = GetComponentTransform().TransformPosition(
				Center + FVector(x * ElementSize, SignY * y * ElementSize, 0));
			float BedHeight = UAGX_TerrainMeshUtilities::GetLineTracedHeight(Pos, Shapes, Up);

			Heights[y * Res.X + x] += BedHeight;
		}
	}
}

void UAGX_MovableTerrainComponent::AddNoiseHeights(
	TArray<float>& Heights, const FIntVector2& Res, bool FlipYAxis) const
{
	float SignY = FlipYAxis ? -1.0 : 1.0;
	FVector Up = GetComponentQuat().GetUpVector();
	FVector Center =
		FVector(ElementSize * (1 - Res.X) / 2.0, ElementSize * SignY * (1 - Res.Y) / 2.0, 0.0);

	for (int y = 0; y < Res.Y; y++)
	{
		for (int x = 0; x < Res.X; x++)
		{
			FVector Pos = GetComponentTransform().TransformPosition(
				Center + FVector(x * ElementSize, SignY * y * ElementSize, 0));

			//Project to plane
			Pos = Pos -  Up*FVector::DotProduct(Pos, Up);
			
			float Noise = UAGX_TerrainMeshUtilities::GetBrownianNoise(
				Pos, BrownianNoise.Octaves, BrownianNoise.Scale, BrownianNoise.Persistance,
				BrownianNoise.Lacunarity, BrownianNoise.Exp);

			Heights[y * Res.X + x] += Noise * BrownianNoise.Height;
		}
	}
}


void UAGX_MovableTerrainComponent::PostEditChangeProperty(FPropertyChangedEvent& event)
{
	Super::PostEditChangeProperty(event);
	UpdateInEditorMesh();
}

void UAGX_MovableTerrainComponent::PostInitProperties()
{
	Super::PostInitProperties();
	UpdateInEditorMesh();
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
