// Copyright 2026, Algoryx Simulation AB.

#pragma once

#include "CoreMinimal.h"
#include "RigidBodyBarrier.h"

#include <memory>

namespace agxDistributed
{
	class Client;
}
namespace agxSDK
{
	class Simulation;
}

/** Native sensor lifetime and grouping requests, independent of Actors and assemblies.
 * Register, Unregister, Poll and Reset must run outside simulation stepping. */
class FDistributedProximitySensors
{
public:
	FDistributedProximitySensors(agxSDK::Simulation& Simulation, agxDistributed::Client& Client);
	~FDistributedProximitySensors();
	bool Register(
		uint64 EntityId, const TArray<FRigidBodyBarrier>& Bodies, const FVector& Center,
		double Radius);
	void Unregister(uint64 EntityId);
	void Poll();
	void Reset();

private:
	struct FImpl;
	std::unique_ptr<FImpl> Impl;
};
