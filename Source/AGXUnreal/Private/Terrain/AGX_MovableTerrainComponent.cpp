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
		HeightField.SetHeights(heights);

		// Create Native using Heightfield
		double maxDepth = 200.0;
		NativeBarrier.AllocateNative(HeightField, maxDepth);

		//Set transform
		NativeBarrier.SetRotation(this->GetComponentQuat());
		NativeBarrier.SetPosition(this->GetComponentLocation());
	}
	else
	{
		// Create Bed ShapeBarriers
		TArray<FShapeBarrier*> bedShapeBarriers;
		for (UAGX_ShapeComponent* shape : GetBedGeometries())
		{
			auto sb = shape->GetOrCreateNative();
			sb->SetWorldPosition(
				shape->GetComponentTransform().GetLocation());
			sb->SetWorldRotation(
				shape->GetComponentTransform().GetRotation());
			bedShapeBarriers.Push(sb);
		}

		// Create Native using BedGeometries
		NativeBarrier.AllocateNative(Resolution, bedShapeBarriers, 10.0, 1.0);

		// Set transform
		NativeBarrier.SetRotation(this->GetComponentQuat());
		this->SetWorldLocation(NativeBarrier.GetPosition());
	}

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
	//Creeate height function
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
		// In-Editor: Postpone the initialization for the next tick
		world->GetTimerManager().SetTimerForNextTick(
			[this, world]
			{
				// TEMP: Clamp Size/Resolution to avoid potential crashes
				Size = FVector2D(FMath::Max(Size.X, 1.0f), FMath::Max(Size.Y, 1.0f));
				Resolution = FMath::Clamp(Resolution, 1, 256);

				int resX = Resolution;
				int resY = (Size.Y / Size.X) * Resolution;

				float cellSize = FMath::Min(Size.X / resX, Size.Y / resY);

				auto heightArray = this->GenerateEditorHeights(resX, resY, cellSize, false);
				this->RebuildHeightMesh(Size, resX, resY, heightArray);
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

TArray<float> UAGX_MovableTerrainComponent::GenerateEditorHeights(
	int resX, int resY, float cellSize, bool flipYAxis) const
{
	auto bedMeshComponents = GetBedGeometriesUMeshComponents();

	float ySign = flipYAxis ? -1.0 : 1.0;
	FVector origin = FVector(
		-resX * cellSize / 2.0 + cellSize / 2, -ySign * resY * cellSize / 2.0 + cellSize / 2, 0.0);

	TArray<float> heights;
	heights.SetNumZeroed(resX * resY);
	for (int y = 0; y < resY; y++)
	{
		for (int x = 0; x < resX; x++)
		{
			FVector pos = GetComponentTransform().TransformPosition(
				origin + FVector(x * cellSize, ySign * y * cellSize, 0));

			//float noise = UAGX_TerrainMeshUtilities::GetBrownianNoise(pos, 3, 100, 0.5, 2.0, 2.0);
			float bedHeight = UAGX_TerrainMeshUtilities::GetRaycastedHeight(pos, bedMeshComponents);

			heights[y * resX + x] = bedHeight;
		}
	}

	return heights;
}
