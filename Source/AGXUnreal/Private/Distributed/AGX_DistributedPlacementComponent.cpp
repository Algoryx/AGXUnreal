// Copyright 2026, Algoryx Simulation AB.

#include "Distributed/AGX_DistributedPlacementComponent.h"
#include "AGX_LogCategory.h"
#include "Distributed/AGX_DistributedPreview.h"
#include "Distributed/AGX_DistributedWorldSubsystem.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

FAGX_DistributedInstanceReference::FAGX_DistributedInstanceReference()
	: FAGX_ComponentReference(UInstancedStaticMeshComponent::StaticClass())
{
}

UAGX_DistributedPlacementComponent::UAGX_DistributedPlacementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.2f;
}
void UAGX_DistributedPlacementComponent::EnsureIds()
{
	if (!CollectionId.IsValid())
		CollectionId = FGuid::NewGuid();
	TSet<FGuid> Seen;
	for (auto& Placement : Placements)
	{
		if (!Placement.Id.IsValid() || Seen.Contains(Placement.Id))
			Placement.Id = FGuid::NewGuid();
		Seen.Add(Placement.Id);
	}
}
void UAGX_DistributedPlacementComponent::OnRegister()
{
	Super::OnRegister();
	Source.SetLocalScope(GetOwner());
#if WITH_EDITOR
	if (GetOwner() != nullptr && !GetOwner()->IsTemplate() && GetWorld() != nullptr &&
		!GetWorld()->IsGameWorld())
		OwnerPlacementId = GetOwner()->GetActorInstanceGuid();
#endif
	if (!IsTemplate() && GetOwner() != nullptr && !GetOwner()->IsTemplate())
	{
		EnsureIds();
		RefreshPreview();
	}
}
void UAGX_DistributedPlacementComponent::PostDuplicate(EDuplicateMode::Type Mode)
{
	Super::PostDuplicate(Mode);
	if (Mode != EDuplicateMode::PIE)
		CollectionId = FGuid::NewGuid();
}
#if WITH_EDITOR
void UAGX_DistributedPlacementComponent::PostEditChangeProperty(FPropertyChangedEvent& Event)
{
	EnsureIds();
	Super::PostEditChangeProperty(Event);
}
#endif
void UAGX_DistributedPlacementComponent::CaptureInstances()
{
	if (GetWorld() == nullptr || GetWorld()->IsGameWorld())
		return;
	Source.SetLocalScope(GetOwner());
	auto* Instances = Source.GetComponent<UInstancedStaticMeshComponent>();
	if (Instances == nullptr)
		return;
	Modify();
	TArray<FAGX_DistributedPlacement> Previous = Placements;
	TSet<FGuid> Reused;
	Placements.Reset();
	for (int32 I = 0; I < Instances->GetInstanceCount(); ++I)
	{
		FTransform WorldTransform, LocalTransform;
		Instances->GetInstanceTransform(I, WorldTransform, true);
		Instances->GetInstanceTransform(I, LocalTransform, false);
		FAGX_DistributedPlacement Placement;
		Placement.Transform = WorldTransform.GetRelativeTransform(GetComponentTransform());
		Placement.SourceIndex = I;
		Placement.SourceTransform = LocalTransform;
		for (const auto& Old : Previous)
			if (!Reused.Contains(Old.Id) && Old.Transform.Equals(Placement.Transform))
			{
				Placement.Id = Old.Id;
				Reused.Add(Old.Id);
				break;
			}
		Placements.Add(Placement);
	}
	EnsureIds();
	RefreshPreview();
	MarkPackageDirty();
}
bool UAGX_DistributedPlacementComponent::AddPlacement(FGuid Id, FTransform Transform)
{
	if (!Id.IsValid() || Transform.ContainsNaN() || Active.Contains(Id))
		return false;
	for (auto& Placement : Placements)
		if (Placement.Id == Id)
		{
			Placement.Transform = Transform;
			return true;
		}
	FAGX_DistributedPlacement Placement;
	Placement.Id = Id;
	Placement.Transform = Transform;
	Placements.Add(Placement);
	return true;
}
void UAGX_DistributedPlacementComponent::BeginPlay()
{
	Super::BeginPlay();
	EnsureIds();
}
void UAGX_DistributedPlacementComponent::SetSourceHidden(
	const FAGX_DistributedPlacement& Placement, FActivePlacement& State, bool bHidden)
{
	if (State.bSourceHidden == bHidden)
		return;
	auto* Instances = Source.GetComponent<UInstancedStaticMeshComponent>();
	if (Placement.SourceIndex == INDEX_NONE)
	{
		const int32 Index = Placements.IndexOfByPredicate([&](const FAGX_DistributedPlacement& Item)
														  { return Item.Id == Placement.Id; });
		for (UInstancedStaticMeshComponent* Preview : PreviewComponents)
		{
			FTransform Transform;
			if (Preview->GetInstanceTransform(Index, Transform))
			{
				// Restore from the saved placement and mesh-local transform, never from a
				// zero-scale instance.
				const auto Visuals = AGX_GetDistributedPreview(Asset);
				const int32 VisualIndex = PreviewComponents.IndexOfByKey(Preview);
				if (!Visuals.IsValidIndex(VisualIndex))
					continue;
				Transform = Visuals[VisualIndex].Transform * Placement.Transform;
				if (bHidden)
					Transform.SetScale3D(FVector::ZeroVector);
				Preview->UpdateInstanceTransform(Index, Transform, false, true, true);
			}
		}
		State.bSourceHidden = bHidden;
		return;
	}
	if (Instances == nullptr)
		return;
	FTransform Current;
	if (!Instances->GetInstanceTransform(Placement.SourceIndex, Current, false))
		return;
	FTransform Hidden = Placement.SourceTransform;
	Hidden.SetScale3D(FVector::ZeroVector);
	const FTransform Expected = State.bSourceHidden ? Hidden : Placement.SourceTransform;
	if (!Current.Equals(Expected))
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Distributed placement source changed; recapture instances before play."));
		return;
	}
	Instances->UpdateInstanceTransform(
		Placement.SourceIndex, bHidden ? Hidden : Placement.SourceTransform, false, true, true);
	State.bSourceHidden = bHidden;
}
void UAGX_DistributedPlacementComponent::TickComponent(
	float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* TickFunction)
{
	Super::TickComponent(DeltaTime, TickType, TickFunction);
	auto* Subsystem = GetWorld()->GetSubsystem<UAGX_DistributedWorldSubsystem>();
	if (Subsystem == nullptr)
		return;
	for (const auto& Placement : Placements)
	{
		auto& State = Active.FindOrAdd(Placement.Id);
		const FTransform WorldTransform = Placement.Transform * GetComponentTransform();
		const bool bWanted =
			!bManagedBySpawnBubbles ||
			Subsystem->IsInsideSpawnBubble(WorldTransform.GetLocation(), State.bWanted);
		if (!bWanted && State.bWanted)
		{
			const FGuid PreviousRequest = State.RequestId;
			State.RequestId.Invalidate();
			Subsystem->ReleaseSpawn(PreviousRequest);
			State.bFailed = false;
		}
		if (!bWanted)
			State.bFailed = false;
		State.bWanted = bWanted;
		if (bWanted && !State.RequestId.IsValid() && !State.bFailed)
		{
			FAGX_DistributedSpawnOptions Options;
			Options.Asset = Asset;
			Options.Transform = WorldTransform;
			Options.ObjectKey = OwnerPlacementId.ToString() + TEXT(":") + CollectionId.ToString() +
								TEXT(":") + Placement.Id.ToString();
			Options.bRequestSimulationAuthority = bRequestSimulationAuthority;
			const FGuid PlacementId = Placement.Id;
			State.RequestId = Subsystem->QueueSpawn(
				Options, FAGX_DistributedSpawnCallback::CreateWeakLambda(
							 this,
							 [this, PlacementId](const FAGX_DistributedSpawnResult& Result)
							 {
								 auto* Current = Active.Find(PlacementId);
								 if (Current == nullptr || Current->RequestId != Result.RequestId)
									 return;
								 Current->Entity = Result.Entity;
								 Current->bFailed =
									 Result.State == EAGX_DistributedSpawnState::Failed;
								 OnSpawnCompleted.Broadcast(Result);
							 }));
			State.bFailed = !State.RequestId.IsValid();
		}
		if (State.RequestId.IsValid() && State.bWanted)
		{
			FAGX_DistributedSpawnResult Result;
			if (Subsystem->GetSpawnResult(State.RequestId, Result) && Result.Entity != nullptr)
				State.Entity = Result.Entity;
		}
		// Other clients can activate this object even when it is outside our own bubbles.
		State.Entity = Subsystem->FindWorldObjectActor(
			OwnerPlacementId.ToString() + TEXT(":") + CollectionId.ToString() + TEXT(":") +
			Placement.Id.ToString());
		SetSourceHidden(Placement, State, State.Entity.IsValid());
	}
}
void UAGX_DistributedPlacementComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	if (auto* Subsystem = GetWorld()->GetSubsystem<UAGX_DistributedWorldSubsystem>())
		for (const auto& Pair : Active)
			Subsystem->ReleaseSpawn(Pair.Value.RequestId);
	Super::EndPlay(Reason);
}

void UAGX_DistributedPlacementComponent::OnUnregister()
{
	for (UInstancedStaticMeshComponent* Preview : PreviewComponents)
		if (Preview != nullptr)
			Preview->DestroyComponent();
	PreviewComponents.Reset();
	Super::OnUnregister();
}
void UAGX_DistributedPlacementComponent::RefreshPreview()
{
	for (UInstancedStaticMeshComponent* Preview : PreviewComponents)
		if (Preview != nullptr)
			Preview->DestroyComponent();
	PreviewComponents.Reset();
	if (GetOwner() == nullptr || !IsRegistered())
		return;
	for (const auto& Visual : AGX_GetDistributedPreview(Asset))
	{
		auto* Mesh = NewObject<UInstancedStaticMeshComponent>(GetOwner(), NAME_None, RF_Transient);
		Mesh->SetStaticMesh(Visual.Mesh);
		for (int32 I = 0; I < Visual.Materials.Num(); ++I)
			Mesh->SetMaterial(I, Visual.Materials[I]);
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Mesh->SetMobility(EComponentMobility::Movable);
		Mesh->SetupAttachment(this);
		Mesh->RegisterComponent();
		for (const auto& Placement : Placements)
		{
			FTransform Transform = Visual.Transform * Placement.Transform;
			if (Placement.SourceIndex != INDEX_NONE)
				Transform.SetScale3D(FVector::ZeroVector);
			Mesh->AddInstance(Transform);
		}
		PreviewComponents.Add(Mesh);
	}
}
