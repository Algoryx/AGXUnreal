// Copyright 2026, Algoryx Simulation AB.

#pragma once

// OpenPLX includes.
#include "BeginAGXIncludes.h"
#include "openplx/HeapControlInterface.h"
#include "openplx/Physics/Optics/Material.h"
#include "openplx/Physics3D/System.h"
#include "openplx/Sensors/Signals/CameraColorOutput.h"
#include "EndAGXIncludes.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxSensor/CameraColorOutput.h>
#include <agxSDK/Assembly.h>
#include "EndAGXIncludes.h"

// Standard library includes.
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

struct FHeapControlInterfacePtr
{
	openplx::HeapControlInterface* Native;
	FHeapControlInterfacePtr() = default;
	FHeapControlInterfacePtr(openplx::HeapControlInterface* InNative)
		: Native(InNative)
	{
	}
	operator openplx::HeapControlInterface*()
	{
		return Native;
	}
};

struct FOpenPLXMarshallingRef
{
	std::shared_ptr<openplx::Marshalling> Marshalling;

	FOpenPLXMarshallingRef() = default;
	FOpenPLXMarshallingRef(std::shared_ptr<openplx::Marshalling> InMarshalling)
		: Marshalling(std::move(InMarshalling))
	{
	}
};

struct FOpenPLXCameraColorOutputViewRef : public FOpenPLXMarshallingRef
{
	std::shared_ptr<openplx::Sensors::Signals::CameraColorOutput> CameraColorOutput;
};

/**
 * Runtime data owned by a particular OpenPLX signal-handler instance.
 *
 * An OpenPLX model can be instantiated more than once, so this data must be scoped to the AGX
 * Assembly of the particular instance instead of the parsed OpenPLX model.
 */
struct FOpenPLXSignalHandlerRuntimeData
{
	std::shared_ptr<openplx::HeapControlInterface> HeapControlInterface;

	/**
	 * Maps an OpenPLX CameraColorOutput to its corresponding native AGX camera output.
	 */
	std::unordered_map<
		const openplx::Sensors::Signals::CameraColorOutput*, agxSensor::CameraColorOutputRef>
		CameraColorOutputs;
};

struct FOpenPLXModelData
{
	openplx::Core::ObjectPtr OpenPLXModel;

	/**
	 * Signal-handler runtime data is stored here instead of in FOpenPLXSignalHandler because of
	 * Blueprint Reconstruction. FOpenPLXSignalHandler is part of UOpenPLX_SignalHandlerComponent
	 * and is thus destroyed during Blueprint Reconstruction. We usually solve this by using
	 * Instance Data to store the pointer address from the old Component and restore it in the new
	 * one, but this doesn't work with std::shared_ptr because they cannot be created from just a
	 * pointer where there may be other std::shared_ptrs already pointing to the object. For this
	 * reason the runtime data is owned by FOpenPLXModelData, which is not destroyed during
	 * Blueprint Reconstruction.
	 *
	 * This table is populated by each FOpenPLXSignalHandler's Init, and the entry removed by
	 * ReleaseNatives.
	 */
	std::unordered_map<agxSDK::Assembly*, FOpenPLXSignalHandlerRuntimeData> RuntimeDataByAssembly;
};

struct FOpenPLXModelDataArray
{
	std::vector<FOpenPLXModelData> ModelData;
};

struct FOpenPLXMaterialRef
{
	std::shared_ptr<openplx::Physics::Optics::Material> Native;

	FOpenPLXMaterialRef() = default;
	FOpenPLXMaterialRef(std::shared_ptr<openplx::Physics::Optics::Material> InNative)
		: Native(std::move(InNative))
	{
	}
};
