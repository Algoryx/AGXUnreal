// Copyright 2024, Algoryx Simulation AB.

#include "OpenPLX/PLXSignalHandler.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "BarrierOnly/AGXRefs.h"
#include "BarrierOnly/OpenPLX/OpenPLXRefs.h"
#include "Constraints/ConstraintBarrier.h"
#include "OpenPLX/PLX_Inputs.h"
#include "OpenPLX/PLX_Outputs.h"
#include "SimulationBarrier.h"
#include "TypeConversions.h"
#include "Utilities/PLXUtilities.h"

// OpenPLX includes.
#include "BeginAGXIncludes.h"
#include "agxOpenPLX/SignalListenerUtils.h"
#include "Math/Vec3.h"
#include "Physics/Signals/BoolInputSignal.h"
#include "Physics/Signals/IntInputSignal.h"
#include "Physics/Signals/RealInputSignal.h"
#include "Physics/Signals/Vec3InputSignal.h"
#include "EndAGXIncludes.h"

FPLXSignalHandler::FPLXSignalHandler()
{
}

void FPLXSignalHandler::Init(
	const FString& PLXFile, FSimulationBarrier& Simulation, FPLXModelRegistry& InModelRegistry,
	TArray<FConstraintBarrier*>& Constraints)
{
	check(Simulation.HasNative());
	check(InModelRegistry.HasNative());

	AssemblyRef = std::make_shared<FAssemblyRef>(new agxSDK::Assembly());
	for (FConstraintBarrier* Constraint : Constraints)
	{
		AGX_CHECK(Constraint->HasNative());
		AssemblyRef->Native->add(Constraint->GetNative()->Native);
	}

	// OpenPLX OutputSignalListener requires the assembly to contain a PowerLine with a
	// certain name. Remove once this has been cleaned up in OpenPLX, it's a bit hacky.
	agxPowerLine::PowerLineRef RequiredDummyPowerLine = new agxPowerLine::PowerLine();
	RequiredDummyPowerLine->setName(agx::Name("OpenPlxPowerLine"));
	AssemblyRef->Native->add(RequiredDummyPowerLine);

	ModelRegistry = &InModelRegistry;
	ModelHandle = ModelRegistry->Register(PLXFile);
	if (ModelHandle == FPLXModelRegistry::InvalidHandle)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT(
				"Could not load OpenPLX model '%s'. The Console Log may contain more information."),
			*PLXFile);
		return;
	}

	FPLXModelData* ModelData = ModelRegistry->GetModelData(ModelHandle);
	if (ModelData == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Unexpected error: Unable to get registered OpenPLX model '%s'. The OpenPLX model "
				 "may not behave as intended."),
			*PLXFile);
		return;
	}

	auto System = std::dynamic_pointer_cast<openplx::Physics3D::System>(ModelData->PLXModel);
	if (System == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Unable to get a openplx::Physics3D::System from the registered OpenPLX model "
				 "'%s'. The OpenPLX model may not behave as intended."),
			*PLXFile);
		return;
	}

	if (FPLXUtilities::HasInputs(System.get()))
	{
		InputQueueRef =
			std::make_shared<FInputSignalQueueRef>(agxopenplx::InputSignalQueue::create());
		InputSignalHandlerRef =
			std::make_shared<FInputSignalHandlerRef>(AssemblyRef->Native, InputQueueRef->Native);
		Simulation.GetNative()->Native->add(InputSignalHandlerRef->Native);
	}

	if (FPLXUtilities::HasOutputs(System.get()))
	{
		OutputQueueRef =
			std::make_shared<FOutputSignalQueueRef>(agxopenplx::OutputSignalQueue::create());
		OutputSignalHandlerRef = std::make_shared<FOutputSignalHandlerRef>(
			AssemblyRef->Native, ModelData->PLXModel, OutputQueueRef->Native);
		Simulation.GetNative()->Native->add(OutputSignalHandlerRef->Native);
	}

	bIsInitialized = true;
}

bool FPLXSignalHandler::IsInitialized() const
{
	return bIsInitialized;
}

namespace PLXSignalHandler_helpers
{
	TOptional<double> ConvertScalar(const FPLX_Input& Input, double Value)
	{
		switch (Input.Type)
		{
			case EPLX_InputType::AngleInput:
			case EPLX_InputType::AngularVelocity1DInput:
				return ConvertAngleToAGX(Value);
			case EPLX_InputType::DurationInput:
			case EPLX_InputType::AutomaticClutchEngagementDurationInput:
			case EPLX_InputType::AutomaticClutchDisengagementDurationInput:
			case EPLX_InputType::FractionInput:
			case EPLX_InputType::Force1DInput:
			case EPLX_InputType::Torque1DInput:
				return Value;
			case EPLX_InputType::Position1DInput:
			case EPLX_InputType::LinearVelocity1DInput:
				return ConvertDistanceToAGX(Value);
		}

		UE_LOG(
			LogAGX, Warning,
			TEXT("Tried to convert scalar value for Input '%s', but the type is either "
				 "not of scalar type or is unsupported."),
			*Input.Name);
		return {};
	}

	TOptional<std::shared_ptr<openplx::Math::Vec3>> ConvertVector(
		const FPLX_Input& Input, const FVector& Value)
	{
		switch (Input.Type)
		{
			case EPLX_InputType::AngularVelocity3DInput:
			{
				return openplx::Math::Vec3::from_xyz(
					ConvertToAGX(FMath::DegreesToRadians(Value.X)),
					-ConvertToAGX(FMath::DegreesToRadians(Value.Y)),
					-ConvertToAGX(FMath::DegreesToRadians(Value.Z)));
			}
			case EPLX_InputType::LinearVelocity3DInput:
			{
				return openplx::Math::Vec3::from_xyz(
					ConvertDistanceToAGX(Value.X), -ConvertDistanceToAGX(Value.Y),
					ConvertDistanceToAGX(Value.X));
			}
		}

		UE_LOG(
			LogAGX, Warning,
			TEXT("Tried to convert vec3 vector value for Input '%s', but the type is either "
				 "not of vec3 vector type or is unsupported."),
			*Input.Name);
		return {};
	}

	TOptional<int64> ConvertInteger(const FPLX_Input& Input, int64 Value)
	{
		switch (Input.Type)
		{
			case EPLX_InputType::IntInput:
				return static_cast<int>(Value);
		}

		UE_LOG(
			LogAGX, Warning,
			TEXT("Tried to convert integer value for Input '%s', but the type is either "
				 "not of integer type or is unsupported."),
			*Input.Name);
		return {};
	}

	TOptional<bool> ConvertBoolean(const FPLX_Input& Input, bool Value)
	{
		switch (Input.Type)
		{
			case EPLX_InputType::BoolInput:
			case EPLX_InputType::ActivateInput:
			case EPLX_InputType::EnableInteractionInput:
			case EPLX_InputType::EngageInput:
			case EPLX_InputType::TorqueConverterLockUpInput:
				return Value;
		}

		UE_LOG(
			LogAGX, Warning,
			TEXT("Tried to convert boolean value for Input '%s', but the type is either "
				 "not of boolean type or is unsupported."),
			*Input.Name);
		return {};
	}

	template <typename ValueT, typename SignalT, typename ConversionFuncT>
	bool Send(
		const FPLX_Input& Input, ValueT Value, FPLXModelRegistry* ModelRegistry,
		FPLXModelRegistry::Handle ModelHandle, FInputSignalQueueRef* InputQueue,
		ConversionFuncT Func)
	{
		if (ModelRegistry == nullptr || ModelHandle == FPLXModelRegistry::InvalidHandle)
			return false;

		if (InputQueue == nullptr || InputQueue->Native == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Tried to send OpenPLX Input signal for Input '%s', but the OpenPLX "
					 "model does not have any registered Inputs."),
				*Input.Name);
			return false;
		}

		FPLXModelData* ModelData = ModelRegistry->GetModelData(ModelHandle);
		if (ModelData == nullptr)
			return false;

		auto PLXInput = ModelData->Inputs.find(Convert(Input.Name));
		if (PLXInput == ModelData->Inputs.end())
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Tried to send OpenPLX signal, but the corresponding OpenPLX Input "
					 "'%s' was not found in the model. The signal will not be sent."),
				*Input.Name);
			return false;
		}

		auto ConvertedValue = Func(Input, Value);
		if (!ConvertedValue.IsSet())
			return false;

		auto Signal = SignalT::create(*ConvertedValue, PLXInput->second);
		InputQueue->Native->send(Signal);
		return true;
	}
}

namespace PLXSignalHandler_helpers
{
	template <typename T>
	TOptional<T> TypeMismatchResult(const FPLX_Output& Output)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Unexpected error: Tried to cast OpenPLX Output '%s' to it's corresponding "
				 "OpenPLX type but got nullptr. Possible type miss match. The signal will not "
				 "be received."),
			*Output.Name);
		return {};
	}

	TOptional<double> GetScalarValueFrom(
		const FPLX_Output& Output, openplx::Physics::Signals::ValueOutputSignal* Signal)
	{
		if (Signal == nullptr)
			return {};

		auto Value =
			std::dynamic_pointer_cast<openplx::Physics::Signals::RealValue>(Signal->value());
		if (Value == nullptr)
			return TypeMismatchResult<double>(Output);

		switch (Output.Type)
		{
			case EPLX_OutputType::DurationOutput:
			case EPLX_OutputType::AutomaticClutchEngagementDurationOutput:
			case EPLX_OutputType::AutomaticClutchDisengagementDurationOutput:
			case EPLX_OutputType::FractionOutput:
			case EPLX_OutputType::Force1DOutput:
			case EPLX_OutputType::Torque1DOutput:
			case EPLX_OutputType::TorqueConverterPumpTorqueOutput:
			case EPLX_OutputType::TorqueConverterTurbineTorqueOutput:
				return Value->value();
			case EPLX_OutputType::AngleOutput:
			case EPLX_OutputType::AngularVelocity1DOutput:
				return ConvertAngleToUnreal<double>(Value->value());
			case EPLX_OutputType::LinearVelocity1DOutput:
			case EPLX_OutputType::Position1DOutput:
			case EPLX_OutputType::RelativeVelocity1DOutput:
				return ConvertDistanceToUnreal<double>(Value->value());
		}

		UE_LOG(
			LogAGX, Warning,
			TEXT("Tried to read scalar type from signal for Output '%s', but the type is either "
				 "not of scalar type or is unsupported."),
			*Output.Name);
		return {};
	}

	TOptional<FVector> GetVectorValueFrom(
		const FPLX_Output& Output, openplx::Physics::Signals::ValueOutputSignal* Signal)
	{
		if (Signal == nullptr)
			return {};

		auto Value =
			std::dynamic_pointer_cast<openplx::Physics::Signals::Vec3Value>(Signal->value());
		if (Value == nullptr)
			return TypeMismatchResult<FVector>(Output);

		switch (Output.Type)
		{
			case EPLX_OutputType::AngularVelocity3DOutput:
			case EPLX_OutputType::MateConnectorAngularAcceleration3DOutput:
			case EPLX_OutputType::MateConnectorRPYOutput:
			case EPLX_OutputType::RPYOutput:
			{
				return FVector(
					FMath::RadiansToDegrees(Value->value()->x()),
					-FMath::RadiansToDegrees(Value->value()->y()),
					-FMath::RadiansToDegrees(Value->value()->z()));
			}
			case EPLX_OutputType::LinearVelocity3DOutput:
			case EPLX_OutputType::MateConnectorAcceleration3DOutput:
			case EPLX_OutputType::MateConnectorPositionOutput:
			case EPLX_OutputType::Position3DOutput:
			{
				return FVector(
					ConvertDistanceToUnreal<double>(Value->value()->x()),
					-ConvertDistanceToUnreal<double>(Value->value()->y()),
					ConvertDistanceToUnreal<double>(Value->value()->z()));
			}
		}

		UE_LOG(
			LogAGX, Warning,
			TEXT("Tried to read vec3 vector type from signal for Output '%s', but the type is "
				 "either not of vec3 vector type or is unsupported."),
			*Output.Name);
		return {};
	}

	TOptional<int64> GetIntegerValueFrom(
		const FPLX_Output& Output, openplx::Physics::Signals::ValueOutputSignal* Signal)
	{
		if (Signal == nullptr)
			return {};

		auto Value =
			std::dynamic_pointer_cast<openplx::Physics::Signals::IntValue>(Signal->value());
		if (Value == nullptr)
			return TypeMismatchResult<int64>(Output);

		switch (Output.Type)
		{
			case EPLX_OutputType::IntOutput:
				return Value->value();
		}

		UE_LOG(
			LogAGX, Warning,
			TEXT("Tried to read integer type from signal for Output '%s', but the type is either "
				 "not of integer type or is unsupported."),
			*Output.Name);
		return {};
	}

	TOptional<bool> GetBooleanValueFrom(
		const FPLX_Output& Output, openplx::Physics::Signals::ValueOutputSignal* Signal)
	{
		if (Signal == nullptr)
			return {};

		auto Value =
			std::dynamic_pointer_cast<openplx::Physics::Signals::BoolValue>(Signal->value());
		if (Value == nullptr)
			return TypeMismatchResult<bool>(Output);

		switch (Output.Type)
		{
			case EPLX_OutputType::BoolOutput:
			case EPLX_OutputType::ActivatedOutput:
			case EPLX_OutputType::InteractionEnabledOutput:
			case EPLX_OutputType::EngagedOutput:
			case EPLX_OutputType::TorqueConverterLockedUpOutput:
				return Value->value();
		}

		UE_LOG(
			LogAGX, Warning,
			TEXT("Tried to read boolean type from signal for Output '%s', but the type is either "
				 "not of boolean type or is unsupported."),
			*Output.Name);
		return {};
	}

	template <typename ValueT, typename ValueGetterFuncT>
	bool Receive(
		const FPLX_Output& Output, ValueT& OutValue, FPLXModelRegistry* ModelRegistry,
		FPLXModelRegistry::Handle ModelHandle, FOutputSignalQueueRef* OutputQueue,
		ValueGetterFuncT Func)
	{
		if (ModelRegistry == nullptr || ModelHandle == FPLXModelRegistry::InvalidHandle)
			return false;

		if (OutputQueue == nullptr || OutputQueue->Native == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Tried to receive OpenPLX Output signal for output '%s', but the OpenPLX "
					 "model does not have any registered outputs."),
				*Output.Name);
			return false;
		}

		auto Signal =
			agxopenplx::getSignalBySourceName<openplx::Physics::Signals::ValueOutputSignal>(
				OutputQueue->Native->getSignals(), Convert(Output.Name));
		if (Signal == nullptr)
			return false;

		auto Value = Func(Output, Signal.get());
		if (!Value.IsSet())
			return false;

		OutValue = *Value;
		return true;
	}
}

bool FPLXSignalHandler::Send(const FPLX_Input& Input, double Value)
{
	check(IsInitialized());
	return PLXSignalHandler_helpers::Send<double, openplx::Physics::Signals::RealInputSignal>(
		Input, Value, ModelRegistry, ModelHandle, InputQueueRef.get(),
		PLXSignalHandler_helpers::ConvertScalar);
}

bool FPLXSignalHandler::Receive(const FPLX_Output& Output, double& OutValue)
{
	check(IsInitialized());
	return PLXSignalHandler_helpers::Receive(
		Output, OutValue, ModelRegistry, ModelHandle, OutputQueueRef.get(),
		PLXSignalHandler_helpers::GetScalarValueFrom);
}

bool FPLXSignalHandler::Send(const FPLX_Input& Input, const FVector& Value)
{
	check(IsInitialized());
	return PLXSignalHandler_helpers::Send<FVector, openplx::Physics::Signals::Vec3InputSignal>(
		Input, Value, ModelRegistry, ModelHandle, InputQueueRef.get(),
		PLXSignalHandler_helpers::ConvertVector);
}

bool FPLXSignalHandler::Receive(const FPLX_Output& Output, FVector& OutValue)
{
	check(IsInitialized());
	return PLXSignalHandler_helpers::Receive(
		Output, OutValue, ModelRegistry, ModelHandle, OutputQueueRef.get(),
		PLXSignalHandler_helpers::GetVectorValueFrom);
}

bool FPLXSignalHandler::Send(const FPLX_Input& Input, int64 Value)
{
	check(IsInitialized());
	return PLXSignalHandler_helpers::Send<int64, openplx::Physics::Signals::IntInputSignal>(
		Input, Value, ModelRegistry, ModelHandle, InputQueueRef.get(),
		PLXSignalHandler_helpers::ConvertInteger);
}

bool FPLXSignalHandler::Receive(const FPLX_Output& Output, int64& OutValue)
{
	check(IsInitialized());
	return PLXSignalHandler_helpers::Receive(
		Output, OutValue, ModelRegistry, ModelHandle, OutputQueueRef.get(),
		PLXSignalHandler_helpers::GetIntegerValueFrom);
}

bool FPLXSignalHandler::Send(const FPLX_Input& Input, bool Value)
{
	check(IsInitialized());
	return PLXSignalHandler_helpers::Send<bool, openplx::Physics::Signals::BoolInputSignal>(
		Input, Value, ModelRegistry, ModelHandle, InputQueueRef.get(),
		PLXSignalHandler_helpers::ConvertBoolean);
}

bool FPLXSignalHandler::Receive(const FPLX_Output& Output, bool& OutValue)
{
	check(IsInitialized());
	return PLXSignalHandler_helpers::Receive(
		Output, OutValue, ModelRegistry, ModelHandle, OutputQueueRef.get(),
		PLXSignalHandler_helpers::GetBooleanValueFrom);
}
