// Copyright 2026, Algoryx Simulation AB.

#pragma once

// OpenPLX includes.
#include "BeginAGXIncludes.h"
#include "openplx/HeapControlInterface.h"
#include "openplx/Physics/Optics/Material.h"
#include "openplx/Physics3D/System.h"
#include "EndAGXIncludes.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
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

struct FOpenPLXModelData
{
	openplx::Core::ObjectPtr OpenPLXModel;

	/**
	 * HeapControlInterface pointers are stored here instead of in FOpenPLXSignalHandler because of
	 * Blueprint Reconstruction. FOpenPLXSignalHandler is part of UOpenPLX_SignalHandlerComponent
	 * and is thus destroyed during Blueprint Reconstruction. We usually solve this by using
	 * Instance Data to store the pointer address from the old Component and restore it in the new
	 * one, but this doesn't work with std::shared_ptr because they cannot be created from just a
	 * pointer where there may be other std::shared_ptrs already pointing to the object. For this
	 * reason we moved ownership of the HeapControlInterface from FOpenPLXSignalHander, which is
	 * destroyed, to FOpenPLXModelData, which is not destroyed during Blueprint Reconstruction.
	 *
	 * This table is populated by each FOpenPLXSignalHanderl's Init, and the entry removed by
	 * ReleaseNatives.
	 *
	 * Using agxSDK::Assembly as a key into the map is a bit weird and using something more
	 * obviously associated with a particular FOpenPLXSignalHandler would be better. Not sure what
	 * that would be.
	 */
	std::unordered_map<agxSDK::Assembly*, std::shared_ptr<openplx::HeapControlInterface>>
		HeapControlInterfaces;
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
