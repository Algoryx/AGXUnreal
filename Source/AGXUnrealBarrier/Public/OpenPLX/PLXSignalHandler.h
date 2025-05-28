// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "OpenPLX/PLXModelRegistry.h"

// Standard library includes.
#include <memory>

class FConstraintBarrier;
class FRigidBodyBarrier;
class FSimulationBarrier;

struct FAssemblyRef;
struct FInputSignalHandlerRef;
struct FInputSignalQueueRef;
struct FOutputSignalHandlerRef;
struct FOutputSignalQueueRef;
struct FPLX_Input;
struct FPLX_Output;
struct FSignalSourceMapperRef;


class AGXUNREALBARRIER_API FPLXSignalHandler
{
public:
	FPLXSignalHandler();

	void Init(
		const FString& PLXFile, FSimulationBarrier& Simulation, FPLXModelRegistry& InModelRegistry,
		TArray<FRigidBodyBarrier*>& Bodies, TArray<FConstraintBarrier*>& Constraints);

	bool IsInitialized() const;

	/// Scalars.
	bool Send(const FPLX_Input& Input, double Value);
	bool Receive(const FPLX_Output& Output, double& OutValue);

	/// Ranges (Vec2 real).
	bool Send(const FPLX_Input& Input, const FVector2D& Value);
	bool Receive(const FPLX_Output& Output, FVector2D& OutValue);

	/// FVectors (Vec3 real).
	bool Send(const FPLX_Input& Input, const FVector& Value);
	bool Receive(const FPLX_Output& Output, FVector& OutValue);

	/// Integers.
	bool Send(const FPLX_Input& Input, int64 Value);
	bool Receive(const FPLX_Output& Output, int64& OutValue);

	/// Booleans.
	bool Send(const FPLX_Input& Input, bool Value);
	bool Receive(const FPLX_Output& Output, bool& OutValue);

private:
	bool bIsInitialized {false};
	FPLXModelRegistry* ModelRegistry {nullptr};
	FPLXModelRegistry::Handle ModelHandle {FPLXModelRegistry::InvalidHandle};

	std::shared_ptr<FAssemblyRef> AssemblyRef;
	std::shared_ptr<FInputSignalHandlerRef> InputSignalHandlerRef;
	std::shared_ptr<FOutputSignalHandlerRef> OutputSignalHandlerRef;
	std::shared_ptr<FSignalSourceMapperRef> SignalSourceMapper;
	std::shared_ptr<FInputSignalQueueRef> InputQueueRef;
	std::shared_ptr<FOutputSignalQueueRef> OutputQueueRef;
};
