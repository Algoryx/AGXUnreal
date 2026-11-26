// Copyright 2026, Algoryx Simulation AB.

#include "DistributedProximitySensors.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "BarrierOnly/AGXRefs.h"
#include "BarrierOnly/AGXTypeConversions.h"

// Unreal Engine includes.
#include "HAL/PlatformTime.h"

// Standard library includes.
#include <algorithm>
#include <chrono>
#include <future>
#include <map>
#include <mutex>
#include <tuple>
#include <utility>
#include <vector>

// AGX Dynamics includes.
#include <BeginAGXIncludes.h>
#include <agx/RigidBody.h>
#include <agxCollide/Geometry.h>
#include <agxCollide/Sphere.h>
#include <agxDistributed/Client.h>
#include <agxSDK/ContactEventListener.h>
#include <agxSDK/Simulation.h>
#include <EndAGXIncludes.h>

struct FDistributedProximitySensors::FImpl
{
	using FPair = std::pair<uint64, uint64>;
	struct FSensor
	{
		agx::RigidBodyRef Body;
		agxCollide::GeometryRef Geometry;
	};
	struct FPairState
	{
		bool bOverlapping {false};
		bool bMergeAccepted {false};
		double NextAttempt {0.0};
		std::future<std::tuple<agxDistributed::EntityId, agxDistributed::EntityId, bool>> Merge;
		std::future<void> Split;
	};
	class FSensorFilter final : public agxSDK::ExecuteFilter
	{
	public:
		bool match(const agxCollide::Geometry* A, const agxCollide::Geometry* B) const override
		{
			return A->isSensor() && B->isSensor();
		}
		using agxSDK::ExecuteFilter::match;
	};
	class FListener final : public agxSDK::ContactEventListener
	{
	public:
		explicit FListener(FImpl& InOwner)
			: ContactEventListener(IMPACT | SEPARATION, new FSensorFilter())
			, Owner(InOwner)
		{
			setName("AGXUnreal/DistributedProximity");
		}
		KeepContactPolicy impact(
			const agx::TimeStamp&, agxCollide::GeometryContact* Contact) override
		{
			Owner.Record(Contact->geometry(0), Contact->geometry(1), true);
			return KEEP_CONTACT;
		}
		void separation(const agx::TimeStamp&, agxCollide::GeometryPair& Pair) override
		{
			Owner.Record(Pair.first, Pair.second, false);
		}

	private:
		FImpl& Owner;
	};

	FImpl(agxSDK::Simulation& InSimulation, agxDistributed::Client& InClient)
		: Simulation(&InSimulation)
		, Client(InClient)
		, Listener(new FListener(*this))
	{
		Simulation->add(Listener);
	}
	~FImpl()
	{
		Reset();
	}

	// Collision callbacks record IDs only. Network requests run later at a safe point.
	void Record(const agxCollide::Geometry* A, const agxCollide::Geometry* B, bool bOverlapping)
	{
		std::lock_guard<std::mutex> Lock(Mutex);
		const auto First = GeometryEntities.find(A), Second = GeometryEntities.find(B);
		if (First == GeometryEntities.end() || Second == GeometryEntities.end() ||
			First->second == Second->second)
			return;
		const auto Pair = std::minmax(First->second, Second->second);
		Contacts.emplace_back(FPair(Pair.first, Pair.second), bOverlapping);
	}

	bool Register(
		uint64 Id, const TArray<FRigidBodyBarrier>& Bodies, const FVector& Center, double Radius)
	{
		if (Sensors.contains(Id))
			return true;
		if (!FMath::IsFinite(Radius) || Radius < 0 || Center.ContainsNaN())
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Invalid distributed proximity radius or center for entity %llu."), Id);
			return false;
		}
		bool bSimulatedAsset = false;
		Client.withEntity(
			agxDistributed::EntityId(Id), [&](const agxDistributed::Entity& Entity)
			{ bSimulatedAsset = Client.getScenarioInfo().isSimulated(Entity.getAssetId()); });
		if (!bSimulatedAsset || Bodies.IsEmpty())
			return true;
		agx::RigidBodyRef HeavyBody;
		for (const auto& Barrier : Bodies)
		{
			if (!Barrier.HasNative())
				return false;
			const auto& Body = Barrier.GetNative()->Native;
			if (HeavyBody == nullptr ||
				Body->getMassProperties()->getMass() > HeavyBody->getMassProperties()->getMass())
				HeavyBody = Body;
		}
		if (HeavyBody == nullptr)
			return true;
		const double NativeRadius = ConvertDistanceToAGX(Radius);
		agxCollide::GeometryRef Sensor =
			new agxCollide::Geometry(new agxCollide::Sphere(NativeRadius));
		Sensor->setName("AGXUnrealDistributedProximity");
		Sensor->setSensor(true, false);
		Sensor->setEnableMassProperties(false);
		const auto LocalCenter = ConvertDisplacement(Center) * HeavyBody->getTransform().inverse();
		HeavyBody->add(Sensor, agx::AffineMatrix4x4::translate(LocalCenter));
		Simulation->add(Sensor);
		Sensors.emplace(Id, FSensor {HeavyBody, Sensor});
		{
			std::lock_guard<std::mutex> Lock(Mutex);
			GeometryEntities.emplace(Sensor.get(), Id);
		}
		UE_LOG(
			LogAGX, Log, TEXT("Distributed proximity sensor for entity %llu: radius %.1f cm."), Id,
			Radius);
		return true;
	}

	void Unregister(uint64 Id)
	{
		const auto It = Sensors.find(Id);
		if (It == Sensors.end())
			return;
		{
			std::lock_guard<std::mutex> Lock(Mutex);
			GeometryEntities.erase(It->second.Geometry.get());
		}
		Simulation->remove(It->second.Geometry);
		It->second.Body->remove(It->second.Geometry);
		Sensors.erase(It);
		for (auto Pair = Pairs.begin(); Pair != Pairs.end();)
			if (Pair->first.first == Id || Pair->first.second == Id)
				Pair = Pairs.erase(Pair);
			else
				++Pair;
	}

	void Poll()
	{
		using namespace std::chrono_literals;
		std::vector<std::pair<FPair, bool>> Pending;
		{
			std::lock_guard<std::mutex> Lock(Mutex);
			Pending.swap(Contacts);
		}
		for (const auto& Contact : Pending)
			if (Sensors.contains(Contact.first.first) && Sensors.contains(Contact.first.second))
			{
				auto& State = Pairs[Contact.first];
				if (Contact.second && !State.bOverlapping)
					State.bMergeAccepted = false;
				State.bOverlapping = Contact.second;
			}

		const double Now = FPlatformTime::Seconds();
		for (auto It = Pairs.begin(); It != Pairs.end();)
		{
			const auto [First, Second] = It->first;
			auto& State = It->second;
			try
			{
				if (State.Merge.valid() && State.Merge.wait_for(0ms) == std::future_status::ready)
				{
					State.bMergeAccepted = std::get<2>(State.Merge.get());
					State.NextAttempt = Now + (State.bMergeAccepted ? 0.0 : 1.0);
				}
				if (State.Split.valid() && State.Split.wait_for(0ms) == std::future_status::ready)
				{
					State.Split.get();
					State.bMergeAccepted = false;
				}
				if (!State.Merge.valid() && !State.Split.valid() && Now >= State.NextAttempt)
				{
					const bool bFirst = Client.isSimulated(agxDistributed::EntityId(First));
					const bool bSecond = Client.isSimulated(agxDistributed::EntityId(Second));

					// An accepted overlap stays handled until a fresh impact, even if
					// a split returns authority before the local separation is processed.
					if (State.bOverlapping && !State.bMergeAccepted && (bFirst || bSecond))
					{
						State.Merge = Client.requestSameSimulationAuthority(
							agxDistributed::EntityId(First), agxDistributed::EntityId(Second));
						UE_LOG(
							LogAGX, Log,
							TEXT(
								"Distributed proximity: requesting common simulation authority for "
								"%llu and %llu."),
							First, Second);
					}
					else if (!State.bOverlapping && State.bMergeAccepted && bFirst && bSecond)
					{
						State.Split = Client.requestSplitSimulationAuthority(
							agxDistributed::EntityId(First), agxDistributed::EntityId(Second));
						UE_LOG(
							LogAGX, Log,
							TEXT(
								"Distributed proximity: requesting separate simulation authority "
								"for %llu and %llu."),
							First, Second);
					}
				}
			}
			catch (const std::exception& Error)
			{
				State.NextAttempt = Now + 1.0;
				UE_LOG(
					LogAGX, Warning,
					TEXT("Distributed proximity request for %llu and %llu failed: %s"), First,
					Second, UTF8_TO_TCHAR(Error.what()));
			}
			if (!State.bOverlapping && !State.bMergeAccepted && !State.Merge.valid() &&
				!State.Split.valid())
				It = Pairs.erase(It);
			else
				++It;
		}
	}

	void Reset()
	{
		if (Listener != nullptr)
		{
			Simulation->remove(Listener);
			Listener = nullptr;
		}
		while (!Sensors.empty())
			Unregister(Sensors.begin()->first);
		Pairs.clear();
		std::lock_guard<std::mutex> Lock(Mutex);
		Contacts.clear();
	}

	agxSDK::SimulationRef Simulation;
	agxDistributed::Client& Client;
	agx::ref_ptr<FListener> Listener;
	std::map<uint64, FSensor> Sensors;
	std::map<const agxCollide::Geometry*, uint64> GeometryEntities;
	std::map<FPair, FPairState> Pairs;
	std::vector<std::pair<FPair, bool>> Contacts;
	std::mutex Mutex;
};

FDistributedProximitySensors::FDistributedProximitySensors(
	agxSDK::Simulation& Simulation, agxDistributed::Client& Client)
	: Impl(std::make_unique<FImpl>(Simulation, Client))
{
}
FDistributedProximitySensors::~FDistributedProximitySensors() = default;
bool FDistributedProximitySensors::Register(
	uint64 Id, const TArray<FRigidBodyBarrier>& Bodies, const FVector& Center, double Radius)
{
	return Impl->Register(Id, Bodies, Center, Radius);
}
void FDistributedProximitySensors::Unregister(uint64 Id)
{
	Impl->Unregister(Id);
}
void FDistributedProximitySensors::Poll()
{
	Impl->Poll();
}
void FDistributedProximitySensors::Reset()
{
	Impl->Reset();
}
