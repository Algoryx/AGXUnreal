// Copyright 2024, Algoryx Simulation AB.

#include "Utilities/PLXUtilities.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Environment.h"
#include "TypeConversions.h"

// OpenPLX includes.
#include "BeginAGXIncludes.h"
#include "openplx/OpenPlxContext.h"
#include "openplx/OpenPlxContextInternal.h"
#include "openplx/OpenPlxCoreAPI.h"
#include "agxOpenPLX/AgxOpenPlxApi.h"
#include "agxOpenPLX/AllocationUtils.h"
#include "DriveTrain/Signals/AutomaticClutchEngagementDurationInput.h"
#include "DriveTrain/Signals/AutomaticClutchDisengagementDurationInput.h"
#include "DriveTrain/Signals/TorqueConverterPumpTorqueOutput.h"
#include "DriveTrain/Signals/TorqueConverterTurbineTorqueOutput.h"
#include "Math/Math_all.h"

#include "Physics/Physics_all.h"
#include "Physics/Signals/AngularVelocity1DInput.h"
#include "Physics/Signals/EnableInteractionInput.h"
#include "Physics/Signals/Force1DInput.h"
#include "Physics/Signals/Force1DOutput.h"
#include "Physics/Signals/IntInput.h"
#include "Physics/Signals/LinearVelocity1DInput.h"
#include "Physics/Signals/Position1DInput.h"
#include "Physics/Signals/Position1DOutput.h"
#include "Physics/Signals/Torque1DInput.h"
#include "Physics1D/Physics1D_all.h"
#include "Physics3D/Physics3D_all.h"
#include "Physics3D/Signals/AngularVelocity3DInput.h"
#include "Physics3D/Signals/AngularVelocity3DOutput.h"
#include "Physics3D/Signals/LinearVelocity3DOutput.h"
#include "Physics3D/Signals/Position3DOutput.h"
#include "Physics3D/Signals/RPYOutput.h"

#include "DriveTrain/DriveTrain_all.h"
#include "Robotics/Robotics_all.h"
#include "Simulation/Simulation_all.h"
#include "Vehicles/Vehicles_all.h"
#include "Terrain/Terrain_all.h"
#include "Visuals/Visuals_all.h"
#include "Urdf/Urdf_all.h"
#include "EndAGXIncludes.h"

// Unreal Engine includes.
#include "Misc/Paths.h"
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

	return GetNestedObjects<openplx::Physics::Signals::Input>(*System).size() > 0;
}

bool FPLXUtilities::HasOutputs(openplx::Physics3D::System* System)
{
	if (System == nullptr)
		return false;

	return GetNestedObjects<openplx::Physics::Signals::Output>(*System).size() > 0;
}

TArray<FPLX_Input> FPLXUtilities::GetInputs(openplx::Physics3D::System* System)
{
	TArray<FPLX_Input> Inputs;
	if (System == nullptr)
		return Inputs;

	auto InputsPLX = GetNestedObjects<openplx::Physics::Signals::Input>(*System);
	Inputs.Reserve(InputsPLX.size());
	for (auto& Input : InputsPLX)
	{
		if (Input == nullptr)
			continue;

		EPLX_InputType Type = GetInputType(*Input);
		Inputs.Add(FPLX_Input(Convert(Input->getName()), Type));
		if (Type == EPLX_InputType::Unsupported)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Imported unsupported PLX Input: %s. The Input may not work as expected."),
				*Convert(Input->getName()));
		}
	}
	return Inputs;
}

TArray<FPLX_Output> FPLXUtilities::GetOutputs(openplx::Physics3D::System* System)
{
	TArray<FPLX_Output> Outputs;
	if (System == nullptr)
		return Outputs;

	auto OutputsPLX = GetNestedObjects<openplx::Physics::Signals::Output>(*System);
	Outputs.Reserve(OutputsPLX.size());
	for (auto& Output : OutputsPLX)
	{
		if (Output == nullptr)
			continue;

		EPLX_OutputType Type = GetOutputType(*Output);
		Outputs.Add(FPLX_Output(Convert(Output->getName()), Type));
		if (Type == EPLX_OutputType::Unsupported)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Imported unsupported PLX Output: %s. The Output may not work as expected."),
				*Convert(Output->getName()));
		}
	}
	return Outputs;
}

EPLX_InputType FPLXUtilities::GetInputType(const openplx::Physics::Signals::Input& Input)
{
	using namespace openplx::Physics::Signals;
	using namespace openplx::Physics3D::Signals;
	using namespace openplx::DriveTrain::Signals;

	if (dynamic_cast<const AutomaticClutchEngagementDurationInput*>(&Input))
	{
		return EPLX_InputType::AutomaticClutchEngagementDurationInput;
	}
	if (dynamic_cast<const AutomaticClutchDisengagementDurationInput*>(&Input))
	{
		return EPLX_InputType::AutomaticClutchDisengagementDurationInput;
	}
	if (dynamic_cast<const DurationInput*>(&Input))
	{
		return EPLX_InputType::DurationInput;
	}
	if (dynamic_cast<const AngleInput*>(&Input))
	{
		return EPLX_InputType::AngleInput;
	}
	if (dynamic_cast<const AngularVelocity1DInput*>(&Input))
	{
		return EPLX_InputType::AngularVelocity1DInput;
	}
	if (dynamic_cast<const FractionInput*>(&Input))
	{
		return EPLX_InputType::FractionInput;
	}
	if (dynamic_cast<const Force1DInput*>(&Input))
	{
		return EPLX_InputType::Force1DInput;
	}
	if (dynamic_cast<const LinearVelocity1DInput*>(&Input))
	{
		return EPLX_InputType::LinearVelocity1DInput;
	}
	if (dynamic_cast<const Position1DInput*>(&Input))
	{
		return EPLX_InputType::Position1DInput;
	}
	if (dynamic_cast<const Torque1DInput*>(&Input))
	{
		return EPLX_InputType::Torque1DInput;
	}
	if (dynamic_cast<const AngularVelocity3DInput*>(&Input))
	{
		return EPLX_InputType::AngularVelocity3DInput;
	}
	if (dynamic_cast<const LinearVelocity3DInput*>(&Input))
	{
		return EPLX_InputType::LinearVelocity3DInput;
	}
	if (dynamic_cast<const IntInput*>(&Input))
	{
		return EPLX_InputType::IntInput;
	}
	if (dynamic_cast<const TorqueConverterLockUpInput*>(&Input))
	{
		return EPLX_InputType::TorqueConverterLockUpInput;
	}
	if (dynamic_cast<const EngageInput*>(&Input))
	{
		return EPLX_InputType::EngageInput;
	}
	if (dynamic_cast<const ActivateInput*>(&Input))
	{
		return EPLX_InputType::ActivateInput;
	}
	if (dynamic_cast<const EnableInteractionInput*>(&Input))
	{
		return EPLX_InputType::EnableInteractionInput;
	}
	if (dynamic_cast<const BoolInput*>(&Input))
	{
		return EPLX_InputType::BoolInput;
	}

	return EPLX_InputType::Unsupported;
}

EPLX_OutputType FPLXUtilities::GetOutputType(const openplx::Physics::Signals::Output& Output)
{
	using namespace openplx::Physics::Signals;
	using namespace openplx::Physics3D::Signals;
	using namespace openplx::DriveTrain::Signals;

	if (dynamic_cast<const AutomaticClutchEngagementDurationOutput*>(&Output))
	{
		return EPLX_OutputType::AutomaticClutchEngagementDurationOutput;
	}
	if (dynamic_cast<const AutomaticClutchDisengagementDurationOutput*>(
			&Output))
	{
		return EPLX_OutputType::AutomaticClutchDisengagementDurationOutput;
	}
	if (dynamic_cast<const DurationOutput*>(&Output))
	{
		return EPLX_OutputType::DurationOutput;
	}
	if (dynamic_cast<const MateConnector::Acceleration3DOutput*>(&Output))
	{
		return EPLX_OutputType::MateConnectorAcceleration3DOutput;
	}
	if (dynamic_cast<const AngleOutput*>(&Output))
	{
		return EPLX_OutputType::AngleOutput;
	}
	if (dynamic_cast<const MateConnector::AngularAcceleration3DOutput*>(&Output))
	{
		return EPLX_OutputType::MateConnectorAngularAcceleration3DOutput;
	}
	if (dynamic_cast<const MateConnector::PositionOutput*>(&Output))
	{
		return EPLX_OutputType::MateConnectorPositionOutput;
	}
	if (dynamic_cast<const MateConnector::RPYOutput*>(&Output))
	{
		return EPLX_OutputType::MateConnectorRPYOutput;
	}
	if (dynamic_cast<const AngularVelocity1DOutput*>(&Output))
	{
		return EPLX_OutputType::AngularVelocity1DOutput;
	}
	if (dynamic_cast<const FractionOutput*>(&Output))
	{
		return EPLX_OutputType::FractionOutput;
	}
	if (dynamic_cast<const Force1DOutput*>(&Output))
	{
		return EPLX_OutputType::Force1DOutput;
	}
	if (dynamic_cast<const LinearVelocity1DOutput*>(&Output))
	{
		return EPLX_OutputType::LinearVelocity1DOutput;
	}
	if (dynamic_cast<const Position1DOutput*>(&Output))
	{
		return EPLX_OutputType::Position1DOutput;
	}
	if (dynamic_cast<const Position3DOutput*>(&Output))
	{
		return EPLX_OutputType::Position3DOutput;
	}
	if (dynamic_cast<const RelativeVelocity1DOutput*>(&Output))
	{
		return EPLX_OutputType::RelativeVelocity1DOutput;
	}
	if (dynamic_cast<const RPYOutput*>(&Output))
	{
		return EPLX_OutputType::RPYOutput;
	}
	if (dynamic_cast<const Torque1DOutput*>(&Output))
	{
		return EPLX_OutputType::Torque1DOutput;
	}
	if (dynamic_cast<const TorqueConverterPumpTorqueOutput*>(&Output))
	{
		return EPLX_OutputType::TorqueConverterPumpTorqueOutput;
	}
	if (dynamic_cast<const TorqueConverterTurbineTorqueOutput*>(&Output))
	{
		return EPLX_OutputType::TorqueConverterTurbineTorqueOutput;
	}
	if (dynamic_cast<const AngularVelocity3DOutput*>(&Output))
	{
		return EPLX_OutputType::AngularVelocity3DOutput;
	}
	if (dynamic_cast<const LinearVelocity3DOutput*>(&Output))
	{
		return EPLX_OutputType::LinearVelocity3DOutput;
	}
	if (dynamic_cast<const IntOutput*>(&Output))
	{
		return EPLX_OutputType::IntOutput;
	}
	if (dynamic_cast<const TorqueConverterLockedUpOutput*>(&Output))
	{
		return EPLX_OutputType::TorqueConverterLockedUpOutput;
	}
	if (dynamic_cast<const EngagedOutput*>(&Output))
	{
		return EPLX_OutputType::EngagedOutput;
	}
	if (dynamic_cast<const ActivatedOutput*>(&Output))
	{
		return EPLX_OutputType::ActivatedOutput;
	}
	if (dynamic_cast<const InteractionEnabledOutput*>(&Output))
	{
		return EPLX_OutputType::InteractionEnabledOutput;
	}
	if (dynamic_cast<const BoolOutput*>(&Output))
	{
		return EPLX_OutputType::BoolOutput;
	}

	return EPLX_OutputType::Unsupported;
}

std::unordered_set<openplx::Core::ObjectPtr> FPLXUtilities::GetNestedObjectFields(
	openplx::Core::Object& Object)
{
	std::unordered_set<openplx::Core::ObjectPtr> ObjectFields;
	GetNestedObjectFields(Object, ObjectFields);
	return ObjectFields;
}

void FPLXUtilities::GetNestedObjectFields(
	openplx::Core::Object& Object, std::unordered_set<openplx::Core::ObjectPtr>& Output)
{
	std::vector<openplx::Core::ObjectPtr> Fields = GetObjectFields(Object);
	for (auto& Field : Fields)
	{
		if (Field == nullptr)
			continue;
		if (Output.find(Field) != Output.end())
			continue;

		Output.insert(Field);
		GetNestedObjectFields(*Field, Output);
	}

	agxopenplx::freeContainerMemory(Fields);
}

std::vector<openplx::Core::ObjectPtr> FPLXUtilities::GetObjectFields(openplx::Core::Object& Object)
{
	std::vector<openplx::Core::ObjectPtr> Result;
	if (auto System = dynamic_cast<openplx::Physics3D::System*>(&Object))
	{
		// See openplx::Physics3D::System::extractObjectFieldsTo.
		System->extractObjectFieldsTo(Result);
	}

	Object.extractObjectFieldsTo(Result);
	return Result;
}
