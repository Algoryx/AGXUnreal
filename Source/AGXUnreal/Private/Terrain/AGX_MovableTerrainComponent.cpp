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
#include <AGX_NativeOwnerInstanceData.h>

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

	if (GIsReconstructingBlueprintInstances)
	{
		// This Component will soon be given a Native Geometry and Shape from a
		// FAGX_NativeOwnerInstanceData, so don't create a new one here.
		return;
	}

	// Create Native
	GetOrCreateNative();

	if (!HasNative())
		return;

	// Init particles
	InitializeParticles();

	// Create Mesh
	RecreateMeshes();

	// Unreal Collision
	this->SetCollisionEnabled(AdditionalUnrealCollision);
		
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
						FVector2D NativeTerrainSize = GetTerrainSize();
						// Callback to check if a Tile contain any ModifiedHeights
						auto IsTileDirty = [&](const MeshTile& Tile) -> bool
						{
							FVector2D Epsilon = FVector2D(ElementSize, ElementSize);
							FVector2D TilePlaneCenter = FVector2D(Tile.Center.X, Tile.Center.Y);
							FBox2D TileBox = FBox2D(
								TilePlaneCenter - Tile.Size / 2 - Epsilon,
								TilePlaneCenter + Tile.Size / 2 + Epsilon);
							for (auto& HeightVertexTuple : ModifiedHeights)
							{
								double x = std::get<0>(HeightVertexTuple);
								double y = std::get<1>(HeightVertexTuple);
								FVector2D ModifiedPos =
									FVector2D(x, y) * ElementSize - NativeTerrainSize / 2;
								if (TileBox.IsInside(ModifiedPos))
								{
									return true;
								}
							}

							return false;
						};

						if (ModifiedHeights.Num() > 0)
						{
							// Update CurrentHeights
							NativeBarrier.GetHeights(CurrentHeights, true);

							// Update Dirty TerrainMesh Tiles
							for (auto& Tile : TerrainMesh)
							{
								if (IsTileDirty(Tile))
									UpdateTileMeshSection(Tile, EAGX_MeshType::Terrain);
							}
						}
					});
	}
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

	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("AGX MovableTerrain '%s' in '%s' failed AllocateNative. "),
			*GetName(), *GetLabelSafe(GetOwner()));

		return;
	}


	// Attach to RigidBody
	if (OwningRigidBody)
		OwningRigidBody->GetNative()->AddTerrain(&NativeBarrier);

	// Set transform
	NativeBarrier.SetRotation(this->GetComponentQuat());
	NativeBarrier.SetPosition(this->GetComponentLocation());

	// Set Native Properties
	UpdateNativeProperties();
	

	// Add Native
	UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this);
	Simulation->Add(*this);
}


float UAGX_MovableTerrainComponent::GetCurrentHeight(const FVector& LocalPos) const
{
	return UAGX_TerrainMeshUtilities::SampleHeightArray(
		ToUv(LocalPos, Size), CurrentHeights, GetTerrainResolution().X, GetTerrainResolution().Y);
}

float UAGX_MovableTerrainComponent::GetBedHeight(const FVector& LocalPos) const
{
	return UAGX_TerrainMeshUtilities::SampleHeightArray(
		ToUv(LocalPos, Size), BedHeights, GetTerrainResolution().X,
		GetTerrainResolution().Y);
}

void UAGX_MovableTerrainComponent::InitializeHeights()
{
	FIntVector2 Res = GetTerrainResolution();
	CurrentHeights.Reset();
	CurrentHeights.SetNumZeroed(Res.X * Res.Y);

	BedHeights.Reset();
	BedHeights.SetNumZeroed(Res.X * Res.Y);

	FVector Center = FVector(ElementSize * (1 - Res.X) / 2.0, ElementSize * (1 - Res.Y) / 2.0, 0.0);

	//This can be run faster
	for (int y = 0; y < Res.Y; y++)
	{
		for (int x = 0; x < Res.X; x++)
		{
			FVector LocalPos = Center + FVector(x * ElementSize, y * ElementSize, 0);
			BedHeights[y * Res.X + x] = CalcInitialBedHeight(LocalPos);

			CurrentHeights[y * Res.X + x] = CalcInitialHeight(LocalPos);
		}
	}
}

float UAGX_MovableTerrainComponent::CalcInitialHeight(const FVector& LocalPos) const
{
	float NoiseHeight = bUseInitialNoise ? UAGX_TerrainMeshUtilities::GetNoiseHeight(
											   LocalPos, GetComponentTransform(), InitialNoise)
										 : 0.0f;
	float BedHeight = bUseBedShapes ? CalcInitialBedHeight(LocalPos) : 0.0f;


	return FMath::Max(InitialHeight, BedHeight) + NoiseHeight;
}

float UAGX_MovableTerrainComponent::CalcInitialBedHeight(const FVector& LocalPos) const
{
	return !bUseBedShapes || GetBedShapes().Num() == 0
			   ? 0.0f
			   : UAGX_TerrainMeshUtilities::GetBedHeight(
					 LocalPos, GetComponentTransform(), GetBedShapes()) +
					 BedZOffset;
}



void UAGX_MovableTerrainComponent::RecreateMeshes()
{
	FIntVector2 AutoMeshResolution = FIntVector2(GetTerrainResolution().X - 1, GetTerrainResolution().Y - 1);
	// Reset MeshSections
	ClearAllMeshSections();

	int MeshIndex = 0;

	// Terrain Mesh 
	TerrainMesh.Reset();
	TerrainMesh = CreateTiledMesh(
		MeshIndex, 
		FVector::Zero(), Size, bAutoMeshResolution ? AutoMeshResolution : MeshResolution,
		GetMeshVertexFunction(EAGX_MeshType::Terrain), 
		Material, MeshLevelOfDetail,
		MeshTilingPattern, true, false, 
		false, !bHideTerrain);
	MeshIndex += TerrainMesh.Num();

	bool bIsUnrealCollision = AdditionalUnrealCollision != ECollisionEnabled::NoCollision;
	int CollisionLOD = 4;

	// Collision Mesh (Low resolution Terrain)
	CollisionMesh.Reset();
	CollisionMesh = CreateTiledMesh(
		MeshIndex, FVector::Zero(), Size, AutoMeshResolution,
		GetMeshVertexFunction(
			EAGX_MeshType::Collision), // GetMeshVertexFunction(EAGX_MeshType::Terrain),
		nullptr, CollisionLOD, EAGX_MeshTilingPattern::StretchedTiles, false, false,
		bIsUnrealCollision, bShowUnrealCollision);
	MeshIndex += CollisionMesh.Num();

	// BackBed (Just a plane at the bottom if there is no BedShapes)
	bool HasShapes = (bUseBedShapes && GetBedShapes().Num() > 0);
	BackBedMesh.Reset();
	BackBedMesh = CreateTiledMesh(
		MeshIndex, FVector::Zero(), Size, HasShapes ? AutoMeshResolution : FIntVector2(1, 1),
		GetMeshVertexFunction(EAGX_MeshType::BackBed), nullptr, CollisionLOD,
		HasShapes ? EAGX_MeshTilingPattern::StretchedTiles : EAGX_MeshTilingPattern::None,
		HasShapes, true, bIsUnrealCollision,
		(!bHideTerrain && bShowMeshBottom) || bShowUnrealCollision);
	MeshIndex += BackBedMesh.Num();

	// DebugPlane 
	DebugPlaneMesh.Reset();
	//Front
	DebugPlaneMesh = CreateTiledMesh(
		MeshIndex, FVector::Zero(), GetTerrainSize(), FIntVector2(1, 1),
		GetMeshVertexFunction(EAGX_MeshType::DebugPlane), nullptr, 0, EAGX_MeshTilingPattern::None,
		true, false, false, bShowDebugPlane);
	MeshIndex += DebugPlaneMesh.Num();
	//Back
	DebugPlaneMesh.Append(CreateTiledMesh(
		MeshIndex, FVector(0, 0, -1.0), GetTerrainSize(), FIntVector2(1, 1),
		GetMeshVertexFunction(EAGX_MeshType::DebugPlane), nullptr, 0, EAGX_MeshTilingPattern::None,
		false, true, false, bShowDebugPlane));
	MeshIndex += DebugPlaneMesh.Num() / 2;


}


TArray<MeshTile> UAGX_MovableTerrainComponent::GenerateMeshTiles(
	const int StartMeshIndex, const FVector& MeshCenter, 
	const FVector2D& MeshSize, const FIntVector2& MeshRes, 
	const int MeshLod, const EAGX_MeshTilingPattern& TilingPattern, bool bWithSkirts,
	bool bReverseWinding) const
{
	int MeshIndex = StartMeshIndex;
	float LodScaling = 1.0f / (FMath::Pow(2.0f, MeshLod));

	FVector2D ScaledResolution = FVector2D(MeshRes.X * LodScaling, MeshRes.Y * LodScaling);


	TArray<MeshTile> Tiles;

	// Create MeshTiles
	if (TilingPattern == EAGX_MeshTilingPattern::None)
	{
		// One single tile
		FVector TileCenter = MeshCenter;
		FVector2D TileSize = MeshSize;
		FIntVector2 TileRes = FIntVector2(
			FMath::Max(1, FMath::RoundToInt(ScaledResolution.X)),
			FMath::Max(1, FMath::RoundToInt(ScaledResolution.Y)));
		FVector2D TileUvScale = FVector2D(1.0 / MeshSize.X, 1.0 / MeshSize.Y);

		Tiles.Add(MeshTile(
			MeshIndex++, TileCenter, TileSize, TileRes, TileUvScale, bWithSkirts, bReverseWinding));
	}
	else if (TilingPattern == EAGX_MeshTilingPattern::StretchedTiles)
	{
		// Multiple tiles in a grid
		FIntVector2 NrOfTiles = FIntVector2(
			FMath::Max(1, FMath::RoundToInt(ScaledResolution.X / MeshTileResolution)),
			FMath::Max(1, FMath::RoundToInt(ScaledResolution.Y / MeshTileResolution)));
		FIntVector2 TileRes = FIntVector2(MeshTileResolution, MeshTileResolution);
		FVector2D TileSize = FVector2D(MeshSize.X / NrOfTiles.X, MeshSize.Y / NrOfTiles.Y);
		FVector2D TileUvScale = FVector2D(1.0 / MeshSize.X, 1.0 / MeshSize.Y);

		for (int Tx = 0; Tx < NrOfTiles.X; Tx++)
		{
			for (int Ty = 0; Ty < NrOfTiles.Y; Ty++)
			{
				FVector2D PlanePos = TileSize / 2 - MeshSize / 2 + FVector2D(Tx * TileSize.X, Ty * TileSize.Y);
				FVector TileCenter = MeshCenter + FVector(PlanePos.X, PlanePos.Y, 0.0);
				Tiles.Add(MeshTile(
					MeshIndex++, TileCenter, TileSize, TileRes, TileUvScale, bWithSkirts,
					bReverseWinding));
			}
		}
	}

	return Tiles;
}

TiledMesh UAGX_MovableTerrainComponent::CreateTiledMesh(
	int StartMeshIndex, const FVector& MeshCenter, const FVector2D& MeshSize,
	const FIntVector2& MeshRes, const FAGX_MeshVertexFunction MeshVertexFunc,
	UMaterialInterface* MeshMaterial, int MeshLod,
	const EAGX_MeshTilingPattern& TilingPattern,
	bool bAddMeshSkirts, bool bMeshReverseWinding, bool bMeshCollision,
	bool bMeshVisible)
{
	TiledMesh Tiles = GenerateMeshTiles(
		StartMeshIndex, MeshCenter, MeshSize, MeshRes, MeshLod, TilingPattern,
		bAddMeshSkirts,
		bMeshReverseWinding);
	for (auto& Tile : Tiles)
	{
		// Create Mesh Description (Vertex Positions)
		auto MeshDesc = UAGX_TerrainMeshUtilities::CreateMeshDescription(
			Tile.Center, Tile.Size, Tile.Resolution, Tile.UvScale,
			MeshVertexFunc, Tile.IsSkirt, Tile.IsReverseWinding);

		// Create MeshSection (Upload to GPU)
		CreateMeshSection(
			Tile.MeshIndex, MeshDesc->Vertices, MeshDesc->Triangles, MeshDesc->Normals,
			MeshDesc->UV0, MeshDesc->UV1, TArray<FVector2D>(), TArray<FVector2D>(),
			MeshDesc->Colors, MeshDesc->Tangents, bMeshCollision);
		SetMaterial(Tile.MeshIndex, MeshMaterial);
		SetMeshSectionVisible(Tile.MeshIndex, bMeshVisible);
	}

	return Tiles;
}

void UAGX_MovableTerrainComponent::UpdateTileMeshSection(MeshTile& Tile, const EAGX_MeshType& MeshType)
{
	// Create Mesh Description (Vertex Positions)
	auto MeshDesc = UAGX_TerrainMeshUtilities::CreateMeshDescription(
		Tile.Center, Tile.Size, Tile.Resolution, Tile.UvScale,
		GetMeshVertexFunction(MeshType), Tile.IsSkirt, Tile.IsReverseWinding);

	// Update MeshSection (Re-Upload to GPU)
	UpdateMeshSection(
		Tile.MeshIndex, MeshDesc->Vertices, MeshDesc->Normals, MeshDesc->UV0, MeshDesc->UV1,
		TArray<FVector2D>(), TArray<FVector2D>(), MeshDesc->Colors,
		MeshDesc->Tangents);

}

FAGX_MeshVertexFunction UAGX_MovableTerrainComponent::GetMeshVertexFunction(
	const EAGX_MeshType& MeshType)
{
	switch (MeshType)
	{

		// Terrain Mesh
		case EAGX_MeshType::Terrain:
			// Pos and UV0 are set as they enter the function
			return [&](FVector& Pos, FVector2D& Uv0, FVector2D& Uv1, FColor Color,
					   bool IsSkirt) -> void
			{

				//Hacky: Clamp skirt vertices on the edge of "Size" to BedHeight
				bool IsSkirtEdgeClamp =
					bShowMeshSides && IsSkirt && DistFromEdge(Pos) < SMALL_NUMBER;

				if (!IsSkirtEdgeClamp)
				{
					// Height Function
					double Height = HasNative() ? GetCurrentHeight(Pos) : CalcInitialHeight(Pos);
					Pos += FVector::UpVector * (Height + MeshZOffset);

					// Set UV1 Tiled to ElementSize
					Uv1 = FVector2D(
						(Pos.X + GetTerrainSize().X / 2) / ElementSize,
						(Pos.Y + GetTerrainSize().Y / 2) / ElementSize);
				}
				else
				{
					// Clamp XY position to edge
					Pos = FVector(
						FMath::Clamp(Pos.X, -Size.X / 2, Size.X / 2),
						FMath::Clamp(Pos.Y, -Size.Y / 2, Size.Y / 2), Pos.Z);

					// Clamp Z position to BedHeight
					double BedHeight = HasNative() ? GetBedHeight(Pos) : CalcInitialBedHeight(Pos);

					// Height Function
					Pos += FVector::UpVector * (BedHeight + MeshZOffset);

					FVector2D OldUv = Uv0;

					// Set Uv0, Uv1 with clamped position
					Uv0 = ToUv(Pos, Size);
					Uv1 = FVector2D(
						(Pos.X + GetTerrainSize().X / 2) / ElementSize,
						(Pos.Y + GetTerrainSize().Y / 2) / ElementSize);

					FVector2D UvDir = Uv0 - OldUv;

					bool IsOnXEdge = FMath::Abs(UvDir.Y) < FMath::Abs(UvDir.X);
					double Height = HasNative() ? GetCurrentHeight(Pos) : CalcInitialHeight(Pos);
					double HeightDist = FMath::Abs(Height - BedHeight);

					// Wrap Uv0 and Uv1 using HeightDist
					Uv0 = IsOnXEdge ? FVector2D(Uv0.X + HeightDist / Size.X, Uv0.Y)
												: FVector2D(Uv0.X, Uv0.Y + HeightDist / Size.Y);
					Uv1 = IsOnXEdge
							  ? FVector2D(Uv1.X + HeightDist / ElementSize, Uv1.Y)
							  : FVector2D(Uv1.X, Uv1.Y + HeightDist / ElementSize);
				}

			};
		//Terrain Collision
		case EAGX_MeshType::Collision:
			return [&](FVector& Pos, FVector2D& Uv0, FVector2D& Uv1, FColor Color,
					   bool IsSkirt) -> void
			{
				Pos = FVector(
					FMath::Clamp(Pos.X, -Size.X / 2, Size.X / 2),
					FMath::Clamp(Pos.Y, -Size.Y / 2, Size.Y / 2), Pos.Z);
				double Height = HasNative() ? GetCurrentHeight(Pos) : CalcInitialHeight(Pos);

				// Clamp vertices on the border for smoother (Unreal) collision
				if (DistFromEdge(Pos) < SMALL_NUMBER)// && IsSkirt)
					Height = HasNative() ? GetBedHeight(Pos) : CalcInitialBedHeight(Pos);
				//UV0 Tiled to Meters
				Uv0 = FVector2D(
					(Pos.X + Size.X / 2) / 100.0,
					(Pos.Y + Size.Y / 2) / 100.0);
				// Height Function
				Pos += FVector::UpVector * Height;
			};
		//BackBed
		case EAGX_MeshType::BackBed:
			return [&](FVector& Pos, FVector2D& Uv0, FVector2D& Uv1, FColor Color,
					   bool IsSkirt) -> void
			{
				Pos = FVector(
					FMath::Clamp(Pos.X, -Size.X / 2, Size.X / 2),
					FMath::Clamp(Pos.Y, -Size.Y / 2, Size.Y / 2), Pos.Z);

				// Height Function
				double BedHeight = HasNative() ? GetBedHeight(Pos) : CalcInitialBedHeight(Pos);

				Pos += FVector::UpVector * (BedHeight + MeshZOffset);

				// UV0 Tiled to Meters
				Uv0 = FVector2D((Pos.X + Size.X / 2) / 100.0, (Pos.Y + Size.Y / 2) / 100.0);

				// UV1 Tiled to ElementSize
				Uv1 = FVector2D(
					(Pos.X + GetTerrainSize().X / 2) / ElementSize,
					(Pos.Y + GetTerrainSize().Y / 2) / ElementSize);
			};
		// DebugPlane
		case EAGX_MeshType::DebugPlane:
			return [&](FVector& Pos, FVector2D& Uv0, FVector2D& Uv1, FColor Color,
					   bool IsSkirt) -> void
			{
				// UV0 Tiled to ElementSize
				Uv0 = FVector2D(
					(Pos.X + GetTerrainSize().X / 2) / ElementSize,
					(Pos.Y + GetTerrainSize().Y / 2) / ElementSize);
			};
		case EAGX_MeshType::None:
		default:
			return [&](FVector& Pos, FVector2D& Uv0, FVector2D& Uv1, FColor Color, bool IsSkirt) -> void{ };
	}
}

void UAGX_MovableTerrainComponent::PostInitProperties()
{
	Super::PostInitProperties();
	UE_LOG(LogAGX, Warning, TEXT("PostInitProperties"));
	// TODO: Only UpdateMesh on certain Property changes
	ForceRebuildMesh();
}

void UAGX_MovableTerrainComponent::PostLoad()
{
	Super::PostLoad();
	UE_LOG(LogAGX, Warning, TEXT("PostLoad"));
	// TODO: Only UpdateMesh on certain Property changes
	ForceRebuildMesh();
}

#if WITH_EDITOR
void UAGX_MovableTerrainComponent::PostEditChangeChainProperty(FPropertyChangedChainEvent& Event)
{
	Super::PostEditChangeChainProperty(Event);
	UE_LOG(LogAGX, Warning, TEXT("PostEditChangeChainProperty"));

	// TODO: Only UpdateMesh on certain Property changes
	ForceRebuildMesh();
}


#endif

void UAGX_MovableTerrainComponent::ForceRebuildMesh()
{
	// Hacky: This bool is used to force an update of the mesh in-editor
	bRebuildMesh = false;

	UWorld* World = GetWorld();

	if (!IsValid(World) || IsTemplate())
		return;
	if (GIsReconstructingBlueprintInstances)
		return;

	// In-Editor
	if (!World->IsGameWorld())
	{
		// Postpone to next tick because all properties are not copied
		World->GetTimerManager().SetTimerForNextTick(
			[this, World]
			{
				if (!IsValid(World))
					return;

				if (!IsValid(this))
					return;
				if (GetBedShapes().Num() > 0 && bUseBedShapes)
				{
					for (auto b : GetBedShapes())
					{
						if (!IsValid(b))
							return;
					}
				}

				// Recreate Mesh
				RecreateMeshes();
			});
	}
	// In-Game
	else if (World->IsGameWorld())
	{
		if (HasNative())
		{
			// Copy CurrentHeights from Native
			NativeBarrier.GetHeights(CurrentHeights, false);

			//TODO: Copy MinimalHeights from Native
			if (BedHeights.Num() != CurrentHeights.Num())
				return;

			// Recreate Mesh
			RecreateMeshes();
		}
	}
}


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
	--- AGX Native Implementation
	------------------------------
*/

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

void UAGX_MovableTerrainComponent::UpdateNativeProperties()
{
	NativeBarrier.SetCanCollide(bCanCollide);
	NativeBarrier.AddCollisionGroups(CollisionGroups);
	NativeBarrier.SetCreateParticles(bCreateParticles);
	NativeBarrier.SetDeleteParticlesOutsideBounds(bDeleteParticlesOutsideBounds);
	NativeBarrier.SetPenetrationForceVelocityScaling(PenetrationForceVelocityScaling);
	NativeBarrier.SetMaximumParticleActivationVolume(MaximumParticleActivationVolume);
	CreateNativeShovels();
	UpdateNativeTerrainMaterial();
	UpdateNativeShapeMaterial();
}

FTerrainBarrier* UAGX_MovableTerrainComponent::GetOrCreateNative()
{
	if (!HasNative())
		CreateNative();

	return &NativeBarrier;
}

bool UAGX_MovableTerrainComponent::HasNative() const
{
	return NativeBarrier.HasNative();
}

uint64 UAGX_MovableTerrainComponent::GetNativeAddress() const
{
	if (!HasNative())
	{
		return 0;
	}
	return static_cast<uint64>(GetNative()->GetNativeAddress());
}

void UAGX_MovableTerrainComponent::SetNativeAddress(uint64 NativeAddress)
{
	if (NativeAddress == GetNativeAddress())
	{
		return;
	}

	if (HasNative())
	{
		NativeBarrier.ReleaseNative();
	}

	if (NativeAddress == 0)
	{
		return;
	}

	NativeBarrier.SetNativeAddress(NativeAddress);
}

void UAGX_MovableTerrainComponent::OnUpdateTransform(
	EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport)
{
	Super::OnUpdateTransform(UpdateTransformFlags, Teleport);
	if (UpdateTransformFlags == EUpdateTransformFlags::PropagateFromParent)
	{
		return;
	}

	if (!HasNative())
	{
		return;
	}

	GetNative()->SetPosition(GetComponentLocation());
	GetNative()->SetRotation(GetComponentQuat());
}

void UAGX_MovableTerrainComponent::OnAttachmentChanged()
{
	if (!HasNative())
	{
		return;
	}
	GetNative()->SetPosition(GetComponentLocation());
	GetNative()->SetRotation(GetComponentQuat());
}

TStructOnScope<FActorComponentInstanceData> UAGX_MovableTerrainComponent::GetComponentInstanceData()
	const
{
	return MakeStructOnScope<FActorComponentInstanceData, FAGX_NativeOwnerInstanceData>(
		this, this,
		[](UActorComponent* Component)
		{
			ThisClass* AsThisClass = Cast<ThisClass>(Component);
			return static_cast<IAGX_NativeOwner*>(AsThisClass);
		});
}

void UAGX_MovableTerrainComponent::OnRegister()
{
	Super::OnRegister();
	//ForceRebuildMesh();
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
		UE_LOG(
			LogAGX, Warning,
			TEXT("AGX MovableTerrain '%s' in '%s' failed to UpdateNativeShapeMaterial. "
				 "Restore original ShapeMaterial. "),
			*GetName(), *GetLabelSafe(GetOwner()));

		// Something went wrong, restore original ShapeMaterial.
		ShapeMaterial = ShapeMaterialOrig;
		return false;
	}

	return true;
}

bool UAGX_MovableTerrainComponent::UpdateNativeTerrainMaterial()
{
	if (!HasNative())
	{
		// Not in play, we are done.
		return false;
	}

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
			TEXT("AGX MovableTerrain '%s' in '%s' failed to UpdateNativeTerrainMaterial. "
					"There is no valid World. "), 
			*GetName(), *GetLabelSafe(GetOwner()));
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

	for (FAGX_ShovelReference& ShovelRef : ShovelComponents)
	{
		UAGX_ShovelComponent* ShovelComponent = ShovelRef.GetShovelComponent();
		if (ShovelComponent == nullptr)
		{
			const FString Message = FString::Printf(
				TEXT("AGX MovableTerrain '%s' in '%s' have a Shovel reference to '%s' in '%s' "
					 "that does not reference a valid Shovel. "
					 "Abandoning shovel. "),
				*GetName(), *GetLabelSafe(GetOwner()),
				*ShovelRef.Name.ToString(),*GetLabelSafe(ShovelRef.OwningActor));
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
			TEXT("AGX MovableTerrain '%s' in '%s' failed to AddNativeShovel '%s' in '%s'. "
				 "Shovel does not reference a valid Native. "),
			*GetName(), *GetLabelSafe(GetOwner()), 
			*ShovelComponent->GetName(), *GetLabelSafe(ShovelComponent->GetOwner()));
		return false;
	}
	check(ShovelBarrier->HasNative());

	bool Added = NativeBarrier.AddShovel(*ShovelBarrier);
	if (!Added)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("AGX MovableTerrain '%s' in '%s' rejected AddNativeShovel '%s' in '%s'.  "
				 "Reversing edge directions and trying again."),
			*GetName(), *GetLabelSafe(GetOwner()), 
			*ShovelComponent->GetName(), *GetLabelSafe(ShovelComponent->GetOwner()));

		ShovelComponent->SwapEdgeDirections();
		Added = NativeBarrier.AddShovel(*ShovelBarrier);
		if (!Added)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("AGX MovableTerrain '%s' in '%s' failed to AddNativeShovel '%s' in '%s' "
					 "after edge directions flip. "
					 "Abandoning shovel. "),
				*GetName(), *GetLabelSafe(GetOwner()), 
				*GetNameSafe(ShovelComponent), *GetLabelSafe(ShovelComponent->GetOwner()));
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
	if (!bEnableParticleRendering || !ParticleSystemAsset)
	{
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
	
	if (ParticleSystemComponent == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("AGX MovableTerrain '%s' in '%s' failed to create ParticleSystemComponent. "
				 "Ensure the selected Niagara System is valid."),
			*GetName(), *GetLabelSafe(GetOwner()));
	}

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
