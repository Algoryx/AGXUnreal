#pragma once

// OpenPLX includes.
#include "OpenPLX/agx-openplx/AgxCache.h"
#include "OpenPLX/agx-openplx/InputSignalListener.h"
#include "OpenPLX/agx-openplx/OutputSignalListener.h"
#include "OpenPLX/Physics3D/System.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agx/ref_ptr.h>
#include <agxSDK/Assembly.h>
#include "EndAGXIncludes.h"

// Standard library includes.
#include <memory>
#include <vector>
#include <unordered_map>

struct FInputSignalHandlerRef
{
	agx::ref_ptr<agxopenplx::InputSignalListener> Native;

	FInputSignalHandlerRef() = default;
	FInputSignalHandlerRef(
		agxSDK::Assembly* Assembly, std::shared_ptr<agxopenplx::InputSignalQueue> InputQueue)
		: Native(new agxopenplx::InputSignalListener(Assembly, InputQueue))
	{
	}
};

struct FOutputSignalHandlerRef
{
	agx::ref_ptr<agxopenplx::OutputSignalListener> Native;

	FOutputSignalHandlerRef() = default;
	FOutputSignalHandlerRef(
		agxSDK::Assembly* Assembly, const std::shared_ptr<openplx::Core::Object>& PlxModel,
		std::shared_ptr<agxopenplx::OutputSignalQueue> OutputQueue)
		: Native(new agxopenplx::OutputSignalListener(Assembly, PlxModel, OutputQueue))
	{
	}
};

struct FInputSignalQueueRef
{
	std::shared_ptr<agxopenplx::InputSignalQueue> Native;
	FInputSignalQueueRef() = default;
	FInputSignalQueueRef(std::shared_ptr<agxopenplx::InputSignalQueue> InNative)
		: Native(InNative)
	{
	}
};

struct FOutputSignalQueueRef
{
	std::shared_ptr<agxopenplx::OutputSignalQueue> Native;
	FOutputSignalQueueRef() = default;
	FOutputSignalQueueRef(std::shared_ptr<agxopenplx::OutputSignalQueue> InNative)
		: Native(InNative)
	{
	}
};

struct FPLXModelData
{
	std::shared_ptr<agxopenplx::AgxCache> AGXCache;
	openplx::Core::ObjectPtr PLXModel;
	std::unordered_map<std::string, std::shared_ptr<openplx::Physics::Signals::Input>> Inputs;
};

struct FPLXModelDataArray
{
	std::vector<FPLXModelData> ModelData;
};
