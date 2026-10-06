// Copyright 2026, Algoryx Simulation AB.

#include "Distributed/AGX_DistributedWorldSubsystem.h"

#include "AGX_LogCategory.h"
#include "AGX_Simulation.h"
#include "Distributed/AGX_DistributedScenarioAsset.h"
#include "Distributed/AGX_DistributedSettings.h"
#include "Engine/World.h"
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
		{
			const TSoftClassPtr<AActor>* Mapped = nullptr;
			if (Event.AssetId <= static_cast<uint64>(MAX_int64))
				Mapped = Scenario->AssetBlueprintsById.Find(static_cast<int64>(Event.AssetId));
			if (Mapped == nullptr)
				Mapped = Scenario->AssetBlueprints.Find(Event.AssetPath);
			// Q: Should a path mapping override the numeric asset ID mapping when both exist?
			if (Mapped == nullptr || Mapped->IsNull())
			{
				UE_LOG(LogAGX, Error,
					TEXT("AGX Distributed entity %llu should spawn asset %llu ('%s'), but no Blueprint is mapped."),
					Event.EntityId, Event.AssetId, *Event.AssetPath);
			}
			else
			{
				// Q: When should the mapped Blueprint be loaded and spawned, and how should
				// its imported rigid bodies be registered before state updates are applied?
				UE_LOG(LogAGX, Log,
					TEXT("AGX Distributed entity %llu should spawn asset %llu ('%s') as Blueprint '%s' at %s."),
					Event.EntityId, Event.AssetId, *Event.AssetPath, *Mapped->ToString(),
					*Event.Transform.ToHumanReadableString());
			}
			break;
		}
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
