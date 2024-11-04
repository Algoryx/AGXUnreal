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
	
	//Get Native size and resolution
	int ResX = NativeBarrier.GetGridSizeX();
	int ResY = NativeBarrier.GetGridSizeY();
	float ElementSize = NativeBarrier.GetElementSize();

	// Copy Native Heights to CurrentHeights
	CurrentHeights.Reserve(ResX * ResY);
	NativeBarrier.GetHeights(CurrentHeights, false);

	//Rebuild mesh
	RebuildHeightMesh(Size, ResX, ResY, CurrentHeights);

	//Check to update mesh each tick
	if (UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this))
	{
		PostStepForwardHandle =
			FAGX_InternalDelegateAccessor::GetOnPostStepForwardInternal(*Simulation)
				.AddLambda(
					[this, ResX, ResY](double)
					{
						//Update CurrentHeights
						NativeBarrier.GetHeights(CurrentHeights, true);

						//Rebuild mesh if needed
						if (NativeBarrier.GetModifiedVertices().Num() > 0)
							RebuildHeightMesh(Size, ResX, ResY, CurrentHeights);
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

	// Size and resolution
	double ElementSize = Size.X / (Resolution-1);
	int ResX = Resolution;
	int ResY = Size.Y / (ElementSize) + 1;

	// Create heightfields
	TArray<float> InitialHeights;
	TArray<float> MinimumHeights;
	SetupHeights(InitialHeights, MinimumHeights, ResX, ResY, ElementSize, true);

	// Create native
	NativeBarrier.AllocateNative(ResX, ResY, ElementSize, InitialHeights, MinimumHeights);

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

				//bool HasBed = BedGeometries.Num() > 0;
				//bool IsAutoFit = false;
				//if (HasBed && IsAutoFit)
				//	AutoFitToBed();

				// Size and resolution
				double ElementSize = Size.X / (Resolution - 1);
				int ResX = Resolution;
				int ResY = Size.Y / (ElementSize) + 1;

				// Create heightfields
				TArray<float> InitialHeights;
				TArray<float> MinimumHeights;
				SetupHeights(InitialHeights, MinimumHeights, ResX, ResY, ElementSize, false);

				// Rebuild Mesh
				if (ResX * ResY == InitialHeights.Num() && InitialHeights.Num() != 0)
					this->RebuildHeightMesh(Size, ResX, ResY, InitialHeights);
			});
	}
}

void UAGX_MovableTerrainComponent::RebuildHeightMesh(
	const FVector2D& MeshSize, const int ResX, const int ResY, const TArray<float>& HeightArray)
{
	FVector MeshCenter = FVector::Zero();
	FIntVector2 MeshRes = FIntVector2(ResX, ResY);
	float UvScaling = 1.0f / 100.0f;


	//Create height function
	auto HeightFunction = [&](const FVector& Pos) -> float
	{
		FVector2D UvCord = FVector2D((Pos.X - MeshCenter.X) / MeshSize.X + 0.5, (Pos.Y - MeshCenter.Y) / MeshSize.Y + 0.5);

		//Samples HeightArray
		return UAGX_TerrainMeshUtilities::SampleHeightArray(UvCord, HeightArray, ResX, ResY);
	};

	// Create mesh description
	auto MeshDesc = UAGX_TerrainMeshUtilities::CreateMeshDescription(
		MeshCenter, MeshSize, MeshRes, UvScaling, HeightFunction);

	// Create mesh section
	CreateMeshSection(
		0, MeshDesc->Vertices, MeshDesc->Triangles, MeshDesc->Normals, MeshDesc->UV0,
		MeshDesc->Colors, MeshDesc->Tangents, false);
	SetMaterial(0, Material);
}

void UAGX_MovableTerrainComponent::SetupHeights(
	TArray<float>& InitialHeights, TArray<float>& MinimumHeights, int ResX, int ResY,
	double ElementSize, bool FlipYAxis) const
{
	//Setup MinimumHeights
	MinimumHeights.SetNumZeroed(ResX * ResY);
	if (GetBedGeometries().Num() != 0)
	{
		//Add raycasted heights
		AddBedHeights(MinimumHeights, ResX, ResY, ElementSize, FlipYAxis);

		//Add bedZOffset
		for (float& h : MinimumHeights)
			h += BedOffset;
	}
	

	// Setup InitialHeights
	InitialHeights.SetNumZeroed(ResX * ResY);

	// Add Noise
	AddNoiseHeights(InitialHeights, ResX, ResY, ElementSize, FlipYAxis);

	//Add StartHeight
	for (float& h : InitialHeights)
		h += StartHeight;
	
	// Put MinimumHeights in InitialHeights
	for (int i = 0; i < InitialHeights.Num(); i++)
		InitialHeights[i] = FMath::Max(InitialHeights[i], MinimumHeights[i]);
}

void UAGX_MovableTerrainComponent::AddBedHeights(
	TArray<float>& Heights, int ResX, int ResY, double ElementSize, bool FlipYAxis) const
{
	float SignY = FlipYAxis ? -1.0 : 1.0;
	TArray<UMeshComponent*> BedMeshes =
		FAGX_ObjectUtilities::Filter<UMeshComponent>(GetBedGeometries());

	FVector Up = GetComponentQuat().GetUpVector();
	FVector Origin =
		FVector(ElementSize * (1 - ResX) / 2.0, ElementSize * SignY * (1 - ResY) / 2.0, 0.0);

	for (int y = 0; y < ResY; y++)
	{
		for (int x = 0; x < ResX; x++)
		{
			FVector Pos = GetComponentTransform().TransformPosition(
				Origin + FVector(x * ElementSize, SignY * y * ElementSize, 0));
			float BedHeight =
				UAGX_TerrainMeshUtilities::GetRaycastedHeight(Pos, BedMeshes, Up);

			Heights[y * ResX + x] += BedHeight;
		}
	}
}

void UAGX_MovableTerrainComponent::AddNoiseHeights(
	TArray<float>& Heights, int ResX, int ResY, double ElementSize, bool FlipYAxis) const
{
	float SignY = FlipYAxis ? -1.0 : 1.0;
	for (int y = 0; y < ResY; y++)
	{
		for (int x = 0; x < ResX; x++)
		{
			FVector Pos = FVector(x * ElementSize, SignY * y * ElementSize, 0);
			
			float Noise = UAGX_TerrainMeshUtilities::GetBrownianNoise(Pos, 3, 20, 0.5f, 2.0f, 2.0f);

			Heights[y * ResX + x] += Noise * NoiseHeight;
		}
	}
}

void UAGX_MovableTerrainComponent::AutoFitToBed()
{
	// Calculate Bounds and BottomCenter
	TArray<UMeshComponent*> BedMeshComponents =
		FAGX_ObjectUtilities::Filter<UMeshComponent>(GetBedGeometries());
	FBox BoundingBox = UAGX_TerrainMeshUtilities::CreateEncapsulatingBoundingBox(
		BedMeshComponents, this->GetComponentTransform());
	FVector BottomCenter = BoundingBox.GetCenter() - FVector(0, 0, BoundingBox.GetExtent().Z);

	// Overwrite Size
	Size = FVector2D(BoundingBox.GetExtent().X * 2, BoundingBox.GetExtent().Y * 2);

	// Overwrite Position
	this->SetRelativeLocation(BottomCenter);
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

TArray<UAGX_ShapeComponent*> UAGX_MovableTerrainComponent::GetBedGeometries() const
{
	TArray<UAGX_ShapeComponent*> Shapes;
	if (GetOwner() != nullptr)
	{
		for (UAGX_ShapeComponent* ShapeComponent :
			 FAGX_ObjectUtilities::Filter<UAGX_ShapeComponent>(GetOwner()->GetComponents()))
		{
			if (BedGeometries.Contains(ShapeComponent->GetFName()))
				Shapes.Add(ShapeComponent);
		}
	}

	return Shapes;
}

TArray<FString> UAGX_MovableTerrainComponent::GetBedGeometryOptions() const
{
	TArray<FString> Options;
	for (FName Name :
		 FAGX_ObjectUtilities::GetChildComponentNamesOfType<UAGX_ShapeComponent>(GetOuter()))
	{
		if (!BedGeometries.Contains(Name))
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

void UAGX_MovableTerrainComponent::SetNoMerge(bool IsNoMerge)
{
	if (HasNative())
		NativeBarrier.SetNoMerge(IsNoMerge);
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
