// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "OpenPLX/PLX_Inputs.h"
#include "OpenPLX/PLX_Outputs.h"

// OpenPLX includes.
#include "BeginAGXIncludes.h"
#include "openplx/Object.h"
#include "agxOpenPLX/AgxCache.h"
#include "EndAGXIncludes.h"

namespace openplx
{
	namespace Physics3D
	{
		class System;
	}
}

class FPLXUtilities
{
public:
	static openplx::Core::ObjectPtr LoadModel(
		const FString& Filename, std::shared_ptr<agxopenplx::AgxCache> AGXCache);

	static bool HasInputs(openplx::Physics3D::System* System);
	static bool HasOutputs(openplx::Physics3D::System* System);

	static TArray<TUniquePtr<FPLX_Input>> GetInputs(openplx::Physics3D::System* System);
	static TArray<TUniquePtr<FPLX_Output>> GetOutputs(openplx::Physics3D::System* System);
};
