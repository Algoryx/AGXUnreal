// Copyright 2026, Algoryx Simulation AB.

#include "Distributed/AGX_DistributedSpawnPoint.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Distributed/AGX_DistributedAsset.h"
#include "Distributed/AGX_DistributedPreview.h"
#include "Distributed/AGX_DistributedSpawnPointVisualizerComponent.h"
#include "Distributed/AGX_DistributedWorldSubsystem.h"

// Unreal Engine includes.
#include "Components/ArrowComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"

AAGX_DistributedSpawnPoint::AAGX_DistributedSpawnPoint()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	auto* Arrow = CreateDefaultSubobject<UArrowComponent>(TEXT("SpawnDirection"));
	Arrow->SetupAttachment(RootComponent);
	Arrow->SetHiddenInGame(true);
#if WITH_EDITORONLY_DATA
	ProximityVisualizer = CreateDefaultSubobject<UAGX_DistributedSpawnPointVisualizerComponent>(
		TEXT("ProximityVisualizer"));
#endif
}
void AAGX_DistributedSpawnPoint::PostLoad()
{
	Super::PostLoad();
	if (!HasAnyFlags(RF_ClassDefaultObject) && !PlacementId.IsValid())
		PlacementId = FGuid::NewGuid();
}
void AAGX_DistributedSpawnPoint::PostActorCreated()
{
	Super::PostActorCreated();
	if (!PlacementId.IsValid())
		PlacementId = FGuid::NewGuid();
}
void AAGX_DistributedSpawnPoint::PostDuplicate(EDuplicateMode::Type Mode)
{
	Super::PostDuplicate(Mode);
	if (Mode != EDuplicateMode::PIE)
		PlacementId = FGuid::NewGuid();
}
void AAGX_DistributedSpawnPoint::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	RefreshPreview();
}
FVector AAGX_DistributedSpawnPoint::GetProximitySensorCenter() const
{
	return GetActorTransform().TransformPosition(AGX_GetDistributedProximityCenter(Asset));
}

void AAGX_DistributedSpawnPoint::RefreshPreview()
{
	for (UStaticMeshComponent* Mesh : PreviewComponents)
		if (Mesh != nullptr)
			Mesh->DestroyComponent();
	PreviewComponents.Reset();
	for (const auto& Preview : AGX_GetDistributedPreview(Asset))
	{
		auto* Mesh = NewObject<UStaticMeshComponent>(this, NAME_None, RF_Transient);
		Mesh->SetStaticMesh(Preview.Mesh);
		for (int32 I = 0; I < Preview.Materials.Num(); ++I)
			Mesh->SetMaterial(I, Preview.Materials[I]);
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Mesh->SetGenerateOverlapEvents(false);
		Mesh->SetMobility(EComponentMobility::Movable);
		Mesh->SetupAttachment(RootComponent);
		Mesh->SetRelativeTransform(Preview.Transform);
		Mesh->RegisterComponent();
		PreviewComponents.Add(Mesh);
	}
}
void AAGX_DistributedSpawnPoint::BeginPlay()
{
	Super::BeginPlay();
	if (!bSpawnOnBeginPlay)
	{
		for (UStaticMeshComponent* Mesh : PreviewComponents)
			if (Mesh != nullptr)
				Mesh->SetHiddenInGame(true);
		return;
	}
	FAGX_DistributedSpawnOptions Options;
	Options.Asset = Asset;
	Options.Transform = GetActorTransform();
	Options.ObjectKey = PlacementId.ToString();
	Options.bRequestSimulationAuthority = bRequestSimulationAuthority;
	auto* Subsystem = GetWorld()->GetSubsystem<UAGX_DistributedWorldSubsystem>();
	const FGuid Id = Subsystem != nullptr
						 ? Subsystem->QueueSpawn(
							   Options, FAGX_DistributedSpawnCallback::CreateUObject(
											this, &ThisClass::Completed))
						 : FGuid();
	if (!Id.IsValid())
	{
		FAGX_DistributedSpawnResult Result;
		Result.State = EAGX_DistributedSpawnState::Failed;
		Result.ErrorMessage =
			TEXT("Could not queue marker: scenario, descriptor or transform is invalid.");
		UE_LOG(LogAGX, Error, TEXT("%s"), *Result.ErrorMessage);
		Completed(Result);
	}
}
void AAGX_DistributedSpawnPoint::Completed(const FAGX_DistributedSpawnResult& Result)
{
	SetActorHiddenInGame(true);
	OnSpawnCompleted.Broadcast(Result);
	Destroy();
}
