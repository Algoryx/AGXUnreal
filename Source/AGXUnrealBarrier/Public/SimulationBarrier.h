#pragma once

#include <memory>

struct FSimulationRef;

class FRigidBodyBarrier;

class AGXUNREALBARRIER_API FSimulationBarrier
{
public:
	FSimulationBarrier();
	~FSimulationBarrier();

	void AddRigidBody(FRigidBodyBarrier* body);
	void Step();

	bool HasNative() const;
	void AllocateNative();
	FSimulationRef* GetNative();
	const FSimulationRef* GetNative() const;
	void ReleaseNative();

private:
	FSimulationBarrier(const FSimulationBarrier&) = delete;
	void operator=(const FSimulationBarrier&) = delete;
private:
	std::unique_ptr<FSimulationRef> NativeRef;
};