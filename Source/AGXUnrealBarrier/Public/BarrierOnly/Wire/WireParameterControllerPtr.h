// Copyright 2025, Algoryx Simulation AB.

#pragma once

#include "BeginAGXIncludes.h"
#include <agxWire/Wire.h>
#include <agxWire/WireParameterController.h>
#include "EndAGXIncludes.h"

struct FWireParameterControllerPtr
{
	// The AGX Dynamics Wire that owns the WireParameterController
	agxWire::WireObserver NativeWire {nullptr};

	/**
	 * Get the native AGX Dynamics Wire Parameter Controller.
	 *
	 * Note that FWireParameterControllerPtr is non-owning, meaning that the returned pointer is
	 * only valid for as long as whatever FWireBarrier was used to create the
	 * FWireParameterControllerPtr remains valid.
	 *
	 * @return The Wire Parameter Controller, or nullptr if the wire has been destroyed.
	 */
	agxWire::WireParameterController* GetNative()
	{
		if (!NativeWire.isValid())
			return nullptr;
		return NativeWire->getParameterController();
	}

	FWireParameterControllerPtr() = default;
	FWireParameterControllerPtr(agxWire::Wire* InNative)
		: NativeWire(InNative)
	{
	}
};
