// Copyright 2026, Algoryx Simulation AB.

#include "Distributed/DistributedApplicationBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "BarrierOnly/AGXRefs.h"
#include "BarrierOnly/AGXTypeConversions.h"
#include "SimulationBarrier.h"
#include "Utilities/TestUtilities.h"

// Unreal Engine includes.
#include "HAL/PlatformProcess.h"
#include "Misc/ScopeExit.h"

// Standard library includes.
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>
#include <future>
#include <limits>
#include <mutex>
#include <stdexcept>
#include <utility>
#include <vector>

// AGX Dynamics includes.
#include <BeginAGXIncludes.h>
#include <agx/AffineMatrix4x4.h>
#include <agx/Quat.h>
#include <agx/RigidBody.h>
#include <agx/Vec3.h>
#include <agxDistributed/Client.h>
#include <agxSDK/Simulation.h>
#include <agxUtil/agxUtil.h>
#if WITH_DEV_AUTOMATION_TESTS
#include <agxDistributed/WorldManager.h>
#endif
#include <EndAGXIncludes.h>

namespace
{
	constexpr size_t BodyStateHeaderSize = 8 + 3 * 8;
	constexpr size_t BodyStateSize = 3 * 2 + 4 * 2 + 6 * 4;
	constexpr double PositionScale = 1000.0;
	constexpr double RotationScale = 32767.0;
	constexpr double MaxPositionDelta = 32.767;
	constexpr uint16 PositionEscape = 0x8000;

	// Matches examples/agxDistributed/AssemblySerializer.h: uint64 count, three double
	// reference coordinates, then per body: three int16 millimetre deltas, four int16
	// quaternion components and six float velocities, all big-endian. Three 0x8000
	// deltas instead precede three double absolute coordinates for distant bodies.
	bool DecodeBodyTransforms(
		const std::vector<uint8_t>& Data, TArray<FTransform>& OutTransforms,
		TArray<FVector>* OutVelocities = nullptr, TArray<FVector>* OutAngularVelocities = nullptr)
	{
		if (Data.size() < BodyStateHeaderSize)
			return false;

		size_t Offset = 0;
		auto ReadUint64 = [&Data, &Offset]()
		{
			uint64 Value = 0;
			for (int32 Byte = 0; Byte < 8; ++Byte)
				Value = (Value << 8) | Data[Offset++];
			return Value;
		};
		auto ReadDouble = [&ReadUint64]()
		{
			const uint64 Bits = ReadUint64();
			double Value;
			static_assert(sizeof(Value) == sizeof(Bits), "Expected 64-bit doubles");
			std::memcpy(&Value, &Bits, sizeof(Value));
			return Value;
		};
		auto ReadUint16 = [&Data, &Offset]()
		{
			const uint16 Value = static_cast<uint16>(Data[Offset]) << 8 | Data[Offset + 1];
			Offset += 2;
			return Value;
		};
		auto SignedDelta = [](uint16 Value)
		{
			return Value <= MAX_int16 ? static_cast<int32>(Value)
									  : static_cast<int32>(Value) - 65536;
		};

		const uint64 Count = ReadUint64();
		// Bound allocation by the minimum record size without overflowing Count * BodyStateSize.
		if (Count > MAX_int32 || Count > (Data.size() - BodyStateHeaderSize) / BodyStateSize)
			return false;

		agx::Vec3 ReferencePosition;
		for (int32 Axis = 0; Axis < 3; ++Axis)
			ReferencePosition[Axis] = ReadDouble();
		if (!ReferencePosition.isFinite())
			return false;

		OutTransforms.Reserve(static_cast<int32>(Count));
		for (uint64 Body = 0; Body < Count; ++Body)
		{
			if (Data.size() - Offset < BodyStateSize)
				return false;
			uint16 Delta[3];
			for (int32 Axis = 0; Axis < 3; ++Axis)
				Delta[Axis] = ReadUint16();
			agx::Vec3 Position;
			if (Delta[0] == PositionEscape && Delta[1] == PositionEscape &&
				Delta[2] == PositionEscape)
			{
				if (Data.size() - Offset < BodyStateSize - 3 * 2 + 3 * 8)
					return false;
				for (int32 Axis = 0; Axis < 3; ++Axis)
					Position[Axis] = ReadDouble();
			}
			else
			{
				for (int32 Axis = 0; Axis < 3; ++Axis)
				{
					if (Delta[Axis] == PositionEscape)
						return false;
					Position[Axis] =
						ReferencePosition[Axis] + SignedDelta(Delta[Axis]) / PositionScale;
				}
			}
			if (!Position.isFinite())
				return false;
			agx::Quat Rotation;
			for (int32 Component = 0; Component < 4; ++Component)
				Rotation[Component] = SignedDelta(ReadUint16()) / RotationScale;
			const double RotationLength = Rotation.normalize();
			if (!FMath::IsFinite(RotationLength) ||
				RotationLength <= std::numeric_limits<double>::epsilon())
				return false;
			const FTransform Transform = Convert(agx::AffineMatrix4x4(Rotation, Position));
			if (Transform.ContainsNaN() || !Transform.GetRotation().IsNormalized())
				return false;
			OutTransforms.Add(Transform);
			auto ReadVelocity = [&Data, &Offset]()
			{
				agx::Vec3 Value;
				for (int32 Axis = 0; Axis < 3; ++Axis)
				{
					uint32 Bits = 0;
					for (int32 Byte = 0; Byte < 4; ++Byte)
						Bits = (Bits << 8) | Data[Offset++];
					float Component;
					std::memcpy(&Component, &Bits, sizeof(Component));
					Value[Axis] = Component;
				}
				return Value;
			};
			const agx::Vec3 Velocity = ReadVelocity();
			const agx::Vec3 AngularVelocity = ReadVelocity();
			if (!Velocity.isFinite() || !AngularVelocity.isFinite())
				return false;
			if (OutVelocities != nullptr)
				OutVelocities->Add(ConvertDisplacement(Velocity));
			if (OutAngularVelocities != nullptr)
				OutAngularVelocities->Add(ConvertAngularVelocity(AngularVelocity));
		}
		return Offset == Data.size();
	}

	// Inverse of DecodeBodyTransforms. Callers supply bodies in AssemblySerializer's name order.
	// Read native state directly to preserve AGX units and handedness.
	bool EncodeBodyTransforms(
		const TArray<FRigidBodyBarrier>& Bodies, std::vector<uint8_t>& OutData)
	{
		for (const FRigidBodyBarrier& Body : Bodies)
		{
			if (!Body.HasNative())
				return false;
		}
		OutData.clear();
		OutData.reserve(BodyStateHeaderSize + static_cast<size_t>(Bodies.Num()) * BodyStateSize);
		auto WriteUint16 = [&OutData](uint16 Value)
		{
			OutData.push_back(static_cast<uint8_t>(Value >> 8));
			OutData.push_back(static_cast<uint8_t>(Value & 0xff));
		};
		auto WriteUint64 = [&OutData](uint64 Value)
		{
			for (int32 Shift = 56; Shift >= 0; Shift -= 8)
				OutData.push_back(static_cast<uint8_t>((Value >> Shift) & 0xff));
		};
		auto WritePosition = [&WriteUint64](const agx::Vec3& Position)
		{
			for (int32 Axis = 0; Axis < 3; ++Axis)
			{
				const double Value = Position[Axis];
				uint64 Bits;
				static_assert(sizeof(Value) == sizeof(Bits), "Expected 64-bit doubles");
				std::memcpy(&Bits, &Value, sizeof(Bits));
				WriteUint64(Bits);
			}
		};
		WriteUint64(static_cast<uint64>(Bodies.Num()));
		const agx::Vec3 ReferencePosition =
			Bodies.IsEmpty() ? agx::Vec3() : Bodies[0].GetNative()->Native->getPosition();
		if (!ReferencePosition.isFinite())
			return false;
		WritePosition(ReferencePosition);
		for (const FRigidBodyBarrier& Body : Bodies)
		{
			const agx::Vec3 Position = Body.GetNative()->Native->getPosition();
			if (!Position.isFinite())
				return false;
			const agx::Vec3 Delta = Position - ReferencePosition;
			bool bFitsInInt16 = true;
			for (int32 Axis = 0; Axis < 3; ++Axis)
				bFitsInInt16 &= FMath::IsFinite(Delta[Axis]) && Delta[Axis] >= -MaxPositionDelta &&
								Delta[Axis] <= MaxPositionDelta;
			if (bFitsInInt16)
			{
				for (int32 Axis = 0; Axis < 3; ++Axis)
				{
					const int16 Millimetres = static_cast<int16>(std::lround(Delta[Axis] * PositionScale));
					WriteUint16(static_cast<uint16>(Millimetres));
				}
			}
			else
			{
				for (int32 Axis = 0; Axis < 3; ++Axis)
					WriteUint16(PositionEscape);
				WritePosition(Position);
			}
			const agx::Quat Rotation = Body.GetNative()->Native->getRotation();
			for (int32 Component = 0; Component < 4; ++Component)
			{
				if (!FMath::IsFinite(Rotation[Component]))
					return false;
				const double Value =
					std::clamp(static_cast<double>(Rotation[Component]), -1.0, 1.0);
				const int16 Quantized = static_cast<int16>(std::lround(Value * RotationScale));
				WriteUint16(static_cast<uint16>(Quantized));
			}
			auto WriteVelocity = [&OutData](const agx::Vec3& Value)
			{
				for (int32 Axis = 0; Axis < 3; ++Axis)
				{
					const float Component = static_cast<float>(Value[Axis]);
					if (!FMath::IsFinite(Component))
						return false;
					uint32 Bits;
					std::memcpy(&Bits, &Component, sizeof(Bits));
					for (int32 Shift = 24; Shift >= 0; Shift -= 8)
						OutData.push_back(static_cast<uint8_t>(Bits >> Shift));
				}
				return true;
			};
			if (!WriteVelocity(Body.GetNative()->Native->getVelocity()) ||
				!WriteVelocity(Body.GetNative()->Native->getAngularVelocity()))
				return false;
		}
		return true;
	}

	struct FEventQueue
	{
		void Push(FAGXDistributedEvent Event)
		{
			std::lock_guard<std::mutex> Lock(Mutex);
			Events.emplace_back(std::move(Event));
		}

		void Drain(TArray<FAGXDistributedEvent>& OutEvents)
		{
			std::vector<FAGXDistributedEvent> Ready;
			{
				std::lock_guard<std::mutex> Lock(Mutex);
				Ready.swap(Events);
			}
			for (FAGXDistributedEvent& Event : Ready)
				OutEvents.Add(MoveTemp(Event));
		}

		std::mutex Mutex;
		std::vector<FAGXDistributedEvent> Events;
	};

	class FUnrealApplicationLayer final : public agxDistributed::Client
	{
	public:
		FUnrealApplicationLayer(
			agxSDK::Simulation& Simulation, const agxDistributed::Node::Config& Config,
			std::shared_ptr<FEventQueue> InEvents)
			: Client(Config)
			, Simulation(Simulation)
			, Events(std::move(InEvents))
		{
		}

		~FUnrealApplicationLayer() override
		{
			closeSession();
		}

		// Unreal owns body creation, safe points and motion control. Use Client rather than
		// AgxApplicationLayer, whose synchronizer requires native entity assemblies.
		std::future<void> startAndLoad(std::string Identity, size_t Timeout)
		{
			AsyncRefPin Pin(this);
			return std::async(
				std::launch::async,
				[this, Identity = std::move(Identity), Timeout, Pin = std::move(Pin)]() mutable
				{
					start(Timeout).get();
					requestMembership(std::move(Identity)).get();
					loadWorld();
					registerControlCallback(
						[this](std::string Key, std::string Action)
						{
							ON_SCOPE_EXIT
							{
								agxUtil::freeContainerMemory(Key);
								agxUtil::freeContainerMemory(Action);
							};

							if (Key == "control/simulation")
							{
								if (Action == "START")
									onSimulationControlMessage(true);
								else if (Action == "STOP" || Action == "PAUSE")
									onSimulationControlMessage(false);
							}
							else if (Key == "control/shutdown")
								onShutdownControlMessage();
						});
					queryControlMessages();
				});
		}

		void shutdownApplicationLayer()
		{
			closeSession();
		}

		void RegisterEntityBodies(uint64 Id, const TArray<FRigidBodyBarrier>& Bodies)
		{
			RegisteredBodies.Add(Id, Bodies);
		}

		void PollEvents(TArray<FAGXDistributedEvent>& OutEvents)
		{
			setLocalSimulationTime(Simulation.getTimeStamp());
			checkForStaleEntityData();
			TArray<FAGXDistributedEvent> Ready;
			Events->Drain(Ready);
			for (const FAGXDistributedEvent& Event : Ready)
			{
				if (Event.Type == EAGXDistributedEventType::SimulationAuthorityChanged &&
					Event.bWasSimulating)
					requestSerializedState(
						agxDistributed::EntityId(Event.EntityId), Event.bAssigned);
				if (Event.Type == EAGXDistributedEventType::SimulationAuthorityChanged &&
					isPossessed(agxDistributed::EntityId(Event.EntityId)))
					if (const auto* Signals = LocalSignals.Find(Event.EntityId))
						for (const auto& Signal : *Signals)
							routeSignal(
								agxDistributed::EntityId(Event.EntityId),
								TCHAR_TO_UTF8(*Signal.Key), Signal.Value);
				if (Event.Type == EAGXDistributedEventType::RemoveEntity)
				{
					RegisteredBodies.Remove(Event.EntityId);
					LocalSignals.Remove(Event.EntityId);
				}
			}
			OutEvents.Append(MoveTemp(Ready));
		}

		void routeSignal(agxDistributed::EntityId Id, const std::string& Name, double Value)
		{
			if (!isPossessed(Id))
				return;
			LocalSignals.FindOrAdd(Id.getId()).Add(UTF8_TO_TCHAR(Name.c_str()), Value);
			if (isSimulated(Id))
				applySignal(Id, Name, Value);
			else
			{
				zenoh::ext::Serializer Writer;
				Writer.serialize(Name);
				Writer.serialize(Value);
				getSession()->put(
					"world/entities/" + std::to_string(Id.getId()) + "/signal",
					std::move(Writer).finish());
			}
		}

		TArray<uint64> GetSimulatedEntities()
		{
			TArray<uint64> Result;
			auto Entities = getSimulatedEntities();
			for (agxDistributed::EntityId Id : Entities.m_entityIds)
				Result.Add(Id.getId());
			return Result;
		}

		void SendEntityUpdates(const TMap<uint64, TArray<FRigidBodyBarrier>>& EntityBodies)
		{
			setLocalSimulationTime(Simulation.getTimeStamp());
			for (const auto& Entry : EntityBodies)
			{
				RegisterEntityBodies(Entry.Key, Entry.Value);
				if (isSimulated(agxDistributed::EntityId(Entry.Key)))
					requestSerializedState(agxDistributed::EntityId(Entry.Key), true);
			}
		}

		void applySignal(agxDistributed::EntityId EntityId, const std::string& Name, double Value)
		{
			// Q: Should signals be queued for the Unreal OpenPLX handler or applied through
			// a native handler during this AGX simulation callback?
			(void) EntityId;
			(void) Name;
			(void) Value;
			UE_LOG(
				LogAGX, Warning,
				TEXT("Unreal Application Layer received signal %s for entity %llu: %f"),
				UTF8_TO_TCHAR(Name.c_str()), EntityId.getId(), Value);
		}

	protected:
		bool requestSerializedState(agxDistributed::EntityId Id, bool bAssigned) override
		{
			check(IsInGameThread());
			const auto* Bodies = RegisteredBodies.Find(Id.getId());
			if (Bodies == nullptr || !withEntity(Id, [](const agxDistributed::Entity&) {}))
				return false;
			try
			{
				agxDistributed::EntityData Update;
				Update.getTransform() = agx::AffineMatrix4x4();
				Update.getFlags() = bAssigned ? agxDistributed::EntityData::ASSIGNED_ENTITY : 0;
				if (!EncodeBodyTransforms(*Bodies, Update.getAssetData()))
					return false;
				return updateEntity(Id, std::move(Update));
			}
			catch (const std::exception& Error)
			{
				UE_LOG(
					LogAGX, Error, TEXT("AGX Distributed failed to serialize entity %llu: %s"),
					Id.getId(), UTF8_TO_TCHAR(Error.what()));
				return false;
			}
		}

		void onSpawnEntity(agxDistributed::EntityId Id) override
		{
			withEntity(
				Id,
				[this, Id](const agxDistributed::Entity& Entity)
				{
					instantiateAsset(
						Id, Entity.getAssetId(), Entity.getEntityData().getTransform());
				});
			onEntityUpdate(Id);
		}

		void onEntityUpdate(agxDistributed::EntityId Id) override
		{
			processIncomingEntityUpdates({Id});
		}

		void onEntitySimulationAuthority(
			agxDistributed::EntityId Id, AuthorityChangeType Change) override
		{
			FAGXDistributedEvent Event;
			Event.Type = EAGXDistributedEventType::SimulationAuthorityChanged;
			Event.EntityId = Id.getId();
			Event.bWasSimulating = Change == AuthorityChangeType::LOST_TO_OTHER ||
								   Change == AuthorityChangeType::LOST_TO_UNASSIGNED;
			Event.bAssigned = Change != AuthorityChangeType::LOST_TO_UNASSIGNED &&
							  Change != AuthorityChangeType::REMOTE_BECAME_UNASSIGNED;
			Events->Push(MoveTemp(Event));
		}

		void onStaleEntityData(agxDistributed::EntityId Id) override
		{
			FAGXDistributedEvent Event;
			Event.Type = EAGXDistributedEventType::StaleEntityState;
			Event.EntityId = Id.getId();
			Events->Push(MoveTemp(Event));
		}

		void onEntityControlSignal(
			agxDistributed::EntityId Id, const std::string& Name, double Value) override
		{
			// Signal application is not implemented yet; log without accessing UObjects.
			applySignal(Id, Name, Value);
		}
		void onRemoveEntity(agxDistributed::EntityId Id) override
		{
			FAGXDistributedEvent Event;
			Event.Type = EAGXDistributedEventType::RemoveEntity;
			Event.EntityId = Id.getId();
			Events->Push(MoveTemp(Event));
		}
		void instantiateAsset(
			agxDistributed::EntityId EntityId, agxDistributed::AssetId AssetId,
			agx::AffineMatrix4x4 AssetTransform)
		{
			FAGXDistributedEvent Event;
			Event.Type = EAGXDistributedEventType::SpawnEntity;
			Event.EntityId = EntityId.getId();
			Event.AssetId = AssetId.getId();
			Event.AssetPath =
				FString(UTF8_TO_TCHAR(getScenarioInfo().getAssetPath(AssetId).c_str()));
			Event.Transform = Convert(AssetTransform);
			// Spawn events precede the initial update event in the same queue. Actor creation
			// and native body registration happen when the world subsystem drains it.

			UE_LOG(
				LogAGX, Warning,
				TEXT("Unreal Application Layer instantiating asset %s for entity %llu"),
				*Event.AssetPath, static_cast<unsigned long long>(Event.EntityId));

			Events->Push(MoveTemp(Event));
		}

		void processIncomingEntityUpdates(std::vector<agxDistributed::EntityId>&& UpdatedEntities)
		{
			for (const agxDistributed::EntityId Id : UpdatedEntities)
			{
				std::vector<uint8_t> AssetData;
				uint32 Flags = 0;
				const bool Found = withEntity(
					Id,
					[&AssetData, &Flags](const agxDistributed::Entity& Entity)
					{
						AssetData = Entity.getEntityData().getAssetData();
						Flags = Entity.getEntityData().getFlags();
					});
				// Spawn requests may carry no body state yet. Leave the spawn transform intact.
				if (!Found || AssetData.empty())
					continue;

				FAGXDistributedEvent Event;
				Event.Type = EAGXDistributedEventType::EntityUpdate;
				Event.EntityId = Id.getId();
				Event.bAssigned = (Flags & agxDistributed::EntityData::ASSIGNED_ENTITY) != 0;
				if (!DecodeBodyTransforms(
						AssetData, Event.BodyTransforms, &Event.BodyVelocities,
						&Event.BodyAngularVelocities))
				{
					Event.BodyTransforms.Reset();
					Event.Message = TEXT("Invalid AssemblySerializer body transform payload.");
				}
				// Called from middleware callbacks: never touch UObjects here.
				Events->Push(MoveTemp(Event));
			}
		}

		void onSimulationControlMessage(bool bStart)
		{
			setNodeStatus(bStart ? "SIMULATING" : "PAUSED");
			FAGXDistributedEvent Event;
			Event.Type = bStart ? EAGXDistributedEventType::SimulationStart
								: EAGXDistributedEventType::SimulationPause;
			Events->Push(MoveTemp(Event));
		}

		void onShutdownControlMessage()
		{
			FAGXDistributedEvent Event;
			Event.Type = EAGXDistributedEventType::ShutdownRequested;
			Events->Push(MoveTemp(Event));
		}

	private:
		agxSDK::Simulation& Simulation;
		TMap<uint64, TArray<FRigidBodyBarrier>> RegisteredBodies;
		TMap<uint64, TMap<FString, double>> LocalSignals;
		std::shared_ptr<FEventQueue> Events;
	};

	bool FutureReady(std::future_status Status)
	{
		return Status == std::future_status::ready;
	}
}

bool FTestUtilities::DecodeDistributedBodyTransforms(
	const TArray<uint8>& Data, TArray<FTransform>& OutTransforms)
{
	OutTransforms.Reset();
	std::vector<uint8_t> Bytes;
	if (!Data.IsEmpty())
		Bytes.assign(Data.GetData(), Data.GetData() + Data.Num());
	const bool Result = DecodeBodyTransforms(Bytes, OutTransforms);
	if (!Result)
		OutTransforms.Reset();
	return Result;
}

struct FDistributedApplicationBarrier::FImpl
{
	struct FPendingSpawn
	{
		FString AssetPath;
		FGuid RequestId;
		std::future<agxDistributed::EntityId> Future;
	};

	struct FPendingAssignment
	{
		uint64 EntityId;
		bool bRequestedValue;
		std::future<std::pair<agxDistributed::EntityId, bool>> Future;
	};

	agx::ref_ptr<FUnrealApplicationLayer> Native;
	std::shared_ptr<FEventQueue> Events {std::make_shared<FEventQueue>()};
	std::future<void> StartFuture;
	std::vector<FPendingSpawn> Spawns;
	struct FPendingRelease
	{
		FGuid RequestId;
		std::future<bool> Future;
	};
	std::vector<FPendingRelease> Releases;
	std::vector<FPendingAssignment> Possessions;
	std::vector<FPendingAssignment> Authorities;
	bool bReady {false};
};

FDistributedApplicationBarrier::FDistributedApplicationBarrier()
	: Impl(std::make_unique<FImpl>())
{
}

FDistributedApplicationBarrier::~FDistributedApplicationBarrier()
{
	Shutdown();
}

bool FDistributedApplicationBarrier::Initialize(
	FSimulationBarrier& Simulation, const FString& ZenohEndpoint)
{
	if (!Simulation.HasNative() || Impl->Native != nullptr)
		return false;

	try
	{
		agxDistributed::Node::Config Config;
		if (!ZenohEndpoint.IsEmpty())
		{
			const std::string Endpoint(TCHAR_TO_UTF8(*ZenohEndpoint));
			Config.addSetting(std::string("connect/endpoints"), "[\"" + Endpoint + "\"]");
			Config.addSetting(std::string("mode"), std::string("\"client\""));
			Config.addSetting(std::string("scouting/multicast/enabled"), std::string("false"));
		}

		Impl->Native =
			new FUnrealApplicationLayer(*Simulation.GetNative()->Native, Config, Impl->Events);
		return true;
	}
	catch (const std::exception& Error)
	{
		UE_LOG(
			LogAGX, Error, TEXT("AGX Distributed initialization failed: %s"),
			*FString(UTF8_TO_TCHAR(Error.what())));
		return false;
	}
}

void FDistributedApplicationBarrier::StartAndLoad(int32 TimeoutSeconds)
{
	check(IsInGameThread());
	if (Impl->Native == nullptr || Impl->bReady || Impl->StartFuture.valid())
		return;
	try
	{
		Impl->StartFuture = Impl->Native->startAndLoad("", FMath::Max(0, TimeoutSeconds));
	}
	catch (const std::exception& Error)
	{
		FAGXDistributedEvent Event;
		Event.Type = EAGXDistributedEventType::StartFailed;
		Event.Message = FString(UTF8_TO_TCHAR(Error.what()));
		Impl->Events->Push(MoveTemp(Event));
	}
}

void FDistributedApplicationBarrier::Poll(TArray<FAGXDistributedEvent>& OutEvents)
{
	// Q: Should std::future cross the Barrier ABI, or should all completions remain here?
	// This first pass keeps futures here and emits copied, Unreal-compatible events.
	using namespace std::chrono_literals;
	if (Impl->StartFuture.valid() && FutureReady(Impl->StartFuture.wait_for(0ms)))
	{
		FAGXDistributedEvent Event;
		try
		{
			Impl->StartFuture.get();
			Impl->bReady = true;
			Event.Type = EAGXDistributedEventType::Started;
			Event.Message = GetNodeId();
		}
		catch (const std::exception& Error)
		{
			Event.Type = EAGXDistributedEventType::StartFailed;
			Event.Message = FString(UTF8_TO_TCHAR(Error.what()));
		}
		Impl->Events->Push(MoveTemp(Event));
	}

	for (auto It = Impl->Spawns.begin(); It != Impl->Spawns.end();)
	{
		if (!FutureReady(It->Future.wait_for(0ms)))
		{
			++It;
			continue;
		}
		FAGXDistributedEvent Event;
		Event.Type = EAGXDistributedEventType::SpawnRequestResult;
		Event.AssetPath = It->AssetPath;
		Event.RequestId = It->RequestId;
		try
		{
			const agxDistributed::EntityId Id = It->Future.get();
			Event.EntityId = Id.getId();
			Event.bAccepted = Id.isValid();
		}
		catch (const std::exception& Error)
		{
			Event.Message = FString(UTF8_TO_TCHAR(Error.what()));
		}
		Impl->Events->Push(MoveTemp(Event));
		It = Impl->Spawns.erase(It);
	}

	auto PollAssignments =
		[this](std::vector<FImpl::FPendingAssignment>& Pending, EAGXDistributedEventType Type)
	{
		for (auto It = Pending.begin(); It != Pending.end();)
		{
			if (!FutureReady(It->Future.wait_for(0ms)))
			{
				++It;
				continue;
			}
			FAGXDistributedEvent Event;
			Event.Type = Type;
			Event.EntityId = It->EntityId;
			Event.bRequestedValue = It->bRequestedValue;
			Event.Message = It->bRequestedValue ? TEXT("acquire") : TEXT("release");
			try
			{
				Event.bAccepted = It->Future.get().second;
				if (!Event.bAccepted)
					Event.Message = TEXT("World Manager rejected the assignment request.");
			}
			catch (const std::exception& Error)
			{
				Event.Message = FString(UTF8_TO_TCHAR(Error.what()));
			}
			Impl->Events->Push(MoveTemp(Event));
			It = Pending.erase(It);
		}
	};
	for (auto It = Impl->Releases.begin(); It != Impl->Releases.end();)
	{
		if (!FutureReady(It->Future.wait_for(0ms)))
		{
			++It;
			continue;
		}
		FAGXDistributedEvent Event;
		Event.Type = EAGXDistributedEventType::ReleaseInterestResult;
		Event.RequestId = It->RequestId;
		try
		{
			Event.bAccepted = It->Future.get();
		}
		catch (const std::exception& Error)
		{
			Event.Message = UTF8_TO_TCHAR(Error.what());
		}
		Impl->Events->Push(MoveTemp(Event));
		It = Impl->Releases.erase(It);
	}
	PollAssignments(Impl->Possessions, EAGXDistributedEventType::PossessionResult);
	PollAssignments(Impl->Authorities, EAGXDistributedEventType::SimulationAuthorityResult);
	if (Impl->bReady && Impl->Native != nullptr)
		Impl->Native->PollEvents(OutEvents);
	else
		Impl->Events->Drain(OutEvents);
}

bool FDistributedApplicationBarrier::IsReady() const
{
	return Impl->bReady;
}

FString FDistributedApplicationBarrier::GetNodeId() const
{
	if (Impl->Native != nullptr)
		return FString(UTF8_TO_TCHAR(Impl->Native->getNodeId().getId().c_str()));
	return {};
}

bool FDistributedApplicationBarrier::ResolveAsset(const FString& Path, uint64& OutId) const
{
	if (!IsReady())
		return false;
	auto Id = Impl->Native->getScenarioInfo().findAssetId(TCHAR_TO_UTF8(*Path));
	if (!Id.has_value())
		return false;
	OutId = Id->getId();
	return true;
}

FString FDistributedApplicationBarrier::GetScenarioId() const
{
	return IsReady() ? UTF8_TO_TCHAR(Impl->Native->getScenarioInfo().getScenarioId().c_str())
					 : FString();
}

bool FDistributedApplicationBarrier::EnsureEntity(
	const FGuid& RequestId, uint64 AssetId, const FTransform& Transform, const FString& ObjectKey)
{
	if (!IsReady())
		return false;
	try
	{
		Impl->Spawns.push_back(
			{FString(), RequestId,
			 Impl->Native->ensureEntity(
				 agxDistributed::AssetId(AssetId), Convert(Transform), TCHAR_TO_UTF8(*ObjectKey),
				 TCHAR_TO_UTF8(*RequestId.ToString()))});
		return true;
	}
	catch (const std::exception& Error)
	{
		UE_LOG(LogAGX, Error, TEXT("Ensure entity: %s"), UTF8_TO_TCHAR(Error.what()));
	}
	return false;
}

bool FDistributedApplicationBarrier::ReleaseInterest(
	const FGuid& RequestId, const FString& ObjectKey)
{
	if (!IsReady())
		return false;
	try
	{
		Impl->Releases.push_back(
			{RequestId, Impl->Native->releaseEntityInterest(
							TCHAR_TO_UTF8(*ObjectKey), TCHAR_TO_UTF8(*RequestId.ToString()))});
		return true;
	}
	catch (const std::exception& Error)
	{
		UE_LOG(LogAGX, Error, TEXT("Release interest: %s"), UTF8_TO_TCHAR(Error.what()));
	}
	return false;
}

bool FDistributedApplicationBarrier::RequestSpawn(
	const FString& AssetPath, const FTransform& Transform, bool bPossess)
{
	if (!IsReady() || AssetPath.IsEmpty())
		return false;
	try
	{
		// Q: What initial rigid-body state should the spawn blob contain?
		Impl->Spawns.push_back(
			{AssetPath, FGuid(),
			 Impl->Native->requestEntitySpawn(
				 std::string(TCHAR_TO_UTF8(*AssetPath)), Convert(Transform), "", bPossess)});
		return true;
	}
	catch (const std::exception& Error)
	{
		UE_LOG(
			LogAGX, Error, TEXT("AGX Distributed spawn request failed: %s"),
			*FString(UTF8_TO_TCHAR(Error.what())));
	}
	return false;
}

bool FDistributedApplicationBarrier::RequestPossession(uint64 EntityId, bool bPossess)
{
	if (!IsReady() || EntityId == MAX_uint64)
		return false;
	try
	{
		Impl->Possessions.push_back(
			{EntityId, bPossess,
			 Impl->Native->agxDistributed::Client::requestPossession(
				 agxDistributed::EntityId(EntityId), bPossess)});
		return true;
	}
	catch (const std::exception& Error)
	{
		UE_LOG(
			LogAGX, Error, TEXT("AGX Distributed possession request failed: %s"),
			*FString(UTF8_TO_TCHAR(Error.what())));
	}
	return false;
}

bool FDistributedApplicationBarrier::RequestSimulationAuthority(uint64 EntityId, bool bSimulate)
{
	if (!IsReady() || EntityId == MAX_uint64)
		return false;
	try
	{
		Impl->Authorities.push_back(
			{EntityId, bSimulate,
			 Impl->Native->agxDistributed::Client::requestSimulationAuthority(
				 agxDistributed::EntityId(EntityId), bSimulate)});
		return true;
	}
	catch (const std::exception& Error)
	{
		UE_LOG(
			LogAGX, Error, TEXT("AGX Distributed authority request failed: %s"),
			*FString(UTF8_TO_TCHAR(Error.what())));
	}
	return false;
}

bool FDistributedApplicationBarrier::RouteSignal(
	uint64 EntityId, const FString& SignalName, double Value)
{
	if (!IsReady() || EntityId == MAX_uint64 || SignalName.IsEmpty())
		return false;
	try
	{
		Impl->Native->routeSignal(
			agxDistributed::EntityId(EntityId), std::string(TCHAR_TO_UTF8(*SignalName)), Value);
		return true;
	}
	catch (const std::exception& Error)
	{
		UE_LOG(
			LogAGX, Error, TEXT("AGX Distributed signal routing failed: %s"),
			*FString(UTF8_TO_TCHAR(Error.what())));
	}
	return false;
}

TArray<uint64> FDistributedApplicationBarrier::GetSimulatedEntities() const
{
	check(IsInGameThread());
	if (!IsReady() || Impl->Native == nullptr)
		return {};
	return Impl->Native->GetSimulatedEntities();
}

void FDistributedApplicationBarrier::RegisterEntityBodies(
	uint64 EntityId, const TArray<FRigidBodyBarrier>& Bodies)
{
	check(IsInGameThread());
	if (IsReady() && Impl->Native != nullptr)
		Impl->Native->RegisterEntityBodies(EntityId, Bodies);
}

void FDistributedApplicationBarrier::SendEntityUpdates(
	const TMap<uint64, TArray<FRigidBodyBarrier>>& EntityBodies)
{
	check(IsInGameThread());
	if (IsReady() && Impl->Native != nullptr)
		Impl->Native->SendEntityUpdates(EntityBodies);
}

void FDistributedApplicationBarrier::Shutdown()
{
	if (Impl->Native != nullptr)
	{
		Impl->Native->shutdownApplicationLayer();
		Impl->Native = nullptr;
	}
	Impl->bReady = false;
	Impl->Spawns.clear();
	Impl->Releases.clear();
	Impl->Possessions.clear();
	Impl->Authorities.clear();
}

bool FDistributedApplicationBarrier::FindWorldObject(
	const FString& ObjectKey, uint64& OutEntityId) const
{
	if (!IsReady())
		return false;
	const auto Id = Impl->Native->findWorldObject(TCHAR_TO_UTF8(*ObjectKey));
	if (!Id.has_value())
		return false;
	OutEntityId = Id->getId();
	return true;
}

#if WITH_DEV_AUTOMATION_TESTS
bool FTestUtilities::TestDistributedEntityState(FString& OutError)
{
	try
	{
		FSimulationBarrier SimulationA, SimulationB;
		SimulationA.AllocateNative();
		SimulationB.AllocateNative();
		agxDistributed::Node::Config Config;
		Config.addSetting("mode", "\"peer\"");
		Config.addSetting("listen/endpoints", "[\"tcp/127.0.0.1:17591\"]");
		Config.addSetting("scouting/multicast/enabled", "false");
		agxDistributed::ScenarioInfo Scenario;
		Scenario.addAssetPath("Car.openplx");
		agx::ref_ptr<agxDistributed::WorldManager> Manager =
			new agxDistributed::WorldManager(Scenario, Config);
		Manager->start();
		FDistributedApplicationBarrier A, B;
		if (!A.Initialize(SimulationA, TEXT("tcp/127.0.0.1:17591")) ||
			!B.Initialize(SimulationB, TEXT("tcp/127.0.0.1:17591")))
			throw std::runtime_error("State test initialization failed");
		A.StartAndLoad(5);
		B.StartAndLoad(5);
		TArray<FAGXDistributedEvent> EventsA, EventsB;
		auto WaitFor = [&](auto Predicate)
		{
			const auto Deadline = std::chrono::steady_clock::now() + std::chrono::seconds(8);
			while (std::chrono::steady_clock::now() < Deadline)
			{
				A.Poll(EventsA);
				B.Poll(EventsB);
				if (Predicate())
					return;
				FPlatformProcess::Sleep(0.005f);
			}
			throw std::runtime_error("Timed out waiting for distributed entity state/authority");
		};
		auto Require = [](bool Condition, const char* Message)
		{
			if (!Condition)
				throw std::runtime_error(Message);
		};
		WaitFor([&] { return A.IsReady() && B.IsReady(); });
		const FGuid Request = FGuid::NewGuid();
		Require(
			A.EnsureEntity(Request, 0, FTransform::Identity, TEXT("state-test")),
			"Ensure submission failed");
		uint64 Id = MAX_uint64;
		WaitFor(
			[&]
			{
				for (const auto& Event : EventsA)
					if (Event.Type == EAGXDistributedEventType::SpawnRequestResult &&
						Event.RequestId == Request && Event.bAccepted)
						Id = Event.EntityId;
				return Id != MAX_uint64 &&
					   EventsB.ContainsByPredicate(
						   [&](const auto& Event)
						   {
							   return Event.Type == EAGXDistributedEventType::SpawnEntity &&
									  Event.EntityId == Id;
						   });
			});
		FRigidBodyBarrier BodyA, BodyB;
		BodyA.AllocateNative();
		BodyB.AllocateNative();
		BodyA.SetPosition(FVector(100, -200, 300));
		BodyA.SetVelocity(FVector(400, -500, 600));
		BodyA.SetAngularVelocity(FVector(10, -20, 30));
		// Independent wire assertion: AGX velocity (4, 5, 6) m/s as big-endian floats.
		std::vector<uint8_t> Wire;
		Require(
			EncodeBodyTransforms({BodyA}, Wire) && Wire.size() == 70 && Wire[7] == 1,
			"Encoded body layout differs");
		const uint8_t ExpectedPosition[] = {0x3f, 0xf0, 0, 0, 0, 0, 0, 0, // 1
											0x40, 0x00, 0, 0, 0, 0, 0, 0, // 2
											0x40, 0x08, 0, 0, 0, 0, 0, 0}; // 3
		Require(
			std::memcmp(Wire.data() + 8, ExpectedPosition, sizeof(ExpectedPosition)) == 0,
			"Encoded reference position differs from AssemblySerializer");
		const uint8_t ExpectedPose[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x7f, 0xff};
		Require(
			std::memcmp(Wire.data() + 32, ExpectedPose, sizeof(ExpectedPose)) == 0,
			"Encoded position deltas or quaternion differ from AssemblySerializer");
		const uint8_t ExpectedVelocity[] = {0x40, 0x80, 0, 0, 0x40, 0xa0, 0, 0, 0x40, 0xc0, 0, 0};
		Require(
			std::memcmp(Wire.data() + 46, ExpectedVelocity, sizeof(ExpectedVelocity)) == 0,
			"Encoded velocity byte order differs from AssemblySerializer");
		BodyB.SetPosition(FVector(0, -250, 300));
		Require(
			EncodeBodyTransforms({BodyA, BodyB}, Wire) && Wire.size() == 108,
			"Encoded two-body layout differs");
		const uint8_t ExpectedDelta[] = {0xfc, 0x18, 0x01, 0xf4, 0, 0};
		Require(
			std::memcmp(Wire.data() + 70, ExpectedDelta, sizeof(ExpectedDelta)) == 0,
			"Encoded signed millimetre deltas differ from AssemblySerializer");
		BodyB.SetPosition(FVector(6400, -200, 300));
		Require(
			EncodeBodyTransforms({BodyA, BodyB}, Wire) && Wire.size() == 132,
			"Encoded escaped position layout differs");
		const uint8_t ExpectedEscape[] = {0x80, 0, 0x80, 0, 0x80, 0, 0x40, 0x50, 0, 0, 0, 0, 0, 0};
		Require(
			std::memcmp(Wire.data() + 70, ExpectedEscape, sizeof(ExpectedEscape)) == 0,
			"Encoded absolute position differs from AssemblySerializer");
		A.RegisterEntityBodies(Id, {BodyA});
		B.RegisterEntityBodies(Id, {BodyB});
		Require(A.RequestSimulationAuthority(Id, true), "Authority submission failed");
		WaitFor([&] { return A.GetSimulatedEntities().Contains(Id); });
		WaitFor(
			[&]
			{
				return EventsB.ContainsByPredicate(
					[&](const auto& Event)
					{
						return Event.Type == EAGXDistributedEventType::SimulationAuthorityChanged &&
							   Event.EntityId == Id;
					});
			});
		EventsA.Reset();
		EventsB.Reset();
		A.SendEntityUpdates({{Id, {BodyA}}});
		FAGXDistributedEvent State;
		auto FindState = [&](bool bAssigned)
		{
			for (const auto& Event : EventsB)
				if (Event.Type == EAGXDistributedEventType::EntityUpdate && Event.EntityId == Id &&
					Event.bAssigned == bAssigned)
				{
					State = Event;
					return true;
				}
			return false;
		};
		WaitFor([&] { return FindState(true); });
		Require(
			State.Message.IsEmpty() && State.BodyTransforms.Num() == 1 &&
				State.BodyVelocities.Num() == 1 && State.BodyAngularVelocities.Num() == 1,
			"Invalid body state payload");
		Require(
			State.BodyTransforms[0].GetTranslation().Equals(FVector(100, -200, 300)),
			"Body transform conversion differs");
		Require(
			State.BodyVelocities[0].Equals(FVector(400, -500, 600), 0.001),
			"Linear velocity conversion differs");
		Require(
			State.BodyAngularVelocities[0].Equals(FVector(10, -20, 30), 0.001),
			"Angular velocity conversion differs");
		EventsB.Reset();
		Require(A.RequestSimulationAuthority(Id, false), "Release submission failed");
		WaitFor([&] { return !A.GetSimulatedEntities().Contains(Id) && FindState(false); });
		EventsA.Reset();
		Require(B.RequestSimulationAuthority(Id, true), "Transfer acquisition failed");
		WaitFor(
			[&]
			{
				return B.GetSimulatedEntities().Contains(Id) &&
					   EventsA.ContainsByPredicate(
						   [&](const auto& Event)
						   {
							   return Event.Type ==
										  EAGXDistributedEventType::SimulationAuthorityChanged &&
									  Event.EntityId == Id;
						   });
			});
		// No updates arrive from B: simulation time advancing beyond the native timeout must stop
		// prediction.
		SimulationA.GetNative()->Native->setTimeStamp(10);
		WaitFor(
			[&]
			{
				return EventsA.ContainsByPredicate(
					[&](const auto& Event)
					{
						return Event.Type == EAGXDistributedEventType::StaleEntityState &&
							   Event.EntityId == Id;
					});
			});
		Require(A.ReleaseInterest(Request, TEXT("state-test")), "Removal submission failed");
		WaitFor(
			[&]
			{
				return EventsB.ContainsByPredicate(
					[&](const auto& Event)
					{
						return Event.Type == EAGXDistributedEventType::RemoveEntity &&
							   Event.EntityId == Id;
					});
			});
		A.Shutdown();
		B.Shutdown();
		return true;
	}
	catch (const std::exception& Error)
	{
		OutError = UTF8_TO_TCHAR(Error.what());
		return false;
	}
}
#endif

#if WITH_DEV_AUTOMATION_TESTS
bool FTestUtilities::TestDistributedControlMessages(FString& OutError)
{
	try
	{
		FSimulationBarrier Simulation;
		Simulation.AllocateNative();
		const double InitialTime = Simulation.GetTimeStamp();
		agxDistributed::Node::Config Config;
		Config.addSetting("mode", "\"peer\"");
		Config.addSetting("listen/endpoints", "[\"tcp/127.0.0.1:17590\"]");
		Config.addSetting("scouting/multicast/enabled", "false");
		agx::ref_ptr<agxDistributed::WorldManager> Manager =
			new agxDistributed::WorldManager(agxDistributed::ScenarioInfo(), Config);
		Manager->start();

		auto PublisherConfig = zenoh::Config::create_default();
		PublisherConfig.insert_json5("mode", "\"client\"");
		PublisherConfig.insert_json5("connect/endpoints", "[\"tcp/127.0.0.1:17590\"]");
		PublisherConfig.insert_json5("scouting/multicast/enabled", "false");
		auto Publisher = zenoh::Session::open(std::move(PublisherConfig));
		FDistributedApplicationBarrier Client;
		if (!Client.Initialize(Simulation, TEXT("tcp/127.0.0.1:17590")))
			throw std::runtime_error("Control test client initialization failed");
		Client.StartAndLoad(5);
		auto WaitFor = [](FDistributedApplicationBarrier& Barrier,
						  TArray<FAGXDistributedEvent>& Pending, EAGXDistributedEventType Type)
		{
			const auto Deadline = std::chrono::steady_clock::now() + std::chrono::seconds(8);
			while (std::chrono::steady_clock::now() < Deadline)
			{
				Barrier.Poll(Pending);
				const int32 Index = Pending.IndexOfByPredicate(
					[Type](const FAGXDistributedEvent& Event) { return Event.Type == Type; });
				if (Index != INDEX_NONE)
				{
					Pending.RemoveAt(Index);
					return;
				}
				FPlatformProcess::Sleep(0.005f);
			}
			throw std::runtime_error(
				"Timed out waiting for a control event from world-tick polling without a physics "
				"step");
		};
		TArray<FAGXDistributedEvent> Pending;
		WaitFor(Client, Pending, EAGXDistributedEventType::Started);
		auto Command = [&](const char* Action, EAGXDistributedEventType Expected)
		{
			Publisher.put(zenoh::KeyExpr("control/simulation"), zenoh::Bytes(Action));
			WaitFor(Client, Pending, Expected);
		};
		Command("START", EAGXDistributedEventType::SimulationStart);
		Command("STOP", EAGXDistributedEventType::SimulationPause);
		Command("START", EAGXDistributedEventType::SimulationStart);
		Command("PAUSE", EAGXDistributedEventType::SimulationPause);
		Command("START", EAGXDistributedEventType::SimulationStart);

		// A second client must also dispatch the cached START while physics is paused.
		FDistributedApplicationBarrier LateClient;
		if (!LateClient.Initialize(Simulation, TEXT("tcp/127.0.0.1:17590")))
			throw std::runtime_error("Late control test client initialization failed");
		LateClient.StartAndLoad(5);
		TArray<FAGXDistributedEvent> LatePending;
		WaitFor(LateClient, LatePending, EAGXDistributedEventType::Started);
		WaitFor(LateClient, LatePending, EAGXDistributedEventType::SimulationStart);
		Command("STOP", EAGXDistributedEventType::SimulationPause);
		WaitFor(LateClient, LatePending, EAGXDistributedEventType::SimulationPause);
		Publisher.put(zenoh::KeyExpr("control/shutdown"), zenoh::Bytes(""));
		WaitFor(Client, Pending, EAGXDistributedEventType::ShutdownRequested);
		WaitFor(LateClient, LatePending, EAGXDistributedEventType::ShutdownRequested);
		if (Simulation.GetTimeStamp() != InitialTime)
			throw std::runtime_error("Control test unexpectedly advanced physics time");
		return true;
	}
	catch (const std::exception& Error)
	{
		OutError = UTF8_TO_TCHAR(Error.what());
		return false;
	}
}
#endif
