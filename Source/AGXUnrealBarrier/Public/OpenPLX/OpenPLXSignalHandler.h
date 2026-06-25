// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "OpenPLX/OpenPLXModelRegistry.h"

// Standard library includes.
#include <memory>

class FConstraintBarrier;
struct FRigidBodyBarrier;
class FSensorEnvironmentBarrier;
class FSimulationBarrier;

struct FAssemblyRef;
struct FControlInterfaceRef;
struct FHeapControlInterfacePtr;
struct FInputSignalListenerRef;
struct FInputSignalQueuePtr;
struct FOutputSignalListenerRef;
struct FOutputSignalQueuePtr;
struct FOpenPLXLidarOutputView;
struct FOpenPLX_Input;
struct FOpenPLX_Output;
struct FOpenPLX_SignalHandlerNativeAddresses;
struct FOpenPLXMappingBarriersCollection;

/**
 * FOpenPLXSignalHandler is responsible for communication between an UOpenPLXSignalHandlerComponent
 * and the underlying OpenPLX model.
 */
class AGXUNREALBARRIER_API FOpenPLXSignalHandler
{
public:
	FOpenPLXSignalHandler();

	void Init(
		const FString& OpenPLXFile, FSimulationBarrier& Simulation,
		FSensorEnvironmentBarrier* Environment,
		FOpenPLXModelRegistry& InModelRegistry, const FOpenPLXMappingBarriersCollection& Barriers);

	bool IsInitialized() const;

	// Real.
	bool Send(const FOpenPLX_Input& Input, double Value);
	bool SendInterface(const FOpenPLX_Input& Input, double Value);
	bool Receive(const FOpenPLX_Output& Output, double& OutValue);
	bool ReceiveInterface(const FOpenPLX_Output& Output, double& OutValue);

	// Ranges / Vec2.
	bool Send(const FOpenPLX_Input& Input, const FVector2D& Value);
	bool SendInterface(const FOpenPLX_Input& Input, const FVector2D& Value);
	bool Receive(const FOpenPLX_Output& Output, FVector2D& OutValue);
	bool ReceiveInterface(const FOpenPLX_Output& Output, FVector2D& OutValue);

	// Vectors / Vec3.
	bool Send(const FOpenPLX_Input& Input, const FVector& Value);
	bool SendInterface(const FOpenPLX_Input& Input, const FVector& Value);
	bool Receive(const FOpenPLX_Output& Output, FVector& OutValue);
	bool ReceiveInterface(const FOpenPLX_Output& Output, FVector& OutValue);

	// Integer.
	bool Send(const FOpenPLX_Input& Input, int64 Value);
	bool SendInterface(const FOpenPLX_Input& Input, int64 Value);
	bool Receive(const FOpenPLX_Output& Output, int64& OutValue);
	bool ReceiveInterface(const FOpenPLX_Output& Output, int64& OutValue);

	// Boolean.
	bool Send(const FOpenPLX_Input& Input, bool Value);
	bool SendInterface(const FOpenPLX_Input& Input, bool Value);
	bool Receive(const FOpenPLX_Output& Output, bool& OutValue);
	bool ReceiveInterface(const FOpenPLX_Output& Output, bool& OutValue);

	FHeapControlInterfacePtr GetHeapControlInterface();
	const FHeapControlInterfacePtr GetHeapControlInterface() const;

	/// Lidar outputs.
	bool ReceiveLidarOutput(const FOpenPLX_Output& Output, FOpenPLXLidarOutputView& OutOutput);

	/// IMU outputs.
	bool ReceiveIMUOutput(const FOpenPLX_Output& Output);

	void ReleaseNatives();

	void SetNativeAddresses(const FOpenPLX_SignalHandlerNativeAddresses& Addresses);
	FOpenPLX_SignalHandlerNativeAddresses GetNativeAddresses() const;

private:
	bool bIsInitialized {false};
	FOpenPLXModelRegistry* ModelRegistry {nullptr};
	FOpenPLXModelRegistry::Handle ModelHandle {FOpenPLXModelRegistry::InvalidHandle};

	std::shared_ptr<FAssemblyRef> AssemblyRef;

	// Queue-based signals.
	std::shared_ptr<FInputSignalListenerRef> InputSignalListenerRef;
	std::shared_ptr<FOutputSignalListenerRef> OutputSignalListenerRef;
};
