#include "Terrain/AGX_MovableTerrainComponent.h"
#include "Terrain/AGX_TerrainMeshUtilities.h"
#include "Shapes/HeightFieldShapeBarrier.h"
#include "AGX_InternalDelegateAccessor.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_LogCategory.h"

#include "Engine/SimpleConstructionScript.h"
#include "Engine/SCS_Node.h"
#include <AGX_Simulation.h>

void UAGX_MovableTerrainComponent::BeginPlay()
{
	Super::BeginPlay();

	Size = FVector2D(FMath::Max(Size.X, 1.0f), FMath::Max(Size.Y, 1.0f));
	FIntVector2 resolutionXY = FIntVector2(
		FMath::Clamp(Resolution, 1, 256), FMath::Clamp((Size.Y / Size.X) * Resolution, 1, 256));
	double cellSize = FMath::Max(Size.X / resolutionXY.X, Size.Y / resolutionXY.Y);

	CreateNative(resolutionXY, cellSize);
	RebuildHeightMesh(Size, resolutionXY, CurrentHeights);

	if (UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this))
	{
		// Call NativeBarrier.GetHeights and RebuildHeightMesh
		auto PostStepForwardHandle =
			FAGX_InternalDelegateAccessor::GetOnPostStepForwardInternal(*Simulation)
				.AddLambda(
					[this, resolutionXY](double)
					{
						NativeBarrier.GetHeights(CurrentHeights, true);
						if (NativeBarrier.GetModifiedVertices().Num() > 0)
							RebuildHeightMesh(Size, resolutionXY, CurrentHeights);
					});
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

void UAGX_MovableTerrainComponent::CreateNative(const FIntVector2& resolutionXY, double cellSize)
{
	// Create Heightfield
	FHeightFieldShapeBarrier HeightField;
	HeightField.AllocateNative(
		resolutionXY.X, resolutionXY.Y, resolutionXY.X * cellSize, resolutionXY.Y * cellSize);
	TArray<float> generatedHeights =
		GenerateHeights(resolutionXY.X, resolutionXY.Y, cellSize, true);
	HeightField.SetHeights(generatedHeights);

	// Create Native
	double maxDepth = 200.0;
	NativeBarrier.AllocateNative(HeightField, maxDepth);
	NativeBarrier.SetRotation(this->GetComponentQuat());
	NativeBarrier.SetPosition(this->GetComponentLocation());

	// Copy Native Heights
	CurrentHeights.Reserve(resolutionXY.X * resolutionXY.Y);
	NativeBarrier.GetHeights(CurrentHeights, false);

	// Add Native
	UAGX_Simulation* simulation = UAGX_Simulation::GetFrom(this);
	simulation->Add(*this);
}

void UAGX_MovableTerrainComponent::RebuildHeightMesh(
	const FVector2D& size, const FIntVector2& resolutionXY, const TArray<float>& heightArray)
{
	auto HeightFunction = [&](const FVector& pos) -> float
	{
		return UAGX_TerrainMeshUtilities::SampleHeightArrayBiLerp(
			FVector2D(pos.X / size.X + 0.5, (pos.Y / size.Y + 0.5)), heightArray, resolutionXY.X,
			resolutionXY.Y);
	};

	// Create mesh description
	auto meshDesc = UAGX_TerrainMeshUtilities::CreateMeshDescription(
		FVector::Zero(), size, resolutionXY, 1.0f / 100.0f, HeightFunction);

	// Create procedural mesh section
	CreateMeshSection(
		0, meshDesc->Vertices, meshDesc->Triangles, meshDesc->Normals, meshDesc->UV0,
		meshDesc->Colors, meshDesc->Tangents, false);
}

TArray<UAGX_RigidBodyComponent*> UAGX_MovableTerrainComponent::GetBedGeometries() const
{
	TArray<UAGX_RigidBodyComponent*> bodies;
	if (GetOwner() != nullptr)
	{
		for (auto component : GetOwner()->GetComponents())
		{
			UAGX_RigidBodyComponent* rbComponent = Cast<UAGX_RigidBodyComponent>(component);
			if (rbComponent != nullptr)
			{
				FName name = component->GetFName();
				if (BedGeometries.Contains(name))
				{
					bodies.Add(rbComponent);
				}
			}
		}
	}
	return bodies;
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
		const UAGX_RigidBodyComponent* rbComponent =
			Cast<UAGX_RigidBodyComponent>(component->ComponentTemplate);
		if (rbComponent != nullptr)
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
				FIntVector2 resolutionXY = FIntVector2(
					FMath::Clamp(Resolution, 1, 256),
					FMath::Clamp((Size.Y / Size.X) * Resolution, 1, 256));
				float cellSize = FMath::Max(Size.X / resolutionXY.X, Size.Y / resolutionXY.Y);

				auto heightArray =
					this->GenerateHeights(resolutionXY.X, resolutionXY.Y, cellSize, false);
				this->RebuildHeightMesh(Size, resolutionXY, heightArray);
			});
	}
}

TArray<UMeshComponent*> UAGX_MovableTerrainComponent::GetBedGeometriesUMeshComponents() const
{
	TArray<UMeshComponent*> meshes;

	if (GetOwner() != nullptr)
	{
		auto rigidBodies = GetBedGeometries();

		for (auto rb : rigidBodies)
		{
			TArray<USceneComponent*> children;
			rb->GetChildrenComponents(true, children);

			for (auto child : children)
			{
				UMeshComponent* uMeshComponent = Cast<UMeshComponent>(child);

				if (uMeshComponent != nullptr)
				{
					meshes.Add(uMeshComponent);
				}
			}
		}
	}

	return meshes;
}

TArray<float> UAGX_MovableTerrainComponent::GenerateHeights(
	int resX, int resY, float cellSize, bool flipYAxis) const
{
	TArray<UMeshComponent*> bedMeshComponents = GetBedGeometriesUMeshComponents();

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

			float noise = UAGX_TerrainMeshUtilities::GetBrownianNoise(pos, 3, 100, 0.5, 2.0, 2.0);
			float bedHeight = UAGX_TerrainMeshUtilities::GetRaycastedHeight(pos, bedMeshComponents);

			heights[y * resX + x] = bedHeight + noise * 50.0f;
		}
	}

	return heights;
}
