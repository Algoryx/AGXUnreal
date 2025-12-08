// Copyright 2025, Algoryx Simulation AB.

#include "Terrain/AGX_MovableTerrainComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_InternalDelegateAccessor.h"
#include "AGX_LogCategory.h"
#include "AGX_NativeOwnerInstanceData.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_Simulation.h"
#include "Materials/AGX_ShapeMaterial.h"
#include "Materials/AGX_TerrainMaterial.h"
#include "Shapes/AGX_ShapeComponent.h"
#include "Shapes/HeightFieldShapeBarrier.h"
#include "Terrain/AGX_TerrainProperties.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_ObjectUtilities.h"
#include "Utilities/AGX_StringUtilities.h"

// Unreal Engine includes.
#include "NiagaraComponent.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "TimerManager.h"

UAGX_MovableTerrainComponent::UAGX_MovableTerrainComponent(
	const FObjectInitializer& ObjectInitializer)
	: UProceduralMeshComponent(ObjectInitializer)
{
	bAllowConcurrentTick = false;
	PrimaryComponentTick.bCanEverTick = true;
	SetCanEverAffectNavigation(false);

	static const TCHAR* DefaultRenderMaterial = TEXT(
		"NiagaraSystem'/AGXUnreal/Terrain/Rendering/HeightField/"
		"MI_MovableTerrain.MI_MovableTerrain'");
	Material = FAGX_ObjectUtilities::GetAssetFromPath<UMaterialInterface>(DefaultRenderMaterial);
}

void UAGX_MovableTerrainComponent::CreateNative()
{
	// Make sure OwningRigidBody is created, if there is one
	UAGX_RigidBodyComponent* OwningRigidBody =
		FAGX_ObjectUtilities::FindFirstAncestorOfType<UAGX_RigidBodyComponent>(*this);
	if (OwningRigidBody)
		OwningRigidBody->GetOrCreateNative();

	FIntVector2 TerrainResolution = GetTerrainResolution();

	InitializeHeights();

	NativeBarrier.AllocateNative(
		TerrainResolution.X, TerrainResolution.Y, ElementSize, CurrentHeights, BedHeights);

	if (!HasNative())
	{
		const FString Message = FString::Printf(
			TEXT("AGX MovableTerrain '%s' in '%s' failed AllocateNative. Output Log may contain "
				 "more details."),
			*GetName(), *GetLabelSafe(GetOwner()));
		FAGX_NotificationUtilities::ShowNotification(Message, SNotificationItem::CS_Fail);

		return;
	}

	// Attach to RigidBody.
	if (OwningRigidBody)
	{
		FHeightFieldShapeBarrier Hf = NativeBarrier.GetHeightField();
		OwningRigidBody->GetNative()->AddShape(&Hf);
	}

	WriteTransformToNative();
	UpdateNativeProperties();
	RecreateMeshes();

	// Create PostHandle callback to update mesh.
	ConnectMeshToNative();

	if (UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this))	
		Simulation->Add(*this);
}

void UAGX_MovableTerrainComponent::ConnectMeshToNative()
{
	// Update Mesh each StepForward.
	if (UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this))
	{
		PostStepForwardHandle =
			FAGX_InternalDelegateAccessor::GetOnPostStepForwardInternal(*Simulation)
				.AddLambda(
					[this](double)
					{
						if (!HasNative())
							return;

						if (!bHeightsInitialized && FetchNativeHeights())
							RecreateMeshes();

						// Get ModifiedHeights
						auto ModifiedHeights = NativeBarrier.GetModifiedVertices();
						FVector2D NativeTerrainSize = GetTerrainSize();

						auto UpdateHeightMesh = [&](const HeightMesh& Mesh) -> void
						{
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

							for (auto& Tile : Mesh.Tiles)
							{
								if (IsTileDirty(Tile))
								{
									// Create Mesh Description (Vertex Positions)
									auto MeshDesc =
										UAGX_TerrainMeshUtilities::CreateHeightMeshTileDescription(
											Tile.Center, Tile.Size, Tile.Resolution, Mesh.Center,
											Mesh.Size, Mesh.Uv0, Mesh.Uv1, Mesh.HeightFunc,
											Mesh.EdgeHeightFunc, Mesh.bCreateEdges, Mesh.bFixSeams,
											Mesh.bReverseWinding,
											/*bCalcFastTerrainBedNormals*/ true);

									// Update MeshSection (Re-Upload to GPU)
									UpdateMeshSection(
										Tile.MeshIndex, MeshDesc->Vertices, MeshDesc->Normals,
										MeshDesc->UV0, MeshDesc->UV1, TArray<FVector2D>(),
										TArray<FVector2D>(), MeshDesc->Colors, MeshDesc->Tangents);
								}
							}
						};

						if (ModifiedHeights.Num() > 0)
						{
							// Update CurrentHeights
							NativeBarrier.GetHeights(CurrentHeights, true);

							// Update Terrain
							UpdateHeightMesh(TerrainMesh);

							// Update Collision
							bool bIsUnrealCollision =
								AdditionalUnrealCollision != ECollisionEnabled::NoCollision;
							if (bShowUnrealCollision || bIsUnrealCollision)
								UpdateHeightMesh(CollisionMesh);
						}
					});
	}
}

bool UAGX_MovableTerrainComponent::FetchNativeHeights()
{
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("AGX MovableTerrain '%s' in '%s' failed to FetchNativeHeights. "
				 "Reason: !HasNative()"),
			*GetName(), *GetLabelSafe(GetOwner()));
		return false;
	}
	TArray<float> FetchedHeights;
	NativeBarrier.GetHeights(FetchedHeights, false);
	const auto TerrainResolution = GetTerrainResolution();
	if (FetchedHeights.Num() != TerrainResolution.X * TerrainResolution.Y)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("AGX MovableTerrain '%s' in '%s' failed to GetHeights from Native. "
				 "Expected: %d, Got: %d"),
			*GetName(), *GetLabelSafe(GetOwner()), TerrainResolution.X * TerrainResolution.Y,
			FetchedHeights.Num());
		return false;
	}

	TArray<float> FetchedMinHeights;
	NativeBarrier.GetMinimumHeights(FetchedMinHeights);
	if (FetchedMinHeights.Num() != TerrainResolution.X * TerrainResolution.Y)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("AGX MovableTerrain '%s' in '%s' failed to GetMinimumHeights from Native. "
				 "Expected: %d, Got: %d"),
			*GetName(), *GetLabelSafe(GetOwner()), TerrainResolution.X * TerrainResolution.Y,
			FetchedMinHeights.Num());
		return false;
	}

	CurrentHeights = FetchedHeights;
	BedHeights = FetchedMinHeights;

	bHeightsInitialized = true;

	return true;
}

float UAGX_MovableTerrainComponent::GetCurrentHeight(const FVector& LocalPos) const
{
	return UAGX_TerrainMeshUtilities::SampleHeightArray(
		ToUv(LocalPos, Size), CurrentHeights, GetTerrainResolution().X, GetTerrainResolution().Y);
}

float UAGX_MovableTerrainComponent::GetBedHeight(const FVector& LocalPos) const
{
	return UAGX_TerrainMeshUtilities::SampleHeightArray(
		ToUv(LocalPos, Size), BedHeights, GetTerrainResolution().X, GetTerrainResolution().Y);
}

void UAGX_MovableTerrainComponent::InitializeHeights()
{
	FIntVector2 Res = GetTerrainResolution();
	CurrentHeights.Reset();
	CurrentHeights.SetNumZeroed(Res.X * Res.Y);

	BedHeights.Reset();
	BedHeights.SetNumZeroed(Res.X * Res.Y);

	FVector Corner = FVector(ElementSize * (1 - Res.X) / 2.0, ElementSize * (1 - Res.Y) / 2.0, 0.0);

	for (int y = 0; y < Res.Y; y++)
	{
		for (int x = 0; x < Res.X; x++)
		{
			FVector LocalPos = Corner + FVector(x * ElementSize, y * ElementSize, 0);

			BedHeights[y * Res.X + x] = CalcInitialBedHeight(LocalPos);
			CurrentHeights[y * Res.X + x] = CalcInitialHeight(LocalPos);
		}
	}

	bHeightsInitialized = true;
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
	bool bIsUnrealCollision = AdditionalUnrealCollision != ECollisionEnabled::NoCollision;
	const auto TerrainResolution = GetTerrainResolution();
	FIntVector2 AutoMeshResolution(TerrainResolution.X - 1, TerrainResolution.Y - 1);
	FAGX_UvParams MeshUv {Size / 2.0, {1.0 / Size.X, 1.0 / Size.Y}};
	FAGX_UvParams TerrainUv {GetTerrainSize() / 2.0, {1.0 / ElementSize, 1.0 / ElementSize}};

	FAGX_MeshVertexFunction TerrainHeightFunc = [&](const FVector& LocalPos) -> double
	{ return bHeightsInitialized ? GetCurrentHeight(LocalPos) : CalcInitialHeight(LocalPos); };
	FAGX_MeshVertexFunction BedHeightFunc = [&](const FVector& LocalPos) -> double
	{ return bHeightsInitialized ? GetBedHeight(LocalPos) : CalcInitialBedHeight(LocalPos); };
	FAGX_MeshVertexFunction FlatHeightFunc = [&](const FVector& LocalPos) -> double
	{ return 0.0f; };

	// Reset MeshSections
	ClearAllMeshSections();

	int MeshIndex = 0;

	// Terrain Mesh (Rendered Mesh)
	TerrainMesh = CreateHeightMesh(
		MeshIndex, FVector(0, 0, MeshZOffset), Size,
		bAutoMeshResolution ? AutoMeshResolution : MeshResolution, MeshUv, TerrainUv,
		TerrainHeightFunc, BedHeightFunc, Material, MeshLevelOfDetail, MeshTilingPattern,
		MeshTileResolution, bCloseMesh, bFixMeshSeams, false, false, true,
		/*bCalcFastTerrainBedNormals*/ true);
	MeshIndex += TerrainMesh.Tiles.Num();

	// Collision Mesh (Low resolution Terrain)
	if (bIsUnrealCollision || bShowUnrealCollision)
	{
		CollisionMesh = CreateHeightMesh(
			MeshIndex, FVector::Zero(), Size, AutoMeshResolution, MeshUv, TerrainUv,
			TerrainHeightFunc, BedHeightFunc, nullptr, UnrealCollisionLOD,
			EAGX_MeshTilingPattern::StretchedTiles, 6, true, false, false, bIsUnrealCollision,
			bShowUnrealCollision, /*bCalcFastTerrainBedNormals*/ true);
		MeshIndex += CollisionMesh.Tiles.Num();
	}

	// BedMesh (Backside. Just a plane at the bottom if there is no BedShapes)
	if (bIsUnrealCollision || bCloseMesh || bShowUnrealCollision)
	{
		bool HasShapes = (bUseBedShapes && GetBedShapes().Num() > 0);
		HeightMesh BedMesh = CreateHeightMesh(
			MeshIndex, FVector::Zero(), Size, HasShapes ? AutoMeshResolution : FIntVector2(1, 1),
			MeshUv, TerrainUv, BedHeightFunc, BedHeightFunc, nullptr, UnrealCollisionLOD,
			HasShapes ? EAGX_MeshTilingPattern::StretchedTiles : EAGX_MeshTilingPattern::None, 10,
			false, false, true, bIsUnrealCollision, bCloseMesh || bShowUnrealCollision,
			/*bCalcFastTerrainBedNormals*/ false);
		MeshIndex += BedMesh.Tiles.Num();
	}

	// DebugPlane
	if (bShowDebugPlane)
	{
		HeightMesh DebugPlaneFront = CreateHeightMesh(
			MeshIndex, FVector(0, 0, MeshZOffset - 0.1f), GetTerrainSize(), FIntVector2(1, 1),
			TerrainUv, MeshUv, FlatHeightFunc, FlatHeightFunc, nullptr, 0,
			EAGX_MeshTilingPattern::None, 10, false, false, false, false, true,
			/*bCalcFastTerrainBedNormals*/ false);
		MeshIndex += DebugPlaneFront.Tiles.Num();
		HeightMesh DebugPlaneBack = CreateHeightMesh(
			MeshIndex, FVector(0, 0, MeshZOffset - 0.1f), GetTerrainSize(), FIntVector2(1, 1),
			TerrainUv, MeshUv, FlatHeightFunc, FlatHeightFunc, nullptr, 0,
			EAGX_MeshTilingPattern::None, 10, false, false, true, false, true,
			/*bCalcFastTerrainBedNormals*/ false);
		MeshIndex += DebugPlaneBack.Tiles.Num();
	}
}

HeightMesh UAGX_MovableTerrainComponent::CreateHeightMesh(
	int StartMeshIndex, const FVector& MeshCenter, const FVector2D& MeshSize,
	const FIntVector2& MeshRes, const FAGX_UvParams& Uv0Params, const FAGX_UvParams& Uv1Params,
	const FAGX_MeshVertexFunction MeshHeightFunc, const FAGX_MeshVertexFunction EdgeHeightFunc,
	UMaterialInterface* MeshMaterial, int MeshLod, EAGX_MeshTilingPattern TilingPattern,
	int TileResolution, bool bCreateEdges, bool bFixSeams, bool bReverseWinding,
	bool bMeshCollision, bool bMeshVisible, bool bCalcFastTerrainBedNormals)
{
	HeightMesh Mesh = UAGX_TerrainMeshUtilities::CreateHeightMesh(
		StartMeshIndex, MeshCenter, MeshSize, MeshRes, Uv0Params, Uv1Params, MeshLod, TilingPattern,
		TileResolution, MeshHeightFunc, EdgeHeightFunc, bCreateEdges, bFixSeams, bReverseWinding);

	for (auto& Tile : Mesh.Tiles)
	{
		// Create Mesh Description (Vertex Positions).
		auto MeshDesc = UAGX_TerrainMeshUtilities::CreateHeightMeshTileDescription(
			Tile.Center, Tile.Size, Tile.Resolution, Mesh.Center, Mesh.Size, Mesh.Uv0, Mesh.Uv1,
			Mesh.HeightFunc, Mesh.EdgeHeightFunc, Mesh.bCreateEdges, Mesh.bFixSeams,
			Mesh.bReverseWinding, bCalcFastTerrainBedNormals);

		// Create MeshSection (Upload to GPU).
		CreateMeshSection(
			Tile.MeshIndex, MeshDesc->Vertices, MeshDesc->Triangles, MeshDesc->Normals,
			MeshDesc->UV0, MeshDesc->UV1, TArray<FVector2D>(), TArray<FVector2D>(),
			MeshDesc->Colors, MeshDesc->Tangents, bMeshCollision);
		SetMaterial(Tile.MeshIndex, MeshMaterial);
		SetMeshSectionVisible(Tile.MeshIndex, bMeshVisible);
	}

	return Mesh;
}

void UAGX_MovableTerrainComponent::OnComponentCreated()
{
	Super::OnComponentCreated();

	if (IsValid(GetWorld()) && !GetWorld()->IsGameWorld())
		RecreateMeshesEditor();
}

void UAGX_MovableTerrainComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GIsReconstructingBlueprintInstances || HasNative())
	{
		// A reconstructed component with inherited properties, (See: EndPlay())
		return;
	}

	GetOrCreateNative();

	// Fetch native heights (as a fail safe).
	if (FetchNativeHeights())
		RecreateMeshes();

	InitializeParticles();

	this->SetCollisionEnabled(AdditionalUnrealCollision);
}

void UAGX_MovableTerrainComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);

	if (HasNative())
	{
		if (UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this))
		{
			FAGX_InternalDelegateAccessor::GetOnPostStepForwardInternal(*Simulation)
				.Remove(PostStepForwardHandle);
		}

		if (!GIsReconstructingBlueprintInstances && Reason != EEndPlayReason::EndPlayInEditor &&
			Reason != EEndPlayReason::Quit && Reason != EEndPlayReason::LevelTransition)
		{
			if (UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this))
				Simulation->Remove(*this);
		}

		NativeBarrier.ReleaseNative();
	}
}

void UAGX_MovableTerrainComponent::PostInitProperties()
{
	Super::PostInitProperties();

#if WITH_EDITOR
	InitPropertyDispatcher();
#endif

	UWorld* World = GetWorld();

	if (IsValid(World) && !World->IsGameWorld() && !IsTemplate() &&
		!GIsReconstructingBlueprintInstances)
	{
		RecreateMeshesEditor();
	}
}

#if WITH_EDITOR
void UAGX_MovableTerrainComponent::PostEditChangeChainProperty(FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);
	Super::PostEditChangeChainProperty(Event);

	UWorld* World = GetWorld();
	if (!IsValid(this))
	{
		return;
	}

	// In-Game
	if (IsValid(World) && World->IsGameWorld())
	{
		if (HasNative() && FetchNativeHeights())
			RecreateMeshes();
	}

	// In Editor
	if (IsValid(World) && !World->IsGameWorld() && !IsTemplate())
	{
		RecreateMeshesEditor();
	}
}

void UAGX_MovableTerrainComponent::InitPropertyDispatcher()
{
	FAGX_PropertyChangedDispatcher<ThisClass>& PropertyDispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (PropertyDispatcher.IsInitialized())
	{
		return;
	}

	// Location and Rotation are not Properties, so they won't trigger PostEditChangeProperty. e.g.,
	// when moving the Component using the Widget in the Level Viewport. They are instead handled in
	// PostEditComponentMove. The local transformations, however, the ones at the top of the Details
	// Panel, are properties and do end up here.
	PropertyDispatcher.Add(
		this->GetRelativeLocationPropertyName(),
		[](ThisClass* This) { This->WriteTransformToNative(); });

	PropertyDispatcher.Add(
		this->GetRelativeRotationPropertyName(),
		[](ThisClass* This) { This->WriteTransformToNative(); });

	PropertyDispatcher.Add(
		this->GetAbsoluteLocationPropertyName(),
		[](ThisClass* This) { This->WriteTransformToNative(); });

	PropertyDispatcher.Add(
		this->GetAbsoluteRotationPropertyName(),
		[](ThisClass* This) { This->WriteTransformToNative(); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, bCanCollide),
		[](ThisClass* This) { This->SetCanCollide(This->bCanCollide); });

	PropertyDispatcher.Add(
		AGX_MEMBER_NAME(ParticleSystemAsset),
		[](ThisClass* This)
		{
			if (This->ParticleSystemAsset != nullptr)
			{
				This->ParticleSystemAsset->RequestCompile(true);
			}
		});

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_MovableTerrainComponent, bShowDebugPlane),
		[](ThisClass* This) { This->SetShowDebugPlane(This->bShowDebugPlane); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, TerrainProperties),
		[](ThisClass* This) { This->SetTerrainProperties(This->TerrainProperties); });

	// Size
	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_MovableTerrainComponent, Size),
		[](ThisClass* This) { This->SetSize(This->Size); });
	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_MovableTerrainComponent, ElementSize),
		[](ThisClass* This) { This->SetElementSize(This->ElementSize); });

	// Unreal collision
	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_MovableTerrainComponent, bShowUnrealCollision),
		[](ThisClass* This) { This->SetShowUnrealCollision(This->bShowUnrealCollision); });
	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_MovableTerrainComponent, UnrealCollisionLOD),
		[](ThisClass* This) { This->SetUnrealCollisionLOD(This->UnrealCollisionLOD); });
	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_MovableTerrainComponent, AdditionalUnrealCollision),
		[](ThisClass* This) { This->SetUnrealCollisionType(This->AdditionalUnrealCollision); });

	// Mesh
	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_MovableTerrainComponent, Material),
		[](ThisClass* This) { This->SetMeshMaterial(This->Material); });
	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_MovableTerrainComponent, bAutoMeshResolution),
		[](ThisClass* This) { This->SetAutoMeshResolution(This->bAutoMeshResolution); });
	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_MovableTerrainComponent, MeshLevelOfDetail),
		[](ThisClass* This) { This->SetMeshLOD(This->MeshLevelOfDetail); });
	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_MovableTerrainComponent, MeshResolution),
		[](ThisClass* This) { This->SetMeshResolution(This->MeshResolution); });
	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_MovableTerrainComponent, MeshZOffset),
		[](ThisClass* This) { This->SetMeshZOffset(This->MeshZOffset); });
	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_MovableTerrainComponent, bCloseMesh),
		[](ThisClass* This) { This->SetCloseMesh(This->bCloseMesh); });
	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_MovableTerrainComponent, MeshTileResolution),
		[](ThisClass* This) { This->SetMeshTileResolution(This->MeshTileResolution); });
	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_MovableTerrainComponent, MeshTilingPattern),
		[](ThisClass* This) { This->SetMeshTilingPattern(This->MeshTilingPattern); });
	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_MovableTerrainComponent, bFixMeshSeams),
		[](ThisClass* This) { This->SetFixMeshSeams(This->bFixMeshSeams); });
}

bool UAGX_MovableTerrainComponent::CanEditChange(const FProperty* InProperty) const
{
	const bool SuperCanEditChange = Super::CanEditChange(InProperty);
	if (!SuperCanEditChange)
		return false;

	if (InProperty == nullptr)
		return SuperCanEditChange;

	const bool bIsPlaying = GetWorld() && GetWorld()->IsGameWorld();
	if (bIsPlaying)
	{
		// List of names of properties that does not support editing after initialization.
		static const TArray<FName> PropertiesNotEditableDuringPlay = {
			GET_MEMBER_NAME_CHECKED(ThisClass, Size),
			GET_MEMBER_NAME_CHECKED(ThisClass, ElementSize),
			GET_MEMBER_NAME_CHECKED(ThisClass, bUseBedShapes),
			GET_MEMBER_NAME_CHECKED(ThisClass, BedShapes),
			GET_MEMBER_NAME_CHECKED(ThisClass, BedZOffset),
			GET_MEMBER_NAME_CHECKED(ThisClass, InitialHeight),
			GET_MEMBER_NAME_CHECKED(ThisClass, bUseInitialNoise),
			GET_MEMBER_NAME_CHECKED(ThisClass, InitialNoise),
			GET_MEMBER_NAME_CHECKED(ThisClass, ParticleSystemAsset)};

		if (PropertiesNotEditableDuringPlay.Contains(InProperty->GetFName()))
		{
			return false;
		}
	}
	return SuperCanEditChange;
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

void UAGX_MovableTerrainComponent::RecreateMeshesEditor()
{
	UWorld* World = GetWorld();
	AActor* Owner = GetOwner();

	if (!IsValid(World) || !IsValid(this) || !IsValid(Owner) || IsBeingDestroyed() ||
		Owner->HasAnyFlags(RF_BeginDestroyed))
	{
		return;
	}

	// In-Editor
	if (!World->IsGameWorld())
	{
		// Postpone mesh creation for next tick, because bedshape geometries needs
		// to be properly created for BedHeights raycast
		FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda(
			[this, World, Owner](float DeltaTime)
			{
				if (!IsValid(this) || IsBeingDestroyed() || !IsValid(World) || !IsValid(Owner) ||
					Owner->HasAnyFlags(RF_BeginDestroyed) ||
					!Owner->HasActorRegisteredAllComponents())
				{
					return false;
				}

				RecreateMeshes();
				return false; // This signals to only run once.
			}));
	}
	// In-Game
	else if (World->IsGameWorld())
	{
		RecreateMeshes();
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

TArray<FName> UAGX_MovableTerrainComponent::GetBedShapesOptions() const
{
	const FName MyName = [this]()
	{
		FName Name = GetFName();
		if (HasAnyFlags(RF_ArchetypeObject))
		{
			FString AsString = Name.ToString();
			AsString.RemoveFromEnd(ComponentTemplateNameSuffix);
			Name = FName(AsString);
		}
		return Name;
	}();

	TArray<FName> Options;
	for (FName Name :
		 FAGX_ObjectUtilities::GetChildComponentNamesOfType<UMeshComponent>(GetOuter()))
	{
		if (Name == MyName)
			continue;
		Options.AddUnique(Name);
	}
	return Options;
}

void UAGX_MovableTerrainComponent::SetSize(FVector2D NewSize)
{
	Size = NewSize;
}

void UAGX_MovableTerrainComponent::SetElementSize(double NewSize)
{
	ElementSize = NewSize;
}

void UAGX_MovableTerrainComponent::SetShowDebugPlane(bool bShow)
{
	bShowDebugPlane = bShow;
}

void UAGX_MovableTerrainComponent::SetShowUnrealCollision(bool bShow)
{
	bShowUnrealCollision = bShow;
}

void UAGX_MovableTerrainComponent::SetUnrealCollisionLOD(int Lod)
{
	UnrealCollisionLOD = Lod;
}

void UAGX_MovableTerrainComponent::SetUnrealCollisionType(
	TEnumAsByte<enum ECollisionEnabled::Type> CollisionType)
{
	AdditionalUnrealCollision = CollisionType;
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
	UpdateNativeTerrainProperties();
	UpdateNativeTerrainMaterial();
	UpdateNativeShapeMaterial();
}

bool UAGX_MovableTerrainComponent::UpdateNativeTerrainProperties()
{
	if (!HasNative())
		return false;

	if (TerrainProperties == nullptr)
		return false; // Nullptr TerrainProperties not allowed.

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Cannot update native Terrain Properties because don't have a world to create "
				 "the Properties instance in."));
		return false;
	}

	UAGX_TerrainProperties* Instance = TerrainProperties->GetOrCreateInstance(World);
	check(Instance);

	if (TerrainProperties != Instance)
		TerrainProperties = Instance;

	FTerrainPropertiesBarrier* TerrainPropertiesBarrier = Instance->GetOrCreateNative();
	check(TerrainPropertiesBarrier);

	GetNative()->SetTerrainProperties(*TerrainPropertiesBarrier);

	return true;
}

FTerrainBarrier* UAGX_MovableTerrainComponent::GetOrCreateNative()
{
	if (!HasNative())
	{
		if (GIsReconstructingBlueprintInstances)
		{
			checkNoEntry();
			UE_LOG(
				LogAGX, Error,
				TEXT("A request for the AGX Dynamics instance for Movable Terrain '%s' in '%s' was "
					 "made "
					 "but we are in the middle of a Blueprint Reconstruction and the requested "
					 "instance has not yet been restored. The instance cannot be returned, which "
					 "may lead to incorrect scene configuration."),
				*GetName(), *GetLabelSafe(GetOwner()));
			return nullptr;
		}
		CreateNative();
	}

	return &NativeBarrier;
}

bool UAGX_MovableTerrainComponent::HasNative() const
{
	return NativeBarrier.HasNative();
}

uint64 UAGX_MovableTerrainComponent::GetNativeAddress() const
{
	return static_cast<uint64>(NativeBarrier.GetNativeAddress());
}

void UAGX_MovableTerrainComponent::SetNativeAddress(uint64 NativeAddress)
{
	NativeBarrier.SetNativeAddress(static_cast<uintptr_t>(NativeAddress));

	if (HasNative())
		ConnectMeshToNative();
}

bool UAGX_MovableTerrainComponent::WriteTransformToNative()
{
	AGX_CHECK(HasNative());
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("AGX MovableTerrain '%s' in '%s' failed to WriteTransformToNative. "
				 "Reason: !HasNative()"),
			*GetName(), *GetLabelSafe(GetOwner()));
		return false;
	}

	NativeBarrier.SetPosition(GetComponentLocation());
	NativeBarrier.SetRotation(GetComponentQuat());
	return true;
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

bool UAGX_MovableTerrainComponent::SetTerrainProperties(UAGX_TerrainProperties* InTerrainProperties)
{
	UAGX_TerrainProperties* TerrainPropertiesOrig = TerrainProperties;
	TerrainProperties = InTerrainProperties;

	if (!HasNative())
	{
		// Not in play, we are done.
		return true;
	}

	// UpdateNativeTerrainProperties is responsible to create an instance if none exists and do the
	// asset/instance swap.
	if (!UpdateNativeTerrainProperties())
	{
		// Something went wrong, restore original TerrainProperties.
		TerrainProperties = TerrainPropertiesOrig;
		return false;
	}

	return true;
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

bool UAGX_MovableTerrainComponent::GetNoMerge() const
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

void UAGX_MovableTerrainComponent::SetMeshMaterial(UMaterialInterface* NewMaterial)
{
	Material = NewMaterial;
}

void UAGX_MovableTerrainComponent::SetMeshLOD(int Lod)
{
	MeshLevelOfDetail = Lod;
}

void UAGX_MovableTerrainComponent::SetAutoMeshResolution(bool bAuto)
{
	bAutoMeshResolution = bAuto;
}

void UAGX_MovableTerrainComponent::SetMeshResolution(FIntVector2 NewResolution)
{
	MeshResolution = NewResolution;
}

void UAGX_MovableTerrainComponent::SetCloseMesh(bool bClose)
{
	bCloseMesh = bClose;
}

void UAGX_MovableTerrainComponent::SetMeshTileResolution(int TileRes)
{
	MeshTileResolution = TileRes;
}

void UAGX_MovableTerrainComponent::SetMeshTilingPattern(EAGX_MeshTilingPattern Pattern)
{
	MeshTilingPattern = Pattern;
}

void UAGX_MovableTerrainComponent::SetFixMeshSeams(bool bFix)
{
	bFixMeshSeams = bFix;
}

void UAGX_MovableTerrainComponent::SetMeshZOffset(double zOffset)
{
	MeshZOffset = zOffset;
}
