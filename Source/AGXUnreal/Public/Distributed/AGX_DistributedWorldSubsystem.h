// Copyright 2026, Algoryx Simulation AB.

#pragma once

#include "CoreMinimal.h"
#include "AGX_RigidBodyComponent.h"
#include "Distributed/AGX_DistributedSpawnRequest.h"
#include "Distributed/DistributedApplicationBarrier.h"
#include "Subsystems/WorldSubsystem.h"

#include "AGX_DistributedWorldSubsystem.generated.h"

class UAGX_DistributedScenarioAsset;

USTRUCT()
struct FAGX_DistributedPendingSpawn
{
	GENERATED_BODY()
	UPROPERTY() FAGX_DistributedSpawnOptions Options;
	UPROPERTY() FAGX_DistributedSpawnResult Result;
	FAGX_DistributedSpawnCallback Callback;
	FString QualifiedKey;
	double Deadline {0.0};
	double NextReleaseAttempt {0.0};
	bool bSubmitted {false};
	bool bAcknowledged {false};
	bool bReleaseRequested {false};
	bool bReleaseInFlight {false};
};

class UAGX_Simulation;
class AActor;
class UAGX_DistributedSpawnBubbleComponent;

/** World-facing entry point for an AGX Distributed client. */
UCLASS()
class AGXUNREAL_API UAGX_DistributedWorldSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "AGX Distributed")
	AActor* FindWorldObjectActor(const FString& ObjectKey) const;
	void RegisterSpawnBubble(UAGX_DistributedSpawnBubbleComponent* Bubble);
	void UnregisterSpawnBubble(UAGX_DistributedSpawnBubbleComponent* Bubble);
	bool IsInsideSpawnBubble(const FVector& Position, bool bAlreadyActive) const;
	/** Accepts requests before connection. Completion is delivered on a subsequent game-thread
	 * tick. */
	FGuid QueueSpawn(
		const FAGX_DistributedSpawnOptions& Options,
		FAGX_DistributedSpawnCallback Callback = FAGX_DistributedSpawnCallback());
	UFUNCTION(BlueprintCallable, Category = "AGX Distributed")
	FGuid QueueEntitySpawn(const FAGX_DistributedSpawnOptions& Options);
	/** Cancel a pending request or release this caller's interest in a completed spawn. */
	UFUNCTION(BlueprintCallable, Category = "AGX Distributed")
	void ReleaseSpawn(FGuid RequestId);
	UFUNCTION(BlueprintPure, Category = "AGX Distributed")
	bool GetSpawnResult(FGuid RequestId, FAGX_DistributedSpawnResult& Result) const;
	UPROPERTY(BlueprintAssignable, Category = "AGX Distributed")
	FAGX_DistributedSpawnCompleted OnSpawnCompleted;
	UPROPERTY(BlueprintAssignable, Category = "AGX Distributed")
	FAGX_DistributedAuthorityCompleted OnSimulationAuthorityCompleted;

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	/** True when this world has no distributed configuration or has received a start command. */
	bool ShouldStepSimulation() const;
	void ShutdownClient();

	UFUNCTION(
		BlueprintCallable, Category = "AGX Distributed",
		meta =
			(DeprecatedFunction,
			 DeprecationMessage =
				 "Use Spawn Distributed Entity for queued spawning and Actor-ready completion."))
	bool RequestSpawn(const FString& AssetPath, FTransform Transform, bool bPossess = true);

	UFUNCTION(BlueprintCallable, Category = "AGX Distributed")
	bool RequestPossession(int64 EntityId, bool bPossess);

	UFUNCTION(BlueprintCallable, Category = "AGX Distributed")
	bool RequestSimulationAuthority(int64 EntityId, bool bSimulate);

	UFUNCTION(BlueprintCallable, Category = "AGX Distributed")
	bool RouteSignal(int64 EntityId, const FString& SignalName, double Value);

	UFUNCTION(BlueprintPure, Category = "AGX Distributed")
	bool IsConnected() const;

private:
	FString QualifyObjectKey(const FString& Key) const;
	void ProcessSpawnRequests();
	void CompleteSpawn(
		FGuid RequestId, EAGX_DistributedSpawnState State, const FString& Error = FString());
	void ApplySimulationAuthority(uint64 EntityId);
	UPROPERTY(Transient) TMap<FGuid, FAGX_DistributedPendingSpawn> SpawnRequests;
	TSet<TWeakObjectPtr<UAGX_DistributedSpawnBubbleComponent>> SpawnBubbles;
	TSet<uint64> RemovedEntityIds;
	TArray<FAGXDistributedEvent> DeferredSpawnEvents;
	TMap<uint64, FAGXDistributedEvent> DeferredEntityUpdates;
	TMap<uint64, FString> EntityErrors;
	TMap<uint64, TArray<EAGX_MotionControl>> OriginalMotionControls;
	void CollectAndSendEntityUpdates(double TimeStamp);
	void HandleEvent(const FAGXDistributedEvent& Event);
	void HandleSpawnEntityEvent(const FAGXDistributedEvent& Event);
	void processIncomingEntityUpdates(const FAGXDistributedEvent& Event);

	// Actors are owned by the world. Do not keep destroyed entities alive.
	TMap<uint64, TWeakObjectPtr<AActor>> EntityActors;
	TSet<uint64> ReportedUpdateErrors;
	TSet<uint64> ReportedSendErrors;
	TWeakObjectPtr<UAGX_Simulation> BoundSimulation;
	FDelegateHandle PostStepHandle;

	UPROPERTY(Transient)
	TObjectPtr<UAGX_DistributedScenarioAsset> Scenario;

	FDistributedApplicationBarrier Client;
	bool bInitializationAttempted {false};
	bool bSimulationRunning {false};
	bool bShutdownRequested {false};
	double NextStartAttemptTime {0.0};
};
