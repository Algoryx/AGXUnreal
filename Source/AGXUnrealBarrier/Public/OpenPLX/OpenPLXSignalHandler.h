// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "OpenPLX/OpenPLXModelRegistry.h"

// Standard library includes.
#include <memory>

class FConstraintBarrier;
class FRigidBodyBarrier;
class FSimulationBarrier;

struct FAssemblyRef;
struct FInputSignalListenerRef;
struct FInputSignalQueuePtr;
struct FOutputSignalListenerRef;
struct FOutputSignalQueuePtr;
struct FOpenPLX_Input;
struct FOpenPLX_Output;
struct FOpenPLX_SignalHandlerNativeAddresses;

class AGXUNREALBARRIER_API FOpenPLXSignalHandler
{
public:
	FOpenPLXSignalHandler();

	void Init(
		const FString& OpenPLXFile, FSimulationBarrier& Simulation,
		FOpenPLXModelRegistry& InModelRegistry, TArray<FRigidBodyBarrier*>& Bodies,
		TArray<FConstraintBarrier*>& Constraints);

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

	void ReleaseNatives();

	void SetNativeAddresses(const FOpenPLX_SignalHandlerNativeAddresses& Addresses);
	FOpenPLX_SignalHandlerNativeAddresses GetNativeAddresses() const;

private:
	bool bIsInitialized {false};
	FOpenPLXModelRegistry* ModelRegistry {nullptr};
	FOpenPLXModelRegistry::Handle ModelHandle {FOpenPLXModelRegistry::InvalidHandle};

	std::shared_ptr<FAssemblyRef> AssemblyRef;
	std::shared_ptr<FInputSignalListenerRef> InputSignalListenerRef;
	std::shared_ptr<FOutputSignalListenerRef> OutputSignalListenerRef;
};
