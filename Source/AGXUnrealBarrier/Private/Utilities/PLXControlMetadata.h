// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "OpenPLX/OpenPLX_Inputs.h"
#include "OpenPLX/OpenPLX_Outputs.h"

namespace openplx::Physics::Signals
{
	class Input;
	class Output;
}

namespace PLXControlMetadata
{
	/** Populate field layout, virtual control type and effective signal type from a native control. */
	void Resolve(const openplx::Physics::Signals::Input& Native, FOpenPLX_Input& Input);
	void Resolve(const openplx::Physics::Signals::Output& Native, FOpenPLX_Output& Output);
}
