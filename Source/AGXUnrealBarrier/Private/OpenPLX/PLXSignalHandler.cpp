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
#include "agxOpenPLX/SignalListenerUtils.h"
#include "Physics/Signals/RealInputSignal.h"

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

bool FPLXSignalHandler::Send(const FPLX_LinearVelocity1DInput& Input, double Value)
{
	check(IsInitialized());
	if (ModelRegistry == nullptr || ModelHandle == FPLXModelRegistry::InvalidHandle)
		return false;

	if (InputQueueRef == nullptr || InputQueueRef->Native == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Tried to send OpenPLX Linear Velocity 1D Input signal, value %f, but the OpenPLX "
				 "model does not have any registered inputs."),
			Value);
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
			TEXT("Tried to send OpenPLX Linear Velocity 1D Input signal, value %f, but the "
				 "corresponding OpenPLX input '%s' was not found. The signal will not be sent."),
			Value, *Input.Name);
		return false;
	}

	auto Signal = openplx::Physics::Signals::RealInputSignal::create(
		ConvertDistanceToAGX(Value), PLXInput->second);

	InputQueueRef->Native->send(Signal);
	return true;
}

bool FPLXSignalHandler::Receive(const FPLX_AngleOutput& Output, double& OutValue)
{
	check(IsInitialized());
	if (ModelRegistry == nullptr || ModelHandle == FPLXModelRegistry::InvalidHandle)
		return false;

	if (OutputQueueRef == nullptr || OutputQueueRef->Native == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Tried to receive OpenPLX Angle Output signal for output '%s', but the OpenPLX "
				 "model does not have any registered outputs."),
			*Output.Name);
		return false;
	}

	auto ValueOutputSignal =
		agxopenplx::getSignalBySourceName<openplx::Physics::Signals::ValueOutputSignal>(
			OutputQueueRef->Native->getSignals(), Convert(Output.Name));
	if (ValueOutputSignal == nullptr)
		return false;

	auto Value = std::dynamic_pointer_cast<openplx::Physics::Signals::AngleValue>(
		ValueOutputSignal->value());
	if (Value == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Unexpected error: Tried to cast OpenPLX Angle Output signal for output '%s', but "
				 "got nullptr. Possible type miss match. The signal will not be received."),
			*Output.Name);
		return false;
	}

	OutValue = ConvertAngleToUnreal<double>(Value->value());
	return true;
}
