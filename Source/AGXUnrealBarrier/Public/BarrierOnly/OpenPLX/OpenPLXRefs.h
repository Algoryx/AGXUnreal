#pragma once

#if AGXUNREAL_USE_OPENPLX

// OpenPLX includes.
#include "BeginAGXIncludes.h"
#include "agxOpenPLX/AgxCache.h"
#include "agxOpenPLX/InputSignalListener.h"
#include "agxOpenPLX/OutputSignalListener.h"
#include "agxOpenPLX/SignalSourceMapper.h"
#include "openplx/Physics3D/System.h"
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

struct FInputSignalListenerRef
{
	agx::ref_ptr<agxopenplx::InputSignalListener> Native;

	FInputSignalListenerRef() = default;
	FInputSignalListenerRef(
		agxSDK::Assembly* Assembly, std::shared_ptr<agxopenplx::InputSignalQueue> InputQueue, std::shared_ptr<agxopenplx::SignalSourceMapper>& Mapper)
		: Native(new agxopenplx::InputSignalListener(Assembly, InputQueue, Mapper))
	{
	}
};

struct FOutputSignalListenerRef
{
	agx::ref_ptr<agxopenplx::OutputSignalListener> Native;

	FOutputSignalListenerRef() = default;
	FOutputSignalListenerRef(
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

struct FOpenPLXModelData
{
	openplx::Core::ObjectPtr OpenPLXModel;
	std::unordered_map<std::string, std::shared_ptr<openplx::Physics::Signals::Input>> Inputs;
};

struct FOpenPLXModelDataArray
{
	std::vector<FOpenPLXModelData> ModelData;
};

#endif
