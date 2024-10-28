#include "Terrain/AGX_MovableTerrainComponent.h"
#include "Terrain/AGX_TerrainMeshUtilities.h"
#include "Shapes/HeightFieldShapeBarrier.h"
#include "Shapes/AGX_ShapeComponent.h"
#include "Materials/AGX_ShapeMaterial.h"
#include "Materials/AGX_TerrainMaterial.h"
#include "AGX_InternalDelegateAccessor.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_LogCategory.h"

#include "Engine/SimpleConstructionScript.h"
#include "Engine/SCS_Node.h"
#include <AGX_Simulation.h>

void UAGX_MovableTerrainComponent::BeginPlay()
{
	Super::BeginPlay();

	//Create Native
	CreateNative();
	
	//Get Native size and resolution
	int ResX = NativeBarrier.GetGridSizeX();
	int ResY = NativeBarrier.GetGridSizeY();
	float ElementSize = NativeBarrier.GetElementSize();

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

	// Copy Native Heights to CurrentHeights
	CurrentHeights.Reserve(NativeBarrier.GetGridSizeX() * NativeBarrier.GetGridSizeY());
	NativeBarrier.GetHeights(CurrentHeights, false);

	// Attach to RigidBody
	if (OwningRigidBody)
		OwningRigidBody->GetNative()->AddTerrain(&NativeBarrier);

	// Set transform
	NativeBarrier.SetRotation(this->GetComponentQuat());
	NativeBarrier.SetPosition(this->GetComponentLocation());

	// Set Native Terrain and Shape properties
	NativeBarrier.SetCanCollide(bCanCollide);
	NativeBarrier.AddCollisionGroups(CollisionGroups);
	NativeBarrier.SetCreateParticles(bCreateParticles);
	NativeBarrier.SetDeleteParticlesOutsideBounds(bDeleteParticlesOutsideBounds);
	NativeBarrier.SetPenetrationForceVelocityScaling(PenetrationForceVelocityScaling);
	NativeBarrier.SetMaximumParticleActivationVolume(MaximumParticleActivationVolume);

	if (!UpdateNativeTerrainMaterial())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("UpdateNativeTerrainMaterial returned false in AGX_Terrain '%s'. "
				 "Ensure the selected Terrain Material is valid."),
			*GetName());
	}

	if (!UpdateNativeShapeMaterial())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("UpdateNativeShapeMaterial returned false in AGX_Terrain '%s'. "
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
	//Create height function
	auto HeightFunction = [&](const FVector& Pos) -> float
	{
		//Samples HeightArray
		return UAGX_TerrainMeshUtilities::SampleHeightArray(
			FVector2D(Pos.X / MeshSize.X + 0.5, (Pos.Y / MeshSize.Y + 0.5)), HeightArray, ResX,
			ResY);
	};

	// Create mesh description
	FIntVector2 MeshRes = FIntVector2(ResX, ResY);
	float UvScaling = 1.0f / 100.0f;
	auto MeshDesc = UAGX_TerrainMeshUtilities::CreateMeshDescription(
		FVector::Zero(), MeshSize, MeshRes, UvScaling, HeightFunction);

	// Create mesh section
	CreateMeshSection(
		0, MeshDesc->Vertices, MeshDesc->Triangles, MeshDesc->Normals, MeshDesc->UV0,
		MeshDesc->Colors, MeshDesc->Tangents, false);
	SetMaterial(0, Material);
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

void UAGX_MovableTerrainComponent::SetupHeights(
	TArray<float>& InitialHeights, TArray<float>& MinimumHeights, int ResX, int ResY,
	double ElementSize, bool FlipYAxis) const
{

	//Setup MinimumHeights
	MinimumHeights.SetNumZeroed(ResX * ResY);
	if (GetBedGeometries().Num() != 0)
	{
		//Transform to origo before raycasting
		const FTransform OldTransform = this->GetAttachmentRoot()->GetComponentTransform();
		this->GetAttachmentRoot()->SetWorldLocationAndRotation(FVector::Zero(), FQuat::Identity);
		
		//Add raycasted heights
		AddBedHeights(MinimumHeights, ResX, ResY, ElementSize, FlipYAxis);

		//Restore transform
		this->GetAttachmentRoot()->SetWorldLocationAndRotation(
			OldTransform.GetLocation(), OldTransform.GetRotation());

		//Add bedZOffset
		for (float& h : MinimumHeights)
			h += BedZOffset;
	}
	
	// Setup InitialHeights
	InitialHeights.SetNumZeroed(ResX * ResY);

	//Add StartHeight
	for (float& h : InitialHeights)
		h += StartHeight;

	//Add Noise
	AddNoiseHeights(InitialHeights, ResX, ResY, ElementSize, FlipYAxis);
	
	// Put MinimumHeights in InitialHeights
	for (int i = 0; i < InitialHeights.Num(); i++)
		InitialHeights[i] = FMath::Max(InitialHeights[i], MinimumHeights[i]);
}

void UAGX_MovableTerrainComponent::AddBedHeights(
	TArray<float>& Heights, int ResX, int ResY, double ElementSize, bool FlipYAxis) const
{
	float RayLength = 1000.0f;
	float SignY = FlipYAxis ? -1.0 : 1.0;
	TArray<UMeshComponent*> BedMeshes =
		FAGX_ObjectUtilities::Filter<UMeshComponent>(GetBedGeometries());

	FTransform WorldTransform = GetComponentTransform();
	FVector Up = WorldTransform.GetRotation().GetUpVector();
	FVector Origin =
		FVector(ElementSize * (1 - ResX) / 2.0, ElementSize * SignY * (1 - ResY) / 2.0, 0.0);

	for (int y = 0; y < ResY; y++)
	{
		for (int x = 0; x < ResX; x++)
		{
			FVector Pos = WorldTransform.TransformPosition(
				Origin + FVector(x * ElementSize, SignY * y * ElementSize, 0));
			float BedHeight =
				UAGX_TerrainMeshUtilities::GetRaycastedHeight(Pos, BedMeshes, Up, RayLength);

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


/*
	--- AGX_Terrain Implementation
	------------------------------
*/

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
