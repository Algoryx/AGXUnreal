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
	float elementSize = NativeBarrier.GetElementSize();
	// FVector2D size = FVector2D(resX * elementSize, resY * elementSize);

	//Rebuild mesh
	RebuildHeightMesh(Size, resX, resY, CurrentHeights);

	//Check to update mesh each tick
	if (UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this))
	{
		PostStepForwardHandle =
			FAGX_InternalDelegateAccessor::GetOnPostStepForwardInternal(*Simulation)
				.AddLambda(
					[this, resX, resY](double)
					{
						//Update CurrentHeights
						NativeBarrier.GetHeights(CurrentHeights, true);

						//Rebuild mesh if needed
						if (NativeBarrier.GetModifiedVertices().Num() > 0)
							RebuildHeightMesh(Size, resX, resY, CurrentHeights);
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

	// Size and resolution
	double elementSize = Size.X / Resolution;
	int resX = Resolution;
	int resY = Size.Y / elementSize;

	// Create heightfields
	TArray<float> initialHeights;
	TArray<float> minimumHeights;
	SetupHeights(initialHeights, minimumHeights, resX, resY, elementSize, true);

	// Create native
	NativeBarrier.AllocateNative(resX, resY, elementSize, initialHeights, minimumHeights);

	// Attach to RigidBody
	if (OwningRigidBody)
		OwningRigidBody->GetNative()->AddTerrain(&NativeBarrier);

	// Set transform
	NativeBarrier.SetRotation(this->GetComponentQuat());
	NativeBarrier.SetPosition(this->GetComponentLocation());

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
				if (!IsValid(world))
					return;

				bool hasBed = BedGeometries.Num() > 0;
				bool isAutoFit = false;

				if (hasBed && isAutoFit)
					AutoFitToBed();
					
				// Size and resolution
				double elementSize = Size.X / Resolution;
				int resX = Resolution;
				int resY = Size.Y / elementSize;

				// Create heightfields
				TArray<float> initialHeights;
				TArray<float> minimumHeights;
				SetupHeights(initialHeights, minimumHeights, resX, resY, elementSize, false);
				
				// Rebuild Mesh
				if (resX * resY == initialHeights.Num() && initialHeights.Num() != 0)
					this->RebuildHeightMesh(Size, resX, resY, initialHeights);
			});
	}
}

void UAGX_MovableTerrainComponent::AutoFitToBed()
{
	// Calculate Bounds and BottomCenter
	auto bedMeshComponents = GetBedGeometriesUMeshComponents();
	FBox bounds = CreateEncapsulatingBoundingBox(bedMeshComponents, this->GetComponentTransform());
	FVector bottomCenter = bounds.GetCenter() - FVector(0, 0, bounds.GetExtent().Z);

	// Overwrite Size and Position
	Size = FVector2D(bounds.GetExtent().X * 2, bounds.GetExtent().Y * 2);
	this->SetRelativeLocation(bottomCenter);
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

void UAGX_MovableTerrainComponent::SetupHeights(
	TArray<float>& initialHeights, TArray<float>& minimumHeights, int resX, int resY,
	double elementSize, bool flipYAxis) const
{
	initialHeights.SetNumZeroed(resX * resY);

	for (float& h : initialHeights)
		h += StartHeight;
	AddNoiseHeights(initialHeights, resX, resY, elementSize, flipYAxis);

	minimumHeights.SetNumZeroed(resX * resY);
	if (GetBedGeometries().Num() != 0)
	{
		//Transform to origo before raycasting
		const FTransform oldTransform = this->GetAttachmentRoot()->GetComponentTransform();
		this->GetAttachmentRoot()->SetWorldLocationAndRotation(FVector::Zero(), FQuat::Identity);
		
		AddRaycastedHeights(
			minimumHeights, GetBedGeometriesUMeshComponents(), GetComponentTransform(), resX, resY,
			elementSize, flipYAxis);

		//Restore transform
		this->GetAttachmentRoot()->SetWorldLocationAndRotation(
			oldTransform.GetLocation(), oldTransform.GetRotation());

		for (float& h : minimumHeights)
			h += BedZOffset;
		for (int i = 0; i < initialHeights.Num(); i++)
			initialHeights[i] = FMath::Max(initialHeights[i], minimumHeights[i]);
	}
}

void UAGX_MovableTerrainComponent::AddRaycastedHeights(
	TArray<float>& heights,
	const TArray<UMeshComponent*>& meshes, const FTransform& origoTransform, int resX, int resY,
	float cellSize, bool flipYAxis) const
{
	float rayLength = 1000.0f;
	float ySign = flipYAxis ? -1.0 : 1.0;
	FVector origin = FVector(cellSize * (1 - resX) / 2.0, cellSize * ySign * (1 - resY) / 2.0, 0.0);

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
		}
	}
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
