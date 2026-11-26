// Copyright 2026, Algoryx Simulation AB.

#include "Distributed/AGX_DistributedWorldSubsystem.h"

// AGX Dynamics for Unreal includes.
#include "AGX_InternalDelegateAccessor.h"
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_Simulation.h"
#include "Distributed/AGX_DistributedScenarioAsset.h"
#include "Distributed/AGX_DistributedAsset.h"
#include "Distributed/AGX_DistributedSpawnBubbleComponent.h"
#include "Distributed/AGX_DistributedSettings.h"
#include "Distributed/OpenPLX_DistributedSignalRerouter.h"
#include "OpenPLX/OpenPLX_Inputs.h"
#include "OpenPLX/OpenPLX_SignalHandlerComponent.h"
#include "Utilities/OpenPLX_Utilities.h"

// Unreal Engine includes.
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "HAL/PlatformTime.h"
#include "Misc/EngineVersionComparison.h"

// Standard library includes.
#include <type_traits>
#include <variant>

bool UAGX_DistributedWorldSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	const UWorld* World = Cast<UWorld>(Outer);
	return Super::ShouldCreateSubsystem(Outer) && World != nullptr && World->IsGameWorld();
}

void UAGX_DistributedWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	SignalHandlerInitializingHandle = UOpenPLX_SignalHandlerComponent::OnInitializing().AddUObject(
		this, &ThisClass::OnSignalHandlerInitializing);
	const UAGX_DistributedSettings* Settings = GetDefault<UAGX_DistributedSettings>();
	if (!Settings->Scenario.IsNull())
		Scenario = Settings->Scenario.LoadSynchronous();
	if (!Settings->Scenario.IsNull() && Scenario == nullptr)
		UE_LOG(
			LogAGX, Error, TEXT("AGX Distributed scenario asset could not be loaded: %s"),
			*Settings->Scenario.ToString());
}

void UAGX_DistributedWorldSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	if (Scenario == nullptr || bInitializationAttempted)
		return;

	bInitializationAttempted = true;
	if (Scenario->bLaunchWorldManager)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT(
				"AGX Distributed WorldManager launch is not implemented; connecting to an external "
				"manager."));
	}

	UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(&InWorld);
	if (Simulation == nullptr || Simulation->GetNative() == nullptr)
	{
		UE_LOG(LogAGX, Error, TEXT("AGX Distributed needs an AGX simulation to start."));
		return;
	}

	if (Client.Initialize(*Simulation->GetNative(), Scenario->ZenohEndpoint))
	{
		UE_LOG(
			LogAGX, Log, TEXT("Starting AGX Distributed client; endpoint: %s"),
			Scenario->ZenohEndpoint.IsEmpty() ? TEXT("default discovery")
											  : *Scenario->ZenohEndpoint);
		BoundSimulation = Simulation;
		PostStepHandle =
			FAGX_InternalDelegateAccessor::GetOnPostStepForwardInternal(*Simulation)
				.AddUObject(this, &UAGX_DistributedWorldSubsystem::CollectAndSendEntityUpdates);
		Client.StartAndLoad();
	}
}

void UAGX_DistributedWorldSubsystem::Deinitialize()
{
	UOpenPLX_SignalHandlerComponent::OnInitializing().Remove(SignalHandlerInitializingHandle);
	SignalHandlerInitializingHandle.Reset();
	ShutdownClient();
	Scenario = nullptr;
	Super::Deinitialize();
}

void UAGX_DistributedWorldSubsystem::ShutdownClient()
{
	if (UAGX_Simulation* Simulation = BoundSimulation.Get())
	{
		FAGX_InternalDelegateAccessor::GetOnPostStepForwardInternal(*Simulation)
			.Remove(PostStepHandle);
	}

	PostStepHandle.Reset();
	BoundSimulation.Reset();
	bShutdownRequested = true;
	TArray<FGuid> RequestIds;
	SpawnRequests.GetKeys(RequestIds);
	for (FGuid Id : RequestIds)
		CompleteSpawn(
			Id, EAGX_DistributedSpawnState::Cancelled, TEXT("Distributed world is shutting down."));
	SpawnRequests.Reset();
	// Remove pending requests before invoking callbacks, which may call back into this subsystem.
	auto PendingPossessions = MoveTemp(PossessionRequests);
	PossessionRequests.Reset();
	for (const auto& Request : PendingPossessions)
		Request.Value.Callback.ExecuteIfBound(
			false, Request.Value.Entity.Get(), TEXT("The distributed client is shutting down."));
	auto PendingAuthorities = MoveTemp(SimulationAuthorityRequests);
	SimulationAuthorityRequests.Reset();
	for (const auto& Request : PendingAuthorities)
		Request.Value.Callback.ExecuteIfBound(
			false, Request.Value.Entity.Get(), TEXT("The distributed client is shutting down."));
	EntityErrors.Reset();
	RemovedEntityIds.Reset();
	DeferredSpawnEvents.Reset();
	DeferredEntityUpdates.Reset();
	DeferredControlSignals.Reset();
	OriginalMotionControls.Reset();
	bSimulationRunning = false;
	for (const auto& Pair : EntityActors)
		if (AActor* Actor = Pair.Value.Get())
			Actor->OnEndPlay.RemoveDynamic(this, &ThisClass::OnEntityEndPlay);
	Client.Shutdown();
	EntityActors.Reset();
	ActorSignalRerouters.Reset();
	ReportedUpdateErrors.Reset();
	ReportedSendErrors.Reset();
}

void UAGX_DistributedWorldSubsystem::CollectAndSendEntityUpdates(double TimeStamp)
{
	check(IsInGameThread());
	(void) TimeStamp;
	if (bShutdownRequested || !Client.IsReady())
		return;

	TMap<uint64, TArray<FRigidBodyBarrier>> EntityBodies;
	for (uint64 EntityId : Client.GetSimulatedEntities())
	{
		auto ReportError = [this, EntityId](const TCHAR* Reason)
		{
			if (!ReportedSendErrors.Contains(EntityId))
			{
				ReportedSendErrors.Add(EntityId);
				UE_LOG(
					LogAGX, Error, TEXT("AGX Distributed cannot send entity %llu: %s"), EntityId,
					Reason);
			}
		};
		const TWeakObjectPtr<AActor>* Entry = EntityActors.Find(EntityId);
		AActor* Actor = Entry != nullptr ? Entry->Get() : nullptr;
		if (!IsValid(Actor))
		{
			ReportError(TEXT("No live actor is registered for this entity."));
			continue;
		}

		TArray<UAGX_RigidBodyComponent*> Bodies;
		Actor->GetComponents<UAGX_RigidBodyComponent>(Bodies, true);
		// Use the same body membership and ordering as processIncomingEntityUpdates.
		Bodies.Sort([](const UAGX_RigidBodyComponent& Lhs, const UAGX_RigidBodyComponent& Rhs)
					{ return Lhs.ImportName < Rhs.ImportName; });

		TArray<FRigidBodyBarrier> Barriers;
		Barriers.Reserve(Bodies.Num());
		bool bComplete = true;
		for (UAGX_RigidBodyComponent* Body : Bodies)
		{
			const FRigidBodyBarrier* Barrier = Body->GetNative();
			if (Barrier == nullptr || !Barrier->HasNative())
			{
				ReportError(TEXT("The Blueprint rigid body has no AGX Dynamics representation."));
				bComplete = false;
				break;
			}
			Barriers.Add(*Barrier);
		}
		if (bComplete)
		{
			EntityBodies.Add(EntityId, MoveTemp(Barriers));
			ReportedSendErrors.Remove(EntityId);
		}
	}
	Client.SendEntityUpdates(EntityBodies);
}

void UAGX_DistributedWorldSubsystem::Tick(float DeltaTime)
{
	(void) DeltaTime;
	if (Scenario == nullptr || bShutdownRequested)
		return;

	TArray<FAGXDistributedEvent> Events;
	Client.Poll(Events);
	for (const FAGXDistributedEvent& Event : Events)
	{
		if (Event.Type == EAGXDistributedEventType::SpawnEntity)
		{
			if (!RemovedEntityIds.Contains(Event.EntityId))
				DeferredSpawnEvents.Add(Event);
		}
		else if (Event.Type == EAGXDistributedEventType::EntityUpdate)
		{
			if (!RemovedEntityIds.Contains(Event.EntityId))
				DeferredEntityUpdates.Add(Event.EntityId, Event);
		}
		else if (Event.Type == EAGXDistributedEventType::ControlSignal)
		{
			if (!RemovedEntityIds.Contains(Event.EntityId) && !Event.SignalName.IsEmpty())
				DeferredControlSignals.FindOrAdd(Event.EntityId)
					.Add(FName(*Event.SignalName), Event.SignalValue);
		}
		else
			HandleEvent(Event);
		if (bShutdownRequested)
			return;
	}
	const int32 SpawnCount =
		FMath::Min(DeferredSpawnEvents.Num(), FMath::Max(1, Scenario->MaxActorCreationsPerTick));
	for (int32 I = 0; I < SpawnCount; ++I)
		HandleSpawnEntityEvent(DeferredSpawnEvents[I]);
	DeferredSpawnEvents.RemoveAt(0, SpawnCount, EAllowShrinking::No);
	for (auto It = DeferredEntityUpdates.CreateIterator(); It; ++It)
	{
		if (EntityActors.Contains(It.Key()))
		{
			processIncomingEntityUpdates(It.Value());
			It.RemoveCurrent();
		}
		else if (EntityErrors.Contains(It.Key()))
			It.RemoveCurrent();
	}
	for (auto It = DeferredControlSignals.CreateIterator(); It; ++It)
	{
		if (EntityActors.Contains(It.Key()))
		{
			ApplyControlSignals(It.Key(), It.Value());
			It.RemoveCurrent();
		}
		else if (EntityErrors.Contains(It.Key()))
			It.RemoveCurrent();
	}
	ProcessSpawnRequests();

	if (bInitializationAttempted && !Client.IsReady() &&
		FPlatformTime::Seconds() >= NextStartAttemptTime)
	{
		Client.StartAndLoad();
		NextStartAttemptTime = FPlatformTime::Seconds() + 5.0;
	}
}

TStatId UAGX_DistributedWorldSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UAGX_DistributedWorldSubsystem, STATGROUP_Tickables);
}

bool UAGX_DistributedWorldSubsystem::ShouldStepSimulation() const
{
	return Scenario == nullptr || bSimulationRunning;
}

bool UAGX_DistributedWorldSubsystem::RequestSpawn(
	const FString& AssetPath, FTransform Transform, bool bPossess)
{
	if (Scenario == nullptr || AssetPath.IsEmpty())
	{
		return false;
	}
	return Client.RequestSpawn(AssetPath, Transform, bPossess);
}

bool UAGX_DistributedWorldSubsystem::RequestPossession(int64 EntityId, bool bPossess)
{
	return EntityId >= 0 && Client.RequestPossession(static_cast<uint64>(EntityId), bPossess);
}

FGuid UAGX_DistributedWorldSubsystem::RequestActorPossession(
	AActor* Entity, bool bPossess, FAGX_DistributedPossessionCallback Callback)
{
	check(IsInGameThread());
	int64 EntityId = -1;
	if (bShutdownRequested || !Client.IsReady() || !GetEntityIdForActor(Entity, EntityId))
		return FGuid();
	const FGuid RequestId = FGuid::NewGuid();
	if (!Client.RequestPossession(static_cast<uint64>(EntityId), bPossess, RequestId))
		return FGuid();
	PossessionRequests.Add(RequestId, {Entity, MoveTemp(Callback)});
	return RequestId;
}

void UAGX_DistributedWorldSubsystem::CompletePossession(const FAGXDistributedEvent& Event)
{
	FPendingPossession Request;
	if (!PossessionRequests.RemoveAndCopyValue(Event.RequestId, Request))
		return;
	if (!Request.Entity.IsValid())
	{
		if (Event.bAccepted && Event.bRequestedValue)
			RequestPossession(static_cast<int64>(Event.EntityId), false);
		Request.Callback.ExecuteIfBound(
			false, nullptr, TEXT("The target Actor was destroyed before possession completed."));
		return;
	}
	Request.Callback.ExecuteIfBound(
		Event.bAccepted, Request.Entity.Get(), Event.bAccepted ? FString() : Event.Message);
}

bool UAGX_DistributedWorldSubsystem::IsPossessed(int64 EntityId) const
{
	return EntityId >= 0 && Client.IsPossessed(static_cast<uint64>(EntityId));
}

bool UAGX_DistributedWorldSubsystem::GetEntityIdForActor(const AActor* Actor, int64& EntityId) const
{
	EntityId = -1;
	if (!IsValid(Actor) || Actor->GetWorld() != GetWorld())
		return false;
	for (const auto& Entry : EntityActors)
	{
		if (Entry.Value.Get() == Actor && Entry.Key <= static_cast<uint64>(MAX_int64) &&
			!RemovedEntityIds.Contains(Entry.Key))
		{
			EntityId = static_cast<int64>(Entry.Key);
			return true;
		}
	}
	return false;
}

bool UAGX_DistributedWorldSubsystem::RequestSimulationAuthority(int64 EntityId, bool bSimulate)
{
	return EntityId >= 0 &&
		   Client.RequestSimulationAuthority(static_cast<uint64>(EntityId), bSimulate);
}

bool UAGX_DistributedWorldSubsystem::TryGetActorPossession(
	const AActor* Actor, bool& bHasPossession) const
{
	bHasPossession = false;
	int64 EntityId = -1;
	if (bShutdownRequested || !Client.IsReady() || !GetEntityIdForActor(Actor, EntityId))
		return false;
	bHasPossession = Client.IsPossessed(static_cast<uint64>(EntityId));
	return true;
}

bool UAGX_DistributedWorldSubsystem::TryGetActorSimulationAuthority(
	const AActor* Actor, bool& bHasSimulationAuthority) const
{
	bHasSimulationAuthority = false;
	int64 EntityId = -1;
	if (bShutdownRequested || !Client.IsReady() || !GetEntityIdForActor(Actor, EntityId))
		return false;
	bHasSimulationAuthority = Client.GetSimulatedEntities().Contains(static_cast<uint64>(EntityId));
	return true;
}

FGuid UAGX_DistributedWorldSubsystem::RequestActorSimulationAuthority(
	AActor* Entity, bool bSimulate, FAGX_DistributedSimulationAuthorityCallback Callback)
{
	check(IsInGameThread());
	int64 EntityId = -1;
	if (bShutdownRequested || !Client.IsReady() || !GetEntityIdForActor(Entity, EntityId))
		return FGuid();
	const FGuid RequestId = FGuid::NewGuid();
	if (!Client.RequestSimulationAuthority(static_cast<uint64>(EntityId), bSimulate, RequestId))
		return FGuid();
	SimulationAuthorityRequests.Add(RequestId, {Entity, MoveTemp(Callback)});
	return RequestId;
}

void UAGX_DistributedWorldSubsystem::CompleteSimulationAuthority(const FAGXDistributedEvent& Event)
{
	FPendingSimulationAuthority Request;
	if (!SimulationAuthorityRequests.RemoveAndCopyValue(Event.RequestId, Request))
		return;
	if (!Request.Entity.IsValid())
	{
		if (Event.bAccepted && Event.bRequestedValue)
			RequestSimulationAuthority(static_cast<int64>(Event.EntityId), false);
		Request.Callback.ExecuteIfBound(
			false, nullptr,
			TEXT("The target Actor was destroyed before simulation authority completed."));
		return;
	}
	Request.Callback.ExecuteIfBound(
		Event.bAccepted, Request.Entity.Get(), Event.bAccepted ? FString() : Event.Message);
}

bool UAGX_DistributedWorldSubsystem::RouteSignal(
	int64 EntityId, const FString& SignalName, double Value)
{
	return RouteSignalValue(EntityId, SignalName, Value);
}

bool UAGX_DistributedWorldSubsystem::RouteSignalValue(
	int64 EntityId, const FString& SignalName, const FAGXDistributedSignalValue& Value)
{
	return EntityId >= 0 && Client.RouteSignal(static_cast<uint64>(EntityId), SignalName, Value);
}

bool UAGX_DistributedWorldSubsystem::IsConnected() const
{
	return Client.IsReady();
}

void UAGX_DistributedWorldSubsystem::HandleEvent(const FAGXDistributedEvent& Event)
{
	switch (Event.Type)
	{
		case EAGXDistributedEventType::Started:
			UE_LOG(LogAGX, Log, TEXT("AGX Distributed joined world as node '%s'."), *Event.Message);
			if (Scenario->EntityIdToPossess >= 0)
				RequestPossession(Scenario->EntityIdToPossess, true);
			break;
		case EAGXDistributedEventType::StartFailed:
			UE_LOG(LogAGX, Warning, TEXT("AGX Distributed join failed: %s"), *Event.Message);
			NextStartAttemptTime = FPlatformTime::Seconds() + 5.0;
			break;
		case EAGXDistributedEventType::SpawnEntity:
			HandleSpawnEntityEvent(Event);
			break;
		case EAGXDistributedEventType::EntityUpdate:
			processIncomingEntityUpdates(Event);
			break;
		case EAGXDistributedEventType::SimulationAuthorityChanged:
			if (!RemovedEntityIds.Contains(Event.EntityId))
				ApplySimulationAuthority(Event.EntityId);
			break;
		case EAGXDistributedEventType::StaleEntityState:
			if (!Client.GetSimulatedEntities().Contains(Event.EntityId))
				if (const auto* Entry = EntityActors.Find(Event.EntityId))
					if (AActor* Actor = Entry->Get())
					{
						TArray<UAGX_RigidBodyComponent*> Bodies;
						Actor->GetComponents<UAGX_RigidBodyComponent>(Bodies, true);
						for (auto* Body : Bodies)
						{
							Body->SetVelocity(FVector::ZeroVector);
							Body->SetAngularVelocity(FVector::ZeroVector);
						}
					}
			break;
		case EAGXDistributedEventType::RemoveEntity:
		{
			RemovedEntityIds.Add(Event.EntityId);
			DeferredSpawnEvents.RemoveAll([&](const FAGXDistributedEvent& Spawn)
										  { return Spawn.EntityId == Event.EntityId; });
			DeferredEntityUpdates.Remove(Event.EntityId);
			DeferredControlSignals.Remove(Event.EntityId);
			EntityErrors.Remove(Event.EntityId);
			ReportedUpdateErrors.Remove(Event.EntityId);
			ReportedSendErrors.Remove(Event.EntityId);
			if (auto* Entry = EntityActors.Find(Event.EntityId))
			{
				ActorSignalRerouters.Remove(*Entry);
				if (AActor* Actor = Entry->Get())
					Actor->Destroy();
			}
			EntityActors.Remove(Event.EntityId);
			for (auto& Pair : SpawnRequests)
				if (Pair.Value.Result.EntityId == static_cast<int64>(Event.EntityId))
					Pair.Value.Result.Entity = nullptr;
			OriginalMotionControls.Remove(Event.EntityId);
			break;
		}
		case EAGXDistributedEventType::ReleaseInterestResult:
			if (auto* Request = SpawnRequests.Find(Event.RequestId))
			{
				Request->bReleaseInFlight = false;
				if (Event.bAccepted)
					SpawnRequests.Remove(Event.RequestId);
				else
					Request->NextReleaseAttempt = FPlatformTime::Seconds() + 2.0;
			}
			break;
		case EAGXDistributedEventType::AssetRegistrationResult:
			if (auto* Request = SpawnRequests.Find(Event.RequestId))
			{
				// ProcessSpawnRequests owns deadline completion, even if a reply arrives late.
				if (Request->bReleaseRequested || FPlatformTime::Seconds() > Request->Deadline)
					break;
				if (Event.bAccepted && Event.AssetId != MAX_uint64)
					Request->AssetId = Event.AssetId;
				else
				{
					CompleteSpawn(
						Event.RequestId, EAGX_DistributedSpawnState::Failed,
						FString::Printf(
							TEXT("Could not register asset '%s': %s"), *Event.AssetPath,
							Event.Message.IsEmpty() ? TEXT("World Manager rejected registration.")
													: *Event.Message));
					ReleaseSpawn(Event.RequestId);
				}
			}
			break;
		case EAGXDistributedEventType::SpawnRequestResult:
			if (auto* Request = SpawnRequests.Find(Event.RequestId))
			{
				Request->bAcknowledged = true;
				if (Event.bAccepted && Event.EntityId <= static_cast<uint64>(MAX_int64))
					Request->Result.EntityId = static_cast<int64>(Event.EntityId);
				else
				{
					CompleteSpawn(
						Event.RequestId, EAGX_DistributedSpawnState::Failed,
						Event.Message.IsEmpty() ? TEXT("World Manager rejected spawn.")
												: Event.Message);
					if (auto* Failed = SpawnRequests.Find(Event.RequestId))
						Failed->bReleaseRequested = true;
				}
			}
			UE_LOG(
				LogAGX, Log, TEXT("AGX Distributed spawn request for '%s': %s (entity %llu). %s"),
				*Event.AssetPath, Event.bAccepted ? TEXT("accepted") : TEXT("rejected"),
				Event.EntityId, *Event.Message);
			break;
		case EAGXDistributedEventType::PossessionResult:
		case EAGXDistributedEventType::SimulationAuthorityResult:
			if (Event.Type == EAGXDistributedEventType::PossessionResult)
			{
				CompletePossession(Event);
				OnPossessionCompleted.Broadcast(
					Event.RequestId, static_cast<int64>(Event.EntityId), Event.bRequestedValue,
					Event.bAccepted, Event.bAccepted ? FString() : Event.Message);
			}
			if (Event.Type == EAGXDistributedEventType::SimulationAuthorityResult)
			{
				if (Event.bAccepted)
					ApplySimulationAuthority(Event.EntityId);
				CompleteSimulationAuthority(Event);
				OnSimulationAuthorityCompleted.Broadcast(
					static_cast<int64>(Event.EntityId), Event.bRequestedValue, Event.bAccepted,
					Event.bAccepted
						? FString()
						: (Event.Message.IsEmpty() ? TEXT("Simulation authority request rejected.")
												   : Event.Message));
			}
			UE_LOG(
				LogAGX, Log, TEXT("AGX Distributed %s request for entity %llu: %s. %s"),
				Event.Type == EAGXDistributedEventType::PossessionResult
					? TEXT("possession")
					: TEXT("simulation authority"),
				Event.EntityId, Event.bAccepted ? TEXT("accepted") : TEXT("rejected"),
				*Event.Message);
			break;
		case EAGXDistributedEventType::SimulationStart:
			bSimulationRunning = true;
			UE_LOG(LogAGX, Log, TEXT("AGX Distributed simulation started."));
			break;
		case EAGXDistributedEventType::SimulationPause:
			bSimulationRunning = false;
			UE_LOG(LogAGX, Log, TEXT("AGX Distributed simulation paused."));
			break;
		case EAGXDistributedEventType::ShutdownRequested:
			UE_LOG(LogAGX, Log, TEXT("AGX Distributed requested client shutdown."));
			ShutdownClient();
			break;
	}
}

void UAGX_DistributedWorldSubsystem::HandleSpawnEntityEvent(const FAGXDistributedEvent& Event)
{
	check(IsInGameThread());
	if (const TWeakObjectPtr<AActor>* Existing = EntityActors.Find(Event.EntityId))
	{
		if (Existing->IsValid())
			return;
		ActorSignalRerouters.Remove(*Existing);
		EntityActors.Remove(Event.EntityId);
	}
	if (Event.EntityId > static_cast<uint64>(MAX_int64))
	{
		EntityErrors.Add(Event.EntityId, TEXT("The entity ID exceeds the supported Unreal range."));
		UE_LOG(
			LogAGX, Error, TEXT("Cannot spawn distributed entity %llu: unsupported entity ID."),
			Event.EntityId);
		return;
	}

	UWorld* World = GetWorld();
	if (Scenario == nullptr || World == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("AGX Distributed cannot spawn entity %llu without a scenario and world."),
			Event.EntityId);
		return;
	}

	const TSoftClassPtr<AActor>* Mapped = nullptr;
	double ProximityRadius = 600.0;
	for (const UAGX_DistributedAsset* Asset : Scenario->Assets)
		if (Asset != nullptr &&
			Asset->AssetPath.Equals(Event.AssetPath, ESearchCase::CaseSensitive))
		{
			Mapped = &Asset->ActorClass;
			ProximityRadius = Asset->SimulationAuthoritySensorRadius;
			break;
		}
	// A queued descriptor can resolve a locally requested asset even before the catalog is updated.
	if (Mapped == nullptr)
		for (const auto& Pair : SpawnRequests)
			if (Pair.Value.Options.Asset != nullptr &&
				Pair.Value.AssetPath.Equals(Event.AssetPath, ESearchCase::CaseSensitive))
			{
				Mapped = &Pair.Value.Options.Asset->ActorClass;
				ProximityRadius = Pair.Value.Options.Asset->SimulationAuthoritySensorRadius;
				break;
			}
	if (Mapped == nullptr && Event.AssetId <= static_cast<uint64>(MAX_int64))
		Mapped = Scenario->AssetBlueprintsById.Find(static_cast<int64>(Event.AssetId));
	if (Mapped == nullptr)
		Mapped = Scenario->AssetBlueprints.Find(Event.AssetPath);
	if (Mapped == nullptr || Mapped->IsNull())
	{
		EntityErrors.Add(
			Event.EntityId, TEXT("No Unreal Actor class is mapped for this distributed asset."));
		UE_LOG(
			LogAGX, Error,
			TEXT(
				"AGX Distributed entity %llu cannot spawn asset %llu ('%s'): no Blueprint is "
				"mapped."),
			Event.EntityId, Event.AssetId, *Event.AssetPath);
		return;
	}

	const TSoftClassPtr<AActor> MappedClass = *Mapped;
	UClass* ActorClass = MappedClass.LoadSynchronous();
	if (ActorClass == nullptr)
	{
		EntityErrors.Add(
			Event.EntityId, TEXT("The mapped Unreal Actor class could not be loaded."));
		UE_LOG(
			LogAGX, Error,
			TEXT(
				"AGX Distributed entity %llu cannot spawn asset %llu: Blueprint '%s' could not be "
				"loaded."),
			Event.EntityId, Event.AssetId, *MappedClass.ToString());
		return;
	}

	FActorSpawnParameters Params;
	// The distributed scenario determines placement, including overlapping actors.
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	const auto Rerouter =
		std::make_shared<FOpenPLX_DistributedSignalRerouter>(static_cast<int64>(Event.EntityId));
	TWeakObjectPtr<AActor> PendingActor;
	// Register before Blueprint components initialize, so their initialization notifications can
	// obtain the same rerouter before any BeginPlay callbacks run.
#if UE_VERSION_OLDER_THAN(5, 8, 0)
	Params.CustomPreSpawnInitalization =
#else
	Params.CustomPreSpawnInitialization =
#endif
		[this, EntityId = Event.EntityId, Rerouter, &PendingActor](AActor* SpawnedActor)
	{
		PendingActor = SpawnedActor;
		EntityActors.Add(EntityId, SpawnedActor);
		SpawnedActor->OnEndPlay.AddUniqueDynamic(this, &ThisClass::OnEntityEndPlay);
		ActorSignalRerouters.Add(PendingActor, Rerouter);
	};
	AActor* Actor = World->SpawnActor<AActor>(ActorClass, Event.Transform, Params);
	if (Actor == nullptr)
	{
		ActorSignalRerouters.Remove(PendingActor);
		EntityActors.Remove(Event.EntityId);
		EntityErrors.Add(Event.EntityId, TEXT("Unreal failed to create the entity Actor."));
		UE_LOG(
			LogAGX, Error,
			TEXT("AGX Distributed entity %llu failed to spawn asset %llu as Blueprint '%s'."),
			Event.EntityId, Event.AssetId, *MappedClass.ToString());
		return;
	}

	ReportedUpdateErrors.Remove(Event.EntityId);

	UE_LOG(
		LogAGX, Log,
		TEXT("AGX Distributed entity %llu spawned asset %llu ('%s') as actor '%s' at %s."),
		Event.EntityId, Event.AssetId, *Event.AssetPath, *Actor->GetName(),
		*Event.Transform.ToHumanReadableString());

	TArray<UAGX_RigidBodyComponent*> Bodies;
	Actor->GetComponents<UAGX_RigidBodyComponent>(Bodies, true);
	Bodies.Sort([](const UAGX_RigidBodyComponent& A, const UAGX_RigidBodyComponent& B)
				{ return A.ImportName < B.ImportName; });
	auto& Original = OriginalMotionControls.FindOrAdd(Event.EntityId);
	TArray<FRigidBodyBarrier> Barriers;
	FVector OriginSum = FVector::ZeroVector;
	for (UAGX_RigidBodyComponent* Body : Bodies)
	{
		Original.Add(Body->GetMotionControl());

		// Spawn completion must also make final-state serialization possible before the first step.
		FRigidBodyBarrier* NativeBody = Body->GetOrCreateNative();
		if (NativeBody != nullptr && NativeBody->HasNative())
		{
			Barriers.Add(*NativeBody);
			OriginSum += Body->GetPosition();
		}
	}
	if (Barriers.Num() == Bodies.Num())
	{
		const FVector Center = Bodies.IsEmpty() ? Event.Transform.GetLocation()
												: OriginSum / static_cast<double>(Bodies.Num());
		Client.RegisterEntityBodies(Event.EntityId, Barriers, Center, ProximityRadius);
	}
	ApplySimulationAuthority(Event.EntityId);
}

void UAGX_DistributedWorldSubsystem::OnEntityEndPlay(AActor* Actor, EEndPlayReason::Type Reason)
{
	(void) Reason;
	for (auto It = EntityActors.CreateIterator(); It; ++It)
		if (It.Value().Get() == Actor)
		{
			const uint64 Id = It.Key();
			Client.UnregisterEntityBodies(Id);
			OriginalMotionControls.Remove(Id);
			DeferredEntityUpdates.Remove(Id);
			DeferredControlSignals.Remove(Id);
			ActorSignalRerouters.Remove(It.Value());
			It.RemoveCurrent();
			break;
		}
}

void UAGX_DistributedWorldSubsystem::ApplyControlSignals(
	uint64 EntityId, const TMap<FName, FAGXDistributedSignalValue>& Signals)
{
	check(IsInGameThread());
	if (bShutdownRequested || RemovedEntityIds.Contains(EntityId) ||
		!Client.GetSimulatedEntities().Contains(EntityId))
		return;
	const auto* Entry = EntityActors.Find(EntityId);
	AActor* Actor = Entry != nullptr ? Entry->Get() : nullptr;
	if (!IsValid(Actor))
		return;

	TArray<UOpenPLX_SignalHandlerComponent*> Handlers;
	Actor->GetComponents<UOpenPLX_SignalHandlerComponent>(Handlers, true);
	Handlers.RemoveAll([this, Actor](const auto* Handler)
					   { return !IsValid(Handler) || FindSignalHandlerEntity(*Handler) != Actor; });
	for (const auto& Signal : Signals)
	{
		UOpenPLX_SignalHandlerComponent* Receiver = nullptr;
		const FOpenPLX_Input* Input = nullptr;
		bool bAmbiguous = false;
		for (auto* Handler : Handlers)
		{
			const FOpenPLX_Input* Found = Handler->Inputs.Find(Signal.Key);
			if (Found == nullptr)
				continue;
			if (Receiver != nullptr)
			{
				bAmbiguous = true;
				break;
			}
			Receiver = Handler;
			Input = Found;
		}
		if (Receiver == nullptr || bAmbiguous)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("AGX Distributed cannot apply signal '%s' to entity %llu: %s."),
				*Signal.Key.ToString(), EntityId,
				bAmbiguous ? TEXT("input name matches multiple signal handlers")
						   : TEXT("no matching input"));
			continue;
		}
		const bool bApplied = std::visit(
			[Receiver, Input](const auto& Value)
			{
				using T = std::decay_t<decltype(Value)>;
				if constexpr (std::is_same_v<T, double>)
					return Receiver->ApplyRealLocally(*Input, Value);
				else if constexpr (std::is_same_v<T, int64>)
					return Receiver->ApplyIntegerLocally(*Input, Value);
				else if constexpr (std::is_same_v<T, bool>)
					return Receiver->ApplyBooleanLocally(*Input, Value);
				else if constexpr (std::is_same_v<T, FVector2D>)
					return FOpenPLX_Utilities::IsRangeType(Input->Type)
							   ? Receiver->ApplyRangeRealLocally(*Input, Value)
							   : Receiver->ApplyVector2Locally(*Input, Value);
				else
					return Receiver->ApplyVectorLocally(*Input, Value);
			},
			Signal.Value);
		if (!bApplied)
			UE_LOG(
				LogAGX, Warning,
				TEXT("AGX Distributed could not apply signal '%s' to entity %llu."),
				*Signal.Key.ToString(), EntityId);
	}
}

AActor* UAGX_DistributedWorldSubsystem::FindSignalHandlerEntity(
	const UOpenPLX_SignalHandlerComponent& Component) const
{
	// Child Actor handlers belong to the nearest registered distributed machine.
	for (AActor* Actor = Component.GetOwner(); Actor != nullptr; Actor = Actor->GetParentActor())
	{
		if (ActorSignalRerouters.Contains(Actor))
			return Actor;
	}
	return nullptr;
}

void UAGX_DistributedWorldSubsystem::OnSignalHandlerInitializing(
	UOpenPLX_SignalHandlerComponent& Component)
{
	if (bShutdownRequested || Component.GetWorld() != GetWorld())
		return;
	if (AActor* Actor = FindSignalHandlerEntity(Component))
		Component.SetSignalRerouter(ActorSignalRerouters.FindChecked(Actor));
}

void UAGX_DistributedWorldSubsystem::processIncomingEntityUpdates(const FAGXDistributedEvent& Event)
{
	check(IsInGameThread());
	if (Client.GetSimulatedEntities().Contains(Event.EntityId))
		return;
	// Report persistent model/protocol problems once per entity, not once per simulation step.
	auto ReportError = [this, &Event](const TCHAR* Reason)
	{
		if (!ReportedUpdateErrors.Contains(Event.EntityId))
		{
			ReportedUpdateErrors.Add(Event.EntityId);
			UE_LOG(
				LogAGX, Error, TEXT("AGX Distributed cannot update entity %llu: %s"),
				Event.EntityId, Reason);
		}
	};
	if (!Event.Message.IsEmpty())
	{
		ReportError(*Event.Message);
		return;
	}

	const TWeakObjectPtr<AActor>* Entry = EntityActors.Find(Event.EntityId);
	AActor* Actor = Entry != nullptr ? Entry->Get() : nullptr;
	if (!IsValid(Actor))
	{
		ReportError(TEXT("No live actor is registered for this entity."));
		return;
	}

	TArray<UAGX_RigidBodyComponent*> Bodies;
	Actor->GetComponents<UAGX_RigidBodyComponent>(Bodies, true);
	if (Bodies.Num() != Event.BodyTransforms.Num() || Bodies.Num() != Event.BodyVelocities.Num() ||
		Bodies.Num() != Event.BodyAngularVelocities.Num())
	{
		ReportError(TEXT("Serialized body count does not match the Blueprint rigid body count."));
		return;
	}
	if (Bodies.IsEmpty())
		return;

	auto OrderByName = [](const UAGX_RigidBodyComponent& Lhs, const UAGX_RigidBodyComponent& Rhs)
	{ return Lhs.ImportName < Rhs.ImportName; };

	Bodies.Sort(OrderByName);

	for (int32 I = 0; I < Bodies.Num(); ++I)
	{
		FRigidBodyBarrier* Body = Bodies[I]->GetNative();
		if (Body == nullptr || !Body->HasNative())
		{
			ReportError(TEXT("The Blueprint rigid body has no AGX Dynamics representation."));
			return;
		}

		// These are absolute world-space transforms. Convert() and the barrier handle
		// AGX/Unreal units and handedness; no actor-relative transform is composed here.
		// Do not set the scene component transform: its normal synchronization reads AGX later.
		Body->SetPosition(Event.BodyTransforms[I].GetTranslation());
		Body->SetRotation(Event.BodyTransforms[I].GetRotation());
		Body->SetVelocity(Event.bAssigned ? Event.BodyVelocities[I] : FVector::ZeroVector);
		Body->SetAngularVelocity(
			Event.bAssigned ? Event.BodyAngularVelocities[I] : FVector::ZeroVector);
		ReportedUpdateErrors.Remove(Event.EntityId);
	}
}

FGuid UAGX_DistributedWorldSubsystem::QueueEntitySpawn(const FAGX_DistributedSpawnOptions& Options)
{
	return QueueSpawn(Options);
}

FGuid UAGX_DistributedWorldSubsystem::QueueSpawn(
	const FAGX_DistributedSpawnOptions& Options, FAGX_DistributedSpawnCallback Callback)
{
	check(IsInGameThread());
	if (bShutdownRequested)
	{
		UE_LOG(LogAGX, Warning, TEXT("Cannot queue distributed spawn: shutdown has been requested."));
		return FGuid();
	}
	if (Scenario == nullptr)
	{
		UE_LOG(LogAGX, Warning, TEXT("Cannot queue distributed spawn: no scenario is configured."));
		return FGuid();
	}
	if (Options.Asset == nullptr)
	{
		UE_LOG(LogAGX, Warning, TEXT("Cannot queue distributed spawn: no asset was provided."));
		return FGuid();
	}
	if (Options.Asset->AssetPath.IsEmpty())
	{
		UE_LOG(LogAGX, Warning, TEXT("Cannot queue distributed spawn: the asset path is empty."));
		return FGuid();
	}
	if (!FMath::IsFinite(Options.Asset->SimulationAuthoritySensorRadius) ||
		Options.Asset->SimulationAuthoritySensorRadius < 0)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT(
				"Cannot queue distributed spawn: proximity radius must be finite and "
				"nonnegative."));
		return FGuid();
	}
	if (Options.Asset->ActorClass.IsNull())
	{
		UE_LOG(LogAGX, Warning, TEXT("Cannot queue distributed spawn: the asset Actor class is not set."));
		return FGuid();
	}
	if (Options.Transform.ContainsNaN())
	{
		UE_LOG(LogAGX, Warning, TEXT("Cannot queue distributed spawn: the transform contains non-finite values."));
		return FGuid();
	}
	if (!Options.Transform.GetRotation().IsNormalized())
	{
		UE_LOG(LogAGX, Warning, TEXT("Cannot queue distributed spawn: the transform rotation is not normalized."));
		return FGuid();
	}
	if (!Options.Transform.GetScale3D().Equals(FVector::OneVector))
	{
		UE_LOG(LogAGX, Warning, TEXT("Cannot queue distributed spawn: the transform scale must be one."));
		return FGuid();
	}
	if (!FMath::IsFinite(Options.TimeoutSeconds))
	{
		UE_LOG(LogAGX, Warning, TEXT("Cannot queue distributed spawn: the timeout must be finite."));
		return FGuid();
	}
	if (Options.TimeoutSeconds <= 0.0)
	{
		UE_LOG(LogAGX, Warning, TEXT("Cannot queue distributed spawn: the timeout must be greater than zero."));
		return FGuid();
	}

	const FGuid Id = FGuid::NewGuid();
	FAGX_DistributedPendingSpawn Request;
	Request.Options = Options;
	Request.AssetPath = Options.Asset->AssetPath;
	Request.AssetType = Options.Asset->AssetType;
	Request.Result.RequestId = Id;
	Request.Callback = MoveTemp(Callback);
	Request.Deadline = FPlatformTime::Seconds() + Options.TimeoutSeconds;
	SpawnRequests.Add(Id, MoveTemp(Request));
	return Id;
}

bool UAGX_DistributedWorldSubsystem::GetSpawnResult(
	FGuid Id, FAGX_DistributedSpawnResult& Result) const
{
	if (const auto* Request = SpawnRequests.Find(Id))
	{
		Result = Request->Result;
		return true;
	}
	return false;
}

void UAGX_DistributedWorldSubsystem::CompleteSpawn(
	FGuid Id, EAGX_DistributedSpawnState State, const FString& Error)
{
	FAGX_DistributedPendingSpawn* Request = SpawnRequests.Find(Id);
	if (Request == nullptr || Request->Result.State == EAGX_DistributedSpawnState::Succeeded ||
		Request->Result.State == EAGX_DistributedSpawnState::Failed ||
		Request->Result.State == EAGX_DistributedSpawnState::Cancelled)
		return;
	Request->Result.State = State;
	Request->Result.ErrorMessage = Error;
	const auto Result = Request->Result;
	const auto Callback = Request->Callback;
	Request->Callback.Unbind();
	if (State == EAGX_DistributedSpawnState::Failed)
		UE_LOG(LogAGX, Error, TEXT("Distributed spawn failed: %s"), *Error);
	Callback.ExecuteIfBound(Result);
	OnSpawnCompleted.Broadcast(Result);
}

void UAGX_DistributedWorldSubsystem::ReleaseSpawn(FGuid Id)
{
	if (auto* Request = SpawnRequests.Find(Id))
	{
		Request->bReleaseRequested = true;
		CompleteSpawn(Id, EAGX_DistributedSpawnState::Cancelled, TEXT("Spawn request cancelled."));
	}
}

void UAGX_DistributedWorldSubsystem::ApplySimulationAuthority(uint64 Id)
{
	const auto* Entry = EntityActors.Find(Id);
	AActor* Actor = Entry != nullptr ? Entry->Get() : nullptr;
	const auto* Original = OriginalMotionControls.Find(Id);
	if (Actor == nullptr || Original == nullptr)
		return;
	TArray<UAGX_RigidBodyComponent*> Bodies;
	Actor->GetComponents<UAGX_RigidBodyComponent>(Bodies, true);
	Bodies.Sort([](const UAGX_RigidBodyComponent& A, const UAGX_RigidBodyComponent& B)
				{ return A.ImportName < B.ImportName; });
	const bool bSimulating = Client.GetSimulatedEntities().Contains(Id);
	for (int32 I = 0; I < Bodies.Num(); ++I)
	{
		const EAGX_MotionControl Target = bSimulating && Original->IsValidIndex(I)
											  ? (*Original)[I]
											  : EAGX_MotionControl::MC_KINEMATICS;
		Bodies[I]->SetMotionControl(Target);
		if (!bSimulating)
		{
			Bodies[I]->SetVelocity(FVector::ZeroVector);
			Bodies[I]->SetAngularVelocity(FVector::ZeroVector);
		}
	}
}

void UAGX_DistributedWorldSubsystem::ProcessSpawnRequests()
{
	TArray<FGuid> Ids;
	SpawnRequests.GetKeys(Ids);
	int32 Submissions = 0;
	for (FGuid Id : Ids)
	{
		FAGX_DistributedPendingSpawn* Request = SpawnRequests.Find(Id);
		if (Request == nullptr)
			continue;
		const double Now = FPlatformTime::Seconds();
		if (Request->bReleaseRequested)
		{
			if (!Request->bSubmitted)
			{
				SpawnRequests.Remove(Id);
				continue;
			}
			if (Request->bAcknowledged && !Request->bReleaseInFlight &&
				Now >= Request->NextReleaseAttempt)
			{
				Request->bReleaseInFlight = Client.ReleaseInterest(Id, Request->QualifiedKey);
				Request->NextReleaseAttempt = Now + 2.0;
			}
			continue;
		}
		if (Request->Result.State == EAGX_DistributedSpawnState::Succeeded)
			continue;
		if (Now > Request->Deadline)
		{
			CompleteSpawn(
				Id, EAGX_DistributedSpawnState::Failed,
				TEXT(
					"Timed out waiting for connection, asset registration, spawn reply or local "
					"Actor."));
			ReleaseSpawn(Id);
			continue;
		}
		if (!Client.IsReady())
			continue;
		if (!Request->bSubmitted)
		{
			// Waiting registrations consume no submission budget and must not block other assets.
			if (Request->bRegistrationRequested && Request->AssetId == MAX_uint64)
				continue;
			if (Submissions >= FMath::Max(1, Scenario->MaxSpawnRequestsPerTick))
				continue;
			++Submissions;
			if (Request->AssetId == MAX_uint64)
			{
				if (!Request->bRegistrationRequested)
				{
					Request->bRegistrationRequested =
						Client.RegisterAsset(Id, Request->AssetPath, Request->AssetType);
					if (Request->bRegistrationRequested)
						Request->Result.State = EAGX_DistributedSpawnState::Registering;
					else
					{
						CompleteSpawn(
							Id, EAGX_DistributedSpawnState::Failed,
							TEXT("Could not submit asset registration."));
						ReleaseSpawn(Id);
					}
				}
				continue;
			}
			const FString Key =
				Request->Options.ObjectKey.IsEmpty() ? Id.ToString() : Request->Options.ObjectKey;
			Request->QualifiedKey = QualifyObjectKey(Key);
			Request->bSubmitted = Client.EnsureEntity(
				Id, Request->AssetId, Request->Options.Transform, Request->QualifiedKey);
			if (!Request->bSubmitted)
			{
				CompleteSpawn(
					Id, EAGX_DistributedSpawnState::Failed,
					TEXT("Could not submit spawn request."));
				ReleaseSpawn(Id);
			}
			else
				Request->Result.State = EAGX_DistributedSpawnState::Submitted;
			continue;
		}
		if (!Request->bAcknowledged || Request->Result.EntityId < 0)
			continue;
		const uint64 EntityId = static_cast<uint64>(Request->Result.EntityId);
		if (const FString* Error = EntityErrors.Find(EntityId))
		{
			const FString Message = *Error;
			CompleteSpawn(Id, EAGX_DistributedSpawnState::Failed, Message);
			ReleaseSpawn(Id);
			continue;
		}
		auto* Entry = EntityActors.Find(EntityId);
		AActor* Actor = Entry != nullptr ? Entry->Get() : nullptr;
		if (!IsValid(Actor))
			continue;
		Request->Result.Entity = Actor;
		const bool bRequestAuthority = Request->Options.bRequestSimulationAuthority;
		CompleteSpawn(Id, EAGX_DistributedSpawnState::Succeeded);
		if (bRequestAuthority && SpawnRequests.Contains(Id) &&
			!SpawnRequests[Id].bReleaseRequested &&
			!RequestSimulationAuthority(static_cast<int64>(EntityId), true))
			OnSimulationAuthorityCompleted.Broadcast(
				static_cast<int64>(EntityId), true, false,
				TEXT("Could not submit simulation authority request."));
	}
}

void UAGX_DistributedWorldSubsystem::RegisterSpawnBubble(
	UAGX_DistributedSpawnBubbleComponent* Bubble)
{
	SpawnBubbles.Add(Bubble);
}
void UAGX_DistributedWorldSubsystem::UnregisterSpawnBubble(
	UAGX_DistributedSpawnBubbleComponent* Bubble)
{
	SpawnBubbles.Remove(Bubble);
}
bool UAGX_DistributedWorldSubsystem::IsInsideSpawnBubble(
	const FVector& Position, bool bAlreadyActive) const
{
	for (const auto& Entry : SpawnBubbles)
	{
		const auto* Bubble = Entry.Get();
		if (Bubble == nullptr || !Bubble->bEnabled)
			continue;
		const double Radius = FMath::Max(
			0.0, bAlreadyActive ? FMath::Max(Bubble->SpawnRadius, Bubble->ReleaseRadius)
								: Bubble->SpawnRadius);
		if (FVector::DistSquared(Position, Bubble->GetComponentLocation()) <= FMath::Square(Radius))
			return true;
	}
	return false;
}

FString UAGX_DistributedWorldSubsystem::QualifyObjectKey(const FString& Key) const
{
	const FString ScenarioId = Client.GetScenarioId();
	const FString Namespace = Scenario != nullptr ? Scenario->WorldNamespace : FString();
	return FString::Printf(
		TEXT("%d:%s%d:%s%s"), ScenarioId.Len(), *ScenarioId, Namespace.Len(), *Namespace, *Key);
}
AActor* UAGX_DistributedWorldSubsystem::FindWorldObjectActor(const FString& ObjectKey) const
{
	uint64 Id;
	if (!Client.FindWorldObject(QualifyObjectKey(ObjectKey), Id))
		return nullptr;
	const auto* Entry = EntityActors.Find(Id);
	return Entry != nullptr ? Entry->Get() : nullptr;
}
