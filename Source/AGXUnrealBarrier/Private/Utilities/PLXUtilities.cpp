// Copyright 2024, Algoryx Simulation AB.

#include "Utilities/PLXUtilities.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Environment.h"
#include "TypeConversions.h"

// OpenPLX includes.
#include "openplx/OpenPlxContext.h"
#include "openplx/OpenPlxContextInternal.h"
#include "openplx/OpenPlxCoreAPI.h"
#include "agxOpenPLX/AgxOpenPlxApi.h"
#include "Math/Math_all.h"
#include "Physics/Physics_all.h"
#include "Physics1D/Physics1D_all.h"
#include "Physics3D/Physics3D_all.h"
#include "Physics/Signals/LinearVelocity1DInput.h"
#include "DriveTrain/DriveTrain_all.h"
#include "Robotics/Robotics_all.h"
#include "Simulation/Simulation_all.h"
#include "Vehicles/Vehicles_all.h"
#include "Terrain/Terrain_all.h"
#include "Visuals/Visuals_all.h"
#include "Urdf/Urdf_all.h"

// Unreal Engine includes.
#include "Templates/UniquePtr.h"

// Standard library includes.
#include <memory>

namespace PLXUtilities_helpers
{
	std::shared_ptr<openplx::Core::Api::OpenPlxContext> CreatePLXContext(
		std::shared_ptr<agxopenplx::AgxCache> AGXCache)
	{
		const FString PLXBundlesPath = FPaths::Combine(
			FAGX_Environment::GetPluginSourcePath(), "Thirdparty", "agx", "openplxbundles");
		auto PLXCtx = std::make_shared<openplx::Core::Api::OpenPlxContext>(
			std::vector<std::string>({Convert(PLXBundlesPath)}));

		auto InternalContext = openplx::Core::Api::OpenPlxContextInternal::fromContext(*PLXCtx);
		auto EvalCtx = InternalContext->evaluatorContext().get();

		Math_register_factories(EvalCtx);
		Physics_register_factories(EvalCtx);
		Physics1D_register_factories(EvalCtx);
		Physics3D_register_factories(EvalCtx);
		DriveTrain_register_factories(EvalCtx);
		Robotics_register_factories(EvalCtx);
		Simulation_register_factories(EvalCtx);
		Vehicles_register_factories(EvalCtx);
		Terrain_register_factories(EvalCtx);
		Visuals_register_factories(EvalCtx);
		Urdf_register_factories(EvalCtx);

		agxopenplx::register_plugins(*PLXCtx, AGXCache);
		return PLXCtx;
	}

	openplx::Core::ObjectPtr LoadModelFromFile(
		const std::string& OpenPLXFile, std::shared_ptr<agxopenplx::AgxCache> AGXCache)
	{
		auto Context = CreatePLXContext(AGXCache);
		if (Context == nullptr)
		{
			UE_LOG(LogAGX, Error, TEXT("Error Creating OpenPLX Context"));
			return nullptr;
		}

		auto LoadedModel = openplx::Core::Api::loadModelFromFile(OpenPLXFile, {}, *Context);

		if (Context->hasErrors())
		{
			LoadedModel = nullptr;
			for (auto Error : Context->getErrors())
				UE_LOG(LogAGX, Error, TEXT("Error in OpenPLX Context : %d"), Error->getErrorCode());

			return nullptr;
		}

		return LoadedModel;
	}
}

openplx::Core::ObjectPtr FPLXUtilities::LoadModel(
	const FString& Filename, std::shared_ptr<agxopenplx::AgxCache> AGXCache)
{
	if (!FPaths::FileExists(Filename))
	{
		UE_LOG(
			LogAGX, Warning, TEXT("Could not read OpenPLX file '%s'. The file does not exist."),
			*Filename);
		return nullptr;
	}

	return PLXUtilities_helpers::LoadModelFromFile(Convert(Filename), AGXCache);
}

bool FPLXUtilities::HasInputs(openplx::Physics3D::System* System)
{
	if (System == nullptr)
		return false;

	return System->getNestedObjects<openplx::Physics::Signals::Input>().size() > 0;
}

bool FPLXUtilities::HasOutputs(openplx::Physics3D::System* System)
{
	if (System == nullptr)
		return false;

	return System->getNestedObjects<openplx::Physics::Signals::Output>().size() > 0;
}

TArray<TUniquePtr<FPLX_Input>> FPLXUtilities::GetInputs(openplx::Physics3D::System* System)
{
	TArray<TUniquePtr<FPLX_Input>> Inputs;
	if (System == nullptr)
		return Inputs;

	for (auto& Input : System->getNestedObjects<openplx::Physics::Signals::Input>())
	{
		if (auto Lvmvi =
				std::dynamic_pointer_cast<openplx::Physics::Signals::LinearVelocity1DInput>(Input))
		{
			Inputs.Emplace(MakeUnique<FPLX_LinearVelocity1DInput>(Convert(Input->getName())));
			continue;
		}

		UE_LOG(LogAGX, Warning, TEXT("Unhandled PLX Input: %s"), *Convert(Input->getName()));
	}
	return Inputs;
}

TArray<TUniquePtr<FPLX_Output>> FPLXUtilities::GetOutputs(openplx::Physics3D::System* System)
{
	TArray<TUniquePtr<FPLX_Output>> Outputs;
	if (System == nullptr)
		return Outputs;

	for (auto& Output : System->getNestedObjects<openplx::Physics::Signals::Output>())
	{
		if (auto Hao = std::dynamic_pointer_cast<openplx::Physics::Signals::AngleOutput>(Output))
		{
			Outputs.Emplace(MakeUnique<FPLX_AngleOutput>(Convert(Output->getName())));
			continue;
		}

		UE_LOG(LogAGX, Warning, TEXT("Unhandled PLX Output: %s"), *Convert(Output->getName()));
	}
	return Outputs;
}
