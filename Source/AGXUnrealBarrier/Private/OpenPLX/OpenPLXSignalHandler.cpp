// Copyright 2026, Algoryx Simulation AB.

#include "OpenPLX/OpenPLXSignalHandler.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "BarrierOnly/AGXRefs.h"
#include "BarrierOnly/OpenPLX/OpenPLXRefs.h"
#include "BarrierOnly/AGXTypeConversions.h"
#include "OpenPLX/OpenPLX_Inputs.h"
#include "OpenPLX/OpenPLX_Outputs.h"
#include "OpenPLX/OpenPLX_SignalHandlerNativeAddresses.h"
#include "OpenPLX/OpenPLXMappingBarriersCollection.h"
#include "SimulationBarrier.h"
#include "Utilities/AGX_EnumUtilities.h"
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
			TEXT("OpenPLX Signal Handler: Could not load OpenPLX model '%s'. The Output Log may "
				 "contain more information."),
			*OpenPLXFile);
		return;
	}

	FOpenPLXModelData* ModelData = ModelRegistry->GetModelData(ModelHandle);
	if (ModelData == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("OpenPLX Signal Handler: Unable to get registered OpenPLX model '%s'. The OpenPLX "
				 "model may not behave as intended."),
			*OpenPLXFile);
		return;
	}

	auto System = std::dynamic_pointer_cast<openplx::Physics3D::System>(ModelData->OpenPLXModel);
	if (System == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("OpenPLX Signal Handler: Unable to get a openplx::Physics3D::System from the "
				 "registered OpenPLX model '%s'. The OpenPLX model may not behave as intended."),
			*OpenPLXFile);
		return;
	}

	AssemblyRef->Native = FPLXUtilitiesInternal::MapRuntimeObjects(System, Simulation, Barriers);
	if (AssemblyRef->Native == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("OpenPLX Signal Handler: Unable to get a valid AGX Assembly from simulated model "
				 "instance for OpenPLX model '%s'. The Output Log may contain more details."),
			*OpenPLXFile);
		return;
	}

	std::shared_ptr<agxopenplx::AgxMetadata> AgxMetadata =
		std::make_shared<agxopenplx::AgxMetadata>();

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

	std::shared_ptr<openplx::ControlDispatch> ControlDispatch =
		std::make_shared<openplx::ControlDispatch>();
	agxopenplx::register_control_handlers(*ControlDispatch, AgxObjectMap, AgxMetadata);
	std::shared_ptr<openplx::ControlInterface> ControlInterface =
		std::make_shared<openplx::ControlInterface>(ControlDispatch);
	std::vector<std::shared_ptr<openplx::Physics::Signals::SignalInterface>> SignalInterfaces =
		FPLXUtilitiesInternal::GetNestedObjects<openplx::Physics::Signals::SignalInterface>(
			*System);
	for (std::shared_ptr<openplx::Physics::Signals::SignalInterface>& SignalInterface :
		 SignalInterfaces)
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
	//
	// Here follows a bunch of type conversion functions that convert OpenPLX typed values to and
	// from Unreal typed values. The functions ensure that Input or Output type makes sense for the
	// C++ type of the value and performs appropriate unit conversion where necessary.
	//
	// Depending on the value type and OpenPLX API used, sometimes the to-OpenPLX conversion
	// produces a by-value return, and sometimes it produces a by-Object-in-shared_ptr return. These
	// functions are separated by a name suffix. Since OpenPLX doesn't provide any value types (that
	// I know of) the by-value functions return AGX Dynamics types, such as agx::Vec3, instead.
	//

	//
	// Conversion functions for Real.
	//

	TOptional<double> ConvertRealToPLX(const FOpenPLX_Input& Input, double Value)
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
			TEXT("OpenPLX Signal Handler: Tried to convert Real value for Input '%s' ('%s'), but "
				 "the type is either not of Real type or is unsupported."),
			*Input.Name.ToString(), *Input.Alias.ToString());
		return {};
	}

	TOptional<double> ConvertRealToUnreal(const FOpenPLX_Output& Output, double Value)
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
			TEXT(
				"OpenPLX Signal Handler: Tried to convert Real value for Output '%s' ('%s') from "
				"AGX Dynamics units to Unreal units, but the type is either not of Real type or is "
				"unsupported."),
			*Output.Name.ToString(), *Output.Alias.ToString());
		return {};
	}

	//
	// Conversion functions for Vec2.
	//

	TOptional<agx::Vec2> ConvertVector2ToPLXValue(
		const FOpenPLX_Input& Input, const FVector2D& Value)
	{
		switch (Input.Type)
		{
			case EOpenPLX_InputType::ForceRangeInput:
			case EOpenPLX_InputType::TorqueRangeInput:
				return {{Value.X, Value.Y}};
		}

		UE_LOG(
			LogAGX, Warning,
			TEXT("OpenPLX Signal Handler: Tried to convert an Unreal Vec2 type to an OpenPLX "
				 "Vec2 type for Input '%s' ('%s'), but the input is either not of Vec2 vector "
				 "type or is unsupported."),
			*Input.Name.ToString(), *Input.Alias.ToString());
		return {};
	}

	TOptional<std::shared_ptr<openplx::Math::Vec2>> ConvertVector2ToPLXObject(
		const FOpenPLX_Input& Input, const FVector2D& Value)
	{
		TOptional<agx::Vec2> ValuePLXMaybe = ConvertVector2ToPLXValue(Input, Value);
		if (!ValuePLXMaybe)
		{
			return {};
		}

		return openplx::Math::Vec2::from_xy(ValuePLXMaybe->x(), ValuePLXMaybe->y());
	}

	TOptional<FVector2D> ConvertVector2ValueToUnreal(
		const FOpenPLX_Output& Output, const agx::Vec2& Value)
	{
		switch (Output.Type)
		{
			case EOpenPLX_OutputType::ForceRangeOutput:
			case EOpenPLX_OutputType::TorqueRangeOutput:
				return {{Value.x(), Value.y()}};
		}

		UE_LOG(
			LogAGX, Warning,
			TEXT("OpenPLX Signal Handler: Tried to read Vec2 vector type from signal for Output "
				 "'%s' ('%s'), but the type is either not of Vec2 vector type or is unsupported."),
			*Output.Name.ToString(), *Output.Alias.ToString());
		return {};
	}

	TOptional<FVector2D> ConvertVector2ObjectToUnreal(
		const FOpenPLX_Output& Output, const openplx::Math::Vec2& Object)
	{
		const agx::Vec2 Value(Object.x(), Object.y());
		return ConvertVector2ValueToUnreal(Output, Value);
	}

	//
	// Conversion functions for Vec3.
	//

	TOptional<agx::Vec3> ConvertVector3ToPLXValue(
		const FOpenPLX_Input& Input, const FVector& Vector)
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
			TEXT("OpenPLX Signal Handler: Tried to read Vec3 type for input '%s' ('%s'), but "
				 "the type is either not of Vec3 vector type or is unsupported."),
			*Input.Name.ToString(), *Input.Alias.ToString());
		return {};
	}

	TOptional<std::shared_ptr<openplx::Math::Vec3>> ConvertVector3ToPLXObject(
		const FOpenPLX_Input& Input, const FVector& Value)
	{
		TOptional<agx::Vec3> ValuePLXMaybe = ConvertVector3ToPLXValue(Input, Value);
		if (!ValuePLXMaybe)
		{
			return {};
		}

		return {openplx::Math::Vec3::from_xyz(
			ValuePLXMaybe->x(), ValuePLXMaybe->y(), ValuePLXMaybe->z())};
	}

	TOptional<FVector> ConvertVector3ValueToUnreal(
		const FOpenPLX_Output& Output, const agx::Vec3& Value)
	{
		switch (Output.Type)
		{
			case EOpenPLX_OutputType::AngularVelocity3DOutput:
			case EOpenPLX_OutputType::MateConnectorAngularAcceleration3DOutput:
				// TODO Are we sure RPY should use the Angular Velocity conversion?
			case EOpenPLX_OutputType::MateConnectorRPYOutput:
			case EOpenPLX_OutputType::RPYOutput:
				return ConvertAngularVelocity(Value);
			case EOpenPLX_OutputType::LinearVelocity3DOutput:
			case EOpenPLX_OutputType::MateConnectorAcceleration3DOutput:
			case EOpenPLX_OutputType::MateConnectorPositionOutput:
			case EOpenPLX_OutputType::Position3DOutput:
				return ConvertDisplacement(Value);
			case EOpenPLX_OutputType::Force3DOutput:
				return ConvertVector(Value);
			case EOpenPLX_OutputType::Torque3DOutput:
				return ConvertTorque(Value);
		}

		UE_LOG(
			LogAGX, Warning,
			TEXT("OpenPLX Signal Handler: Tried to read Vec3 vector type from signal for Output "
				 "'%s' ('%s'), but the type is either not of Vec3 vector type or is unsupported."),
			*Output.Name.ToString(), *Output.Alias.ToString());
		return {};
	}

	TOptional<FVector> ConvertVector3ObjectToUnreal(
		const FOpenPLX_Output& Output, const openplx::Math::Vec3& Object)
	{
		const agx::Vec3 Value {Object.x(), Object.y(), Object.z()};
		return ConvertVector3ValueToUnreal(Output, Value);
	}

	//
	// Conversion functions for Integer.
	//

	TOptional<int64_t> ConvertIntegerToPLX(const FOpenPLX_Input& Input, int64 Value)
	{
		switch (Input.Type)
		{
			case EOpenPLX_InputType::IntInput:
				return {static_cast<int64_t>(Value)};
		}

		UE_LOG(
			LogAGX, Warning,
			TEXT("OpenPLX Signal Handler: Tried to convert integer value for Input '%s' ('%s'), "
				 "but the type is either not of integer type or is unsupported."),
			*Input.Name.ToString(), *Input.Alias.ToString());
		return {};
	}

	TOptional<int64> ConvertIntegerToUnreal(const FOpenPLX_Output& Output, int64_t Value)
	{
		switch (Output.Type)
		{
			case EOpenPLX_OutputType::IntOutput:
				return {static_cast<int64>(Value)};
		}

		UE_LOG(
			LogAGX, Warning,
			TEXT("OpenPLX Signal Handler: Tried to convert Integer value for Output '%s' ('%s') "
				 "from AGX Dynamics units to Unreal units, but the type is either not of integer "
				 "type or is unsupported."),
			*Output.Name.ToString(), *Output.Alias.ToString());
		return {};
	}

	//
	// Conversion functions for Boolean.
	//

	TOptional<bool> ConvertBooleanToPLX(const FOpenPLX_Input& Input, bool Value)
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
			TEXT("OpenPLX Signal Handler: Tried to convert Boolean value for Input '%s' ('%s'), "
				 "but the type is either not of Boolean type or is unsupported."),
			*Input.Name.ToString(), *Input.Alias.ToString());
		return {};
	}

	TOptional<bool> ConvertBooleanToUnreal(const FOpenPLX_Output& Output, bool Value)
	{
		switch (Output.Type)
		{
			case EOpenPLX_OutputType::BoolOutput:
			case EOpenPLX_OutputType::ActivatedOutput:
			case EOpenPLX_OutputType::InteractionEnabledOutput:
			case EOpenPLX_OutputType::EngagedOutput:
			case EOpenPLX_OutputType::TorqueConverterLockedUpOutput:
				return Value;
		}

		UE_LOG(
			LogAGX, Warning,
			TEXT("OpenPLX Signal Handler: Tried to convert Boolean value for Output '%s' "
				 "('%s') from AGX Dynamics units to Unreal units, but the type is "
				 "either not of Boolean type or is unsupported."),
			*Output.Name.ToString(), *Output.Alias.ToString());
		return {};
	}

	//
	// Functions for getting Unreal-typed values from an OpenPLX signal.
	//

	/**
	 * Get the Unreal representation of the value held by the given signal.
	 *
	 * Base helper function, called by the type-specific functions.
	 *
	 * @tparam PLXValueT A subclass of openplx::Physics::Signals::Value.
	 * @tparam UnrealValueT The Unreal representation of the value held by the signal.
	 * @tparam ConvertFuncT Function that converts from the signal's value to Unreal representation.
	 * @param Output The OpenPLX output that the signal came from.
	 * @param Signal The signal containing the value to get.
	 * @param ConvertFunc Function to convert the signal's value to Unreal representation.
	 * @return Unreal representation of the signal's value.
	 */
	template <typename PLXValueT, typename UnrealValueT, typename ConvertFuncT>
	TOptional<UnrealValueT> GetUnrealValueFromSignal(
		const FOpenPLX_Output& Output, openplx::Physics::Signals::ValueOutputSignal* Signal,
		ConvertFuncT ConvertFunc)
	{
		if (Signal == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("OpenPLX Signal Handler: Cannot read value from signal from output '%s' "
					 "('%s') because the signal is nullptr."),
				*Output.Name.ToString(), *Output.Alias.ToString());
			return {};
		}

		std::shared_ptr<PLXValueT> Value = std::dynamic_pointer_cast<PLXValueT>(Signal->value());
		if (Value == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("OpenPLX Signal Handler: Tried to cast output value output '%s' ('%s') to the "
					 "requested concrete signal value type but the cast failed. Possible type "
					 "mismatch. The signal will not be received."),
				*Output.Name.ToString(), *Output.Alias.ToString());
			return {};
		}

		return ConvertFunc(Output, *Value.get());
	}

	TOptional<double> GetUnrealRealValueFromSignal(
		const FOpenPLX_Output& Output, openplx::Physics::Signals::ValueOutputSignal* Signal)
	{
		using PLXType = openplx::Physics::Signals::RealValue;
		return GetUnrealValueFromSignal<PLXType, double>(
			Output, Signal,
			[](const FOpenPLX_Output& Output, PLXType& Value)
			{ return ConvertRealToUnreal(Output, Value.value()); });
	}

	TOptional<FVector2D> GetUnrealVector2ValueFromSignal(
		const FOpenPLX_Output& Output, openplx::Physics::Signals::ValueOutputSignal* Signal)
	{
		using PLXType = openplx::Physics::Signals::Vec2Value;
		return GetUnrealValueFromSignal<PLXType, FVector2D>(
			Output, Signal,
			[](const FOpenPLX_Output& Output, PLXType& Value)
			{ return ConvertVector2ObjectToUnreal(Output, *Value.value()); });
	}

	TOptional<FVector> GetUnrealVector3ValueFromSignal(
		const FOpenPLX_Output& Output, openplx::Physics::Signals::ValueOutputSignal* Signal)
	{
		using PLXType = openplx::Physics::Signals::Vec3Value;
		return GetUnrealValueFromSignal<PLXType, FVector>(
			Output, Signal,
			[](const FOpenPLX_Output& Output, PLXType& Value)
			{ return ConvertVector3ObjectToUnreal(Output, *Value.value()); });
	}

	TOptional<int64> GetUnrealIntegerValueFromSignal(
		const FOpenPLX_Output& Output, openplx::Physics::Signals::ValueOutputSignal* Signal)
	{
		using PLXType = openplx::Physics::Signals::IntValue;
		return GetUnrealValueFromSignal<PLXType, int64>(
			Output, Signal,
			[](const FOpenPLX_Output& Output, PLXType& Value)
			{ return ConvertIntegerToUnreal(Output, Value.value()); });
	}

	TOptional<bool> GetUnrealBooleanValueFromSignal(
		const FOpenPLX_Output& Output, openplx::Physics::Signals::ValueOutputSignal* Signal)
	{
		using PLXType = openplx::Physics::Signals::BoolValue;
		return GetUnrealValueFromSignal<PLXType, bool>(
			Output, Signal,
			[](const FOpenPLX_Output& Output, PLXType& Value)
			{ return ConvertBooleanToUnreal(Output, Value.value()); });
	}

	//
	// Functions for writing a value to a Control Interface.
	//

	bool InterfaceWriteReal(
		openplx::HeapControlInterface& Interface, const std::string& Alias, const agx::Real Value)
	{
		return Interface.write(Alias, Value);
	}

	bool InterfaceWriteVector2(
		openplx::HeapControlInterface& Interface, const std::string& Alias, const agx::Vec2 Value)
	{
		std::shared_ptr<openplx::Marshalling> marshalling = Interface.prepare_write(Alias);
		if (marshalling == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("OpenPLX Signal Handler: Could not write to '%s' through the Control "
					 "Interface because a marshalling object could not be created."),
				UTF8_TO_TCHAR(Alias.c_str()));
			return false;
		}

		const bool bWroteX = marshalling->write_real("x", Value.x());
		const char* xError = marshalling->get_latest_error_message();
		const bool bWroteY = marshalling->write_real("y", Value.y());
		const char* yError = marshalling->get_latest_error_message();
		Interface.flush();
		if (!bWroteX || !bWroteY)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("OpenPLX Signal Handler: Could not marshall Vec2 value to '%s' through the "
					 "Control Interface because a write error occurred."),
				UTF8_TO_TCHAR(Alias.c_str()));
			if (!bWroteX)
			{
				UE_LOG(LogAGX, Warning, TEXT("   x: %s"), UTF8_TO_TCHAR(xError));
			}
			if (!bWroteY)
			{
				UE_LOG(LogAGX, Warning, TEXT("   y: %s"), UTF8_TO_TCHAR(yError));
			}
			return false;
		}

		return true;
	}

	bool InterfaceWriteVector3(
		openplx::HeapControlInterface& Interface, const std::string& Alias, const agx::Vec3 Value)
	{
		std::shared_ptr<openplx::Marshalling> marshalling = Interface.prepare_write(Alias);
		if (marshalling == nullptr)
		{
			UE_LOG(
				LogAGX, Warning, TEXT("TODO: Move marhsalling creation to Send/ReceiveInterface"));
			return false;
		}

		const bool bWroteX = marshalling->write_real("x", Value.x());
		const char* xError = marshalling->get_latest_error_message();
		const bool bWroteY = marshalling->write_real("y", Value.y());
		const char* yError = marshalling->get_latest_error_message();
		const bool bWroteZ = marshalling->write_real("z", Value.z());
		const char* zError = marshalling->get_latest_error_message();
		Interface.flush();
		if (!bWroteX || !bWroteY || !bWroteZ)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("OpenPLX Signal Handler: Could not marshall Vec3 value to '%s' through the "
					 "Control Interface because a write error occurred."),
				UTF8_TO_TCHAR(Alias.c_str()));
			if (!bWroteX)
			{
				UE_LOG(LogAGX, Warning, TEXT("   x: %s"), UTF8_TO_TCHAR(xError));
			}
			if (!bWroteY)
			{
				UE_LOG(LogAGX, Warning, TEXT("   y: %s"), UTF8_TO_TCHAR(yError));
			}
			if (!bWroteZ)
			{
				UE_LOG(LogAGX, Warning, TEXT("   z: %s"), UTF8_TO_TCHAR(zError));
			}
			return false;
		}

		return true;
	}

	//
	// Functions for reading a value from a Control Interface.
	//

	TOptional<agx::Real> InterfaceReadReal(
		openplx::HeapControlInterface& Interface, const std::string& Alias)
	{
		std::optional<double> ValueMaybe = Interface.read<double>(Alias);
		if (!ValueMaybe)
			return {};
		return {ValueMaybe.value()};
	}

	TOptional<agx::Vec2> InterfaceReadVector2(
		openplx::HeapControlInterface& Interface, const std::string& Alias)
	{
		std::shared_ptr<openplx::Marshalling> marshalling = Interface.prepare_read(Alias);
		if (marshalling == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("OpenPLX Signal Handler: Could not read from '%s' through the Control "
					 "Interface because a marshalling object could not be created."),
				UTF8_TO_TCHAR(Alias.c_str()));
			return {};
		}

		std::optional<agx::Real> xMaybe = marshalling->read_real<agx::Real>("x");
		const char* xError = marshalling->get_latest_error_message();
		std::optional<agx::Real> yMaybe = marshalling->read_real<agx::Real>("y");
		const char* yError = marshalling->get_latest_error_message();
		if (!xMaybe || !yMaybe)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("OpenPLX Signal Handler: Could not marshall Vec2 value from '%s' through the "
					 "Control Interface because a read error occurred."),
				UTF8_TO_TCHAR(Alias.c_str()));
			if (!xMaybe)
			{
				UE_LOG(LogAGX, Warning, TEXT("    x: %s"), UTF8_TO_TCHAR(xError));
			}
			if (!yMaybe)
			{
				UE_LOG(LogAGX, Warning, TEXT("    y: %s"), UTF8_TO_TCHAR(yError));
			}
			return {};
		}

		return {{*xMaybe, *yMaybe}};
	}

	TOptional<agx::Vec3> InterfaceReadVector3(
		openplx::HeapControlInterface& Interface, const std::string& Alias)
	{
		std::shared_ptr<openplx::Marshalling> marshalling = Interface.prepare_read(Alias);
		if (marshalling == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("OpenPLX Signal Handler: Could not read from '%s' through the Control "
					 "Interface because a marshalling object could not be created."),
				UTF8_TO_TCHAR(Alias.c_str()));
			return {};
		}

		std::optional<agx::Real> xMaybe = marshalling->read_real<agx::Real>("x");
		const char* xError = marshalling->get_latest_error_message();
		std::optional<agx::Real> yMaybe = marshalling->read_real<agx::Real>("y");
		const char* yError = marshalling->get_latest_error_message();
		std::optional<agx::Real> zMaybe = marshalling->read_real<agx::Real>("z");
		const char* zError = marshalling->get_latest_error_message();
		if (!xMaybe || !yMaybe || !zMaybe)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("OpenPLX Signal Handler: Could not marshall Vec3 value from '%s' through the "
					 "Control Interface because a read error occurred."),
				UTF8_TO_TCHAR(Alias.c_str()));
			if (!xMaybe)
			{
				UE_LOG(LogAGX, Warning, TEXT("    x: %s"), UTF8_TO_TCHAR(xError));
			}
			if (!yMaybe)
			{
				UE_LOG(LogAGX, Warning, TEXT("    y: %s"), UTF8_TO_TCHAR(yError));
			}
			if (!zMaybe)
			{
				UE_LOG(LogAGX, Warning, TEXT("    z: %s"), UTF8_TO_TCHAR(zError));
			}
			return {};
		}

		return {{*xMaybe, *yMaybe, *zMaybe}};
	}

	//
	// Functions for sending signals.
	//

	template <typename ValueT, typename SignalT, typename ConversionFuncT>
	bool Send(
		const FOpenPLX_Input& Input, ValueT Value, FOpenPLXModelRegistry* ModelRegistry,
		FOpenPLXModelRegistry::Handle ModelHandle,
		std::shared_ptr<agxopenplx::InputSignalQueue> InputQueue, ConversionFuncT ConversionFunc)
	{
		if (ModelRegistry == nullptr || ModelHandle == FOpenPLXModelRegistry::InvalidHandle)
			return false;

		if (InputQueue == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("OpenPLX Signal Handler: Tried to send OpenPLX Input signal for Input '%s' "
					 "('%s'), but the OpenPLX model does not have any registered Inputs."),
				*Input.Name.ToString(), *Input.Alias.ToString());
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
				TEXT("OpenPLX Signal Handler: Tried to send OpenPLX signal, but the corresponding "
					 "OpenPLX Input '%s' ('%s') was not found in the model. The signal will not be "
					 "sent."),
				*Input.Name.ToString(), *Input.Alias.ToString());
			return false;
		}

		auto ConvertedValue = ConversionFunc(Input, Value);
		if (!ConvertedValue.IsSet())
			return false;

		auto Signal = SignalT::create(*ConvertedValue, PLXInput->second);
		InputQueue->send(Signal);
		return true;
	}

	template <typename ValuePLXT, typename ValueUnrealT, typename ConvertFuncT, typename WriteFuncT>
	bool SendInterfaceImpl(
		const FOpenPLX_Input& Input, ValueUnrealT ValueUnreal,
		FHeapControlInterfaceRef* HeapControlInterfaceRef, ConvertFuncT ConvertFunc,
		WriteFuncT WriteFunc)
	{
		if (HeapControlInterfaceRef == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("OpenPLX Signal Handler: Tried to send OpenPLX Output signal for output '%s' "
					 "('%s'), but don't have a Control Interface wrapper reference"),
				*Input.Name.ToString(), *Input.Alias.ToString());
			return false;
		}

		openplx::HeapControlInterface* Interface = HeapControlInterfaceRef->Native.get();
		if (Interface == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("OpenPLX Signal Handler: Tried to send OpenPLX Input signal for input '%s' "
					 "('%s'), through the Control Interface, but don't have a Control Interface "
					 "instance."),
				*Input.Name.ToString(), *Input.Alias.ToString());
			return false;
		}

		TOptional<ValuePLXT> ValuePLXMaybe = ConvertFunc(Input, ValueUnreal);
		if (!ValuePLXMaybe)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("OpenPLX Control Interface: Type and unit conversion from Unreal to OpenPLX "
					 "failed for input '%s' ('%s')."),
				*Input.Name.ToString(), *Input.Alias.ToString());
			return false;
		}

		ValuePLXT ValuePLX = *ValuePLXMaybe;
		std::string Alias = Convert(Input.Alias.ToString());
		const bool bWritten = WriteFunc(*Interface, Alias, ValuePLX);
		if (!bWritten)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("OpenPLX Control Interface: Tried to send to input '%s' ('%s') but the write "
					 "was rejected by the Control Interface"),
				*Input.Name.ToString(), *Input.Alias.ToString());
			return false;
		}

		return true;
	}

	//
	// Functions for receiving signals.
	//

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

	template <typename ValuePLXT, typename ValueUnrealT, typename ReadFuncT, typename ConvertFuncT>
	bool ReceiveInterfaceImpl(
		const FOpenPLX_Output& Output, ValueUnrealT& OutValue,
		FHeapControlInterfaceRef* HeapControlInterfaceRef, ReadFuncT ReadFunc,
		ConvertFuncT ConvertFunc)
	{
		OutValue = {};

		if (HeapControlInterfaceRef == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("OpenPLX Signal Handler: Tried to receive OpenPLX Output signal for output "
					 "'%s' ('%s') through the Control Interface, but don't have a Control "
					 "Interface wrapper reference."),
				*Output.Name.ToString(), *Output.Alias.ToString());
			return false;
		}

		openplx::HeapControlInterface* Interface = HeapControlInterfaceRef->Native.get();
		if (Interface == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("OpenPLX Signal Handler: Tried to receive OpenPLX Output signal for output "
					 "'%s' ('%s') through the Control Interface, but don't have a Control "
					 "Interface pointer."),
				*Output.Name.ToString(), *Output.Alias.ToString());
			return false;
		}

		std::string Alias = Convert(Output.Alias.ToString());
		TOptional<ValuePLXT> ValuePLXMaybe = ReadFunc(*Interface, Alias);
		if (!ValuePLXMaybe)
		{
			const FString TypeName = AGX_EnumUtilities::GetEnumName(Output.Type);
			UE_LOG(
				LogAGX, Warning,
				TEXT("OpenPLX Signal Handler: Could not read output signal '%s' ('%s') of type "
					 "'%s'."),
				*Output.Name.ToString(), *Output.Alias.ToString(), *TypeName);
			return false;
		}

		TOptional<ValueUnrealT> ConvertedMaybe = ConvertFunc(Output, *ValuePLXMaybe);
		if (!ConvertedMaybe)
		{
			const FString TypeName = AGX_EnumUtilities::GetEnumName(Output.Type);
			UE_LOG(
				LogAGX, Warning,
				TEXT("OpenPLX Signal Handler: Could not convert output signal '%s' ('%s') of type "
					 "'%s' from OpenPLX type and unit to Unreal type and unit."),
				*Output.Name.ToString(), *Output.Alias.ToString(), *TypeName);
			return false;
		}

		OutValue = *ConvertedMaybe;
		return true;
	}
}

//
// End of helper functions, start of member function implementations.
//

bool FOpenPLXSignalHandler::Send(const FOpenPLX_Input& Input, double Value)
{
	using namespace OpenPLXSignalHandler_helpers;
	check(IsInitialized());
	return OpenPLXSignalHandler_helpers::Send<double, openplx::Physics::Signals::RealInputSignal>(
		Input, Value, ModelRegistry, ModelHandle, InputSignalListenerRef->Native->getQueue(),
		ConvertRealToPLX);
}

bool FOpenPLXSignalHandler::SendInterface(const FOpenPLX_Input& Input, double Value)
{
	using namespace OpenPLXSignalHandler_helpers;
	check(IsInitialized());
	return OpenPLXSignalHandler_helpers::SendInterfaceImpl<agx::Real>(
		Input, Value, HeapControlInterfaceRef.get(), ConvertRealToPLX, InterfaceWriteReal);
}

bool FOpenPLXSignalHandler::Receive(const FOpenPLX_Output& Output, double& OutValue)
{
	using namespace OpenPLXSignalHandler_helpers;
	check(IsInitialized());
	return OpenPLXSignalHandler_helpers::Receive(
		Output, OutValue, ModelRegistry, ModelHandle, OutputSignalListenerRef->Native->getQueue(),
		GetUnrealRealValueFromSignal);
}

bool FOpenPLXSignalHandler::ReceiveInterface(const FOpenPLX_Output& Output, double& OutValue)
{
	using namespace OpenPLXSignalHandler_helpers;
	check(IsInitialized());
	return ReceiveInterfaceImpl<agx::Real>(
		Output, OutValue, HeapControlInterfaceRef.get(), InterfaceReadReal, ConvertRealToUnreal);
}

bool FOpenPLXSignalHandler::Send(const FOpenPLX_Input& Input, const FVector2D& Value)
{
	check(IsInitialized());
	return OpenPLXSignalHandler_helpers::Send<
		FVector2D, openplx::Physics::Signals::RealRangeInputSignal>(
		Input, Value, ModelRegistry, ModelHandle, InputSignalListenerRef->Native->getQueue(),
		OpenPLXSignalHandler_helpers::ConvertVector2ToPLXObject);
}

bool FOpenPLXSignalHandler::SendInterface(const FOpenPLX_Input& Input, const FVector2D& Value)
{
	using namespace OpenPLXSignalHandler_helpers;
	check(IsInitialized());
	return SendInterfaceImpl<agx::Vec2>(
		Input, Value, HeapControlInterfaceRef.get(), ConvertVector2ToPLXValue,
		InterfaceWriteVector2);
}

bool FOpenPLXSignalHandler::Receive(const FOpenPLX_Output& Output, FVector2D& OutValue)
{
	using namespace OpenPLXSignalHandler_helpers;
	check(IsInitialized());
	return OpenPLXSignalHandler_helpers::Receive(
		Output, OutValue, ModelRegistry, ModelHandle, OutputSignalListenerRef->Native->getQueue(),
		GetUnrealVector2ValueFromSignal);
}

bool FOpenPLXSignalHandler::ReceiveInterface(const FOpenPLX_Output& Output, FVector2D& OutValue)
{
	using namespace OpenPLXSignalHandler_helpers;
	check(IsInitialized());
	return ReceiveInterfaceImpl<agx::Vec2>(
		Output, OutValue, HeapControlInterfaceRef.get(), InterfaceReadVector2,
		ConvertVector2ValueToUnreal);
}

bool FOpenPLXSignalHandler::Send(const FOpenPLX_Input& Input, const FVector& Value)
{
	check(IsInitialized());
	return OpenPLXSignalHandler_helpers::Send<FVector, openplx::Physics::Signals::Vec3InputSignal>(
		Input, Value, ModelRegistry, ModelHandle, InputSignalListenerRef->Native->getQueue(),
		OpenPLXSignalHandler_helpers::ConvertVector3ToPLXObject);
}

bool FOpenPLXSignalHandler::SendInterface(const FOpenPLX_Input& Input, const FVector& Value)
{
	using namespace OpenPLXSignalHandler_helpers;
	check(IsInitialized());
	return SendInterfaceImpl<agx::Vec3>(
		Input, Value, HeapControlInterfaceRef.get(), ConvertVector3ToPLXValue,
		InterfaceWriteVector3);
}

bool FOpenPLXSignalHandler::Receive(const FOpenPLX_Output& Output, FVector& OutValue)
{
	check(IsInitialized());
	return OpenPLXSignalHandler_helpers::Receive(
		Output, OutValue, ModelRegistry, ModelHandle, OutputSignalListenerRef->Native->getQueue(),
		OpenPLXSignalHandler_helpers::GetUnrealVector3ValueFromSignal);
}

bool FOpenPLXSignalHandler::ReceiveInterface(const FOpenPLX_Output& Output, FVector& OutValue)
{
	using namespace OpenPLXSignalHandler_helpers;
	check(IsInitialized());
	return ReceiveInterfaceImpl<agx::Vec3>(
		Output, OutValue, HeapControlInterfaceRef.get(), InterfaceReadVector3,
		ConvertVector3ValueToUnreal);
}

bool FOpenPLXSignalHandler::Send(const FOpenPLX_Input& Input, int64 Value)
{
	check(IsInitialized());
	return OpenPLXSignalHandler_helpers::Send<int64, openplx::Physics::Signals::IntInputSignal>(
		Input, Value, ModelRegistry, ModelHandle, InputSignalListenerRef->Native->getQueue(),
		OpenPLXSignalHandler_helpers::ConvertIntegerToPLX);
}

bool FOpenPLXSignalHandler::Receive(const FOpenPLX_Output& Output, int64& OutValue)
{
	check(IsInitialized());
	return OpenPLXSignalHandler_helpers::Receive(
		Output, OutValue, ModelRegistry, ModelHandle, OutputSignalListenerRef->Native->getQueue(),
		OpenPLXSignalHandler_helpers::GetUnrealIntegerValueFromSignal);
}

bool FOpenPLXSignalHandler::Send(const FOpenPLX_Input& Input, bool Value)
{
	check(IsInitialized());
	return OpenPLXSignalHandler_helpers::Send<bool, openplx::Physics::Signals::BoolInputSignal>(
		Input, Value, ModelRegistry, ModelHandle, InputSignalListenerRef->Native->getQueue(),
		OpenPLXSignalHandler_helpers::ConvertBooleanToPLX);
}

bool FOpenPLXSignalHandler::Receive(const FOpenPLX_Output& Output, bool& OutValue)
{
	check(IsInitialized());
	return OpenPLXSignalHandler_helpers::Receive(
		Output, OutValue, ModelRegistry, ModelHandle, OutputSignalListenerRef->Native->getQueue(),
		OpenPLXSignalHandler_helpers::GetUnrealBooleanValueFromSignal);
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
