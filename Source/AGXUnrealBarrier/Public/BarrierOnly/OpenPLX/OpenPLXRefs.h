#pragma once

// OpenPLX includes.
#include "BeginAGXIncludes.h"
#include "agxOpenPLX/AgxCache.h"
#include "agxOpenPLX/InputSignalListener.h"
#include "agxOpenPLX/OutputSignalListener.h"
#include "agxOpenPLX/SignalSourceMapper.h"
#include "Physics3D/System.h"
#include "EndAGXIncludes.h"

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
		agxSDK::Assembly* Assembly, std::shared_ptr<agxopenplx::InputSignalQueue> InputQueue, std::shared_ptr<agxopenplx::SignalSourceMapper>& Mapper)
		: Native(new agxopenplx::InputSignalListener(Assembly, InputQueue, Mapper))
	{
	}
};

struct FOutputSignalHandlerRef
{
	agx::ref_ptr<agxopenplx::OutputSignalListener> Native;

	FOutputSignalHandlerRef() = default;
	FOutputSignalHandlerRef(
		const std::shared_ptr<openplx::Core::Object>& PlxModel,
		std::shared_ptr<agxopenplx::OutputSignalQueue> OutputQueue,
		std::shared_ptr<agxopenplx::SignalSourceMapper> Mapper)
		: Native(new agxopenplx::OutputSignalListener(PlxModel, OutputQueue, Mapper))
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

struct FSignalSourceMapperRef
{
	std::shared_ptr<agxopenplx::SignalSourceMapper> Native;

	FSignalSourceMapperRef() = default;

	FSignalSourceMapperRef(std::shared_ptr<agxopenplx::SignalSourceMapper> InNative)
		: Native(InNative)
	{
	}

	FSignalSourceMapperRef(agxSDK::Assembly* Assembly)
		: Native(agxopenplx::SignalSourceMapper::create(Assembly))
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
