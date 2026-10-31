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
struct FHeapControlInterfacePtr;
struct FOpenPLX_Input;
struct FOpenPLXIMUOutputView;
struct FOpenPLXLidarOutputView;
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
	bool Send(const FOpenPLX_Input& Input, double Value, bool bConvertUnits = true);
	bool Receive(const FOpenPLX_Output& Output, double& OutValue, bool bConvertUnits = true);

	// Ranges / Vec2.
	bool Send(const FOpenPLX_Input& Input, const FVector2D& Value, bool bConvertUnits = true);
	bool Receive(const FOpenPLX_Output& Output, FVector2D& OutValue, bool bConvertUnits = true);

	// Vectors / Vec3.
	bool Send(const FOpenPLX_Input& Input, const FVector& Value, bool bConvertUnits = true);
	bool Receive(const FOpenPLX_Output& Output, FVector& OutValue, bool bConvertUnits = true);

	// Integer.
	bool Send(const FOpenPLX_Input& Input, int64 Value, bool bConvertUnits = true);
	bool Receive(const FOpenPLX_Output& Output, int64& OutValue, bool bConvertUnits = true);

	// Boolean.
	bool Send(const FOpenPLX_Input& Input, bool Value, bool bConvertUnits = true);
	bool Receive(const FOpenPLX_Output& Output, bool& OutValue, bool bConvertUnits = true);

	FHeapControlInterfacePtr GetHeapControlInterface();
	const FHeapControlInterfacePtr GetHeapControlInterface() const;

	/// Lidar outputs.
	bool ReceiveLidarOutput(
		const FOpenPLX_Output& Output, FOpenPLXLidarOutputView& OutOutput,
		bool bConvertUnits = true);

	/// IMU outputs.
	bool ReceiveIMUOutput(
		const FOpenPLX_Output& Output, FOpenPLXIMUOutputView& OutOutput, bool bConvertUnits = true);

	void ReleaseNatives();

	void SetNativeAddresses(const FOpenPLX_SignalHandlerNativeAddresses& Addresses);
	FOpenPLX_SignalHandlerNativeAddresses GetNativeAddresses() const;

private:
	bool bIsInitialized {false};
	FOpenPLXModelRegistry* ModelRegistry {nullptr};
	FOpenPLXModelRegistry::Handle ModelHandle {FOpenPLXModelRegistry::InvalidHandle};

	std::shared_ptr<FAssemblyRef> AssemblyRef;
};
