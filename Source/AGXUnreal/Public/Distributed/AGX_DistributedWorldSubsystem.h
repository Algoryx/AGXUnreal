// Copyright 2026, Algoryx Simulation AB.

#pragma once

#include "CoreMinimal.h"
#include "AGX_RigidBodyComponent.h"
#include "Distributed/AGX_DistributedEnums.h"
#include "Distributed/AGX_DistributedSpawnRequest.h"
#include "Distributed/DistributedApplicationBarrier.h"
#include "Engine/EngineTypes.h"
#include "Subsystems/WorldSubsystem.h"

// Standard library includes.
#include <memory>

#include "AGX_DistributedWorldSubsystem.generated.h"

class FOpenPLX_SignalRerouter;
class UAGX_DistributedScenarioAsset;
class UOpenPLX_SignalHandlerComponent;

USTRUCT()
struct FAGX_DistributedPendingSpawn
{
	GENERATED_BODY()
	UPROPERTY() FAGX_DistributedSpawnOptions Options;
	UPROPERTY() FAGX_DistributedSpawnResult Result;
	FAGX_DistributedSpawnCallback Callback;
	FString QualifiedKey;
	FString AssetPath;
	EAGX_DistributedAssetType AssetType {EAGX_DistributedAssetType::Possessable};
	uint64 AssetId {MAX_uint64};
	bool bRegistrationRequested {false};
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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(
	FAGX_DistributedPossessionCompleted, FGuid, RequestId, int64, EntityId, bool, bPossess, bool,
	bAccepted, const FString&, ErrorMessage);
DECLARE_DELEGATE_ThreeParams(FAGX_DistributedPossessionCallback, bool, AActor*, const FString&);
DECLARE_DELEGATE_ThreeParams(
	FAGX_DistributedSimulationAuthorityCallback, bool, AActor*, const FString&);

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
	UPROPERTY(BlueprintAssignable, Category = "AGX Distributed")
	FAGX_DistributedPossessionCompleted OnPossessionCompleted;

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

	/** Submit a request. OnPossessionCompleted reports acceptance; simulation authority is
	 * unchanged. */
	UFUNCTION(BlueprintCallable, Category = "AGX Distributed")
	bool RequestPossession(int64 EntityId, bool bPossess);

	/**
	 * Request Actor possession with one completion callback. Returns an invalid ID if submission
	 * fails. Completion is delivered on a subsequent world tick, or when the client shuts down.
	 */
	FGuid RequestActorPossession(
		AActor* Entity, bool bPossess, FAGX_DistributedPossessionCallback Callback);

	/** Find the entity corresponding to a registered Actor in this world. Failure returns -1. */
	bool GetEntityIdForActor(const AActor* Actor, int64& EntityId) const;

	/** True if this client currently possesses the entity. */
	bool IsPossessed(int64 EntityId) const;

	/**
	 * Query this client's input possession of a registered Actor.
	 * Returns false if the client is unavailable or the Actor is invalid or unregistered.
	 * bHasPossession is false on failure.
	 */
	UFUNCTION(BlueprintPure, Category = "AGX Distributed")
	bool TryGetActorPossession(const AActor* Actor, bool& bHasPossession) const;

	/**
	 * Query this client's simulation authority over a registered Actor.
	 * Returns false if the client is unavailable or the Actor is invalid or unregistered.
	 * bHasSimulationAuthority is false on failure.
	 */
	UFUNCTION(BlueprintPure, Category = "AGX Distributed")
	bool TryGetActorSimulationAuthority(const AActor* Actor, bool& bHasSimulationAuthority) const;

	UFUNCTION(BlueprintCallable, Category = "AGX Distributed")
	bool RequestSimulationAuthority(int64 EntityId, bool bSimulate);

	/**
	 * Request Actor simulation authority with one completion callback, without changing possession.
	 * Returns an invalid ID if submission fails. Completion is delivered on a subsequent world
	 * tick, or when the client shuts down.
	 */
	FGuid RequestActorSimulationAuthority(
		AActor* Entity, bool bSimulate, FAGX_DistributedSimulationAuthorityCallback Callback);

	UFUNCTION(BlueprintCallable, Category = "AGX Distributed")
	bool RouteSignal(int64 EntityId, const FString& SignalName, double Value);

	/** C++ routing entry point for all OpenPLX payloads. Blueprint callers use the signal handler.
	 */
	bool RouteSignalValue(
		int64 EntityId, const FString& SignalName, const FAGXDistributedSignalValue& Value);

	UFUNCTION(BlueprintPure, Category = "AGX Distributed")
	bool IsConnected() const;

private:
	struct FPendingPossession
	{
		TWeakObjectPtr<AActor> Entity;
		FAGX_DistributedPossessionCallback Callback;
	};
	struct FPendingSimulationAuthority
	{
		TWeakObjectPtr<AActor> Entity;
		FAGX_DistributedSimulationAuthorityCallback Callback;
	};

	FString QualifyObjectKey(const FString& Key) const;
	void ProcessSpawnRequests();
	void CompleteSpawn(
		FGuid RequestId, EAGX_DistributedSpawnState State, const FString& Error = FString());
	void ApplySimulationAuthority(uint64 EntityId);
	void CompletePossession(const FAGXDistributedEvent& Event);
	void CompleteSimulationAuthority(const FAGXDistributedEvent& Event);
	TMap<FGuid, FPendingPossession> PossessionRequests;
	TMap<FGuid, FPendingSimulationAuthority> SimulationAuthorityRequests;
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
	void ApplyControlSignals(
		uint64 EntityId, const TMap<FName, FAGXDistributedSignalValue>& Signals);
	AActor* FindSignalHandlerEntity(const UOpenPLX_SignalHandlerComponent& Component) const;
	void OnSignalHandlerInitializing(UOpenPLX_SignalHandlerComponent& Component);

	UFUNCTION()
	void OnEntityEndPlay(AActor* Actor, EEndPlayReason::Type Reason);
	void processIncomingEntityUpdates(const FAGXDistributedEvent& Event);

	// Actors are owned by the world. Do not keep destroyed entities alive.
	TMap<uint64, TWeakObjectPtr<AActor>> EntityActors;
	/** Latest Real input values retained while their entity's Actor is waiting to spawn. */
	TMap<uint64, TMap<FName, FAGXDistributedSignalValue>> DeferredControlSignals;
	TMap<TWeakObjectPtr<AActor>, std::shared_ptr<FOpenPLX_SignalRerouter>> ActorSignalRerouters;
	FDelegateHandle SignalHandlerInitializingHandle;
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
