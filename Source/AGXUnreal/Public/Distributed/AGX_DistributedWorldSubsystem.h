// Copyright 2026, Algoryx Simulation AB.

#pragma once

#include "CoreMinimal.h"
#include "Distributed/DistributedApplicationBarrier.h"
#include "Subsystems/WorldSubsystem.h"

#include "AGX_DistributedWorldSubsystem.generated.h"

class UAGX_DistributedScenarioAsset;
class AActor;

/** World-facing entry point for an AGX Distributed client. */
UCLASS()
class AGXUNREAL_API UAGX_DistributedWorldSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	/** True when this world has no distributed configuration or has received a start command. */
	bool ShouldStepSimulation() const;
	void ShutdownClient();

	UFUNCTION(BlueprintCallable, Category = "AGX Distributed")
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
	void HandleEvent(const FAGXDistributedEvent& Event);
	void HandleSpawnEntityEvent(const FAGXDistributedEvent& Event);
	void processIncomingEntityUpdates(const FAGXDistributedEvent& Event);

	// Actors are owned by the world. Do not keep destroyed entities alive.
	TMap<uint64, TWeakObjectPtr<AActor>> EntityActors;
	TSet<uint64> ReportedUpdateErrors;

	UPROPERTY(Transient)
	TObjectPtr<UAGX_DistributedScenarioAsset> Scenario;

	FDistributedApplicationBarrier Client;
	bool bInitializationAttempted {false};
	bool bSimulationRunning {false};
	bool bShutdownRequested {false};
	double NextStartAttemptTime {0.0};
};
