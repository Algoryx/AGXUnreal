// Copyright 2026, Algoryx Simulation AB.

#include "OpenPLX/OpenPLXSignalHandler.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "BarrierOnly/AGXRefs.h"
#include "BarrierOnly/OpenPLX/OpenPLXRefs.h"
#include "BarrierOnly/AGXTypeConversions.h"
#include "Constraints/ConstraintBarrier.h"
#include "OpenPLX/OpenPLX_Inputs.h"
#include "OpenPLX/OpenPLX_Outputs.h"
#include "OpenPLX/OpenPLX_SignalHandlerNativeAddresses.h"
#include "OpenPLX/OpenPLXMappingBarriersCollection.h"
#include "RigidBodyBarrier.h"
#include "SimulationBarrier.h"
#include "Utilities/PLXUtilitiesInternal.h"

// OpenPLX includes.
#include "BeginAGXIncludes.h"
#include "agxOpenPLX/SignalListenerUtils.h"
#include "agxOpenPLX/AgxObjectMap.h"
#include "agxOpenPLX/AgxOpenPlxApi.h"
#include "openplx/ControlDispatch.h"
#include "openplx/ControlInterface.h"
#include <openplx/HeapControlInterface.h>
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
	, HeapControlInterfaceRef {std::make_shared<FHeapControlInterfaceRef>()}
{
}

void FOpenPLXSignalHandler::Init(
	const FString& OpenPLXFile, FSimulationBarrier& Simulation,
	FOpenPLXModelRegistry& InModelRegistry, const FOpenPLXMappingBarriersCollection& Barriers)
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

	AssemblyRef->Native = FPLXUtilitiesInternal::MapRuntimeObjects(System, Simulation, Barriers);
	if (AssemblyRef->Native == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Unable to get a valid AGX Assembly from simulated model instance for OpenPLX "
				 "model '%s'. The Output Log may contain more details."),
			*OpenPLXFile);
		return;
	}

	// OpenPLX uses pretty deep namespace nesting which causes very long lines. Namespace
	// aliases helps us cut those lengths down a bit while still keeping it clear which types comes
	// from OpenPLX.
	namespace oplx = openplx;
	namespace oplxSignals = openplx::Physics::Signals;
	namespace aoplx = agxopenplx;
	using ControlDispatchSPtr = std::shared_ptr<oplx::ControlDispatch>;
	using ControlInterfaceSPtr = std::shared_ptr<oplx::ControlInterface>;
	using SignalInterfaceSPtr = std::shared_ptr<oplxSignals::SignalInterface>;
	using std::make_shared;

	std::shared_ptr<aoplx::AgxMetadata> AgxMetadata = std::make_shared<aoplx::AgxMetadata>();

	std::shared_ptr<agxopenplx::AgxObjectMap> AgxObjectMap;
	if (FPLXUtilitiesInternal::HasInputs(System.get()) ||
		FPLXUtilitiesInternal::HasOutputs(System.get()))
	{
		auto PlxPowerLine = dynamic_cast<agxPowerLine::PowerLine*>(
			AssemblyRef->Native->getAssembly(FPLXUtilitiesInternal::GetDefaultPowerLineName()));

		AgxObjectMap = agxopenplx::AgxObjectMap::create(
			AssemblyRef->Native, PlxPowerLine, nullptr, agxopenplx::AgxObjectMapMode::Name);
	}

	if (FPLXUtilitiesInternal::HasInputs(System.get()))
	{
		auto InputSignalQue = agxopenplx::InputSignalQueue::create();
		InputSignalListenerRef->Native =
			new agxopenplx::InputSignalListener(InputSignalQue, AgxObjectMap, AgxMetadata);
		Simulation.GetNative()->Native->add(InputSignalListenerRef->Native);
	}

	if (FPLXUtilitiesInternal::HasOutputs(System.get()))
	{
		auto OutputSignalQueue = agxopenplx::OutputSignalQueue::create();
		OutputSignalListenerRef->Native = new agxopenplx::OutputSignalListener(
			ModelData->OpenPLXModel, OutputSignalQueue, AgxObjectMap, AgxMetadata);
		Simulation.GetNative()->Native->add(OutputSignalListenerRef->Native);
	}

	/*
	 * Control Interface setup.
	 */

	ControlDispatchSPtr ControlDispatch = make_shared<oplx::ControlDispatch>();
	aoplx::register_control_handlers(*ControlDispatch, AgxObjectMap, AgxMetadata);
	ControlInterfaceSPtr ControlInterface = make_shared<oplx::ControlInterface>(ControlDispatch);
	std::vector<SignalInterfaceSPtr> SignalInterfaces =
		FPLXUtilitiesInternal::GetNestedObjects<oplxSignals::SignalInterface>(*System);
	for (SignalInterfaceSPtr& SignalInterface : SignalInterfaces)
	{
		ControlInterface->add_controls_from_signal_interface(SignalInterface);
	}
	ControlInterface->prepare_controls();
	HeapControlInterfaceRef->Native =
		std::make_shared<openplx::HeapControlInterface>(ControlInterface);

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
		std::shared_ptr<agxopenplx::InputSignalQueue> InputQueue, ConversionFuncT Func)
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

	TOptional<double> TypeConvertRealToUnreal(const FOpenPLX_Output& Output, double Value)
	{
		switch (Output.Type)
		{
			case EOpenPLX_OutputType::DurationOutput:
			case EOpenPLX_OutputType::AutomaticClutchEngagementDurationOutput:
			case EOpenPLX_OutputType::AutomaticClutchDisengagementDurationOutput:
			case EOpenPLX_OutputType::FractionOutput:
			case EOpenPLX_OutputType::Force1DOutput:
			case EOpenPLX_OutputType::MassOutput:
			case EOpenPLX_OutputType::RatioOutput:
			case EOpenPLX_OutputType::RpmOutput:
			case EOpenPLX_OutputType::Torque1DOutput:
			case EOpenPLX_OutputType::TorqueConverterPumpTorqueOutput:
			case EOpenPLX_OutputType::TorqueConverterTurbineTorqueOutput:
				return Value;
			case EOpenPLX_OutputType::AngleOutput:
			case EOpenPLX_OutputType::AngularVelocity1DOutput:
				return ConvertAngleToUnreal<double>(Value);
			case EOpenPLX_OutputType::LinearVelocity1DOutput:
			case EOpenPLX_OutputType::Position1DOutput:
			case EOpenPLX_OutputType::RelativeVelocity1DOutput:
				return ConvertDistanceToUnreal<double>(Value);
		}

		UE_LOG(
			LogAGX, Warning,
			TEXT("Tried to convert Real type from value for Output '%s', but the type is either "
				 "not of Real type or is unsupported."),
			*Output.Alias.ToString());
		return {};
	}

	TOptional<double> TypeConvertRealToAGX(const FOpenPLX_Input& Input, double Value)
	{
		switch (Input.Type)
		{
			case EOpenPLX_InputType::ActivateInput:
			case EOpenPLX_InputType::DurationInput:
			case EOpenPLX_InputType::EnableInteractionInput:
			case EOpenPLX_InputType::EngageInput:
			case EOpenPLX_InputType::AutomaticClutchEngagementDurationInput:
			case EOpenPLX_InputType::AutomaticClutchDisengagementDurationInput:
			case EOpenPLX_InputType::FractionInput:
			case EOpenPLX_InputType::Force1DInput:
			case EOpenPLX_InputType::ForceRangeInput:
			case EOpenPLX_InputType::IntInput:
			case EOpenPLX_InputType::BoolInput:
			case EOpenPLX_InputType::Torque1DInput:
			case EOpenPLX_InputType::TorqueRangeInput:
			case EOpenPLX_InputType::TorqueConverterLockUpInput:
				return Value;
			case EOpenPLX_InputType::AngleInput:
			case EOpenPLX_InputType::AngularVelocity1DInput:
			case EOpenPLX_InputType::AngularVelocity3DInput:
				return ConvertAngleToAGX(Value);
			case EOpenPLX_InputType::LinearVelocity1DInput:
			case EOpenPLX_InputType::LinearVelocity3DInput:
			case EOpenPLX_InputType::Position1DInput:
				return ConvertDistanceToAGX(Value);
		}

		UE_LOG(
			LogAGX, Warning,
			TEXT("Tried to convert Real type from value for Output '%s', but the type is either "
				 "not of Real type or is unsupported."),
			*Input.Alias.ToString());
		return {};
	}

	TOptional<FVector2D> TypeConvertRangeRealToUnreal(
		const FOpenPLX_Output& Output, const FVector2D& Value)
	{
		switch (Output.Type)
		{
			case EOpenPLX_OutputType::ForceRangeOutput:
			case EOpenPLX_OutputType::TorqueRangeOutput:
				return FVector2D(Value.X, Value.Y);
		}

		UE_LOG(
			LogAGX, Warning,
			TEXT("OpenPLX Control Interface: Tried to read vec2 vector type from signal for Output "
				 "'%s', but the type is either not of vec2 vector type or is unsupported."),
			*Output.Alias.ToString());
		return {};
	}

	TOptional<FVector2D> TypeConvertRangeRealToAGX(
		const FOpenPLX_Input& Input, const FVector2D& Value)
	{
		switch (Input.Type)
		{
			case EOpenPLX_InputType::ForceRangeInput:
			case EOpenPLX_InputType::TorqueRangeInput:
				return Value;
		}

		UE_LOG(
			LogAGX, Warning,
			TEXT("OpenPLX Control Interface: Tried to write vec2 vector type for Input '%s', but "
				 "the type is either not of vec2 vector type or is unsupported."),
			*Input.Alias.ToString());
		return {};
	}

	TOptional<FVector> TypeConvertVectorToUnreal(
		const FOpenPLX_Output& Output, const agx::Vec3& Value)
	{
		switch (Output.Type)
		{
			case EOpenPLX_OutputType::Force3DOutput:
				return ConvertVector(Value);
			case EOpenPLX_OutputType::Torque3DOutput:
				return ConvertTorque(Value);
			case EOpenPLX_OutputType::LinearVelocity3DOutput:
			case EOpenPLX_OutputType::MateConnectorAcceleration3DOutput:
			case EOpenPLX_OutputType::Position3DOutput:
				return ConvertDisplacement(Value);
			case EOpenPLX_OutputType::MateConnectorRPYOutput:
			case EOpenPLX_OutputType::RPYOutput:
				return ConvertAngle(Value);
			case EOpenPLX_OutputType::AngularVelocity3DOutput:
				return ConvertAngularVelocity(Value);
		}

		UE_LOG(
			LogAGX, Warning,
			TEXT("OpenPLX Control Interface: Tried to write Vec3 vector type for input '%s', but "
				 "the type is either not of Vec3 vector type or is  unsupported."),
			*Output.Alias.ToString());
		return {};
	}

	TOptional<agx::Vec3> TypeConvertVectorToAGX(const FOpenPLX_Input& Input, const FVector& Vector)
	{
		switch (Input.Type)
		{
			case EOpenPLX_InputType::AngularVelocity3DInput:
				return ConvertAngularVelocity(Vector);
			case EOpenPLX_InputType::LinearVelocity3DInput:
				return ConvertDisplacement(Vector);
		}

		UE_LOG(
			LogAGX, Warning,
			TEXT("OpenPLX Control Interface: Tried to read Vec3 type for output '%s', but the type "
				 "is either not of Vec3 vector type or is unsupported."),
			*Input.Alias.ToString());
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
			case EOpenPLX_OutputType::MassOutput:
			case EOpenPLX_OutputType::RatioOutput:
			case EOpenPLX_OutputType::RpmOutput:
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
		std::shared_ptr<agxopenplx::OutputSignalQueue> OutputQueue, ValueGetterFuncT Func)
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

	template <typename ValueT, typename ValueGetterFuncT>
	bool SendInterface(
		const FOpenPLX_Input& Input, ValueT Value,
		FHeapControlInterfaceRef* HeapControlInterfaceRef, ValueGetterFuncT ValueGetterFunc)
	{
		if (HeapControlInterfaceRef == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Tried to send OpenPLX Output signal for output '%s', but don't have a "
					 "Control Interface wrapper reference"),
				*Input.Alias.ToString());
			return false;
		}

		openplx::HeapControlInterface* Interface = HeapControlInterfaceRef->Native.get();
		if (Interface == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Tried to send OpenPLX Input signal for input '%s', through the Control "
					 "Interface, but don't have a Control Interface instance."),
				*Input.Alias.ToString());
			return false;
		}

		TOptional<ValueT> ConvertedValueMaybe = ValueGetterFunc(Input, Value);
		if (!ConvertedValueMaybe)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("OpenPLX Control Interface: Type and unit conversion from Unreal to AGX "
					 "Dynamics failed for OpenPLX input '%s'."),
				*Input.Alias.ToString());
			return false;
		}

		ValueT ConvertedValue = *ConvertedValueMaybe;
		std::string Alias = Convert(Input.Alias.ToString());
		bool bWritten = Interface->write<ValueT>(Alias, ConvertedValue);
		if (!bWritten)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("OpenPLX Control Interface: Tried to send to Input '%s' but the write was "
					 "rejected by the Control Interface"),
				*Input.Alias.ToString());
			return false;
		}

		return true;
	}

	template <typename ValueT, typename ValueGetterFuncT>
	bool ReceiveInterface(
		const FOpenPLX_Output& Output, ValueT& OutValue,
		FHeapControlInterfaceRef* HeapControlInterfaceRef, ValueGetterFuncT ValueGetterFunc)
	{
		if (HeapControlInterfaceRef == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Tried to receive OpenPLX Output signal for output '%s' through the Control "
					 "Interface, but don't have a Control Interface wrapper reference."),
				*Output.Name.ToString());
			OutValue = {};
			return false;
		}

		openplx::HeapControlInterface* Interface = HeapControlInterfaceRef->Native.get();
		if (Interface == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Tried to receive OpenPLX Output signal for output '%s' through the Control "
					 "Interface, but don't have a Control Interface pointer."),
				*Output.Name.ToString());
			OutValue = {};
			return false;
		}

		std::string Alias = Convert(Output.Alias.ToString());
		std::optional<ValueT> Value = Interface->read<ValueT>(Alias);
		if (!Value)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("OpenPLX Signal Interface: Received null value for Output '%s'."),
				*Output.Name.ToString());
			OutValue = {};
			return false;
		}

		TOptional<ValueT> ConvertedValue = ValueGetterFunc(Output, *Value);
		if (!ConvertedValue)
		{
			const FString TypeName = StaticEnum<EOpenPLX_OutputType>()
										 ->GetDisplayNameTextByValue((int64) Output.Type)
										 .ToString();
			UE_LOG(
				LogAGX, Warning,
				TEXT("OpenPLX Signal Interface: Could not convert value for '%s' of type '%s' to "
					 "Unreal type and unit."),
				*Output.Alias.ToString(), *TypeName);
			OutValue = {};
			return false;
		}
		OutValue = *ConvertedValue;
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

bool FOpenPLXSignalHandler::SendInterface(const FOpenPLX_Input& Input, double Value)
{
	check(IsInitialized());
	return OpenPLXSignalHandler_helpers::SendInterface(
		Input, Value, HeapControlInterfaceRef.get(),
		OpenPLXSignalHandler_helpers::TypeConvertRealToAGX);
}

bool FOpenPLXSignalHandler::Receive(const FOpenPLX_Output& Output, double& OutValue)
{
	check(IsInitialized());
	return OpenPLXSignalHandler_helpers::Receive(
		Output, OutValue, ModelRegistry, ModelHandle, OutputSignalListenerRef->Native->getQueue(),
		OpenPLXSignalHandler_helpers::GetRealValueFrom);
}

bool FOpenPLXSignalHandler::ReceiveInterface(const FOpenPLX_Output& Output, double& OutValue)
{
	check(IsInitialized());
	return OpenPLXSignalHandler_helpers::ReceiveInterface(
		Output, OutValue, HeapControlInterfaceRef.get(),
		OpenPLXSignalHandler_helpers::TypeConvertRealToUnreal);
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

bool FOpenPLXSignalHandler::ReceiveInterface(const FOpenPLX_Output& Output, FVector2D& OutValue)
{
	using namespace OpenPLXSignalHandler_helpers;
	check(IsInitialized());
	OutValue = {};

	if (HeapControlInterfaceRef == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Tried to receive OpenPLX Output signal for output '%s' through the Control "
				 "Interface, but don't have a Control Interface wrapper reference."),
			*Output.Alias.ToString());
		return false;
	}

	openplx::HeapControlInterface* Interface = HeapControlInterfaceRef->Native.get();
	if (Interface == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Tried to receive OpenPLX Output signal for output '%s' through the Control "
				 "Interface, but don't have a Control Interface pointer."),
			*Output.Alias.ToString());
		return false;
	}

	std::string Alias = Convert(Output.Alias.ToString());
	std::shared_ptr<openplx::Marshalling> M = Interface->prepare_read(Alias);
	if (M == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("OpenPLX Control Interface: Could not prepare read for Output '%s'. The alias may "
				 "not match any registered output."),
			*Output.Alias.ToString());
		return false;
	}

	auto Min = M->read_real<double>("min");
	auto Max = M->read_real<double>("max");
	if (!Min || !Max)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("OpenPLX Control Interface: Could not read 'min'/'max' fields for Output '%s'."),
			*Output.Alias.ToString());
		return false;
	}

	TOptional<FVector2D> Converted = TypeConvertRangeRealToUnreal(Output, FVector2D(*Min, *Max));
	if (!Converted)
	{
		const FString TypeName = StaticEnum<EOpenPLX_OutputType>()
									 ->GetDisplayNameTextByValue((int64) Output.Type)
									 .ToString();
		UE_LOG(
			LogAGX, Warning,
			TEXT("OpenPLX Control Interface: Could not convert value for '%s' of type '%s' to "
				 "Unreal type and unit."),
			*Output.Alias.ToString(), *TypeName);
		return false;
	}

	OutValue = *Converted;
	return true;
}

bool FOpenPLXSignalHandler::SendInterface(const FOpenPLX_Input& Input, const FVector2D& Value)
{
	using namespace OpenPLXSignalHandler_helpers;
	check(IsInitialized());

	if (HeapControlInterfaceRef == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Tried to send OpenPLX Input signal for input '%s' through the Control Interface, "
				 "but don't have a Control Interface wrapper reference."),
			*Input.Alias.ToString());
		return false;
	}

	openplx::HeapControlInterface* Interface = HeapControlInterfaceRef->Native.get();
	if (Interface == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Tried to send OpenPLX Input signal for input '%s' through the Control Interface, "
				 "but don't have a Control Interface pointer."),
			*Input.Alias.ToString());
		return false;
	}

	TOptional<FVector2D> ConvertedMaybe = TypeConvertRangeRealToAGX(Input, Value);
	if (!ConvertedMaybe)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("OpenPLX Control Interface: Could not convert value for Input '%s' from Unreal "
				 "type and unit to AGX."),
			*Input.Alias.ToString());
		return false;
	}

	std::string Alias = Convert(Input.Alias.ToString());
	std::shared_ptr<openplx::Marshalling> M = Interface->prepare_write(Alias);
	if (M == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("OpenPLX Control Interface: Could not prepare write for Input '%s'. The alias may "
				 "not match any registered input."),
			*Input.Alias.ToString());
		return false;
	}

	const bool bWroteMin = M->write_real<double>("min", ConvertedMaybe->X);
	const bool bWroteMax = M->write_real<double>("max", ConvertedMaybe->Y);
	Interface->flush();

	if (!bWroteMin || !bWroteMax)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("OpenPLX Control Interface: Failed to write 'min'/'max' fields for Input '%s'."),
			*Input.Alias.ToString());
		return false;
	}

	return true;
}

bool FOpenPLXSignalHandler::Send(const FOpenPLX_Input& Input, const FVector& Value)
{
	check(IsInitialized());
	return OpenPLXSignalHandler_helpers::Send<FVector, openplx::Physics::Signals::Vec3InputSignal>(
		Input, Value, ModelRegistry, ModelHandle, InputSignalListenerRef->Native->getQueue(),
		OpenPLXSignalHandler_helpers::ConvertVector3D);
}

bool FOpenPLXSignalHandler::SendInterface(const FOpenPLX_Input& Input, const FVector& Value)
{
	using namespace OpenPLXSignalHandler_helpers;
	check(IsInitialized());

	if (HeapControlInterfaceRef == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("FOpenPLXSignalHandler::SendInterface(FVector): Tried to send OpenPLX input "
				 "signal for input '%s' through the Control Interrace, but don't have a Control "
				 "Interface wrapper reference."),
			*Input.Alias.ToString());
		return false;
	}

	openplx::HeapControlInterface* Interface = HeapControlInterfaceRef->Native.get();
	if (Interface == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("OpenPLXSignalHandler::SendInterface(FVector): Tried to send OpenPLX signal for "
				 "input '%s' through Control Interface, but don't have a Control Interface "
				 "pointer."),
			*Input.Alias.ToString());
		return false;
	}

	TOptional<agx::Vec3> ValueAGXMaybe = TypeConvertVectorToAGX(Input, Value);
	if (!ValueAGXMaybe)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("OpenPLXSignalHandler::SendInterface(FVector): Could not convert value for input "
				 "'%s' from Unreal type and unit to AGX Dynamics"),
			*Input.Alias.ToString());
		return false;
	}

	agx::Vec3 ValueAGX = *ValueAGXMaybe;

	std::string Alias = Convert(Input.Alias.ToString());
	std::shared_ptr<openplx::Marshalling> Marshalling = Interface->prepare_write(Alias);
	if (Marshalling == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("OpenPLXSignalHandler::SendInterface(FVector): Could not prepare write for input "
				 "'%s'. The alias may not match any registered input."),
			*Input.Alias.ToString());
		return false;
	}

	const bool bWroteX = Marshalling->write_real("x", ValueAGXMaybe->x());
	const bool bWroteY = Marshalling->write_real("y", ValueAGXMaybe->y());
	const bool bWroteZ = Marshalling->write_real("z", ValueAGXMaybe->z());
	Interface->flush();
	if (!bWroteX || !bWroteY || !bWroteZ)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("OpenPLXSignalHandler::SendInterface(FVector): Marshalling failed to write data "
				 "for input '%s'."),
			*Input.Alias.ToString());
		return false;
	}

	return true;
}

bool FOpenPLXSignalHandler::Receive(const FOpenPLX_Output& Output, FVector& OutValue)
{
	check(IsInitialized());
	return OpenPLXSignalHandler_helpers::Receive(
		Output, OutValue, ModelRegistry, ModelHandle, OutputSignalListenerRef->Native->getQueue(),
		OpenPLXSignalHandler_helpers::GetVectorValueFrom);
}

bool FOpenPLXSignalHandler::ReceiveInterface(const FOpenPLX_Output& Output, FVector& OutValue)
{
	using namespace OpenPLXSignalHandler_helpers;
	check(IsInitialized());
	OutValue = {};

	if (HeapControlInterfaceRef == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("OpenPLXSignalHandler::ReceiveInterface(FVector): Tried to receive OpenPLX signal "
				 "for input '%s' through Control Interface, but don't have a Control Interface "
				 "wrapper reference"),
			*Output.Alias.ToString());
		return false;
	}

	openplx::HeapControlInterface* Interface = HeapControlInterfaceRef->Native.get();
	if (Interface == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("OpenPLXSignalHandler::ReceiveInterface(FVector): Tried to receive OpenPLX signal "
				 "for input '%s' through Control Interface, but don't have a Control Interface "
				 "pointer."),
			*Output.Alias.ToString());
		return false;
	}

	std::string Alias = Convert(Output.Alias.ToString());
	std::shared_ptr<openplx::Marshalling> Marshalling = Interface->prepare_read(Alias);
	if (Marshalling == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("OpenPLXSignalHandler::ReceiveInterface(FVector): Tried to receive OpenPLX signal "
				 "for input '%s' through Control Interface, but could not prepare read. The alias "
				 "might not match any registered output."),
			*Output.Alias.ToString());
		return false;
	}

	std::optional<double> XMaybe = Marshalling->read_real<double>("x");
	std::optional<double> YMaybe = Marshalling->read_real<double>("y");
	std::optional<double> ZMaybe = Marshalling->read_real<double>("z");
	if (!XMaybe || !YMaybe || !ZMaybe)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("OpenPLXSignalHandler::ReceiveInterface(FVector): Tried to receive OpenPLX signal "
				 "for input '%s' through Control Interface, but could not read at least one field "
				 "from the output."),
			*Output.Alias.ToString());
		return false;
	}

	agx::Vec3 ValueAGX(*XMaybe, *YMaybe, *ZMaybe);
	TOptional<FVector> ValueUnreal = TypeConvertVectorToUnreal(Output, ValueAGX);
	if (!ValueUnreal)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("OpenPLXSignalHandler::ReceiveInterface(FVector): Tried to receive OpenPLX signal "
				 "for input '%s' through Control Interface, but could not convert AGX Dynamics "
				 "type and unit to Unreal."),
			*Output.Alias.ToString());
		return false;
	}

	OutValue = *ValueUnreal;
	return true;
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
	AssemblyRef->Native = nullptr;
	InputSignalListenerRef->Native = nullptr;
	OutputSignalListenerRef->Native = nullptr;
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
