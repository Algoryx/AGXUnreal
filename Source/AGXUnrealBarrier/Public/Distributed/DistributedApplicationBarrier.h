// Copyright 2026, Algoryx Simulation AB.

#pragma once

#include "CoreMinimal.h"

#include <memory>

class FSimulationBarrier;

enum class EAGXDistributedEventType : uint8
{
	Started,
	StartFailed,
	SpawnEntity,
	SpawnRequestResult,
	PossessionResult,
	SimulationAuthorityResult,
	SimulationStart,
	SimulationPause,
	ShutdownRequested
};

/** A copy of an AGX Distributed event that is safe to pass to the Unreal-facing module. */
struct AGXUNREALBARRIER_API FAGXDistributedEvent
{
	EAGXDistributedEventType Type {EAGXDistributedEventType::StartFailed};
	uint64 EntityId {MAX_uint64};
	uint64 AssetId {MAX_uint64};
	FString AssetPath;
	FTransform Transform;
	FString Message;
	bool bAccepted {false};
};

/** Owns the native AGX Distributed client without exposing AGX headers to AGXUnreal. */
class AGXUNREALBARRIER_API FDistributedApplicationBarrier
{
public:
	FDistributedApplicationBarrier();
	~FDistributedApplicationBarrier();

	FDistributedApplicationBarrier(const FDistributedApplicationBarrier&) = delete;
	FDistributedApplicationBarrier& operator=(const FDistributedApplicationBarrier&) = delete;

	bool Initialize(FSimulationBarrier& Simulation, const FString& ZenohEndpoint);
	void StartAndLoad(int32 TimeoutSeconds = 60);
	void Poll(TArray<FAGXDistributedEvent>& OutEvents);
	bool IsReady() const;
	FString GetNodeId() const;

	bool RequestSpawn(const FString& AssetPath, const FTransform& Transform, bool bPossess);
	bool RequestPossession(uint64 EntityId, bool bPossess);
	bool RequestSimulationAuthority(uint64 EntityId, bool bSimulate);
	bool RouteSignal(uint64 EntityId, const FString& SignalName, double Value);

	void Shutdown();

private:
	struct FImpl;
	std::unique_ptr<FImpl> Impl;
};
