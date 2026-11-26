// Copyright 2026, Algoryx Simulation AB.

#pragma once

#include "CoreMinimal.h"
#include "Distributed/AGX_DistributedEnums.h"
#include "RigidBodyBarrier.h"

#include <memory>
#include <variant>

class FSimulationBarrier;
struct FOpenPLX_Input;

/** Signal storage: routing takes Unreal conventions; received events contain OpenPLX
 * conventions until ConvertIncomingSignal converts them using their resolved input type. */
using FAGXDistributedSignalValue = std::variant<double, int64, bool, FVector2D, FVector>;

enum class EAGXDistributedEventType : uint8
{
	Started,
	RemoveEntity,
	ReleaseInterestResult,
	StartFailed,
	SpawnEntity,
	EntityUpdate,
	SpawnRequestResult,
	AssetRegistrationResult,
	PossessionResult,
	SimulationAuthorityResult,
	SimulationAuthorityChanged,
	StaleEntityState,
	SimulationStart,
	SimulationPause,
	ShutdownRequested,
	ControlSignal
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
	/** World-space velocities in Unreal units (cm/s and degrees/s). */
	TArray<FVector> BodyVelocities;
	TArray<FVector> BodyAngularVelocities;
	bool bAssigned {false};
	bool bWasSimulating {false};
	FString Message;
	bool bAccepted {false};
	FString SignalName;

	/** OpenPLX units and coordinates, stored without conversion in Unreal scalar/vector types. */
	FAGXDistributedSignalValue SignalValue {0.0};
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
	/** Start asynchronous connection and world loading from the game thread. */
	void StartAndLoad(int32 TimeoutSeconds = 60);
	void Poll(TArray<FAGXDistributedEvent>& OutEvents);
	bool IsReady() const;
	FString GetNodeId() const;

	bool ResolveAsset(const FString& Path, uint64& OutId) const;
	/** Resolve or register an asset asynchronously. Matching in-flight requests share one query.
	 * AssetRegistrationResult is delivered by Poll, including for locally known assets. */
	bool RegisterAsset(const FGuid& RequestId, const FString& Path, EAGX_DistributedAssetType Type);
	FString GetScenarioId() const;
	bool FindWorldObject(const FString& ObjectKey, uint64& OutEntityId) const;
	bool EnsureEntity(
		const FGuid& RequestId, uint64 AssetId, const FTransform& Transform,
		const FString& ObjectKey);
	bool ReleaseInterest(const FGuid& RequestId, const FString& ObjectKey);
	bool RequestSpawn(const FString& AssetPath, const FTransform& Transform, bool bPossess);
	bool RequestPossession(uint64 EntityId, bool bPossess, const FGuid& RequestId = FGuid());
	/** True if this client currently possesses the entity. */
	bool IsPossessed(uint64 EntityId) const;
	bool RequestSimulationAuthority(
		uint64 EntityId, bool bSimulate, const FGuid& RequestId = FGuid());

	/** Convert an Unreal input to OpenPLX conventions before caching and routing it. */
	bool RouteSignal(
		uint64 EntityId, const FOpenPLX_Input& Input, const FAGXDistributedSignalValue& Value);

	/** Convert an incoming OpenPLX control value to Unreal conventions using its resolved input. */
	static TOptional<FAGXDistributedSignalValue> ConvertIncomingSignal(
		const FOpenPLX_Input& Input, const FAGXDistributedSignalValue& Value);

	/** Copy of the native simulation-authority set; no native lock escapes this call. */
	TArray<uint64> GetSimulatedEntities() const;

	/** Register bodies for final-state serialization and a proximity sensor.
	 * ProximityCenter is the initial world-space center [cm]; ProximityRadius is the radius [cm].
	 * The sensor follows the heaviest body. */
	void RegisterEntityBodies(
		uint64 EntityId, const TArray<FRigidBodyBarrier>& Bodies, const FVector& ProximityCenter,
		double ProximityRadius = 600.0);
	/** Queue local sensor/body-reference cleanup. Does not despawn the distributed entity. */
	void UnregisterEntityBodies(uint64 EntityId);
	/** Publish complete body lists in AssemblySerializer order. Called on the game thread. */
	void SendEntityUpdates(const TMap<uint64, TArray<FRigidBodyBarrier>>& EntityBodies);

	void Shutdown();

private:
	struct FImpl;
	std::unique_ptr<FImpl> Impl;
};
