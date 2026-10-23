// Copyright 2026, Algoryx Simulation AB.

#include "Distributed/DistributedApplicationBarrier.h"

#include "AGX_LogCategory.h"
#include "SimulationBarrier.h"

#include <chrono>
#include <cstring>
#include <future>
#include <mutex>
#include <utility>
#include <vector>

#include "Utilities/TestUtilities.h"

#include "BarrierOnly/AGXRefs.h"
#include "BarrierOnly/AGXTypeConversions.h"

#include <BeginAGXIncludes.h>
#include <agxDistributed/AgxApplicationLayer.h>
#include <EndAGXIncludes.h>

namespace
{
	// Matches examples/agxDistributed/AssemblySerializer.h: big-endian uint64 count,
	// followed by 16 big-endian IEEE-754 doubles per body, in AGX matrix row order.
	bool DecodeBodyTransforms(const std::vector<uint8_t>& Data, TArray<FTransform>& OutTransforms)
	{
		constexpr size_t HeaderSize = 8;
		constexpr size_t MatrixSize = 16 * 8;
		if (Data.size() < HeaderSize)
			return false;

		size_t Offset = 0;
		auto ReadUint64 = [&Data, &Offset]()
		{
			uint64 Value = 0;
			for (int32 Byte = 0; Byte < 8; ++Byte)
				Value = (Value << 8) | Data[Offset++];
			return Value;
		};

		const uint64 Count = ReadUint64();
		// Validate before allocation or reading any matrix, without overflowing Count * MatrixSize.
		if ((Data.size() - HeaderSize) % MatrixSize != 0 ||
			Count != (Data.size() - HeaderSize) / MatrixSize || Count > MAX_int32)
			return false;

		OutTransforms.Reserve(static_cast<int32>(Count));
		for (uint64 Body = 0; Body < Count; ++Body)
		{
			agx::AffineMatrix4x4 Matrix;
			for (int32 RowIndex = 0; RowIndex < 4; ++RowIndex)
			{
				agx::Vec4 Row;
				for (int32 Column = 0; Column < 4; ++Column)
				{
					const uint64 Bits = ReadUint64();
					double Value;
					static_assert(sizeof(Value) == sizeof(Bits), "Expected 64-bit doubles");
					std::memcpy(&Value, &Bits, sizeof(Value));
					if (!FMath::IsFinite(Value))
						return false;
					Row[Column] = Value;
				}
				Matrix.setRow(RowIndex, Row);
			}
			const FTransform Transform = Convert(Matrix);
			if (Transform.ContainsNaN() || !Transform.GetRotation().IsNormalized())
				return false;
			OutTransforms.Add(Transform);
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

	class FUnrealApplicationLayer final : public agxDistributed::AgxApplicationLayer
	{
	public:
		FUnrealApplicationLayer(
			agxSDK::Simulation& Simulation, const agxDistributed::Node::Config& Config,
			std::shared_ptr<FEventQueue> InEvents)
			: AgxApplicationLayer(Simulation, Config)
			, Events(std::move(InEvents))
		{
		}

		void applySignal(
			agxDistributed::EntityId EntityId, const std::string& Name, double Value) override
		{
			// Q: Should signals be queued for the Unreal OpenPLX handler or applied through
			// a native handler during this AGX simulation callback?
			(void) EntityId;
			(void) Name;
			(void) Value;
			UE_LOG(
				LogAGX, Warning,
				TEXT("Unreal Application Layer received signal %s for entity %d: %f"),
				UTF8_TO_TCHAR(Name.c_str()), EntityId.getId(), Value);
		}

	protected:
		void instantiateAsset(
			agxDistributed::EntityId EntityId, agxDistributed::AssetId AssetId,
			agx::AffineMatrix4x4 AssetTransform) override
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

		void processIncomingEntityUpdates(
			std::vector<agxDistributed::EntityId>&& UpdatedEntities) override
		{
			for (const agxDistributed::EntityId Id : UpdatedEntities)
			{
				std::vector<uint8_t> AssetData;
				const bool Found = withEntity(Id, [&AssetData](const agxDistributed::Entity& Entity)
				{
					AssetData = Entity.getEntityData().getAssetData();
				});
				// Spawn requests may carry no body state yet. Leave the spawn transform intact.
				if (!Found || AssetData.empty())
					continue;

				FAGXDistributedEvent Event;
				Event.Type = EAGXDistributedEventType::EntityUpdate;
				Event.EntityId = Id.getId();
				if (!DecodeBodyTransforms(AssetData, Event.BodyTransforms))
				{
					Event.BodyTransforms.Reset();
					Event.Message = TEXT("Invalid AssemblySerializer body transform payload.");
				}
				// Also used by AGX's pre-collide listener: never touch UObjects here.
				Events->Push(MoveTemp(Event));
			}
		}

		void collectAndSendUpdates() override
		{
			// Q: Which native bodies belong to each entity, and when is their state safe to read?
		}

		void onSimulationControlMessage(bool bStart) override
		{
			FAGXDistributedEvent Event;
			Event.Type = bStart ? EAGXDistributedEventType::SimulationStart
								: EAGXDistributedEventType::SimulationPause;
			Events->Push(MoveTemp(Event));
		}

		void onShutdownControlMessage() override
		{
			FAGXDistributedEvent Event;
			Event.Type = EAGXDistributedEventType::ShutdownRequested;
			Events->Push(MoveTemp(Event));
		}

	private:
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
			Event.Message = It->bRequestedValue ? TEXT("acquire") : TEXT("release");
			try
			{
				Event.bAccepted = It->Future.get().second;
			}
			catch (const std::exception& Error)
			{
				Event.Message = FString(UTF8_TO_TCHAR(Error.what()));
			}
			Impl->Events->Push(MoveTemp(Event));
			It = Pending.erase(It);
		}
	};
	PollAssignments(Impl->Possessions, EAGXDistributedEventType::PossessionResult);
	PollAssignments(Impl->Authorities, EAGXDistributedEventType::SimulationAuthorityResult);
	if (Impl->bReady && Impl->Native != nullptr)
	{
		// Drain AGX's network queues on the game thread even when physics is paused.
		// Q: Should this happen before each AGX step instead of from the world tick?
		Impl->Native->triggerNewEntitiesProcessing();
		Impl->Native->triggerEntityUpdatesProcessing();
		Impl->Native->triggerSignalProcessing();
	}
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

bool FDistributedApplicationBarrier::RequestSpawn(
	const FString& AssetPath, const FTransform& Transform, bool bPossess)
{
	if (!IsReady() || AssetPath.IsEmpty())
		return false;
	try
	{
		// Q: What initial rigid-body state should the spawn blob contain?
		Impl->Spawns.push_back(
			{AssetPath,
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

void FDistributedApplicationBarrier::Shutdown()
{
	if (Impl->Native != nullptr)
	{
		Impl->Native->shutdownApplicationLayer();
		Impl->Native = nullptr;
	}
	Impl->bReady = false;
	Impl->Spawns.clear();
	Impl->Possessions.clear();
	Impl->Authorities.clear();
}
