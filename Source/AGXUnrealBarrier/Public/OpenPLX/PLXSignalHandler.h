// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "OpenPLX/PLXModelRegistry.h"

// Standard library includes.
#include <memory>

class FConstraintBarrier;
class FSimulationBarrier;

struct FAssemblyRef;
struct FInputSignalHandlerRef;
struct FOutputSignalHandlerRef;
struct FInputSignalQueueRef;
struct FOutputSignalQueueRef;
struct FPLX_AngleOutput;
struct FPLX_LinearVelocity1DInput;

class AGXUNREALBARRIER_API FPLXSignalHandler
{
public:
	FPLXSignalHandler();

	void Init(
		const FString& PLXFile, FSimulationBarrier& Simulation, FPLXModelRegistry& InModelRegistry,
		TArray<FConstraintBarrier*>& Constraints);

	bool IsInitialized() const;

	bool Send(const FPLX_LinearVelocity1DInput& Input, double Value);

	bool Receive(const FPLX_AngleOutput& Output, double& OutValue);

private:
	bool bIsInitialized {false};
	FPLXModelRegistry* ModelRegistry {nullptr};
	FPLXModelRegistry::Handle ModelHandle {FPLXModelRegistry::InvalidHandle};

	std::shared_ptr<FAssemblyRef> AssemblyRef;
	std::shared_ptr<FInputSignalHandlerRef> InputSignalHandlerRef;
	std::shared_ptr<FOutputSignalHandlerRef> OutputSignalHandlerRef;
	std::shared_ptr<FInputSignalQueueRef> InputQueueRef;
	std::shared_ptr<FOutputSignalQueueRef> OutputQueueRef;
};
