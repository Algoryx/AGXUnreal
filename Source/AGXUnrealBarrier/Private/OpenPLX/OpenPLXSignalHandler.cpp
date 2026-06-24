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
#include "Utilities/OpenPLX_Utilities.h"
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
			TEXT(
				"OpenPLX Signal Handler: Could not load OpenPLX model '%s'. The Output Log may "
				"contain more information."),
			*OpenPLXFile);
		return;
	}

	FOpenPLXModelData* ModelData = ModelRegistry->GetModelData(ModelHandle);
	if (ModelData == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT(
				"OpenPLX Signal Handler: Unable to get registered OpenPLX model '%s'. The OpenPLX "
				"model may not behave as intended."),
			*OpenPLXFile);
		return;
	}

	auto System = std::dynamic_pointer_cast<openplx::Physics3D::System>(ModelData->OpenPLXModel);
	if (System == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT(
				"OpenPLX Signal Handler: Unable to get a openplx::Physics3D::System from the "
				"registered OpenPLX model '%s'. The OpenPLX model may not behave as intended."),
			*OpenPLXFile);
		return;
	}

	AssemblyRef->Native = FPLXUtilitiesInternal::MapRuntimeObjects(System, Simulation, Barriers);
	if (AssemblyRef->Native == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT(
				"OpenPLX Signal Handler: Unable to get a valid AGX Assembly from simulated model "
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
	ModelData->HeapControlInterfaces.insert(
		{AssemblyRef->Native.get(),
		 std::make_shared<openplx::HeapControlInterface>(ControlInterface)});

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
			case EOpenPLX_InputType::DurationInput:
			case EOpenPLX_InputType::AutomaticClutchEngagementDurationInput:
			case EOpenPLX_InputType::AutomaticClutchDisengagementDurationInput:
			case EOpenPLX_InputType::FractionInput:
			case EOpenPLX_InputType::Force1DInput:
			case EOpenPLX_InputType::Torque1DInput:
				return Value;
			case EOpenPLX_InputType::AngleInput:
			case EOpenPLX_InputType::AngularVelocity1DInput:
				return ConvertAngleToAGX(Value);
			case EOpenPLX_InputType::Position1DInput:
			case EOpenPLX_InputType::LinearVelocity1DInput:
				return ConvertDistanceToAGX(Value);
		}

		UE_LOG(
			LogAGX, Warning,
			TEXT(
				"OpenPLX Signal Handler: Tried to convert Real value for Input '%s' ('%s'), but "
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
			TEXT(
				"OpenPLX Signal Handler: Tried to convert an Unreal Vec2 type to an OpenPLX "
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
			TEXT(
				"OpenPLX Signal Handler: Tried to read Vec2 vector type from signal for Output "
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
			TEXT(
				"OpenPLX Signal Handler: Tried to read Vec3 type for input '%s' ('%s'), but "
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
				return ConvertAngularVelocity(Value);
			case EOpenPLX_OutputType::MateConnectorRPYOutput:
			case EOpenPLX_OutputType::RPYOutput:
				return ConvertRPY(Value);
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
			TEXT(
				"OpenPLX Signal Handler: Tried to read Vec3 vector type from signal for Output "
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
			TEXT(
				"OpenPLX Signal Handler: Tried to convert integer value for Input '%s' ('%s'), "
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
			TEXT(
				"OpenPLX Signal Handler: Tried to convert Integer value for Output '%s' ('%s') "
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
			TEXT(
				"OpenPLX Signal Handler: Tried to convert Boolean value for Input '%s' ('%s'), "
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
			TEXT(
				"OpenPLX Signal Handler: Tried to convert Boolean value for Output '%s' "
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
				TEXT(
					"OpenPLX Signal Handler: Cannot read value from signal from output '%s' "
					"('%s') because the signal is nullptr."),
				*Output.Name.ToString(), *Output.Alias.ToString());
			return {};
		}

		std::shared_ptr<PLXValueT> Value = std::dynamic_pointer_cast<PLXValueT>(Signal->value());
		if (Value == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT(
					"OpenPLX Signal Handler: Tried to cast output value output '%s' ('%s') to the "
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
			Output, Signal, [](const FOpenPLX_Output& Output, PLXType& Value)
			{ return ConvertRealToUnreal(Output, Value.value()); });
	}

	TOptional<FVector2D> GetUnrealVector2ValueFromSignal(
		const FOpenPLX_Output& Output, openplx::Physics::Signals::ValueOutputSignal* Signal)
	{
		using PLXType = openplx::Physics::Signals::Vec2Value;
		return GetUnrealValueFromSignal<PLXType, FVector2D>(
			Output, Signal, [](const FOpenPLX_Output& Output, PLXType& Value)
			{ return ConvertVector2ObjectToUnreal(Output, *Value.value()); });
	}

	TOptional<FVector> GetUnrealVector3ValueFromSignal(
		const FOpenPLX_Output& Output, openplx::Physics::Signals::ValueOutputSignal* Signal)
	{
		using PLXType = openplx::Physics::Signals::Vec3Value;
		return GetUnrealValueFromSignal<PLXType, FVector>(
			Output, Signal, [](const FOpenPLX_Output& Output, PLXType& Value)
			{ return ConvertVector3ObjectToUnreal(Output, *Value.value()); });
	}

	TOptional<int64> GetUnrealIntegerValueFromSignal(
		const FOpenPLX_Output& Output, openplx::Physics::Signals::ValueOutputSignal* Signal)
	{
		using PLXType = openplx::Physics::Signals::IntValue;
		return GetUnrealValueFromSignal<PLXType, int64>(
			Output, Signal, [](const FOpenPLX_Output& Output, PLXType& Value)
			{ return ConvertIntegerToUnreal(Output, Value.value()); });
	}

	TOptional<bool> GetUnrealBooleanValueFromSignal(
		const FOpenPLX_Output& Output, openplx::Physics::Signals::ValueOutputSignal* Signal)
	{
		using PLXType = openplx::Physics::Signals::BoolValue;
		return GetUnrealValueFromSignal<PLXType, bool>(
			Output, Signal, [](const FOpenPLX_Output& Output, PLXType& Value)
			{ return ConvertBooleanToUnreal(Output, Value.value()); });
	}

	//
	// Functions for writing to or reading from a Control Interface.
	//

	// Control Interface write / read functions for primitive types.

	bool InterfaceWriteBool(
		openplx::HeapControlInterface& Interface, const std::string& Alias, bool Value)
	{
		return Interface.write(Alias, Value);
	}

	TOptional<bool> InterfaceReadBool(
		openplx::HeapControlInterface& Interface, const std::string& Alias)
	{
		std::optional<bool> ValueMaybe = Interface.read<bool>(Alias);
		if (!ValueMaybe)
			return {};
		return {ValueMaybe.value()};
	}

	bool InterfaceWriteSignedInteger(
		openplx::HeapControlInterface& Interface, const std::string& Alias, const int64_t Value)
	{
		return Interface.write(Alias, Value);
	}

	TOptional<int64_t> InterfaceReadSignedInteger(
		openplx::HeapControlInterface& Interface, const std::string& Alias)
	{
		std::optional<int64_t> ValueMaybe = Interface.read<int64_t>(Alias);
		return Convert(ValueMaybe);
	}

	bool InterfaceWriteUnsignedInteger(
		openplx::HeapControlInterface& Interface, const std::string& Alias, const uint64_t Value)
	{
		return Interface.write(Alias, Value);
	}

	TOptional<uint64_t> InterfaceReadUnsignedInteger(
		openplx::HeapControlInterface& Interface, const std::string& Alias)
	{
		std::optional<uint64_t> ValueMaybe = Interface.read<uint64_t>(Alias);
		return Convert(ValueMaybe);
	}

	bool InterfaceWriteReal(
		openplx::HeapControlInterface& Interface, const std::string& Alias, const agx::Real Value)
	{
		return Interface.write(Alias, Value);
	}

	TOptional<agx::Real> InterfaceReadReal(
		openplx::HeapControlInterface& Interface, const std::string& Alias)
	{
		return Convert(Interface.read<double>(Alias));
	}

	// Control Interface write / read functions for compound types, which must use marshalling.
	// This block starts with a bunch of helper types to make it possible to have generic leaf write
	// and read functions.

	template <typename ValueT>
	struct FWriteField
	{
		const char* Name {nullptr};
		ValueT Value {};
	};

	template <typename ValueT>
	struct FReadField
	{
		const char* Name {nullptr};
		ValueT* Value {nullptr};
	};

	template <typename ValueT>
	bool InterfaceWriteField(openplx::Marshalling& Marshalling, const FWriteField<ValueT>& Field)
	{
		// The order is important here because there are overlaps in the Venn diagram of the types.
		// For example, both int and double are signed but only double is floating-point so we must
		// check for write_real before write_int.
		if constexpr (std::is_floating_point_v<ValueT>)
			return Marshalling.write_real(Field.Name, Field.Value);
		else if constexpr (std::is_same_v<ValueT, bool>)
			return Marshalling.write_bool(Field.Name, Field.Value);
		else if constexpr (std::is_unsigned_v<ValueT>)
			return Marshalling.write_uint(Field.Name, Field.Value);
		else if constexpr (std::is_signed_v<ValueT>)
			return Marshalling.write_int(Field.Name, Field.Value);
		else
			static_assert(false, "InterfaceWriteField was called with an unsupported type.");
	}

	template <typename ValueT>
	bool InterfaceReadField(openplx::Marshalling& Marshalling, const FReadField<ValueT>& Field)
	{
		std::optional<ValueT> ValueMaybe;

		// The order is important here because there are overlaps in the Venn diagram of the types.
		// For example, both int and double are signed but only double is floating-point so we must
		// check for write_real before write_int.
		if constexpr (std::is_floating_point_v<ValueT>)
			ValueMaybe = Marshalling.read_real<ValueT>(Field.Name);
		else if constexpr (std::is_same_v<ValueT, bool>)
			ValueMaybe = Marshalling.read_bool(Field.Name);
		else if constexpr (std::is_unsigned_v<ValueT>)
			ValueMaybe = Marshalling.read_uint<ValueT>(Field.Name);
		else if constexpr (std::is_signed_v<ValueT>)
			ValueMaybe = Marshalling.read_int<ValueT>(Field.Name);
		else
			static_assert(false, "InterfaceReadField was called with an unsupported type.");

		if (!ValueMaybe)
		{
			return false;
		}

		*Field.Value = *ValueMaybe;
		return true;
	}

	struct FInterfaceError
	{
		FString FieldName;
		FString ErrorMessage;
	};

	template <typename... FWriteFields>
	bool InterfaceWriteFields(
		openplx::HeapControlInterface& Interface, const std::string& Alias,
		FWriteFields... InterfaceFields)
	{
		std::shared_ptr<openplx::Marshalling> Marshalling = Interface.prepare_write(Alias);
		if (Marshalling == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT(
					"OpenPLX Signal Handler: Could not write to '%s' through the Control "
					"Interface because a marshalling object could not be created. This may be "
					"caused by a type handler not being registered in AgxOpenPlxApi.cpp."),
				UTF8_TO_TCHAR(Alias.c_str()));
			return false;
		}

		TArray<FInterfaceError> Errors;
		auto WriteField = [&](auto Field)
		{
			const bool bSuccess = InterfaceWriteField(*Marshalling, Field);
			if (!bSuccess)
			{
				const char* Error = Marshalling->get_latest_error_message();
				Errors.Emplace(UTF8_TO_TCHAR(Field.Name), UTF8_TO_TCHAR(Error));
			}
		};
		(WriteField(InterfaceFields), ...);
		Interface.flush();

		if (!Errors.IsEmpty())
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT(
					"OpenPLX Signal Handler: Could not marshall value to '%s' through the "
					"Control Interface because one or more write errors occurred:"),
				UTF8_TO_TCHAR(Alias.c_str()));

			for (auto [FieldName, Error] : Errors)
			{
				UE_LOG(LogAGX, Warning, TEXT("    %s: %s"), *FieldName, *Error);
			}
			const std::unordered_map<std::string, openplx::Field>& Fields =
				Marshalling->get_field_map();
			UE_LOG(LogAGX, Warning, TEXT("  Known fields:"));
			for (const auto& [Name, Field] : Fields)
			{
				UE_LOG(LogAGX, Warning, TEXT("    %s"), UTF8_TO_TCHAR(Name.c_str()));
			}
			return false;
		}

		return true;
	}

	template <typename... FReadFields>
	bool InterfaceReadFields(
		openplx::HeapControlInterface& Interface, const std::string& Alias,
		FReadFields... ReadFields)
	{
		std::shared_ptr<openplx::Marshalling> Marshalling = Interface.prepare_read(Alias);
		if (Marshalling == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT(
					"OpenPLX Signal Handler: Could not read from '%s' through the Control "
					"Interface because a marshalling object could not be created. This may be "
					"caused by a type handler not being registered in AgxOpenPlxApi.cpp"),
				UTF8_TO_TCHAR(Alias.c_str()));
			return {};
		}

		TArray<FInterfaceError> Errors;
		auto ReadField = [&](auto Field)
		{
			const bool bSuccess = InterfaceReadField(*Marshalling, Field);
			if (!bSuccess)
			{
				const char* Error = Marshalling->get_latest_error_message();
				Errors.Emplace(UTF8_TO_TCHAR(Field.Name), UTF8_TO_TCHAR(Error));
			}
		};
		(ReadField(ReadFields), ...);
		Interface.flush();

		if (!Errors.IsEmpty())
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT(
					"OpenPLX Signal Handler: Could not marshal value to '%s' through the "
					"Control Interface because one or more read errors occurred:"),
				UTF8_TO_TCHAR(Alias.c_str()));
			for (auto [FieldName, Error] : Errors)
			{
				UE_LOG(LogAGX, Warning, TEXT("    %s: %s"), *FieldName, *Error);
			}
			const std::unordered_map<std::string, openplx::Field>& Fields =
				Marshalling->get_field_map();
			for (const auto& [Name, Field] : Fields)
			{
				UE_LOG(LogAGX, Warning, TEXT("    %s"), UTF8_TO_TCHAR(Name.c_str()));
			}
			return false;
		}

		return true;
	}

	bool InterfaceWriteRangeReal(
		openplx::HeapControlInterface& Interface, const std::string& Alias, const agx::Vec2 Value)
	{
		// clang-format off
		return InterfaceWriteFields(
			Interface, Alias,
			FWriteField {"min", Value.x()},
			FWriteField {"max", Value.y()});
		// clang-format on
	}

	TOptional<agx::Vec2> InterfaceReadRangeReal(
		openplx::HeapControlInterface& Interface, const std::string& Alias)
	{
		agx::Vec2 Value;
		// clang-format off
		const bool bSuccess =
			InterfaceReadFields(
				Interface, Alias,
				FReadField{"min", &Value.x()},
				FReadField{"max", &Value.y()});
		// clang-format on
		return bSuccess ? Value : TOptional<agx::Vec2>();
	}

	bool InterfaceWriteVector2(
		openplx::HeapControlInterface& Interface, const std::string& Alias, const agx::Vec2 Value)
	{
		// clang-format off
		return InterfaceWriteFields(
			Interface, Alias,
			FWriteField {"x", Value.x()},
			FWriteField {"y", Value.y()});
		// clang-format on
	}

	TOptional<agx::Vec2> InterfaceReadVector2(
		openplx::HeapControlInterface& Interface, const std::string& Alias)
	{
		agx::Vec2 Value;
		// clang-format off
		const bool bSuccess =
			InterfaceReadFields(
				Interface, Alias,
				FReadField{"x", &Value.x()},
				FReadField{"y", &Value.y()});
		// clang-format on
		return bSuccess ? Value : TOptional<agx::Vec2>();
	}

	bool InterfaceWriteRPY(openplx::HeapControlInterface& Interface, const std::string& Alias, const agx::Vec3 Value)
	{
		// clang-format off
		return InterfaceWriteFields(
			Interface, Alias,
			FWriteField {"r", Value.x()},
			FWriteField {"p", Value.y()},
			FWriteField {"y", Value.z()});
		// clang-format on
	}

	TOptional<agx::Vec3> InterfaceReadRPY(
		openplx::HeapControlInterface& Interface, const std::string& Alias)
	{
		agx::Vec3 Value;
		// clang-format off
		const bool bSuccess =
			InterfaceReadFields(
				Interface, Alias,
				FReadField{"r", &Value.x()},
				FReadField{"p", &Value.y()},
				FReadField{"y", &Value.z()});
		// clang-format on
		return bSuccess ? Value : TOptional<agx::Vec3>();
	}

	bool InterfaceWriteVector3(
		openplx::HeapControlInterface& Interface, const std::string& Alias, const agx::Vec3 Value)
	{
		// clang-format off
		return InterfaceWriteFields(
			Interface, Alias,
			FWriteField {"x", Value.x()},
			FWriteField {"y", Value.y()},
			FWriteField {"z", Value.z()});
		// clang-format on
	}

	TOptional<agx::Vec3> InterfaceReadVector3(
		openplx::HeapControlInterface& Interface, const std::string& Alias)
	{
		agx::Vec3 Value;
		// clang-format off
		const bool bSuccess =
			InterfaceReadFields(
				Interface, Alias,
				FReadField{"x", &Value.x()},
				FReadField{"y", &Value.y()},
				FReadField{"z", &Value.z()});
		// clang-format on
		return bSuccess ? Value : TOptional<agx::Vec3>();
	}

	//
	// Functions for type look-ups.
	//
	// Different types require different Control Interface read and write functions. Since some
	// types, e.g. Vec2, require different real / write code depending on the input / output
	// type, e.g. x/y vs min/max, we want to have a central location where this type-to-code
	// information is stored so we don't need to repeat long if-else of switch-case statements
	// all over the place. For now that location is here. There is a lot of templates here since
	// the main read and write functions are templated, meaning a single code path must be able
	// to work with many value types. Due to the verbose syntax required to communicate all of
	// this to the compiler this block of code became way larger than I expected it to. It may
	// be possible to simplify all of this, or perhaps we should scrap all of this helper stuff
	// an just list the types explicitly much more.
	//

	// Function pointer types used when getting a read or write helper function. There is one
	// function pointer type for each "primitive" type we support.
	// clang-format off
	using BoolWriteFuncPtr =
		bool (*)(openplx::HeapControlInterface&, const std::string&, bool);
	using IntWriteFuncPtr =
		bool (*)(openplx::HeapControlInterface&, const std::string&, int64_t);
	using UIntWriteFuncPtr =
		bool (*)(openplx::HeapControlInterface&, const std::string&, uint64_t);
	using RealWriteFuncPtr =
		bool (*)(openplx::HeapControlInterface&, const std::string&, agx::Real);
	using Vec2WriteFuncPtr =
		bool (*)(openplx::HeapControlInterface&, const std::string&, agx::Vec2);
	using Vec3WriteFuncPtr =
		bool (*)(openplx::HeapControlInterface&, const std::string&, agx::Vec3);

	using BoolReadFuncPtr =
		TOptional<bool> (*)(openplx::HeapControlInterface&, const std::string&);
	using IntReadFuncPtr =
		TOptional<int64_t> (*)(openplx::HeapControlInterface&, const std::string&);
	using UIntReadFuncPtr =
		TOptional<uint64_t> (*)(openplx::HeapControlInterface&, const std::string&);
	using RealReadFuncPtr =
		TOptional<agx::Real> (*)(openplx::HeapControlInterface&, const std::string&);
	using Vec2ReadFuncPtr =
		TOptional<agx::Vec2> (*)(openplx::HeapControlInterface&, const std::string&);
	using Vec3ReadFuncPtr =
		TOptional<agx::Vec3> (*)(openplx::HeapControlInterface&, const std::string&);
	// clang-format on

	// Trait types that given a "primitive" type tell you the type of the function used to write or
	// read that type. First an empty base-base to produce compiler errors if an unsupported type is
	// passed, then follows the type-specific implementation that actually provides the function
	// types.
	template <typename UnrealTypeT>
	struct SelectInterfaceFunction
	{
	};

	template <>
	struct SelectInterfaceFunction<bool>
	{
		using Write = BoolWriteFuncPtr;
		using Read = BoolReadFuncPtr;
	};

	template <>
	struct SelectInterfaceFunction<int64_t>
	{
		using Write = IntWriteFuncPtr;
		using Read = IntReadFuncPtr;
	};

	template <>
	struct SelectInterfaceFunction<uint64_t>
	{
		using Write = UIntWriteFuncPtr;
		using Read = UIntReadFuncPtr;
	};

	template <>
	struct SelectInterfaceFunction<agx::Real>
	{
		using Write = RealWriteFuncPtr;
		using Read = RealReadFuncPtr;
	};

	template <>
	struct SelectInterfaceFunction<agx::Vec2>
	{
		using Write = Vec2WriteFuncPtr;
		using Read = Vec2ReadFuncPtr;
	};

	template <>
	struct SelectInterfaceFunction<agx::Vec3>
	{
		using Write = Vec3WriteFuncPtr;
		using Read = Vec3ReadFuncPtr;
	};

	// Functions that do the actual type-to-function look-up, based on both the type of the value
	// and the type of the input or output, to separate e.g. x/y from min/max.

	template <typename ValueT>
	SelectInterfaceFunction<ValueT>::Write GetInterfaceWriteFunction(const FOpenPLX_Input& Input)
	{
		static_assert(false, "GetInterfaceWriteFunction not implemented for this type");
	}

	template <typename ValueT>
	SelectInterfaceFunction<ValueT>::Read GetInterfaceReadFunction(const FOpenPLX_Output& Output)
	{
		static_assert(false, "GetInterfaceReadFunction not implemented for this type");
	}

	template <>
	BoolWriteFuncPtr GetInterfaceWriteFunction<bool>(const FOpenPLX_Input& Input)
	{
		return FOpenPLX_Utilities::IsBooleanType(Input.Type) ? InterfaceWriteBool : nullptr;
	}

	template <>
	BoolReadFuncPtr GetInterfaceReadFunction<bool>(const FOpenPLX_Output& Input)
	{
		return FOpenPLX_Utilities::IsBooleanType(Input.Type) ? InterfaceReadBool : nullptr;
	}

	template <>
	IntWriteFuncPtr GetInterfaceWriteFunction<int64_t>(const FOpenPLX_Input& Input)
	{
		return FOpenPLX_Utilities::IsIntegerType(Input.Type) ? InterfaceWriteSignedInteger
															 : nullptr;
	}

	template <>
	IntReadFuncPtr GetInterfaceReadFunction<int64_t>(const FOpenPLX_Output& Output)
	{
		return FOpenPLX_Utilities::IsIntegerType(Output.Type) ? InterfaceReadSignedInteger
															  : nullptr;
	}

	template <>
	UIntWriteFuncPtr GetInterfaceWriteFunction<uint64_t>(const FOpenPLX_Input& Input)
	{
		return FOpenPLX_Utilities::IsUnsignedIntegerType(Input.Type) ? InterfaceWriteUnsignedInteger
																	 : nullptr;
	}

	template <>
	UIntReadFuncPtr GetInterfaceReadFunction<uint64_t>(const FOpenPLX_Output& Output)
	{
		return FOpenPLX_Utilities::IsUnsignedIntegerType(Output.Type) ? InterfaceReadUnsignedInteger
																	  : nullptr;
	}

	template <>
	RealWriteFuncPtr GetInterfaceWriteFunction<agx::Real>(const FOpenPLX_Input& Input)
	{
		return FOpenPLX_Utilities::IsRealType(Input.Type) ? InterfaceWriteReal : nullptr;
	}

	template <>
	RealReadFuncPtr GetInterfaceReadFunction<agx::Real>(const FOpenPLX_Output& Output)
	{
		return FOpenPLX_Utilities::IsRealType(Output.Type) ? InterfaceReadReal : nullptr;
	}

	template <>
	Vec2WriteFuncPtr GetInterfaceWriteFunction<agx::Vec2>(const FOpenPLX_Input& Input)
	{
		if (FOpenPLX_Utilities::IsRangeType(Input.Type))
		{
			return InterfaceWriteRangeReal;
		}
		if (FOpenPLX_Utilities::IsVector2Type(Input.Type))
		{
			return InterfaceWriteVector2;
		}
		return nullptr;
	}

	template <>
	Vec2ReadFuncPtr GetInterfaceReadFunction<agx::Vec2>(const FOpenPLX_Output& Output)
	{
		if (FOpenPLX_Utilities::IsRangeType(Output.Type))
		{
			return InterfaceReadRangeReal;
		}
		if (FOpenPLX_Utilities::IsVector2Type(Output.Type))
		{
			return InterfaceReadVector2;
		}
		return nullptr;
	}

	template <>
	Vec3WriteFuncPtr GetInterfaceWriteFunction<agx::Vec3>(const FOpenPLX_Input& Input)
	{
		if (FOpenPLX_Utilities::IsRPYType(Input.Type))
		{
			return InterfaceWriteRPY;
		}
		if (FOpenPLX_Utilities::IsVectorType(Input.Type))
		{
			return InterfaceWriteVector3;
		}
		return nullptr;
	}

	template <>
	Vec3ReadFuncPtr GetInterfaceReadFunction<agx::Vec3>(const FOpenPLX_Output& Output)
	{
		if (FOpenPLX_Utilities::IsRPYType(Output.Type))
		{
			return InterfaceReadRPY;
		}
		if (FOpenPLX_Utilities::IsVectorType(Output.Type))
		{
			return InterfaceReadVector3;
		}
		return nullptr;
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
				TEXT(
					"OpenPLX Signal Handler: Tried to send OpenPLX Input signal for Input '%s' "
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
				TEXT(
					"OpenPLX Signal Handler: Tried to send OpenPLX signal, but the corresponding "
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

	template <typename ValuePLXT, typename ValueUnrealT, typename ConvertFuncT>
	bool SendInterfaceImpl(
		const FOpenPLX_Input& Input, ValueUnrealT ValueUnreal,
		openplx::HeapControlInterface* Interface, ConvertFuncT ConvertFunc)
	{
		if (Interface == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT(
					"OpenPLX Signal Handler: Tried to send OpenPLX Input signal for input '%s' "
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
				TEXT(
					"OpenPLX Signal Handler: Type and unit conversion from Unreal to OpenPLX "
					"failed for input '%s' ('%s') of type %s. This can mean that the input was "
					"used with the wrong value type, e.g. using SendVector2Interface and passing "
					"in an input expecting a Real value. This input expects values of type %s."),
				*Input.Name.ToString(), *Input.Alias.ToString(),
				*AGX_EnumUtilities::GetEnumName(Input.Type),
				FOpenPLX_Utilities::GetPrimitiveTypeName(Input.Type));
			return false;
		}

		ValuePLXT ValuePLX = *ValuePLXMaybe;
		std::string Alias = Convert(Input.Alias.ToString());
		auto WriteFunc = GetInterfaceWriteFunction<ValuePLXT>(Input);
		const bool bWritten = WriteFunc(*Interface, Alias, ValuePLX);
		if (!bWritten)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT(
					"OpenPLX Control Interface: Tried to send to input '%s' ('%s') but the write "
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
				TEXT(
					"Tried to receive OpenPLX Output signal for output '%s', but the OpenPLX "
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

	template <typename ValuePLXT, typename ValueUnrealT, typename ConvertFuncT>
	bool ReceiveInterfaceImpl(
		const FOpenPLX_Output& Output, ValueUnrealT& OutValue,
		openplx::HeapControlInterface* Interface, ConvertFuncT ConvertFunc)
	{
		OutValue = {};
		if (Interface == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT(
					"OpenPLX Signal Handler: Tried to receive OpenPLX Output signal for output "
					"'%s' ('%s') through the Control Interface, but don't have a Control "
					"Interface pointer."),
				*Output.Name.ToString(), *Output.Alias.ToString());
			return false;
		}

		std::string Alias = Convert(Output.Alias.ToString());
		auto ReadFunc = GetInterfaceReadFunction<ValuePLXT>(Output);
		TOptional<ValuePLXT> ValuePLXMaybe = ReadFunc(*Interface, Alias);
		if (!ValuePLXMaybe)
		{
			const FString TypeName = AGX_EnumUtilities::GetEnumName(Output.Type);
			UE_LOG(
				LogAGX, Warning,
				TEXT(
					"OpenPLX Signal Handler: Could not read output signal '%s' ('%s') of type "
					"'%s'."),
				*Output.Name.ToString(), *Output.Alias.ToString(), *TypeName);
			return false;
		}

		TOptional<ValueUnrealT> ConvertedMaybe = ConvertFunc(Output, *ValuePLXMaybe);
		if (!ConvertedMaybe)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT(
					"OpenPLX Signal Handler: Type and unit conversion from OpenPLX to Unreal "
					"failed for output '%s' ('%s') of type %s. This can mean that the output was "
					"used with the wrong value type, e.g. using ReceiveVector2Interface and "
					"passing in an output providing a Real value. This output provides values of "
					"type %s."),
				*Output.Name.ToString(), *Output.Alias.ToString(),
				*AGX_EnumUtilities::GetEnumName(Output.Type),
				FOpenPLX_Utilities::GetPrimitiveTypeName(Output.Type));
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
		Input, Value, GetHeapControlInterface(), ConvertRealToPLX);
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
		Output, OutValue, GetHeapControlInterface(), ConvertRealToUnreal);
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
		Input, Value, GetHeapControlInterface(), ConvertVector2ToPLXValue);
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
		Output, OutValue, GetHeapControlInterface(), ConvertVector2ValueToUnreal);
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
		Input, Value, GetHeapControlInterface(), ConvertVector3ToPLXValue);
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
		Output, OutValue, GetHeapControlInterface(), ConvertVector3ValueToUnreal);
}

bool FOpenPLXSignalHandler::Send(const FOpenPLX_Input& Input, int64 Value)
{
	check(IsInitialized());
	return OpenPLXSignalHandler_helpers::Send<int64, openplx::Physics::Signals::IntInputSignal>(
		Input, Value, ModelRegistry, ModelHandle, InputSignalListenerRef->Native->getQueue(),
		OpenPLXSignalHandler_helpers::ConvertIntegerToPLX);
}

bool FOpenPLXSignalHandler::SendInterface(const FOpenPLX_Input& Input, int64 Value)
{
	using namespace OpenPLXSignalHandler_helpers;
	check(IsInitialized());
	return SendInterfaceImpl<int64_t>(Input, Value, GetHeapControlInterface(), ConvertIntegerToPLX);
}

bool FOpenPLXSignalHandler::Receive(const FOpenPLX_Output& Output, int64& OutValue)
{
	check(IsInitialized());
	return OpenPLXSignalHandler_helpers::Receive(
		Output, OutValue, ModelRegistry, ModelHandle, OutputSignalListenerRef->Native->getQueue(),
		OpenPLXSignalHandler_helpers::GetUnrealIntegerValueFromSignal);
}

bool FOpenPLXSignalHandler::ReceiveInterface(const FOpenPLX_Output& Output, int64& OutValue)
{
	using namespace OpenPLXSignalHandler_helpers;
	check(IsInitialized());
	return ReceiveInterfaceImpl<int64_t>(
		Output, OutValue, GetHeapControlInterface(), ConvertIntegerToUnreal);
}

bool FOpenPLXSignalHandler::Send(const FOpenPLX_Input& Input, bool Value)
{
	check(IsInitialized());
	return OpenPLXSignalHandler_helpers::Send<bool, openplx::Physics::Signals::BoolInputSignal>(
		Input, Value, ModelRegistry, ModelHandle, InputSignalListenerRef->Native->getQueue(),
		OpenPLXSignalHandler_helpers::ConvertBooleanToPLX);
}

bool FOpenPLXSignalHandler::SendInterface(const FOpenPLX_Input& Input, bool Value)
{
	using namespace OpenPLXSignalHandler_helpers;
	check(IsInitialized());
	return SendInterfaceImpl<bool>(Input, Value, GetHeapControlInterface(), ConvertBooleanToPLX);
}

bool FOpenPLXSignalHandler::Receive(const FOpenPLX_Output& Output, bool& OutValue)
{
	check(IsInitialized());
	return OpenPLXSignalHandler_helpers::Receive(
		Output, OutValue, ModelRegistry, ModelHandle, OutputSignalListenerRef->Native->getQueue(),
		OpenPLXSignalHandler_helpers::GetUnrealBooleanValueFromSignal);
}

bool FOpenPLXSignalHandler::ReceiveInterface(const FOpenPLX_Output& Output, bool& OutValue)
{
	using namespace OpenPLXSignalHandler_helpers;
	check(IsInitialized());
	return ReceiveInterfaceImpl<bool>(
		Output, OutValue, GetHeapControlInterface(), ConvertBooleanToUnreal);
}

FHeapControlInterfacePtr FOpenPLXSignalHandler::GetHeapControlInterface()
{
	return const_cast<const FOpenPLXSignalHandler*>(this)->GetHeapControlInterface();
}

const FHeapControlInterfacePtr FOpenPLXSignalHandler::GetHeapControlInterface() const
{
	if (ModelRegistry == nullptr)
		return {nullptr};

	FOpenPLXModelData* ModelData = ModelRegistry->GetModelData(ModelHandle);
	if (ModelData == nullptr)
		return {nullptr};

	auto It = ModelData->HeapControlInterfaces.find(AssemblyRef->Native.get());
	if (It == ModelData->HeapControlInterfaces.end())
		return {nullptr};

	return {It->second.get()};
}

void FOpenPLXSignalHandler::ReleaseNatives()
{
	FOpenPLXModelData* ModelData = ModelRegistry->GetModelData(ModelHandle);
	if (ModelData)
	{
		ModelData->HeapControlInterfaces.erase(AssemblyRef->Native.get());
	}

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
