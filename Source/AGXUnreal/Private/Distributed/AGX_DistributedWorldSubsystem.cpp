// Copyright 2026, Algoryx Simulation AB.

#include "Distributed/AGX_DistributedWorldSubsystem.h"

#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_Simulation.h"
#include "Distributed/AGX_DistributedScenarioAsset.h"
#include "Distributed/AGX_DistributedSettings.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "HAL/PlatformTime.h"

bool UAGX_DistributedWorldSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	const UWorld* World = Cast<UWorld>(Outer);
	return Super::ShouldCreateSubsystem(Outer) && World != nullptr && World->IsGameWorld();
}

void UAGX_DistributedWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	const UAGX_DistributedSettings* Settings = GetDefault<UAGX_DistributedSettings>();
	if (!Settings->Scenario.IsNull())
		Scenario = Settings->Scenario.LoadSynchronous();
	if (!Settings->Scenario.IsNull() && Scenario == nullptr)
		UE_LOG(LogAGX, Error, TEXT("AGX Distributed scenario asset could not be loaded: %s"),
			*Settings->Scenario.ToString());
}

void UAGX_DistributedWorldSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	if (Scenario == nullptr || bInitializationAttempted)
		return;

	bInitializationAttempted = true;
	if (Scenario->bLaunchWorldManager)
		UE_LOG(LogAGX, Warning,
			TEXT("AGX Distributed WorldManager launch is not implemented; connecting to an external manager."));

	UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(&InWorld);
	if (Simulation == nullptr || Simulation->GetNative() == nullptr)
	{
		UE_LOG(LogAGX, Error, TEXT("AGX Distributed needs an AGX simulation to start."));
		return;
	}

	if (Client.Initialize(*Simulation->GetNative(), Scenario->ZenohEndpoint))
	{
		UE_LOG(LogAGX, Log, TEXT("Starting AGX Distributed client; endpoint: %s"),
			Scenario->ZenohEndpoint.IsEmpty() ? TEXT("default discovery") : *Scenario->ZenohEndpoint);
		Client.StartAndLoad();
	}
}

void UAGX_DistributedWorldSubsystem::Deinitialize()
{
	ShutdownClient();
	Scenario = nullptr;
	Super::Deinitialize();
}

void UAGX_DistributedWorldSubsystem::ShutdownClient()
{
	bShutdownRequested = true;
	bSimulationRunning = false;
	Client.Shutdown();
	EntityActors.Reset();
	ReportedUpdateErrors.Reset();
}

void UAGX_DistributedWorldSubsystem::Tick(float DeltaTime)
{
	(void) DeltaTime;
	if (Scenario == nullptr || bShutdownRequested)
		return;

	TArray<FAGXDistributedEvent> Events;
	Client.Poll(Events);
	for (const FAGXDistributedEvent& Event : Events)
		HandleEvent(Event);

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

bool UAGX_DistributedWorldSubsystem::RequestSimulationAuthority(int64 EntityId, bool bSimulate)
{
	return EntityId >= 0 && Client.RequestSimulationAuthority(static_cast<uint64>(EntityId), bSimulate);
}

bool UAGX_DistributedWorldSubsystem::RouteSignal(
	int64 EntityId, const FString& SignalName, double Value)
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
			UE_LOG(LogAGX, Log, TEXT("AGX Distributed joined world as node '%s'."),
				*Event.Message);
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
		case EAGXDistributedEventType::SpawnRequestResult:
			UE_LOG(LogAGX, Log,
				TEXT("AGX Distributed spawn request for '%s': %s (entity %llu). %s"),
				*Event.AssetPath, Event.bAccepted ? TEXT("accepted") : TEXT("rejected"),
				Event.EntityId, *Event.Message);
			break;
		case EAGXDistributedEventType::PossessionResult:
		case EAGXDistributedEventType::SimulationAuthorityResult:
			UE_LOG(LogAGX, Log, TEXT("AGX Distributed %s request for entity %llu: %s. %s"),
				Event.Type == EAGXDistributedEventType::PossessionResult ? TEXT("possession")
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
	}

	UWorld* World = GetWorld();
	if (Scenario == nullptr || World == nullptr)
	{
		UE_LOG(LogAGX, Error,
			TEXT("AGX Distributed cannot spawn entity %llu without a scenario and world."),
			Event.EntityId);
		return;
	}

	const TSoftClassPtr<AActor>* Mapped = nullptr;
	if (Event.AssetId <= static_cast<uint64>(MAX_int64))
		Mapped = Scenario->AssetBlueprintsById.Find(static_cast<int64>(Event.AssetId));
	if (Mapped == nullptr)
		Mapped = Scenario->AssetBlueprints.Find(Event.AssetPath);
	if (Mapped == nullptr || Mapped->IsNull())
	{
		UE_LOG(LogAGX, Error,
			TEXT("AGX Distributed entity %llu cannot spawn asset %llu ('%s'): no Blueprint is mapped."),
			Event.EntityId, Event.AssetId, *Event.AssetPath);
		return;
	}

	UClass* ActorClass = Mapped->LoadSynchronous();
	if (ActorClass == nullptr)
	{
		UE_LOG(LogAGX, Error,
			TEXT("AGX Distributed entity %llu cannot spawn asset %llu: Blueprint '%s' could not be loaded."),
			Event.EntityId, Event.AssetId, *Mapped->ToString());
		return;
	}

	FActorSpawnParameters Params;
	// The distributed scenario determines placement, including overlapping actors.
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AActor* Actor = World->SpawnActor<AActor>(ActorClass, Event.Transform, Params);
	if (Actor == nullptr)
	{
		UE_LOG(LogAGX, Error,
			TEXT("AGX Distributed entity %llu failed to spawn asset %llu as Blueprint '%s'."),
			Event.EntityId, Event.AssetId, *Mapped->ToString());
		return;
	}

	EntityActors.Add(Event.EntityId, Actor);
	ReportedUpdateErrors.Remove(Event.EntityId);

	UE_LOG(LogAGX, Log,
		TEXT("AGX Distributed entity %llu spawned asset %llu ('%s') as actor '%s' at %s."),
		Event.EntityId, Event.AssetId, *Event.AssetPath, *Actor->GetName(),
		*Event.Transform.ToHumanReadableString());

	TArray<UAGX_RigidBodyComponent*> Bodies;
	Actor->GetComponents<UAGX_RigidBodyComponent>(Bodies);
	for (UAGX_RigidBodyComponent* Body : Bodies)
	{
		// This entity is driven by its remote simulation authority. Kinematic motion
		// prevents local integration from fighting incoming distributed transforms.
		Body->SetMotionControl(EAGX_MotionControl::MC_KINEMATICS);
		UE_LOG(LogAGX, Log,
			TEXT("  Name='%s', UUID='%s', ImportUUID='%s'"),
			*Body->GetNative()->GetName(), *Body->GetNative()->GetGuid().ToString(), *Body->ImportGuid.ToString());
	}
}

void UAGX_DistributedWorldSubsystem::processIncomingEntityUpdates(const FAGXDistributedEvent& Event)
{
	check(IsInGameThread());
	// Report persistent model/protocol problems once per entity, not once per simulation step.
	auto ReportError = [this, &Event](const TCHAR* Reason)
	{
		if (!ReportedUpdateErrors.Contains(Event.EntityId))
		{
			ReportedUpdateErrors.Add(Event.EntityId);
			UE_LOG(LogAGX, Error, TEXT("AGX Distributed cannot update entity %llu: %s"),
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
	if (Bodies.Num() != Event.BodyTransforms.Num())
	{
		ReportError(TEXT("Serialized body count does not match the Blueprint rigid body count."));
		return;
	}
	if (Bodies.IsEmpty())
		return;

	auto OrderByName = [](const UAGX_RigidBodyComponent& Lhs, const UAGX_RigidBodyComponent& Rhs)
	{
		return Lhs.ImportName < Rhs.ImportName;
	};

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
		ReportedUpdateErrors.Remove(Event.EntityId);
	}
}
