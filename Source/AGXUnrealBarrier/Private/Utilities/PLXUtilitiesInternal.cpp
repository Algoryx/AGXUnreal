// Copyright 2024, Algoryx Simulation AB.

#include "Utilities/PLXUtilitiesInternal.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "BarrierOnly/AGXRefs.h"
#include "Constraints/ConstraintBarrier.h"
#include "SimulationBarrier.h"
#include "TypeConversions.h"
#include "Utilities/PLXUtilities.h"

// OpenPLX includes.
#include "BeginAGXIncludes.h"
#include "openplx/OpenPlxContext.h"
#include "openplx/OpenPlxContextInternal.h"
#include "openplx/OpenPlxCoreAPI.h"
#include "agxOpenPLX/AgxOpenPlxApi.h"
#include "agxOpenPLX/OpenPlxDriveTrainMapper.h"
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
#include "Physics/Signals/Force3DOutput.h"
#include "Physics/Signals/ForceRangeInput.h"
#include "Physics/Signals/ForceRangeOutput.h"
#include "Physics/Signals/IntInput.h"
#include "Physics/Signals/LinearVelocity1DInput.h"
#include "Physics/Signals/Position1DInput.h"
#include "Physics/Signals/Position1DOutput.h"
#include "Physics/Signals/SignalInterface.h"
#include "Physics/Signals/Torque1DInput.h"
#include "Physics/Signals/Torque3DOutput.h"
#include "Physics/Signals/TorqueRangeInput.h"
#include "Physics/Signals/TorqueRangeOutput.h"
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

namespace PLXUtilities_helpers
{
	std::shared_ptr<openplx::Core::Api::OpenPlxContext> CreatePLXContext(
		std::shared_ptr<agxopenplx::AgxCache> AGXCache)
	{
		const FString PLXBundlesPath = FPLXUtilities::GetBundlePath();
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

		openplx::Core::ObjectPtr LoadedModel;
		auto LogErrors = [&]()
		{
			return FPLXUtilitiesInternal::LogErrorsSafe(
				Context->getErrors(), TEXT("LoadModelFromFile got OpenPLX Error: "));
		};

		try
		{
			LoadedModel = openplx::Core::Api::loadModelFromFile(OpenPLXFile, {}, *Context);
		}
		catch (const std::runtime_error& Excep)
		{
			UE_LOG(
				LogAGX, Error, TEXT("LoadModelFromFile: Could not read OpenPLX file '%s':\n\n%s"),
				*Convert(OpenPLXFile), UTF8_TO_TCHAR(Excep.what()));
			LogErrors();
			return nullptr;
		}

		if (LogErrors())
			return nullptr;

		return LoadedModel;
	}

	template <typename T>
	std::optional<std::string> FindKeyByObject(
		const std::vector<std::pair<std::string, T>>& Lookup, const T& Object)
	{
		for (const auto& Pair : Lookup)
		{
			if (Pair.second == Object)
				return Pair.first;
		}

		return std::nullopt;
	}
}

openplx::Core::ObjectPtr FPLXUtilitiesInternal::LoadModel(
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

bool FPLXUtilitiesInternal::HasInputs(openplx::Physics3D::System* System)
{
	if (System == nullptr)
		return false;

	return GetNestedObjects<openplx::Physics::Signals::Input>(*System).size() > 0;
}

bool FPLXUtilitiesInternal::HasOutputs(openplx::Physics3D::System* System)
{
	if (System == nullptr)
		return false;

	return GetNestedObjects<openplx::Physics::Signals::Output>(*System).size() > 0;
}

TArray<FPLX_Input> FPLXUtilitiesInternal::GetInputs(openplx::Physics3D::System* System)
{
	TArray<FPLX_Input> Inputs;
	if (System == nullptr)
		return Inputs;

	std::vector<std::pair<std::string, std::shared_ptr<openplx::Physics::Signals::Input>>>
		SigInterfInputs;
	auto SignalInterfaces = GetNestedObjects<openplx::Physics::Signals::SignalInterface>(*System);
	if (SignalInterfaces.size() > 0)
		SigInterfInputs = GetEntries<openplx::Physics::Signals::Input>(*SignalInterfaces[0]);

	auto InputsPLX = GetNestedObjects<openplx::Physics::Signals::Input>(*System);
	Inputs.Reserve(InputsPLX.size());
	for (auto& Input : InputsPLX)
	{
		if (Input == nullptr)
			continue;

		auto OptionalAlias = PLXUtilities_helpers::FindKeyByObject(SigInterfInputs, Input);
		const FString Alias = OptionalAlias.has_value() ? Convert(OptionalAlias.value()) : "";
		EPLX_InputType Type = GetInputType(*Input);
		Inputs.Add(FPLX_Input(Convert(Input->getName()), Alias, Type));
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

TArray<FPLX_Output> FPLXUtilitiesInternal::GetOutputs(openplx::Physics3D::System* System)
{
	TArray<FPLX_Output> Outputs;
	if (System == nullptr)
		return Outputs;

	std::vector<std::pair<std::string, std::shared_ptr<openplx::Physics::Signals::Output>>>
		SigInterfOutputs;
	auto SignalInterfaces = GetNestedObjects<openplx::Physics::Signals::SignalInterface>(*System);
	if (SignalInterfaces.size() > 0)
		SigInterfOutputs = GetEntries<openplx::Physics::Signals::Output>(*SignalInterfaces[0]);

	auto OutputsPLX = GetNestedObjects<openplx::Physics::Signals::Output>(*System);
	Outputs.Reserve(OutputsPLX.size());
	for (auto& Output : OutputsPLX)
	{
		if (Output == nullptr)
			continue;

		auto OptionalAlias = PLXUtilities_helpers::FindKeyByObject(SigInterfOutputs, Output);
		const FString Alias = OptionalAlias.has_value() ? Convert(OptionalAlias.value()) : "";
		EPLX_OutputType Type = GetOutputType(*Output);
		Outputs.Add(FPLX_Output(Convert(Output->getName()), Alias, Type, Output->enabled()));
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

EPLX_InputType FPLXUtilitiesInternal::GetInputType(const openplx::Physics::Signals::Input& Input)
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
	if (dynamic_cast<const ForceRangeInput*>(&Input))
	{
		return EPLX_InputType::ForceRangeInput;
	}
	if (dynamic_cast<const TorqueRangeInput*>(&Input))
	{
		return EPLX_InputType::TorqueRangeInput;
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

EPLX_OutputType FPLXUtilitiesInternal::GetOutputType(
	const openplx::Physics::Signals::Output& Output)
{
	using namespace openplx::Physics::Signals;
	using namespace openplx::Physics3D::Signals;
	using namespace openplx::DriveTrain::Signals;

	if (dynamic_cast<const AutomaticClutchEngagementDurationOutput*>(&Output))
	{
		return EPLX_OutputType::AutomaticClutchEngagementDurationOutput;
	}
	if (dynamic_cast<const AutomaticClutchDisengagementDurationOutput*>(&Output))
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
	if (dynamic_cast<const openplx::Physics::Signals::Torque3DOutput*>(&Output))
	{
		return EPLX_OutputType::Torque3DOutput;
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
	if (dynamic_cast<const ForceRangeOutput*>(&Output))
	{
		return EPLX_OutputType::ForceRangeOutput;
	}
	if (dynamic_cast<const TorqueRangeOutput*>(&Output))
	{
		return EPLX_OutputType::TorqueRangeOutput;
	}
	if (dynamic_cast<const AngularVelocity3DOutput*>(&Output))
	{
		return EPLX_OutputType::AngularVelocity3DOutput;
	}
	if (dynamic_cast<const Force3DOutput*>(&Output))
	{
		return EPLX_OutputType::Force3DOutput;
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

TArray<FString> FPLXUtilitiesInternal::GetFileDependencies(const FString& Filepath)
{
	using namespace PLXUtilities_helpers;
	TArray<FString> Dependencies;

	if (!FPaths::FileExists(Filepath))
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("GetFileDependencies: Could not read OpenPLX file '%s'. The file does not exist."),
			*Filepath);
		return Dependencies;
	}

	const FString PLXBundlesPath = FPLXUtilities::GetBundlePath();
	agxSDK::SimulationRef Simulation {new agxSDK::Simulation()};

	agxopenplx::LoadResult Result;
	auto LogErrors = [&]()
	{
		return FPLXUtilitiesInternal::LogErrorsSafe(
			Result.errors(), TEXT("GetFileDependencies got OpenPLX Error: "));
	};

	try
	{
		Result = agxopenplx::load_from_file(Simulation, Convert(Filepath), Convert(PLXBundlesPath));
	}
	catch (const std::runtime_error& Excep)
	{
		UE_LOG(
			LogAGX, Error, TEXT("GetFileDependencies: Could not read OpenPLX file '%s':\n\n%s"),
			*Filepath, UTF8_TO_TCHAR(Excep.what()));
		LogErrors();
		return Dependencies;
	}

	if (LogErrors())
		return Dependencies;

	auto System = std::dynamic_pointer_cast<openplx::Physics3D::System>(Result.scene());
	if (System == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("GetFileDependencies: Could not read OpenPLX file '%s'. The Log category LogAGX "
				 "may include more details."),
			*Filepath);
		return Dependencies;
	}

	for (auto G : GetNestedObjects<openplx::Visuals::Geometries::ExternalTriMeshGeometry>(*System))
	{
		if (G == nullptr)
			continue;

		std::string PathPLX = G->path();
		const FString Path = FPaths::ConvertRelativePathToFull(Convert(PathPLX));
		agxUtil::freeContainerMemory(PathPLX); // Allocated in OpenPLX, deallocate safely.
		if (FPaths::FileExists(Path))
			Dependencies.AddUnique(Path);
	}

	// Get the dependencies from the OpenPLX context.
	auto ContextInternal =
		openplx::Core::Api::OpenPlxContextInternal::fromContext(*Result.context());
	const auto& Docs = ContextInternal->documents();
	const FString BundlePath = FPLXUtilities::GetBundlePath();
	for (auto& D : Docs)
	{
		const FString Path = FPaths::ConvertRelativePathToFull(Convert(D->path.string()));
		if (!Path.StartsWith(BundlePath))
		{
			if (FPaths::FileExists(Path))
				Dependencies.AddUnique(Path);

			const FString BundleConfig =
				FPaths::ConvertRelativePathToFull(Convert(D->bundle.config_file_path.string()));
			if (!BundleConfig.StartsWith(BundlePath))
			{
				if (FPaths::FileExists(BundleConfig))
					Dependencies.AddUnique(BundleConfig);
			}
		}
	}
	return Dependencies;
}

std::unordered_set<openplx::Core::ObjectPtr> FPLXUtilitiesInternal::GetNestedObjectFields(
	openplx::Core::Object& Object)
{
	std::unordered_set<openplx::Core::ObjectPtr> ObjectFields;
	GetNestedObjectFields(Object, ObjectFields);
	return ObjectFields;
}

void FPLXUtilitiesInternal::GetNestedObjectFields(
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

std::vector<openplx::Core::ObjectPtr> FPLXUtilitiesInternal::GetObjectFields(
	openplx::Core::Object& Object)
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

TArray<FString> FPLXUtilitiesInternal::GetErrorStrings(const openplx::Errors& Errors)
{
	TArray<FString> ErrorStrs;
	std::vector<std::string> ErrorStrsPlx = agxopenplx::get_error_strings(Errors);

	for (auto& Err : ErrorStrsPlx)
	{
		ErrorStrs.Add(Convert(Err));
	}

	agxUtil::freeContainerMemory(ErrorStrsPlx);
	return ErrorStrs;
}

bool FPLXUtilitiesInternal::LogErrorsSafe(
	openplx::Errors&& Errors, const FString& ErrorMessagePostfix)
{
	if (Errors.size() > 0)
	{
		for (auto Err : GetErrorStrings(Errors))
		{
			UE_LOG(LogAGX, Error, TEXT("%s%s"), *ErrorMessagePostfix, *Err);
		}
		agxopenplx::freeContainerMemory(Errors);
		return true;
	}

	return false;
}

agxSDK::AssemblyRef FPLXUtilitiesInternal::MapRuntimeObjects(
	std::shared_ptr<openplx::Physics3D::System> System, FSimulationBarrier& Simulation,
	TArray<FRigidBodyBarrier*>& Bodies, TArray<FConstraintBarrier*>& Constraints)
{
	AGX_CHECK(System != nullptr);
	AGX_CHECK(Simulation.HasNative());
	if (System == nullptr)
	{
		UE_LOG(LogAGX, Warning, TEXT("MapRuntimeObjects: Got nullptr System."));
		return nullptr;
	}

	agxSDK::AssemblyRef Assembly = new agxSDK::Assembly();

	agx::RigidBodyRefSetVector OldBodiesAGX;
	for (FRigidBodyBarrier* Body : Bodies)
	{
		AGX_CHECK(Body->HasNative());
		auto BodyAGX = Body->GetNative()->Native;
		Assembly->add(BodyAGX);
		OldBodiesAGX.push_back(BodyAGX);
	}

	agx::ConstraintRefSetVector OldConstraintsAGX;
	for (FConstraintBarrier* Constraint : Constraints)
	{
		AGX_CHECK(Constraint->HasNative());
		auto ConstraintAGX = Constraint->GetNative()->Native;
		Assembly->add(ConstraintAGX);
		OldConstraintsAGX.push_back(ConstraintAGX);
	}

	// OpenPLX OutputSignalListener requires the assembly to contain a PowerLine with a
	// certain name. This is the PowerLine we will use to map the OpenPLX DriveTrain.
	agxPowerLine::PowerLineRef RequiredPowerLine = new agxPowerLine::PowerLine();
	RequiredPowerLine->setName(agx::Name("OpenPlxPowerLine"));
	Assembly->add(RequiredPowerLine);

	// Map DriveTrain.
	auto ErrorReporter = std::make_shared<openplx::ErrorReporter>();
	agxopenplx::OpenPlxDriveTrainMapper DriveTrainMapper(
		ErrorReporter, agxopenplx::DriveTrainConstraintMapMode::Name);
	DriveTrainMapper.mapDriveTrainIntoPowerLine(System, RequiredPowerLine, Assembly);

	if (ErrorReporter->getErrorCount() > 0)
	{
		for (auto Err : FPLXUtilitiesInternal::GetErrorStrings(ErrorReporter->getErrors()))
		{
			UE_LOG(LogAGX, Error, TEXT("MapRuntimeObjects got error: %s"), *Err);
		}
	}

	// All objects created within this function must be added to the Simulation.
	Simulation.GetNative()->Native->add(RequiredPowerLine);

	for (auto B : Assembly->getRigidBodies())
	{
		if (!OldBodiesAGX.contains(B))
			Simulation.GetNative()->Native->add(B);
	}

	for (auto C : Assembly->getConstraints())
	{
		if (!OldConstraintsAGX.contains(C))
			Simulation.GetNative()->Native->add(C);
	}

	return Assembly;
}
