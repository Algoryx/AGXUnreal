// Copyright 2025, Algoryx Simulation AB.

#pragma once

#if AGXUNREAL_USE_OPENPLX

// AGX Dynamics for Unreal includes.
#include "OpenPLX/OpenPLXModelRegistry.h"

// Standard library includes.
#include <memory>

class FConstraintBarrier;
class FRigidBodyBarrier;
class FSimulationBarrier;

struct FAssemblyRef;
struct FInputSignalListenerRef;
struct FInputSignalQueueRef;
struct FOutputSignalListenerRef;
struct FOutputSignalQueueRef;
struct FOpenPLX_Input;
struct FOpenPLX_Output;
struct FSignalSourceMapperRef;


class AGXUNREALBARRIER_API FOpenPLXSignalHandler
{
public:
	FOpenPLXSignalHandler();

	void Init(
		const FString& OpenPLXFile, FSimulationBarrier& Simulation, FOpenPLXModelRegistry& InModelRegistry,
		TArray<FRigidBodyBarrier*>& Bodies, TArray<FConstraintBarrier*>& Constraints);

	bool IsInitialized() const;

	/// Scalars.
	bool Send(const FOpenPLX_Input& Input, double Value);
	bool Receive(const FOpenPLX_Output& Output, double& OutValue);

	/// Ranges (Vec2 real).
	bool Send(const FOpenPLX_Input& Input, const FVector2D& Value);
	bool Receive(const FOpenPLX_Output& Output, FVector2D& OutValue);

	/// FVectors (Vec3 real).
	bool Send(const FOpenPLX_Input& Input, const FVector& Value);
	bool Receive(const FOpenPLX_Output& Output, FVector& OutValue);

	/// Integers.
	bool Send(const FOpenPLX_Input& Input, int64 Value);
	bool Receive(const FOpenPLX_Output& Output, int64& OutValue);

	/// Booleans.
	bool Send(const FOpenPLX_Input& Input, bool Value);
	bool Receive(const FOpenPLX_Output& Output, bool& OutValue);

private:
	bool bIsInitialized {false};
	FOpenPLXModelRegistry* ModelRegistry {nullptr};
	FOpenPLXModelRegistry::Handle ModelHandle {FOpenPLXModelRegistry::InvalidHandle};

	std::shared_ptr<FAssemblyRef> AssemblyRef;
	std::shared_ptr<FInputSignalListenerRef> InputSignalListenerRef;
	std::shared_ptr<FOutputSignalListenerRef> OutputSignalListenerRef;
	std::shared_ptr<FSignalSourceMapperRef> SignalSourceMapper;
	std::shared_ptr<FInputSignalQueueRef> InputQueueRef;
	std::shared_ptr<FOutputSignalQueueRef> OutputQueueRef;
};

#endif
