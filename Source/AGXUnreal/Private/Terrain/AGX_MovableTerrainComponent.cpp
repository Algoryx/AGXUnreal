#include "Terrain/AGX_MovableTerrainComponent.h"
#include "Terrain/AGX_TerrainMeshUtilities.h"
#include "Shapes/HeightFieldShapeBarrier.h"
#include "Shapes/AGX_ShapeComponent.h"
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
	int resX = NativeBarrier.GetGridSizeX();
	int resY = NativeBarrier.GetGridSizeY();
	FVector2D size = NativeBarrier.GetSize();

	//Rebuild mesh
	RebuildHeightMesh(size, resX, resY, CurrentHeights);

	//Check to update mesh each tick
	if (UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this))
	{
		PostStepForwardHandle =
			FAGX_InternalDelegateAccessor::GetOnPostStepForwardInternal(*Simulation)
				.AddLambda(
					[this, resX, resY, size](double)
					{
						//Update CurrentHeights
						NativeBarrier.GetHeights(CurrentHeights, true);

						//Rebuild mesh if needed
						if (NativeBarrier.GetModifiedVertices().Num() > 0)
							RebuildHeightMesh(size, resX, resY, CurrentHeights);
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
			// @todo Figure out how to handle Terrain Materials. A Terrain Material can be
			// shared between many Terrains in theory. We only want to remove the Terrain
			// Material from the simulation if this Terrain is the last one using it. Some
			// reference counting may be needed.
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


	if (GetBedGeometries().Num() == 0)
	{
		// Size and resolution
		int resX = Resolution;
		int resY = (Size.Y / Size.X) * Resolution;
		double cellSize = FMath::Min(Size.X / resX, Size.Y / resY);

		// Create HeightfieldBarrier
		FHeightFieldShapeBarrier HeightField;
		HeightField.AllocateNative(resX, resY, resX * cellSize, resY * cellSize);
		TArray<float> heights;
		heights.SetNumZeroed(resX * resY);
		for (float& h : heights)
			h += StartHeight;
		AddNoiseHeights(heights, resX, resY, cellSize, true);
		HeightField.SetHeights(heights);

		// Create Native using Heightfield
		double maxDepth = 200.0;
		NativeBarrier.AllocateNative(HeightField, maxDepth);

	}
	else
	{
		//Set local position to origo
		this->SetRelativeLocation(FVector::Zero());

		TArray<FShapeBarrier*> bedShapeBarriers;
		TArray<FTransform> storedTransforms;
		for (UAGX_ShapeComponent* shape : GetBedGeometries())
		{
			FTransform rootTransform = shape->GetAttachmentRoot()->GetComponentTransform();
			if (UAGX_RigidBodyComponent* OwningBody =
					FAGX_ObjectUtilities::FindFirstAncestorOfType<UAGX_RigidBodyComponent>(*shape))
			{
				OwningBody->GetOrCreateNative();
				rootTransform = OwningBody->GetComponentTransform();
			}

			// Get Bed ShapeBarrier
			auto sb = shape->GetOrCreateNative();
			bedShapeBarriers.Push(sb);

			// Store its current transform
			storedTransforms.Push(
				FTransform(sb->GetLocalRotation(), sb->GetLocalPosition(), FVector::One()));
			
			// Calculate transform to this component's frame of reference
			FVector localPos = rootTransform.InverseTransformPosition(
					this->GetComponentTransform().InverseTransformPosition(
						shape->GetComponentLocation()));
			FQuat localRot = rootTransform.InverseTransformRotation(
					this->GetComponentTransform().InverseTransformRotation(
						shape->GetComponentQuat()));
			
			// Temporarily overwrite transform
			sb->SetLocalPosition(localPos);
			sb->SetLocalRotation(localRot);
		}

		// Create Native using BedGeometries
		NativeBarrier.AllocateNative(Resolution, bedShapeBarriers, BedMarigin, BedZOffset);

		// Restore original shape transforms
		for (int i = 0; i < bedShapeBarriers.Num(); i++)
		{
			auto sb = bedShapeBarriers[i];
			auto st = storedTransforms[i];

			sb->SetLocalPosition(st.GetLocation());
			sb->SetLocalRotation(st.GetRotation());
		}

		// Set local position to nativebarrier
		this->SetRelativeLocation(NativeBarrier.GetPosition());
	}

	// Attach to RigidBody
	if (OwningRigidBody)
		OwningRigidBody->GetNative()->AddTerrain(&NativeBarrier);

	// Set transform
	NativeBarrier.SetRotation(this->GetComponentQuat());
	NativeBarrier.SetPosition(this->GetComponentLocation());


	this->SetWorldLocation(NativeBarrier.GetPosition());
	this->SetWorldRotation(NativeBarrier.GetRotation());

	// Copy Native Heights
	CurrentHeights.Reserve(NativeBarrier.GetGridSizeX() * NativeBarrier.GetGridSizeY());

	NativeBarrier.GetHeights(CurrentHeights, false);


	// Add Native
	UAGX_Simulation* simulation = UAGX_Simulation::GetFrom(this);
	simulation->Add(*this);
}

void UAGX_MovableTerrainComponent::RebuildHeightMesh(
	const FVector2D& size, const int resX, const int resY, const TArray<float>& heightArray)
{
	//Create height function
	auto HeightFunction = [&](const FVector& pos) -> float
	{
		//Samples heightArray
		return UAGX_TerrainMeshUtilities::SampleHeightArray(
			FVector2D(pos.X / size.X + 0.5, (pos.Y / size.Y + 0.5)), heightArray, resX, resY);
	};

	// Create mesh description
	auto meshDesc = UAGX_TerrainMeshUtilities::CreateMeshDescription(
		FVector::Zero(), size, FIntVector2(resX, resY), 1.0f / 100.0f, HeightFunction);

	// Create mesh section
	CreateMeshSection(
		0, meshDesc->Vertices, meshDesc->Triangles, meshDesc->Normals, meshDesc->UV0,
		meshDesc->Colors, meshDesc->Tangents, false);
	SetMaterial(0, Material);
}

TArray<UAGX_ShapeComponent*> UAGX_MovableTerrainComponent::GetBedGeometries() const
{
	TArray<UAGX_ShapeComponent*> shapes;
	if (GetOwner() != nullptr)
	{
		for (auto component : GetOwner()->GetComponents())
		{
			UAGX_ShapeComponent* sComponent = Cast<UAGX_ShapeComponent>(component);
			if (sComponent != nullptr)
			{
				FName name = component->GetFName();
				if (BedGeometries.Contains(name))
				{
					shapes.Add(sComponent);
				}
			}
		}
	}
	return shapes;
}

TArray<FString> UAGX_MovableTerrainComponent::GetBedGeometryOptions() const
{
	TArray<FString> options;
	UBlueprintGeneratedClass* owningGenClass = Cast<UBlueprintGeneratedClass>(GetOuter());
	if (owningGenClass == nullptr)
		return options;

	const TObjectPtr<USimpleConstructionScript> constructionScript =
		owningGenClass->SimpleConstructionScript;
	if (constructionScript == nullptr)
		return options;

	for (const USCS_Node* component : constructionScript->GetAllNodes())
	{
		const UAGX_ShapeComponent* sComponent =
			Cast<UAGX_ShapeComponent>(component->ComponentTemplate);
		if (sComponent != nullptr)
		{
			FName name = component->GetVariableName();
			if (!BedGeometries.Contains(name))
				options.Add(name.ToString());
		}
	}
	return options;
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

void UAGX_MovableTerrainComponent::UpdateInEditorMesh()
{
	if (UWorld* world = GetWorld(); IsValid(world) && !world->IsGameWorld() && !IsTemplate())
	{
		// In-Editor: Postpone the initialization for the next tick because all properties are not copied yet
		world->GetTimerManager().SetTimerForNextTick(
			[this, world]
			{
				if (IsValid(world))
				{
					bool hasBed = BedGeometries.Num() > 0;
					
					int resX, resY;
					double elementSize;
					TArray<float> heightArray;

					// Store world transform and temporarily move to origo
					auto storedTransform =
						FTransform(this->GetAttachmentRoot()->GetComponentTransform());
					this->GetAttachmentRoot()->SetWorldLocationAndRotation(
						FVector::Zero(), FQuat::Identity);


					if (!hasBed)
					{
						// Setup size and resolution
						resX = Resolution;
						resY = (Size.Y / Size.X) * Resolution;
						elementSize = Size.X / (Resolution - 1);
						
						// Setup heightArray
						heightArray.SetNumZeroed(resX * resY); 
						for (float& h : heightArray)
							h += StartHeight;
						AddNoiseHeights(heightArray, resX, resY, elementSize, false);
					}
					else
					{
						//Calculate Bounds and BottomCenter
						auto bedMeshComponents = GetBedGeometriesUMeshComponents();
						FBox bounds = CreateEncapsulatingBoundingBox(
							bedMeshComponents, this->GetComponentTransform());
						FVector bottomCenter =
							bounds.GetCenter() - FVector(0, 0, bounds.GetExtent().Z);

						// Setup size and resolution
						Size = FVector2D(bounds.GetExtent().X*2, bounds.GetExtent().Y*2) - FVector2D(BedMarigin*2, BedMarigin*2);
						elementSize = Size.X / (Resolution - 1);
						resX = Resolution;
						resY = (Size.Y / elementSize) + 1;
						
						// Setup heightArray
						heightArray.SetNumZeroed(resX * resY);
						for (float& h : heightArray)
							h += StartHeight;
						AddNoiseHeights(heightArray, resX, resY, elementSize, false);
				
						// Add bedHeight
						TArray<float> bedHeights;
						bedHeights.SetNumZeroed(resX * resY);
						this->SetRelativeLocation(bottomCenter);
						float minHeight = AddRaycastedHeights(
							bedHeights,
							bedMeshComponents, this->GetComponentTransform(), resX, resY,
							elementSize, false);
						for (float& h : bedHeights)
							h -= minHeight;
						for (int i = 0; i < heightArray.Num(); i++)
							heightArray[i] = FMath::Max(heightArray[i], bedHeights[i]);

						// Overwrite Position
						this->SetRelativeLocation(
							bottomCenter + FVector(0, 0, minHeight + BedZOffset));
					}

					// Restore world transform
					this->GetAttachmentRoot()->SetWorldLocationAndRotation(
						storedTransform.GetLocation(), storedTransform.GetRotation());

					//Build heightmesh
					if (resX * resY == heightArray.Num() && heightArray.Num() != 0)
						this->RebuildHeightMesh(Size, resX, resY, heightArray);
				}
			});
	}
}

TArray<UMeshComponent*> UAGX_MovableTerrainComponent::GetBedGeometriesUMeshComponents() const
{
	TArray<UMeshComponent*> meshes;

	if (GetOwner() != nullptr)
	{
		auto shapes = GetBedGeometries();

		for (auto shape : shapes)
		{
			UMeshComponent* uMeshComponent = Cast<UMeshComponent>(shape);

			if (uMeshComponent != nullptr)
			{
				meshes.Add(uMeshComponent);
			}
		}
	}

	return meshes;
}

float UAGX_MovableTerrainComponent::AddRaycastedHeights(
	TArray<float>& heights,
	const TArray<UMeshComponent*>& meshes, const FTransform& origoTransform, int resX, int resY,
	float cellSize, bool flipYAxis) const
{
	float rayLength = 1000.0f;
	float ySign = flipYAxis ? -1.0 : 1.0;
	FVector origin = FVector(
		-resX * cellSize / 2.0 + cellSize / 2, -ySign * resY * cellSize / 2.0 + cellSize / 2, 0.0);

	float minElement = rayLength;
	FVector up = origoTransform.GetRotation().GetUpVector();

	for (int y = 0; y < resY; y++)
	{
		for (int x = 0; x < resX; x++)
		{
			FVector pos = origoTransform.TransformPosition(origin + 
				FVector(x * cellSize, ySign * y * cellSize, 0));
			float bedHeight =
				UAGX_TerrainMeshUtilities::GetRaycastedHeight(pos, meshes, up, rayLength);

			heights[y * resX + x] += bedHeight;
			minElement = FMath::Min(minElement, bedHeight);
		}
	}

	return minElement;
}

void UAGX_MovableTerrainComponent::AddNoiseHeights(
	TArray<float>& heights, int resX, int resY, float cellSize, bool flipYAxis) const
{

	float ySign = flipYAxis ? -1.0 : 1.0;
	for (int y = 0; y < resY; y++)
	{
		for (int x = 0; x < resX; x++)
		{
			FVector pos = FVector(x * cellSize, ySign * y * cellSize, 0);
			
			float noise = UAGX_TerrainMeshUtilities::GetBrownianNoise(pos, 3, 20, 0.5f, 2.0f, 2.0f);

			heights[y * resX + x] += noise*NoiseHeight;
		}
	}
}

FBox UAGX_MovableTerrainComponent::CreateEncapsulatingBoundingBox(
	const TArray<UMeshComponent*>& Meshes, const FTransform& origoTransform)
{
	FBox EncapsulatingBoundingBox(EForceInit::ForceInit);
	for (auto* Mesh : Meshes)
	{
		if (Mesh)
		{
			FVector Origin, BoxExtent;
			BoxExtent = origoTransform.InverseTransformVector(Mesh->Bounds.BoxExtent);
			Origin = origoTransform.InverseTransformPosition(Mesh->Bounds.Origin);

			FBox ComponentBox(Origin - BoxExtent, Origin + BoxExtent);

			EncapsulatingBoundingBox += ComponentBox;
		}

	}
	return EncapsulatingBoundingBox;
}
