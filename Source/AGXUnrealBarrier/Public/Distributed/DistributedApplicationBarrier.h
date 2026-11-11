// Copyright 2026, Algoryx Simulation AB.

#pragma once

#include "CoreMinimal.h"
#include "RigidBodyBarrier.h"

#include <memory>

class FSimulationBarrier;

enum class EAGXDistributedEventType : uint8
{
	Started,
	RemoveEntity,
	ReleaseInterestResult,
	StartFailed,
	SpawnEntity,
	EntityUpdate,
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
	FGuid RequestId;
	bool bRequestedValue {false};
	FTransform Transform;
	/** World-space body transforms in AssemblySerializer order, not component discovery order. */
	TArray<FTransform> BodyTransforms;
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
	/** Must be called on the game thread; AGX attaches its simulation listener synchronously. */
	void StartAndLoad(int32 TimeoutSeconds = 60);
	void Poll(TArray<FAGXDistributedEvent>& OutEvents);
	bool IsReady() const;
	FString GetNodeId() const;

	bool ResolveAsset(const FString& Path, uint64& OutId) const;
	FString GetScenarioId() const;
	bool FindWorldObject(const FString& ObjectKey, uint64& OutEntityId) const;
	bool EnsureEntity(
		const FGuid& RequestId, uint64 AssetId, const FTransform& Transform,
		const FString& ObjectKey);
	bool ReleaseInterest(const FGuid& RequestId, const FString& ObjectKey);
	bool RequestSpawn(const FString& AssetPath, const FTransform& Transform, bool bPossess);
	bool RequestPossession(uint64 EntityId, bool bPossess);
	bool RequestSimulationAuthority(uint64 EntityId, bool bSimulate);
	bool RouteSignal(uint64 EntityId, const FString& SignalName, double Value);

	/** Copy of the native simulation-authority set; no native lock escapes this call. */
	TArray<uint64> GetSimulatedEntities() const;
	/** Publish complete body lists in AssemblySerializer order. Called on the game thread. */
	void SendEntityUpdates(const TMap<uint64, TArray<FRigidBodyBarrier>>& EntityBodies);

	void Shutdown();

private:
	struct FImpl;
	std::unique_ptr<FImpl> Impl;
};
