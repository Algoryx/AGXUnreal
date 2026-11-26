// Copyright 2026, Algoryx Simulation AB.

#include "Distributed/DistributedApplicationBarrier.h"
#include "DistributedProximitySensors.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "BarrierOnly/AGXRefs.h"
#include "BarrierOnly/AGXTypeConversions.h"
#include "SimulationBarrier.h"
#include "Utilities/TestUtilities.h"

// Unreal Engine includes.
#include "HAL/PlatformProcess.h"
#include "HAL/PlatformTime.h"
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
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

// AGX Dynamics includes.
#include <BeginAGXIncludes.h>
#include <agx/AffineMatrix4x4.h>
#include <agx/Quat.h>
#include <agx/RigidBody.h>
#include <agx/Vec2.h>
#include <agx/Vec3.h>
#include <agxCollide/Box.h>
#include <agxCollide/Geometry.h>
#include <agxCollide/Sphere.h>
#include <agxDistributed/Client.h>
#include <agxDistributed/SignalValue.h>
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
			Proximity = std::make_unique<FDistributedProximitySensors>(Simulation, *this);
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
			Proximity->Reset();
			RegisteredBodies.Reset();
			PendingBodyRemovals.Reset();
			closeSession();
		}

		void RegisterEntityBodies(
			uint64 Id, const TArray<FRigidBodyBarrier>& Bodies, const FVector& Center,
			double Radius)
		{
			RegisteredBodies.Add(Id, Bodies);
			Proximity->Register(Id, Bodies, Center, Radius);
		}
		void UnregisterEntityBodies(uint64 Id)
		{
			PendingBodyRemovals.Add(Id);
		}

		void PollEvents(TArray<FAGXDistributedEvent>& OutEvents)
		{
			for (const uint64 Id : PendingBodyRemovals)
			{
				Proximity->Unregister(Id);
				RegisteredBodies.Remove(Id);
				LocalSignals.Remove(Id);
			}
			PendingBodyRemovals.Reset();
			Proximity->Poll();
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
					{
						const auto Replay = *Signals;
						for (const auto& Signal : Replay)
							routeSignal(
								agxDistributed::EntityId(Event.EntityId),
								TCHAR_TO_UTF8(*Signal.Key), Signal.Value);
					}
				if (Event.Type == EAGXDistributedEventType::RemoveEntity)
				{
					Proximity->Unregister(Event.EntityId);
					RegisteredBodies.Remove(Event.EntityId);
					LocalSignals.Remove(Event.EntityId);
				}
			}
			OutEvents.Append(MoveTemp(Ready));
		}

		void routeSignal(
			agxDistributed::EntityId Id, const std::string& Name,
			const agxDistributed::SignalValue& Value)
		{
			if (!isPossessed(Id))
				return;
			LocalSignals.FindOrAdd(Id.getId()).Add(UTF8_TO_TCHAR(Name.c_str()), Value);
			if (isSimulated(Id))
				applySignal(Id, Name, Value);
			else
			{
				publishControlSignal(Id, Name, Value);
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
				RegisteredBodies.Add(Entry.Key, Entry.Value);
				if (isSimulated(agxDistributed::EntityId(Entry.Key)))
					requestSerializedState(agxDistributed::EntityId(Entry.Key), true);
			}
		}

		void applySignal(
			agxDistributed::EntityId EntityId, const std::string& Name,
			const agxDistributed::SignalValue& Value)
		{
			// Network callbacks must not access UObjects. Local signals use the same delivery path.
			FAGXDistributedEvent Event;
			Event.Type = EAGXDistributedEventType::ControlSignal;
			Event.EntityId = EntityId.getId();
			Event.SignalName = UTF8_TO_TCHAR(Name.c_str());
			// Copy components without changing units; the receiving signal-handler barrier
			// performs the same OpenPLX conversion as a local Send call.
			Event.SignalValue = std::visit(
				[](const auto& Payload) -> FAGXDistributedSignalValue
				{
					using T = std::decay_t<decltype(Payload)>;
					if constexpr (std::is_same_v<T, agx::Vec2>)
						return FVector2D(Payload.x(), Payload.y());
					else if constexpr (std::is_same_v<T, agx::Vec3>)
						return FVector(Payload.x(), Payload.y(), Payload.z());
					else if constexpr (std::is_same_v<T, int64_t>)
						return static_cast<int64>(Payload);
					else
						return Payload;
				},
				Value);
			Events->Push(MoveTemp(Event));
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
				// Finish encoding with Unreal's allocator before making the native-owned copy.
				std::vector<uint8_t> EncodedData;
				if (!EncodeBodyTransforms(*Bodies, EncodedData))
					return false;

				agxDistributed::EntityData Update;
				ON_SCOPE_EXIT
				{
					// Release any native buffer not consumed by updateEntity, including on
					// exceptions.
					agxUtil::freeContainerMemory(Update.getAssetData());
				};
				Update.getTransform() = agx::AffineMatrix4x4();
				Update.getFlags() = bAssigned ? agxDistributed::EntityData::ASSIGNED_ENTITY : 0;
				// Move the AGX-allocated copy into Update; do not grow its buffer in Unreal.
				Update.getAssetData() = agxUtil::copyContainerMemory(EncodedData);
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
			agxDistributed::EntityId Id, const std::string& Name,
			const agxDistributed::SignalValue& Value) override
		{
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
			std::string NativePath = getScenarioInfo().getAssetPath(AssetId);
			ON_SCOPE_EXIT
			{
				agxUtil::freeContainerMemory(NativePath);
			};
			Event.AssetPath = Convert(NativePath);
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
		TSet<uint64> PendingBodyRemovals;
		std::unique_ptr<FDistributedProximitySensors> Proximity;
		TMap<uint64, TMap<FString, agxDistributed::SignalValue>> LocalSignals;
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

	struct FRegisteringAsset
	{
		FString Path;
		EAGX_DistributedAssetType Type;
		std::shared_future<agxDistributed::AssetId> Future;
	};
	struct FPendingRegistration
	{
		FGuid RequestId;
		FString Path;
		std::shared_future<agxDistributed::AssetId> Future;
	};
	struct FPendingLegacySpawn
	{
		FString Path;
		FTransform Transform;
		bool bPossess;
		std::shared_future<agxDistributed::AssetId> Registration;
	};

	std::shared_future<agxDistributed::AssetId> RegisterAsset(
		const FString& Path, EAGX_DistributedAssetType Type)
	{
		const auto NativeType = Convert(Type);
		const auto& Scenario = Native->getScenarioInfo();
		if (const auto Id = Scenario.findAssetId(TCHAR_TO_UTF8(*Path)))
		{
			if (Scenario.getAssetType(*Id) != NativeType)
				throw std::runtime_error("Asset path already registered with a different type.");
			std::promise<agxDistributed::AssetId> Promise;
			Promise.set_value(*Id);
			return Promise.get_future().share();
		}
		for (const auto& Pending : RegisteringAssets)
			if (Pending.Path.Equals(Path, ESearchCase::CaseSensitive) && Pending.Type == Type)
				return Pending.Future;

		// AGX owns the by-value string. Allocate its buffer using the native allocator.
		std::string NativePath = agxUtil::copyContainerMemory(Convert(Path));
		ON_SCOPE_EXIT
		{
			agxUtil::freeContainerMemory(NativePath);
		};
		auto Future = Native->registerAsset(std::move(NativePath), NativeType).share();
		RegisteringAssets.push_back({Path, Type, Future});
		return Future;
	}

	struct FPendingAssignment
	{
		uint64 EntityId;
		bool bRequestedValue;
		std::future<std::pair<agxDistributed::EntityId, bool>> Future;
		FGuid RequestId;
	};

	agx::ref_ptr<FUnrealApplicationLayer> Native;
	std::shared_ptr<FEventQueue> Events {std::make_shared<FEventQueue>()};
	std::future<void> StartFuture;
	std::vector<FPendingSpawn> Spawns;
	std::vector<FRegisteringAsset> RegisteringAssets;
	std::vector<FPendingRegistration> Registrations;
	std::vector<FPendingLegacySpawn> LegacySpawns;
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

	for (auto It = Impl->Registrations.begin(); It != Impl->Registrations.end();)
	{
		if (!FutureReady(It->Future.wait_for(0ms)))
		{
			++It;
			continue;
		}
		FAGXDistributedEvent Event;
		Event.Type = EAGXDistributedEventType::AssetRegistrationResult;
		Event.RequestId = It->RequestId;
		Event.AssetPath = It->Path;
		try
		{
			const auto Id = It->Future.get();
			Event.AssetId = Id.getId();
			Event.bAccepted = Id.isValid();
		}
		catch (const std::exception& Error)
		{
			Event.Message = UTF8_TO_TCHAR(Error.what());
		}
		Impl->Events->Push(MoveTemp(Event));
		It = Impl->Registrations.erase(It);
	}
	for (auto It = Impl->LegacySpawns.begin(); It != Impl->LegacySpawns.end();)
	{
		if (!FutureReady(It->Registration.wait_for(0ms)))
		{
			++It;
			continue;
		}
		try
		{
			It->Registration.get();
			std::string NativePath = agxUtil::copyContainerMemory(Convert(It->Path));
			ON_SCOPE_EXIT
			{
				agxUtil::freeContainerMemory(NativePath);
			};
			Impl->Spawns.push_back(
				{It->Path, FGuid(),
				 Impl->Native->requestEntitySpawn(
					 std::move(NativePath), Convert(It->Transform), "", It->bPossess)});
		}
		catch (const std::exception& Error)
		{
			FAGXDistributedEvent Event;
			Event.Type = EAGXDistributedEventType::SpawnRequestResult;
			Event.AssetPath = It->Path;
			Event.Message = UTF8_TO_TCHAR(Error.what());
			Impl->Events->Push(MoveTemp(Event));
		}
		It = Impl->LegacySpawns.erase(It);
	}
	Impl->RegisteringAssets.erase(
		std::remove_if(
			Impl->RegisteringAssets.begin(), Impl->RegisteringAssets.end(),
			[](const auto& Pending) { return FutureReady(Pending.Future.wait_for(0ms)); }),
		Impl->RegisteringAssets.end());

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
			Event.RequestId = It->RequestId;
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

bool FDistributedApplicationBarrier::RegisterAsset(
	const FGuid& RequestId, const FString& Path, EAGX_DistributedAssetType Type)
{
	check(IsInGameThread());
	if (!IsReady() || !RequestId.IsValid() || Path.IsEmpty())
		return false;
	try
	{
		Impl->Registrations.push_back({RequestId, Path, Impl->RegisterAsset(Path, Type)});
	}
	catch (const std::exception& Error)
	{
		FAGXDistributedEvent Event;
		Event.Type = EAGXDistributedEventType::AssetRegistrationResult;
		Event.RequestId = RequestId;
		Event.AssetPath = Path;
		Event.Message = UTF8_TO_TCHAR(Error.what());
		Impl->Events->Push(MoveTemp(Event));
	}
	return true;
}

FString FDistributedApplicationBarrier::GetScenarioId() const
{
	if (!IsReady())
		return {};
	std::string NativeId = Impl->Native->getScenarioInfo().getScenarioId();
	ON_SCOPE_EXIT
	{
		agxUtil::freeContainerMemory(NativeId);
	};
	return Convert(NativeId);
}

bool FDistributedApplicationBarrier::EnsureEntity(
	const FGuid& RequestId, uint64 AssetId, const FTransform& Transform, const FString& ObjectKey)
{
	if (!IsReady())
		return false;
	try
	{
		// AGX destroys these by-value parameters, so their buffers must be allocated there.
		std::string ObjectKeyAGX = agxUtil::copyContainerMemory(Convert(ObjectKey));
		ON_SCOPE_EXIT
		{
			// Also release any buffers left here if an exception prevents ownership transfer.
			agxUtil::freeContainerMemory(ObjectKeyAGX);
		};
		std::string InterestIdAGX = agxUtil::copyContainerMemory(Convert(RequestId.ToString()));
		ON_SCOPE_EXIT
		{
			agxUtil::freeContainerMemory(InterestIdAGX);
		};
		Impl->Spawns.push_back(
			{FString(), RequestId,
			 Impl->Native->ensureEntity(
				 agxDistributed::AssetId(AssetId), Convert(Transform), std::move(ObjectKeyAGX),
				 std::move(InterestIdAGX))});
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
		// Match EnsureEntity's allocator ownership when releasing the same interest.
		std::string ObjectKeyAGX = agxUtil::copyContainerMemory(Convert(ObjectKey));
		ON_SCOPE_EXIT
		{
			agxUtil::freeContainerMemory(ObjectKeyAGX);
		};
		std::string InterestIdAGX = agxUtil::copyContainerMemory(Convert(RequestId.ToString()));
		ON_SCOPE_EXIT
		{
			agxUtil::freeContainerMemory(InterestIdAGX);
		};
		Impl->Releases.push_back(
			{RequestId, Impl->Native->releaseEntityInterest(
							std::move(ObjectKeyAGX), std::move(InterestIdAGX))});
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
		// The legacy string API has no category argument. Preserve a known category;
		// previously unknown assets use the native registration default, Possessable.
		auto Type = EAGX_DistributedAssetType::Possessable;
		const auto& Scenario = Impl->Native->getScenarioInfo();
		if (const auto Id = Scenario.findAssetId(TCHAR_TO_UTF8(*AssetPath)))
			Type = Convert(Scenario.getAssetType(*Id));
		Impl->LegacySpawns.push_back(
			{AssetPath, Transform, bPossess, Impl->RegisterAsset(AssetPath, Type)});
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

bool FDistributedApplicationBarrier::RequestPossession(
	uint64 EntityId, bool bPossess, const FGuid& RequestId)
{
	check(IsInGameThread());
	if (!IsReady() || EntityId == MAX_uint64)
		return false;
	if (std::any_of(
			Impl->Possessions.begin(), Impl->Possessions.end(),
			[EntityId](const auto& Pending) { return Pending.EntityId == EntityId; }) ||
		(!bPossess && !IsPossessed(EntityId)))
		return false;
	try
	{
		Impl->Possessions.push_back(
			{EntityId, bPossess,
			 Impl->Native->agxDistributed::Client::requestPossession(
				 agxDistributed::EntityId(EntityId), bPossess),
			 RequestId});
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

bool FDistributedApplicationBarrier::IsPossessed(uint64 EntityId) const
{
	check(IsInGameThread());
	return IsReady() && Impl->Native != nullptr && EntityId != MAX_uint64 &&
		   Impl->Native->isPossessed(agxDistributed::EntityId(EntityId));
}

bool FDistributedApplicationBarrier::RequestSimulationAuthority(
	uint64 EntityId, bool bSimulate, const FGuid& RequestId)
{
	check(IsInGameThread());
	if (!IsReady() || EntityId == MAX_uint64)
		return false;
	if (std::any_of(
			Impl->Authorities.begin(), Impl->Authorities.end(),
			[EntityId](const auto& Pending) { return Pending.EntityId == EntityId; }))
		return false;
	try
	{
		Impl->Authorities.push_back(
			{EntityId, bSimulate,
			 Impl->Native->agxDistributed::Client::requestSimulationAuthority(
				 agxDistributed::EntityId(EntityId), bSimulate),
			 RequestId});
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
	uint64 EntityId, const FString& SignalName, const FAGXDistributedSignalValue& Value)
{
	if (!IsReady() || EntityId == MAX_uint64 || SignalName.IsEmpty())
		return false;
	try
	{
		const agxDistributed::SignalValue NativeValue = std::visit(
			[](const auto& Payload) -> agxDistributed::SignalValue
			{
				using T = std::decay_t<decltype(Payload)>;
				if constexpr (std::is_same_v<T, FVector2D>)
					return agx::Vec2(Payload.X, Payload.Y);
				else if constexpr (std::is_same_v<T, FVector>)
					return agx::Vec3(Payload.X, Payload.Y, Payload.Z);
				else if constexpr (std::is_same_v<T, int64>)
					return static_cast<int64_t>(Payload);
				else
					return Payload;
			},
			Value);
		Impl->Native->routeSignal(
			agxDistributed::EntityId(EntityId), std::string(TCHAR_TO_UTF8(*SignalName)),
			NativeValue);
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
	uint64 EntityId, const TArray<FRigidBodyBarrier>& Bodies, const FVector& ProximityCenter,
	double ProximityRadius)
{
	check(IsInGameThread());
	if (IsReady() && Impl->Native != nullptr)
		Impl->Native->RegisterEntityBodies(EntityId, Bodies, ProximityCenter, ProximityRadius);
}

void FDistributedApplicationBarrier::UnregisterEntityBodies(uint64 EntityId)
{
	check(IsInGameThread());
	if (Impl->Native != nullptr)
		Impl->Native->UnregisterEntityBodies(EntityId);
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
	Impl->Registrations.clear();
	Impl->RegisteringAssets.clear();
	Impl->LegacySpawns.clear();
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
bool FTestUtilities::WithDistributedTestManager(
	uint16 Port, TFunctionRef<bool()> Test, FString& OutError, bool bEmptyCatalog)
{
	try
	{
		FSimulationBarrier Simulation;
		Simulation.AllocateNative();
		agxDistributed::Node::Config Config;
		Config.addSetting("mode", "\"peer\"");
		Config.addSetting("listen/endpoints", "[\"tcp/127.0.0.1:" + std::to_string(Port) + "\"]");
		Config.addSetting("scouting/multicast/enabled", "false");
		agxDistributed::ScenarioInfo Scenario;
		// Exercise native string ownership beyond the standard library's small-string buffer.
		std::string ScenarioId = agxUtil::copyContainerMemory(
			Convert(FString(TEXT("AGXUnrealDistributedAutomationScenario"))));
		ON_SCOPE_EXIT
		{
			agxUtil::freeContainerMemory(ScenarioId);
		};
		Scenario.setScenarioId(std::move(ScenarioId));
		if (!bEmptyCatalog)
			Scenario.addAssetPath("Car.openplx");
		agx::ref_ptr<agxDistributed::WorldManager> Manager =
			new agxDistributed::WorldManager(Scenario, Config);
		Manager->start();
		return Test();
	}
	catch (const std::exception& Error)
	{
		OutError = UTF8_TO_TCHAR(Error.what());
		return false;
	}
}

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
		A.RegisterEntityBodies(Id, {BodyA}, BodyA.GetPosition());
		B.RegisterEntityBodies(Id, {BodyB}, BodyB.GetPosition());
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
		// Input possession on B must be independent of simulation authority on A.
		const FGuid PossessionRequestId = FGuid::NewGuid();
		Require(B.RequestPossession(Id, true, PossessionRequestId), "Possession submission failed");
		Require(!B.RequestPossession(Id, true), "Overlapping possession request was accepted");
		auto HasPossessionResult =
			[&](const TArray<FAGXDistributedEvent>& Events, bool bPossess, bool bAccepted)
		{
			return Events.ContainsByPredicate(
				[&](const auto& Event)
				{
					return Event.Type == EAGXDistributedEventType::PossessionResult &&
						   Event.EntityId == Id && Event.bRequestedValue == bPossess &&
						   Event.bAccepted == bAccepted;
				});
		};
		WaitFor([&] { return HasPossessionResult(EventsB, true, true) && B.IsPossessed(Id); });
		Require(
			EventsB.ContainsByPredicate(
				[&](const auto& Event)
				{
					return Event.Type == EAGXDistributedEventType::PossessionResult &&
						   Event.RequestId == PossessionRequestId && Event.bAccepted;
				}),
			"Possession completion lost its request ID");
		Require(A.RequestPossession(Id, true), "Competing possession submission failed");
		WaitFor([&] { return HasPossessionResult(EventsA, true, false); });
		Require(
			!A.IsPossessed(Id) && !A.RequestPossession(Id, false),
			"Non-possessor could release another node's control");
		Require(
			A.GetSimulatedEntities().Contains(Id) && !B.GetSimulatedEntities().Contains(Id),
			"Possession changed simulation authority");
		Require(B.RequestPossession(Id, false), "Possession release submission failed");
		WaitFor([&] { return HasPossessionResult(EventsB, false, true) && !B.IsPossessed(Id); });
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

#if WITH_DEV_AUTOMATION_TESTS
bool FTestUtilities::TestDistributedSignals(FString& OutError)
{
	return WithDistributedTestManager(
		17592,
		[&]
		{
			FSimulationBarrier SimulationA, SimulationB;
			SimulationA.AllocateNative();
			SimulationB.AllocateNative();
			FDistributedApplicationBarrier A, B;
			auto Require = [](bool Condition, const char* Message)
			{
				if (!Condition)
					throw std::runtime_error(Message);
			};
			Require(
				A.Initialize(SimulationA, TEXT("tcp/127.0.0.1:17592")),
				"Client A initialization failed");
			Require(
				B.Initialize(SimulationB, TEXT("tcp/127.0.0.1:17592")),
				"Client B initialization failed");
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
				throw std::runtime_error("Timed out waiting for typed distributed signal delivery");
			};
			WaitFor([&] { return A.IsReady() && B.IsReady(); });
			const FGuid Request = FGuid::NewGuid();
			Require(
				A.EnsureEntity(Request, 0, FTransform::Identity, TEXT("typed-signals")),
				"Ensure failed");
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
			A.RegisterEntityBodies(Id, {}, FVector::ZeroVector);
			B.RegisterEntityBodies(Id, {}, FVector::ZeroVector);
			Require(A.RequestPossession(Id, true), "Possession submission failed");
			Require(A.RequestSimulationAuthority(Id, true), "Authority submission failed");
			WaitFor([&] { return A.IsPossessed(Id) && A.GetSimulatedEntities().Contains(Id); });

			const TMap<FString, FAGXDistributedSignalValue> Expected = {
				{TEXT("throttle"), 0.375},
				{TEXT("gear"), int64(-3)},
				{TEXT("integer_max"), int64(MAX_int64)},
				{TEXT("integer_min"), int64(MIN_int64)},
				{TEXT("enabled"), true},
				{TEXT("disabled"), false},
				{TEXT("range"), FVector2D(-123.5, 987.25)},
				{TEXT("vector"), FVector(1.25, -2.5, 3.75)}};
			auto Send = [&]
			{
				for (const auto& Signal : Expected)
					Require(
						A.RouteSignal(Id, Signal.Key, Signal.Value), "Signal submission failed");
			};
			auto Matches = [&](const TArray<FAGXDistributedEvent>& Events)
			{
				for (const auto& Signal : Expected)
					if (!Events.ContainsByPredicate(
							[&](const auto& Event)
							{
								return Event.Type == EAGXDistributedEventType::ControlSignal &&
									   Event.EntityId == Id && Event.SignalName == Signal.Key &&
									   Event.SignalValue == Signal.Value;
							}))
						return false;
				return true;
			};
			EventsA.Reset();
			Send();
			WaitFor([&] { return Matches(EventsA); });

			EventsB.Reset();
			Require(A.RequestSimulationAuthority(Id, false), "Authority release failed");
			WaitFor([&] { return !A.GetSimulatedEntities().Contains(Id); });
			Require(B.RequestSimulationAuthority(Id, true), "Remote authority request failed");
			WaitFor([&] { return B.GetSimulatedEntities().Contains(Id) && Matches(EventsB); });
			EventsB.Reset();
			Send();
			WaitFor([&] { return Matches(EventsB); });

			EventsA.Reset();
			Require(B.RequestSimulationAuthority(Id, false), "Remote release failed");
			WaitFor([&] { return !B.GetSimulatedEntities().Contains(Id); });
			Require(A.RequestSimulationAuthority(Id, true), "Local reacquisition failed");
			WaitFor([&] { return A.GetSimulatedEntities().Contains(Id) && Matches(EventsA); });
			A.Shutdown();
			B.Shutdown();
			return true;
		},
		OutError);
}
#endif

#if WITH_DEV_AUTOMATION_TESTS
static bool RunDistributedProximityTest(FString& OutError, bool bDelaySeparation)
{
	try
	{
		FSimulationBarrier SimulationA, SimulationB;
		SimulationA.AllocateNative();
		SimulationB.AllocateNative();
		SimulationA.GetNative()->Native->setUniformGravity(agx::Vec3());
		SimulationB.GetNative()->Native->setUniformGravity(agx::Vec3());
		agxDistributed::Node::Config Config;
		Config.addSetting("mode", "\"peer\"");
		Config.addSetting(
			"listen/endpoints",
			bDelaySeparation ? "[\"tcp/127.0.0.1:17597\"]" : "[\"tcp/127.0.0.1:17596\"]");
		Config.addSetting("scouting/multicast/enabled", "false");
		agx::ref_ptr<agxDistributed::WorldManager> Manager =
			new agxDistributed::WorldManager(agxDistributed::ScenarioInfo(), Config);
		Manager->start();
		FDistributedApplicationBarrier A, B;
		auto Check = [&](bool bCondition, const TCHAR* Message)
		{
			if (!bCondition)
				OutError = Message;
			return bCondition;
		};
		const FString Endpoint =
			bDelaySeparation ? TEXT("tcp/127.0.0.1:17597") : TEXT("tcp/127.0.0.1:17596");
		if (!Check(
				A.Initialize(SimulationA, Endpoint) && B.Initialize(SimulationB, Endpoint),
				TEXT("Proximity client initialization failed.")))
			return false;
		A.StartAndLoad(5);
		B.StartAndLoad(5);
		TArray<FAGXDistributedEvent> EventsA, EventsB;
		auto Wait = [&](TFunctionRef<bool()> Predicate)
		{
			const double Deadline = FPlatformTime::Seconds() + 8.0;
			while (FPlatformTime::Seconds() < Deadline)
			{
				A.Poll(EventsA);
				B.Poll(EventsB);
				if (Predicate())
					return true;
				FPlatformProcess::Sleep(0.005f);
			}
			return false;
		};
		if (!Check(
				Wait([&] { return A.IsReady() && B.IsReady(); }),
				TEXT("Proximity clients did not join.")))
			return false;
		const FGuid AssetRequest = FGuid::NewGuid();
		A.RegisterAsset(
			AssetRequest, TEXT("ProximityTest.openplx"), EAGX_DistributedAssetType::Possessable);
		uint64 AssetId = MAX_uint64;
		if (!Check(
				Wait([&] { return A.ResolveAsset(TEXT("ProximityTest.openplx"), AssetId); }),
				TEXT("Proximity asset registration failed.")))
			return false;
		const FGuid LeftRequest = FGuid::NewGuid(), RightRequest = FGuid::NewGuid();
		A.EnsureEntity(
			LeftRequest, AssetId, FTransform(FVector(-500, 0, 0)), TEXT("proximity-left"));
		B.EnsureEntity(
			RightRequest, AssetId, FTransform(FVector(500, 0, 0)), TEXT("proximity-right"));
		uint64 Left = MAX_uint64, Right = MAX_uint64;
		if (!Check(
				Wait(
					[&]
					{
						for (const auto* Events : {&EventsA, &EventsB})
							for (const auto& Event : *Events)
								if (Event.Type == EAGXDistributedEventType::SpawnRequestResult &&
									Event.bAccepted)
								{
									if (Event.RequestId == LeftRequest)
										Left = Event.EntityId;
									if (Event.RequestId == RightRequest)
										Right = Event.EntityId;
								}
						return Left != MAX_uint64 && Right != MAX_uint64;
					}),
				TEXT("Proximity entities did not spawn.")))
			return false;
		auto AddBody = [](FSimulationBarrier& Simulation, const FVector& Position)
		{
			FRigidBodyBarrier Body;
			Body.AllocateNative();
			Body.SetPosition(Position);
			Body.GetNative()->Native->add(
				new agxCollide::Geometry(new agxCollide::Box(1, 1, 1)),
				agx::AffineMatrix4x4::translate(agx::Vec3(0.4, 0.2, 0.1)));
			Simulation.GetNative()->Native->add(Body.GetNative()->Native);
			return Body;
		};
		auto ALeft = AddBody(SimulationA, FVector(-500, 0, 0));
		auto ARight = AddBody(SimulationA, FVector(500, 0, 0));
		auto BLeft = AddBody(SimulationB, FVector(-500, 0, 0));
		auto BRight = AddBody(SimulationB, FVector(500, 0, 0));
		A.RegisterEntityBodies(Left, {ALeft}, FVector(-500, 0, 0), 225.0);
		A.RegisterEntityBodies(Right, {ARight}, FVector(500, 0, 0), 225.0);
		B.RegisterEntityBodies(Left, {BLeft}, FVector(-500, 0, 0), 225.0);
		B.RegisterEntityBodies(Right, {BRight}, FVector(500, 0, 0), 225.0);
		if (!Check(
				ALeft.GetNative()->Native->getGeometries().size() == 2 &&
					ARight.GetNative()->Native->getGeometries().size() == 2,
				TEXT("Sensor geometries were not attached to the bodies.")))
			return false;

		// The configured 225 cm radius must reach AGX as exactly 2.25 m, without bounds padding.
		for (const auto* Body : {&ALeft, &ARight, &BLeft, &BRight})
			for (const auto& Geometry : Body->GetNative()->Native->getGeometries())
				if (Geometry->isSensor())
				{
					const auto* Sphere = Geometry->getShape()->as<agxCollide::Sphere>();
					if (!Check(
							Sphere != nullptr && FMath::IsNearlyEqual(Sphere->getRadius(), 2.25),
							TEXT("Sensor did not use the exact configured radius in AGX units.")))
						return false;
					if (!Check(
							agx::equivalent(
								Geometry->getPosition(), Body->GetNative()->Native->getPosition()),
							TEXT("Offset physics bounds moved the configured sensor center.")))
						return false;
				}

		// Registering twice must not add a second sphere.
		A.RegisterEntityBodies(Left, {ALeft}, FVector(-500, 0, 0), 225.0);
		if (!Check(
				ALeft.GetNative()->Native->getGeometries().size() == 2,
				TEXT("Sensor registration was not idempotent.")))
			return false;
		A.RequestSimulationAuthority(Left, true);
		B.RequestSimulationAuthority(Right, true);
		A.RequestPossession(Left, true);
		B.RequestPossession(Right, true);
		if (!Check(
				Wait(
					[&]
					{
						return A.GetSimulatedEntities().Contains(Left) &&
							   B.GetSimulatedEntities().Contains(Right) && A.IsPossessed(Left) &&
							   B.IsPossessed(Right);
					}),
				TEXT("Initial proximity roles were not assigned.")))
			return false;
		ALeft.SetMotionControl(EAGX_MotionControl::MC_DYNAMICS);
		ARight.SetMotionControl(EAGX_MotionControl::MC_KINEMATICS);
		BLeft.SetMotionControl(EAGX_MotionControl::MC_KINEMATICS);
		BRight.SetMotionControl(EAGX_MotionControl::MC_DYNAMICS);
		SimulationA.GetNative()->Native->stepForward();
		SimulationB.GetNative()->Native->stepForward();
		A.Poll(EventsA);
		B.Poll(EventsB);
		if (!Check(
				!A.GetSimulatedEntities().Contains(Right) &&
					!B.GetSimulatedEntities().Contains(Left),
				TEXT("Separated bodies were grouped.")))
			return false;

		// The preferred node is independent of which client submits the first overlap.
		const bool bPreferA = A.GetNodeId().Compare(B.GetNodeId(), ESearchCase::CaseSensitive) < 0;
		auto& Winner = bPreferA ? A : B;
		auto& Loser = bPreferA ? B : A;
		auto& WinnerSimulation = bPreferA ? SimulationA : SimulationB;
		auto& LoserSimulation = bPreferA ? SimulationB : SimulationA;
		auto& WinnerRight = bPreferA ? ARight : BRight;
		auto& LoserRight = bPreferA ? BRight : ARight;
		auto& WinnerEvents = bPreferA ? EventsA : EventsB;
		auto& LoserEvents = bPreferA ? EventsB : EventsA;
		auto OriginalOwners = [&]
		{
			return A.GetSimulatedEntities().Contains(Left) &&
				   !A.GetSimulatedEntities().Contains(Right) &&
				   B.GetSimulatedEntities().Contains(Right) &&
				   !B.GetSimulatedEntities().Contains(Left);
		};
		for (int32 Cycle = 0; Cycle < (bDelaySeparation ? 3 : 1); ++Cycle)
		{
			ALeft.SetMotionControl(EAGX_MotionControl::MC_DYNAMICS);
			ARight.SetMotionControl(EAGX_MotionControl::MC_KINEMATICS);
			BLeft.SetMotionControl(EAGX_MotionControl::MC_KINEMATICS);
			BRight.SetMotionControl(EAGX_MotionControl::MC_DYNAMICS);

			// Explicit 225 cm radii make sensors overlap at 3.8 m while the physical
			// boxes remain separated.
			ARight.SetPosition(FVector(-120, 0, 0));
			BRight.SetPosition(FVector(-120, 0, 0));
			if (!bDelaySeparation || Cycle == 0)
				WinnerSimulation.GetNative()->Native->stepForward();
			LoserSimulation.GetNative()->Native->stepForward();

			// Let the eventual losing client submit before it can lose authority.
			// Later cycles require its fresh impact to start the next merge.
			Loser.Poll(LoserEvents);
			if (!Check(
					Wait(
						[&]
						{
							const auto Simulated = Winner.GetSimulatedEntities();
							return Simulated.Contains(Left) && Simulated.Contains(Right) &&
								   Loser.GetSimulatedEntities().IsEmpty();
						}),
					TEXT("Sensor overlap did not move both entities to the preferred client.")))
				return false;
			if (!Check(
					A.IsPossessed(Left) && B.IsPossessed(Right),
					TEXT("Proximity transfer changed input possession.")))
				return false;

			ALeft.SetMotionControl(
				bPreferA ? EAGX_MotionControl::MC_DYNAMICS : EAGX_MotionControl::MC_KINEMATICS);
			ARight.SetMotionControl(
				bPreferA ? EAGX_MotionControl::MC_DYNAMICS : EAGX_MotionControl::MC_KINEMATICS);
			BLeft.SetMotionControl(
				bPreferA ? EAGX_MotionControl::MC_KINEMATICS : EAGX_MotionControl::MC_DYNAMICS);
			BRight.SetMotionControl(
				bPreferA ? EAGX_MotionControl::MC_KINEMATICS : EAGX_MotionControl::MC_DYNAMICS);
			if (bDelaySeparation && Cycle > 0)
			{
				WinnerSimulation.GetNative()->Native->stepForward();
				Winner.Poll(WinnerEvents);
			}

			WinnerRight.SetPosition(FVector(500, 0, 0));
			WinnerSimulation.GetNative()->Native->stepForward();
			if (!bDelaySeparation)
			{
				LoserRight.SetPosition(FVector(500, 0, 0));
				LoserSimulation.GetNative()->Native->stepForward();
			}
			if (!Check(
					Wait(OriginalOwners),
					TEXT("Sensor separation did not restore the original owners.")))
				return false;

			// The losing replica has not stepped: its overlap is deliberately stale
			// while authority returns. Network polling must not re-arm that overlap.
			if (bDelaySeparation)
			{
				const double ObserveUntil = FPlatformTime::Seconds() + 0.5;
				do
				{
					A.Poll(EventsA);
					B.Poll(EventsB);
					if (!Check(
							OriginalOwners(), TEXT(
												  "Stale overlap re-merged the entities after "
												  "authority was restored.")))
						return false;
					FPlatformProcess::Sleep(0.005f);
				} while (FPlatformTime::Seconds() < ObserveUntil);
				LoserRight.SetPosition(FVector(500, 0, 0));
				LoserSimulation.GetNative()->Native->stepForward();
				Loser.Poll(LoserEvents);
				if (!Check(
						OriginalOwners(), TEXT("Delayed separation changed restored authority.")))
					return false;
			}
		}
		A.UnregisterEntityBodies(Left);
		A.UnregisterEntityBodies(Right);
		A.UnregisterEntityBodies(Left);
		A.Poll(EventsA);
		if (!Check(
				ALeft.GetNative()->Native->getGeometries().size() == 1 &&
					ARight.GetNative()->Native->getGeometries().size() == 1,
				TEXT("Unregister did not detach sensor geometry.")))
			return false;
		B.Shutdown();
		if (!Check(
				BLeft.GetNative()->Native->getGeometries().size() == 1 &&
					BRight.GetNative()->Native->getGeometries().size() == 1,
				TEXT("Shutdown did not detach remaining sensors.")))
			return false;
		A.Shutdown();
		return true;
	}
	catch (const std::exception& Error)
	{
		OutError = UTF8_TO_TCHAR(Error.what());
		return false;
	}
}

bool FTestUtilities::TestDistributedProximity(FString& OutError)
{
	return RunDistributedProximityTest(OutError, false);
}

bool FTestUtilities::TestDistributedProximityDelayedSeparation(FString& OutError)
{
	return RunDistributedProximityTest(OutError, true);
}
#endif
