// Copyright 2025, Algoryx Simulation AB.

#include "OpenPLX/OpenPLXSignalHandler.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "BarrierOnly/AGXRefs.h"
#include "BarrierOnly/OpenPLX/OpenPLXRefs.h"
#include "Constraints/ConstraintBarrier.h"
#include "OpenPLX/OpenPLX_Inputs.h"
#include "OpenPLX/OpenPLX_Outputs.h"
#include "OpenPLX/OpenPLX_SignalHandlerNativeAddresses.h"
#include "RigidBodyBarrier.h"
#include "SimulationBarrier.h"
#include "TypeConversions.h"
#include "Utilities/PLXUtilitiesInternal.h"

// OpenPLX includes.
#include "BeginAGXIncludes.h"
#include "agxOpenPLX/SignalListenerUtils.h"
#include "agxOpenPLX/SignalSourceMapper.h"
#include "openplx/Math/Vec3.h"
#include "openplx/Physics/Signals/BoolInputSignal.h"
#include "openplx/Physics/Signals/IntInputSignal.h"
#include "openplx/Physics/Signals/RealInputSignal.h"
#include "openplx/Physics/Signals/Vec3InputSignal.h"
#include "openplx/Physics/Signals/AngleOutput.h"
#include "EndAGXIncludes.h"

// Standard library includes.
#include <cstdint>

FOpenPLXSignalHandler::FOpenPLXSignalHandler()
	: AssemblyRef {new FAssemblyRef()}
	, InputSignalListenerRef {new FInputSignalListenerRef()}
	, OutputSignalListenerRef {new FOutputSignalListenerRef()}
{
}

void FOpenPLXSignalHandler::Init(
	const FString& OpenPLXFile, FSimulationBarrier& Simulation,
	FOpenPLXModelRegistry& InModelRegistry, TArray<FRigidBodyBarrier*>& Bodies,
	TArray<FConstraintBarrier*>& Constraints)
{
	check(Simulation.HasNative());
	check(InModelRegistry.HasNative());

	ModelRegistry = &InModelRegistry;
	ModelHandle = ModelRegistry->Register(OpenPLXFile);
	if (ModelHandle == FOpenPLXModelRegistry::InvalidHandle)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Could not load OpenPLX model '%s'. The Output Log may contain more information."),
			*OpenPLXFile);
		return;
	}

	FOpenPLXModelData* ModelData = ModelRegistry->GetModelData(ModelHandle);
	if (ModelData == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Unexpected error: Unable to get registered OpenPLX model '%s'. The OpenPLX model "
				 "may not behave as intended."),
			*OpenPLXFile);
		return;
	}

	auto System = std::dynamic_pointer_cast<openplx::Physics3D::System>(ModelData->OpenPLXModel);
	if (System == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Unable to get a openplx::Physics3D::System from the registered OpenPLX model "
				 "'%s'. The OpenPLX model may not behave as intended."),
			*OpenPLXFile);
		return;
	}

	AssemblyRef->Native =
		FPLXUtilitiesInternal::MapRuntimeObjects(System, Simulation, Bodies, Constraints);
	if (AssemblyRef->Native == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Unable to get a valid AGX Assembly from simulated model instance for OpenPLX "
				 "model '%s'. The Output Log may contain more details."),
			*OpenPLXFile);
		return;
	}

	std::shared_ptr<agxopenplx::SignalSourceMapper> SignalSourceMapper;
	if (FPLXUtilitiesInternal::HasInputs(System.get()) ||
		FPLXUtilitiesInternal::HasOutputs(System.get()))
	{
		auto PlxPowerLine = dynamic_cast<agxPowerLine::PowerLine*>(
			AssemblyRef->Native->getAssembly(FPLXUtilitiesInternal::GetDefaultPowerLineName()));

		SignalSourceMapper = agxopenplx::SignalSourceMapper::create(
			AssemblyRef->Native, PlxPowerLine, agxopenplx::SignalSourceMapMode::Name);
	}

	if (FPLXUtilitiesInternal::HasInputs(System.get()))
	{
		auto InputSignalQue = agxopenplx::InputSignalQueue::create();
		InputSignalListenerRef->Native =
			new agxopenplx::InputSignalListener(InputSignalQue, SignalSourceMapper);
		Simulation.GetNative()->Native->add(InputSignalListenerRef->Native);
	}

	if (FPLXUtilitiesInternal::HasOutputs(System.get()))
	{
		auto OutputSignalQueue = agxopenplx::OutputSignalQueue::create();
		OutputSignalListenerRef->Native = new agxopenplx::OutputSignalListener(
			ModelData->OpenPLXModel, OutputSignalQueue, SignalSourceMapper);
		Simulation.GetNative()->Native->add(OutputSignalListenerRef->Native);
	}

	bIsInitialized = true;
}

bool FOpenPLXSignalHandler::IsInitialized() const
{
	return bIsInitialized;
}

namespace OpenPLXSignalHandler_helpers
{
	TOptional<double> ConvertReal(const FOpenPLX_Input& Input, double Value)
	{
		switch (Input.Type)
		{
			case EOpenPLX_InputType::AngleInput:
			case EOpenPLX_InputType::AngularVelocity1DInput:
				return ConvertAngleToAGX(Value);
			case EOpenPLX_InputType::DurationInput:
			case EOpenPLX_InputType::AutomaticClutchEngagementDurationInput:
			case EOpenPLX_InputType::AutomaticClutchDisengagementDurationInput:
			case EOpenPLX_InputType::FractionInput:
			case EOpenPLX_InputType::Force1DInput:
			case EOpenPLX_InputType::Torque1DInput:
				return Value;
			case EOpenPLX_InputType::Position1DInput:
			case EOpenPLX_InputType::LinearVelocity1DInput:
				return ConvertDistanceToAGX(Value);
		}

		UE_LOG(
			LogAGX, Warning,
			TEXT("Tried to convert Real value for Input '%s', but the type is either "
				 "not of Real type or is unsupported."),
			*Input.Name.ToString());
		return {};
	}

	TOptional<std::shared_ptr<openplx::Math::Vec2>> ConvertVector2D(
		const FOpenPLX_Input& Input, const FVector2D& Value)
	{
		switch (Input.Type)
		{
			case EOpenPLX_InputType::ForceRangeInput:
			case EOpenPLX_InputType::TorqueRangeInput:
				return openplx::Math::Vec2::from_xy(Value.X, Value.Y);
		}

		UE_LOG(
			LogAGX, Warning,
			TEXT("Tried to convert vec2 vector value for Input '%s', but the type is either "
				 "not of vec2 vector type or is unsupported."),
			*Input.Name.ToString());
		return {};
	}

	TOptional<std::shared_ptr<openplx::Math::Vec3>> ConvertVector3D(
		const FOpenPLX_Input& Input, const FVector& Value)
	{
		switch (Input.Type)
		{
			case EOpenPLX_InputType::AngularVelocity3DInput:
			{
				return openplx::Math::Vec3::from_xyz(
					ConvertToAGX(FMath::DegreesToRadians(Value.X)),
					-ConvertToAGX(FMath::DegreesToRadians(Value.Y)),
					-ConvertToAGX(FMath::DegreesToRadians(Value.Z)));
			}
			case EOpenPLX_InputType::LinearVelocity3DInput:
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
			*Input.Name.ToString());
		return {};
	}

	TOptional<int64_t> ConvertInteger(const FOpenPLX_Input& Input, int64 Value)
	{
		switch (Input.Type)
		{
			case EOpenPLX_InputType::IntInput:
				return static_cast<int64_t>(Value);
		}

		UE_LOG(
			LogAGX, Warning,
			TEXT("Tried to convert integer value for Input '%s', but the type is either "
				 "not of integer type or is unsupported."),
			*Input.Name.ToString());
		return {};
	}

	TOptional<bool> ConvertBoolean(const FOpenPLX_Input& Input, bool Value)
	{
		switch (Input.Type)
		{
			case EOpenPLX_InputType::BoolInput:
			case EOpenPLX_InputType::ActivateInput:
			case EOpenPLX_InputType::EnableInteractionInput:
			case EOpenPLX_InputType::EngageInput:
			case EOpenPLX_InputType::TorqueConverterLockUpInput:
				return Value;
		}

		UE_LOG(
			LogAGX, Warning,
			TEXT("Tried to convert boolean value for Input '%s', but the type is either "
				 "not of boolean type or is unsupported."),
			*Input.Name.ToString());
		return {};
	}

	template <typename ValueT, typename SignalT, typename ConversionFuncT>
	bool Send(
		const FOpenPLX_Input& Input, ValueT Value, FOpenPLXModelRegistry* ModelRegistry,
		FOpenPLXModelRegistry::Handle ModelHandle,
		std::shared_ptr<agxopenplx::InputSignalQueue> InputQueue,
		ConversionFuncT Func)
	{
		if (ModelRegistry == nullptr || ModelHandle == FOpenPLXModelRegistry::InvalidHandle)
			return false;

		if (InputQueue == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Tried to send OpenPLX Input signal for Input '%s', but the OpenPLX "
					 "model does not have any registered Inputs."),
				*Input.Name.ToString());
			return false;
		}

		FOpenPLXModelData* ModelData = ModelRegistry->GetModelData(ModelHandle);
		if (ModelData == nullptr)
			return false;

		auto PLXInput = ModelData->Inputs.find(Convert(Input.Name.ToString()));
		if (PLXInput == ModelData->Inputs.end())
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Tried to send OpenPLX signal, but the corresponding OpenPLX Input "
					 "'%s' was not found in the model. The signal will not be sent."),
				*Input.Name.ToString());
			return false;
		}

		auto ConvertedValue = Func(Input, Value);
		if (!ConvertedValue.IsSet())
			return false;

		auto Signal = SignalT::create(*ConvertedValue, PLXInput->second);
		InputQueue->send(Signal);
		return true;
	}

	template <typename T>
	TOptional<T> TypeMismatchResult(const FOpenPLX_Output& Output)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Unexpected error: Tried to cast OpenPLX Output '%s' to it's corresponding "
				 "OpenPLX type but got nullptr. Possible type miss match. The signal will not "
				 "be received."),
			*Output.Name.ToString());
		return {};
	}

	TOptional<double> GetRealValueFrom(
		const FOpenPLX_Output& Output, openplx::Physics::Signals::ValueOutputSignal* Signal)
	{
		if (Signal == nullptr)
			return {};

		auto Value =
			std::dynamic_pointer_cast<openplx::Physics::Signals::RealValue>(Signal->value());
		if (Value == nullptr)
			return TypeMismatchResult<double>(Output);

		switch (Output.Type)
		{
			case EOpenPLX_OutputType::DurationOutput:
			case EOpenPLX_OutputType::AutomaticClutchEngagementDurationOutput:
			case EOpenPLX_OutputType::AutomaticClutchDisengagementDurationOutput:
			case EOpenPLX_OutputType::FractionOutput:
			case EOpenPLX_OutputType::Force1DOutput:
			case EOpenPLX_OutputType::Torque1DOutput:
			case EOpenPLX_OutputType::TorqueConverterPumpTorqueOutput:
			case EOpenPLX_OutputType::TorqueConverterTurbineTorqueOutput:
				return Value->value();
			case EOpenPLX_OutputType::AngleOutput:
			case EOpenPLX_OutputType::AngularVelocity1DOutput:
				return ConvertAngleToUnreal<double>(Value->value());
			case EOpenPLX_OutputType::LinearVelocity1DOutput:
			case EOpenPLX_OutputType::Position1DOutput:
			case EOpenPLX_OutputType::RelativeVelocity1DOutput:
				return ConvertDistanceToUnreal<double>(Value->value());
		}

		UE_LOG(
			LogAGX, Warning,
			TEXT("Tried to read Real type from signal for Output '%s', but the type is either "
				 "not of Real type or is unsupported."),
			*Output.Name.ToString());
		return {};
	}

	TOptional<FVector2D> GetVector2DValueFrom(
		const FOpenPLX_Output& Output, openplx::Physics::Signals::ValueOutputSignal* Signal)
	{
		if (Signal == nullptr)
			return {};

		auto Value =
			std::dynamic_pointer_cast<openplx::Physics::Signals::Vec2Value>(Signal->value());
		if (Value == nullptr)
			return TypeMismatchResult<FVector2D>(Output);

		switch (Output.Type)
		{
			case EOpenPLX_OutputType::ForceRangeOutput:
			case EOpenPLX_OutputType::TorqueRangeOutput:
				return FVector2D(Value->value()->x(), Value->value()->y());
		}

		UE_LOG(
			LogAGX, Warning,
			TEXT("Tried to read vec2 vector type from signal for Output '%s', but the type is "
				 "either not of vec2 vector type or is unsupported."),
			*Output.Name.ToString());
		return {};
	}

	TOptional<FVector> GetVectorValueFrom(
		const FOpenPLX_Output& Output, openplx::Physics::Signals::ValueOutputSignal* Signal)
	{
		if (Signal == nullptr)
			return {};

		auto Value =
			std::dynamic_pointer_cast<openplx::Physics::Signals::Vec3Value>(Signal->value());
		if (Value == nullptr)
			return TypeMismatchResult<FVector>(Output);

		switch (Output.Type)
		{
			case EOpenPLX_OutputType::AngularVelocity3DOutput:
			case EOpenPLX_OutputType::MateConnectorAngularAcceleration3DOutput:
			case EOpenPLX_OutputType::MateConnectorRPYOutput:
			case EOpenPLX_OutputType::RPYOutput:
			{
				return FVector(
					FMath::RadiansToDegrees(Value->value()->x()),
					-FMath::RadiansToDegrees(Value->value()->y()),
					-FMath::RadiansToDegrees(Value->value()->z()));
			}
			case EOpenPLX_OutputType::LinearVelocity3DOutput:
			case EOpenPLX_OutputType::MateConnectorAcceleration3DOutput:
			case EOpenPLX_OutputType::MateConnectorPositionOutput:
			case EOpenPLX_OutputType::Position3DOutput:
			{
				return FVector(
					ConvertDistanceToUnreal<double>(Value->value()->x()),
					-ConvertDistanceToUnreal<double>(Value->value()->y()),
					ConvertDistanceToUnreal<double>(Value->value()->z()));
			}
			case EOpenPLX_OutputType::Force3DOutput:
				return ConvertVector(
					agx::Vec3(Value->value()->x(), Value->value()->y(), Value->value()->z()));
			case EOpenPLX_OutputType::Torque3DOutput:
				return ConvertTorque(
					agx::Vec3(Value->value()->x(), Value->value()->y(), Value->value()->z()));
		}

		UE_LOG(
			LogAGX, Warning,
			TEXT("Tried to read vec3 vector type from signal for Output '%s', but the type is "
				 "either not of vec3 vector type or is unsupported."),
			*Output.Name.ToString());
		return {};
	}

	TOptional<int64> GetIntegerValueFrom(
		const FOpenPLX_Output& Output, openplx::Physics::Signals::ValueOutputSignal* Signal)
	{
		if (Signal == nullptr)
			return {};

		auto Value =
			std::dynamic_pointer_cast<openplx::Physics::Signals::IntValue>(Signal->value());
		if (Value == nullptr)
			return TypeMismatchResult<int64>(Output);

		switch (Output.Type)
		{
			case EOpenPLX_OutputType::IntOutput:
				return Value->value();
		}

		UE_LOG(
			LogAGX, Warning,
			TEXT("Tried to read integer type from signal for Output '%s', but the type is "
				 "either "
				 "not of integer type or is unsupported."),
			*Output.Name.ToString());
		return {};
	}

	TOptional<bool> GetBooleanValueFrom(
		const FOpenPLX_Output& Output, openplx::Physics::Signals::ValueOutputSignal* Signal)
	{
		if (Signal == nullptr)
			return {};

		auto Value =
			std::dynamic_pointer_cast<openplx::Physics::Signals::BoolValue>(Signal->value());
		if (Value == nullptr)
			return TypeMismatchResult<bool>(Output);

		switch (Output.Type)
		{
			case EOpenPLX_OutputType::BoolOutput:
			case EOpenPLX_OutputType::ActivatedOutput:
			case EOpenPLX_OutputType::InteractionEnabledOutput:
			case EOpenPLX_OutputType::EngagedOutput:
			case EOpenPLX_OutputType::TorqueConverterLockedUpOutput:
				return Value->value();
		}

		UE_LOG(
			LogAGX, Warning,
			TEXT("Tried to read boolean type from signal for Output '%s', but the type is "
				 "either "
				 "not of boolean type or is unsupported."),
			*Output.Name.ToString());
		return {};
	}

	template <typename ValueT, typename ValueGetterFuncT>
	bool Receive(
		const FOpenPLX_Output& Output, ValueT& OutValue, FOpenPLXModelRegistry* ModelRegistry,
		FOpenPLXModelRegistry::Handle ModelHandle,
		std::shared_ptr<agxopenplx::OutputSignalQueue> OutputQueue,
		ValueGetterFuncT Func)
	{
		if (ModelRegistry == nullptr || ModelHandle == FOpenPLXModelRegistry::InvalidHandle)
			return false;

		if (OutputQueue == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Tried to receive OpenPLX Output signal for output '%s', but the OpenPLX "
					 "model does not have any registered outputs."),
				*Output.Name.ToString());
			return false;
		}

		auto Signal =
			agxopenplx::getSignalBySourceName<openplx::Physics::Signals::ValueOutputSignal>(
				OutputQueue->getSignals(), Convert(Output.Name.ToString()));
		if (Signal == nullptr)
			return false;

		auto Value = Func(Output, Signal.get());
		if (!Value.IsSet())
			return false;

		OutValue = *Value;
		return true;
	}
}

bool FOpenPLXSignalHandler::Send(const FOpenPLX_Input& Input, double Value)
{
	check(IsInitialized());
	return OpenPLXSignalHandler_helpers::Send<double, openplx::Physics::Signals::RealInputSignal>(
		Input, Value, ModelRegistry, ModelHandle, InputSignalListenerRef->Native->getQueue(),
		OpenPLXSignalHandler_helpers::ConvertReal);
}

bool FOpenPLXSignalHandler::Receive(const FOpenPLX_Output& Output, double& OutValue)
{
	check(IsInitialized());
	return OpenPLXSignalHandler_helpers::Receive(
		Output, OutValue, ModelRegistry, ModelHandle, OutputSignalListenerRef->Native->getQueue(),
		OpenPLXSignalHandler_helpers::GetRealValueFrom);
}

bool FOpenPLXSignalHandler::Send(const FOpenPLX_Input& Input, const FVector2D& Value)
{
	check(IsInitialized());
	return OpenPLXSignalHandler_helpers::Send<
		FVector2D, openplx::Physics::Signals::RealRangeInputSignal>(
		Input, Value, ModelRegistry, ModelHandle, InputSignalListenerRef->Native->getQueue(),
		OpenPLXSignalHandler_helpers::ConvertVector2D);
}

bool FOpenPLXSignalHandler::Receive(const FOpenPLX_Output& Output, FVector2D& OutValue)
{
	check(IsInitialized());
	return OpenPLXSignalHandler_helpers::Receive(
		Output, OutValue, ModelRegistry, ModelHandle, OutputSignalListenerRef->Native->getQueue(),
		OpenPLXSignalHandler_helpers::GetVector2DValueFrom);
}

bool FOpenPLXSignalHandler::Send(const FOpenPLX_Input& Input, const FVector& Value)
{
	check(IsInitialized());
	return OpenPLXSignalHandler_helpers::Send<FVector, openplx::Physics::Signals::Vec3InputSignal>(
		Input, Value, ModelRegistry, ModelHandle, InputSignalListenerRef->Native->getQueue(),
		OpenPLXSignalHandler_helpers::ConvertVector3D);
}

bool FOpenPLXSignalHandler::Receive(const FOpenPLX_Output& Output, FVector& OutValue)
{
	check(IsInitialized());
	return OpenPLXSignalHandler_helpers::Receive(
		Output, OutValue, ModelRegistry, ModelHandle, OutputSignalListenerRef->Native->getQueue(),
		OpenPLXSignalHandler_helpers::GetVectorValueFrom);
}

bool FOpenPLXSignalHandler::Send(const FOpenPLX_Input& Input, int64 Value)
{
	check(IsInitialized());
	return OpenPLXSignalHandler_helpers::Send<int64, openplx::Physics::Signals::IntInputSignal>(
		Input, Value, ModelRegistry, ModelHandle, InputSignalListenerRef->Native->getQueue(),
		OpenPLXSignalHandler_helpers::ConvertInteger);
}

bool FOpenPLXSignalHandler::Receive(const FOpenPLX_Output& Output, int64& OutValue)
{
	check(IsInitialized());
	return OpenPLXSignalHandler_helpers::Receive(
		Output, OutValue, ModelRegistry, ModelHandle, OutputSignalListenerRef->Native->getQueue(),
		OpenPLXSignalHandler_helpers::GetIntegerValueFrom);
}

bool FOpenPLXSignalHandler::Send(const FOpenPLX_Input& Input, bool Value)
{
	check(IsInitialized());
	return OpenPLXSignalHandler_helpers::Send<bool, openplx::Physics::Signals::BoolInputSignal>(
		Input, Value, ModelRegistry, ModelHandle, InputSignalListenerRef->Native->getQueue(),
		OpenPLXSignalHandler_helpers::ConvertBoolean);
}

bool FOpenPLXSignalHandler::Receive(const FOpenPLX_Output& Output, bool& OutValue)
{
	check(IsInitialized());
	return OpenPLXSignalHandler_helpers::Receive(
		Output, OutValue, ModelRegistry, ModelHandle, OutputSignalListenerRef->Native->getQueue(),
		OpenPLXSignalHandler_helpers::GetBooleanValueFrom);
}

void FOpenPLXSignalHandler::ReleaseNatives()
{
	ModelRegistry = nullptr;
	AssemblyRef = nullptr;
	InputSignalListenerRef = nullptr;
	OutputSignalListenerRef = nullptr;
}

void FOpenPLXSignalHandler::SetNativeAddresses(
	const FOpenPLX_SignalHandlerNativeAddresses& Addresses)
{
	AssemblyRef->Native = reinterpret_cast<agxSDK::Assembly*>(Addresses.AssemblyAddress);
	InputSignalListenerRef->Native =
		reinterpret_cast<agxopenplx::InputSignalListener*>(Addresses.InputSignalListenerAddress);
	OutputSignalListenerRef->Native =
		reinterpret_cast<agxopenplx::OutputSignalListener*>(Addresses.OutputSignalListenerAddress);
	ModelRegistry = reinterpret_cast<FOpenPLXModelRegistry*>(Addresses.ModelRegistryAddress);
	ModelHandle = Addresses.ModelHandle;
	bIsInitialized = true;
}

FOpenPLX_SignalHandlerNativeAddresses FOpenPLXSignalHandler::GetNativeAddresses() const
{
	FOpenPLX_SignalHandlerNativeAddresses Addresses;
	if (AssemblyRef->Native != nullptr)
		Addresses.AssemblyAddress = reinterpret_cast<uint64>(AssemblyRef->Native.get());

	if (InputSignalListenerRef->Native != nullptr)
		Addresses.InputSignalListenerAddress =
			reinterpret_cast<uint64>(InputSignalListenerRef->Native.get());

	if (OutputSignalListenerRef->Native != nullptr)
		Addresses.OutputSignalListenerAddress =
			reinterpret_cast<uint64>(OutputSignalListenerRef->Native.get());

	if (ModelRegistry != nullptr)
		Addresses.ModelRegistryAddress = reinterpret_cast<uint64>(ModelRegistry);

	Addresses.ModelHandle = ModelHandle;

	return Addresses;
}
