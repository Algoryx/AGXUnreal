// Copyright 2025, Algoryx Simulation AB.

#include "OpenPLX/PLX_SignalHandlerComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_Simulation.h"
#include "Constraints/AGX_ConstraintComponent.h"
#include "Import/AGX_ImportContext.h"
#include "Import/AGX_ModelSourceComponent.h"
#include "OpenPLX/PLX_ModelRegistry.h"
#include "Utilities/AGX_ObjectUtilities.h"
#include "Utilities/AGX_StringUtilities.h"

UPLX_SignalHandlerComponent::UPLX_SignalHandlerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

namespace PLX_SignalHandlerComponent_helpers
{
	template <typename BarrierT, typename ComponentT>
	TArray<BarrierT*> CollectBarriers(AActor* Owner)
	{
		TArray<BarrierT*> Barriers;
		if (Owner == nullptr)
			return Barriers;

		TArray<ComponentT*> ComponentsInThisActor =
			FAGX_ObjectUtilities::Filter<ComponentT>(Owner->GetComponents());
		for (ComponentT* Component : ComponentsInThisActor)
		{
			if (auto CBarrier = Component->GetOrCreateNative())
			{
				if (CBarrier->HasNative())
					Barriers.Add(CBarrier);
			}
		}

		return Barriers;
	}

	TOptional<FString> GetPLXFilePath(AActor* Owner)
	{
		if (Owner == nullptr)
			return {};

		auto ModelSource = Owner->GetComponentByClass<UAGX_ModelSourceComponent>();
		if (ModelSource == nullptr)
			return {};

		return ModelSource->FilePath;
	}
}

bool UPLX_SignalHandlerComponent::GetInput(FName Name, FPLX_Input& OutInput)
{
	if (const FName* FullName = InputAliases.Find(Name))
	{
		if (const FPLX_Input* Input = Inputs.Find(*FullName))
		{
			OutInput = *Input;
			return true;
		}
	}

	if (const FPLX_Input* Input = Inputs.Find(Name))
	{
		OutInput = *Input;
		return true;
	}

	return false;
}

bool UPLX_SignalHandlerComponent::GetInputFromType(
	EPLX_InputType Type, FName Name, FPLX_Input& OutInput)
{
	if (const FName* FullName = InputAliases.Find(Name))
	{
		if (const FPLX_Input* Input = Inputs.Find(*FullName))
		{
			if (Input->Type == Type)
			{
				OutInput = *Input;
				return true;
			}
		}
	}

	if (const FPLX_Input* Input = Inputs.Find(Name))
	{
		if (Input->Type == Type)
		{
			OutInput = *Input;
			return true;
		}
	}

	return false;
}

bool UPLX_SignalHandlerComponent::GetOutput(FName Name, FPLX_Output& OutOutput)
{
	if (const FName* FullName = OutputAliases.Find(Name))
	{
		if (const FPLX_Output* Output = Outputs.Find(*FullName))
		{
			OutOutput = *Output;
			return true;
		}
	}

	if (const FPLX_Output* Output = Outputs.Find(Name))
	{
		OutOutput = *Output;
		return true;
	}

	return false;
}

bool UPLX_SignalHandlerComponent::GetOutputFromType(
	EPLX_OutputType Type, FName Name, FPLX_Output& OutOutput)
{
	if (const FName* FullName = OutputAliases.Find(Name))
	{
		if (const FPLX_Output* Output = Outputs.Find(*FullName))
		{
			if (Output->Type == Type)
			{
				OutOutput = *Output;
				return true;
			}
		}
	}

	if (const FPLX_Output* Output = Outputs.Find(Name))
	{
		if (Output->Type == Type)
		{
			OutOutput = *Output;
			return true;
		}
	}

	return false;
}

bool UPLX_SignalHandlerComponent::SendReal(const FPLX_Input& Input, double Value)
{
	if (!SignalHandler.IsInitialized())
		return false;

	return SignalHandler.Send(Input, Value);
}

bool UPLX_SignalHandlerComponent::SendRealByName(FName NameOrAlias, double Value)
{
	FPLX_Input Input;
	const bool Found = GetInput(NameOrAlias, Input);
	if (!Found)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("SendRealByname: Unable to find Input matching Name or Alias '%s'."),
			*NameOrAlias.ToString());
		return false;
	}

	return SendReal(Input, Value);
}

bool UPLX_SignalHandlerComponent::ReceiveReal(const FPLX_Output& Output, double& OutValue)
{
	if (!SignalHandler.IsInitialized())
		return false;

	return SignalHandler.Receive(Output, OutValue);
}

bool UPLX_SignalHandlerComponent::ReceiveRealByName(FName NameOrAlias, double& Value)
{
	FPLX_Output Output;
	const bool Found = GetOutput(NameOrAlias, Output);
	if (!Found)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("ReceiveRealByName: Unable to find Output matching Name or Alias '%s'."),
			*NameOrAlias.ToString());
		return false;
	}

	return ReceiveReal(Output, Value);
}

bool UPLX_SignalHandlerComponent::SendRangeReal(const FPLX_Input& Input, FVector2D Value)
{
	if (!SignalHandler.IsInitialized())
		return false;

	return SignalHandler.Send(Input, Value);
}

bool UPLX_SignalHandlerComponent::SendRangeRealByName(FName NameOrAlias, FVector2D Value)
{
	FPLX_Input Input;
	const bool Found = GetInput(NameOrAlias, Input);
	if (!Found)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("SendRangeRealByName: Unable to find Input matching Name or Alias '%s'."),
			*NameOrAlias.ToString());
		return false;
	}

	return SendRangeReal(Input, Value);
}

bool UPLX_SignalHandlerComponent::ReceiveRangeReal(const FPLX_Output& Output, FVector2D& OutValue)
{
	if (!SignalHandler.IsInitialized())
		return false;

	return SignalHandler.Receive(Output, OutValue);
}

bool UPLX_SignalHandlerComponent::ReceiveRangeRealByName(
	FName NameOrAlias, FVector2D& OutValue)
{
	FPLX_Output Output;
	const bool Found = GetOutput(NameOrAlias, Output);
	if (!Found)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("ReceiveRangeRealByName: Unable to find Output matching Name or Alias '%s'."),
			*NameOrAlias.ToString());
		return false;
	}

	return ReceiveRangeReal(Output, OutValue);
}

bool UPLX_SignalHandlerComponent::SendVector(const FPLX_Input& Input, FVector Value)
{
	if (!SignalHandler.IsInitialized())
		return false;

	return SignalHandler.Send(Input, Value);
}

bool UPLX_SignalHandlerComponent::SendVectorByName(FName NameOrAlias, FVector Value)
{
	FPLX_Input Input;
	const bool Found = GetInput(NameOrAlias, Input);
	if (!Found)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("SendVectorByName: Unable to find Input matching Name or Alias '%s'."),
			*NameOrAlias.ToString());
		return false;
	}

	return SendVector(Input, Value);
}

bool UPLX_SignalHandlerComponent::ReceiveVector(const FPLX_Output& Output, FVector& OutValue)
{
	if (!SignalHandler.IsInitialized())
		return false;

	return SignalHandler.Receive(Output, OutValue);
}

bool UPLX_SignalHandlerComponent::ReceiveVectorByName(FName NameOrAlias, FVector& OutValue)
{
	FPLX_Output Output;
	const bool Found = GetOutput(NameOrAlias, Output);
	if (!Found)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("ReceiveVectorByName: Unable to find Output matching Name or Alias '%s'."),
			*NameOrAlias.ToString());
		return false;
	}

	return ReceiveVector(Output, OutValue);
}

bool UPLX_SignalHandlerComponent::SendInteger(const FPLX_Input& Input, int64 Value)
{
	if (!SignalHandler.IsInitialized())
		return false;

	return SignalHandler.Send(Input, Value);
}

bool UPLX_SignalHandlerComponent::SendIntegerByName(FName NameOrAlias, int64 Value)
{
	FPLX_Input Input;
	const bool Found = GetInput(NameOrAlias, Input);
	if (!Found)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("SendIntegerByName: Unable to find Input matching Name or Alias '%s'."),
			*NameOrAlias.ToString());
		return false;
	}

	return SendInteger(Input, Value);
}

bool UPLX_SignalHandlerComponent::ReceiveInteger(const FPLX_Output& Output, int64& OutValue)
{
	if (!SignalHandler.IsInitialized())
		return false;

	return SignalHandler.Receive(Output, OutValue);
}

bool UPLX_SignalHandlerComponent::ReceiveIntegerByName(FName NameOrAlias, int64& OutValue)
{
	FPLX_Output Output;
	const bool Found = GetOutput(NameOrAlias, Output);
	if (!Found)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("ReceiveIntegerByName: Unable to find Output matching Name or Alias '%s'."),
			*NameOrAlias.ToString());
		return false;
	}

	return ReceiveInteger(Output, OutValue);
}

bool UPLX_SignalHandlerComponent::SendBoolean(const FPLX_Input& Input, bool Value)
{
	if (!SignalHandler.IsInitialized())
		return false;

	return SignalHandler.Send(Input, Value);
}

bool UPLX_SignalHandlerComponent::SendBooleanByName(FName NameOrAlias, bool Value)
{
	FPLX_Input Input;
	const bool Found = GetInput(NameOrAlias, Input);
	if (!Found)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("SendBooleanByName: Unable to find Input matching Name or Alias '%s'."),
			*NameOrAlias.ToString());
		return false;
	}

	return SendBoolean(Input, Value);
}

bool UPLX_SignalHandlerComponent::ReceiveBoolean(const FPLX_Output& Output, bool& OutValue)
{
	if (!SignalHandler.IsInitialized())
		return false;

	return SignalHandler.Receive(Output, OutValue);
}

bool UPLX_SignalHandlerComponent::ReceiveBooleanByName(FName NameOrAlias, bool& OutValue)
{
	FPLX_Output Output;
	const bool Found = GetOutput(NameOrAlias, Output);
	if (!Found)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("ReceiveBooleanByName: Unable to find Output matching Name or Alias '%s'."),
			*NameOrAlias.ToString());
		return false;
	}

	return ReceiveBoolean(Output, OutValue);
}

void UPLX_SignalHandlerComponent::BeginPlay()
{
	using namespace PLX_SignalHandlerComponent_helpers;
	Super::BeginPlay();

	auto PLXFile = GetPLXFilePath(GetOwner());
	if (!PLXFile.IsSet())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("UPLX_SignalHandlerComponent '%s' in '%s' was unable to get OpenPLX file path "
				 "from UAGX_ModelSourceComponent. OpenPLX Signals will not work properly."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return;
	}

	auto Sim = UAGX_Simulation::GetFrom(this);
	auto SimulationBarrier = Sim != nullptr ? Sim->GetNative() : nullptr;
	if (SimulationBarrier == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("PLX Signal Hander Component in '%s' was unable to get the native AGX Simulation. "
				 "Signal handling may not work."),
			*GetLabelSafe(GetOwner()));
		return;
	}

	auto PLXModelRegistry = UPLX_ModelRegistry::GetFrom(GetWorld());
	auto PLXModelRegistryBarrier =
		PLXModelRegistry != nullptr ? PLXModelRegistry->GetNative() : nullptr;
	if (PLXModelRegistryBarrier == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("PLX Signal Hander Component in '%s' was unable to get the model registry barrier "
				 "object. Signal handling may not work."),
			*GetLabelSafe(GetOwner()));
		return;
	}

	// Collect all relevant AGX objects in the same AActor as us.
	auto ConstraintBarriers =
		CollectBarriers<FConstraintBarrier, UAGX_ConstraintComponent>(GetOwner());
	auto RigidBodyBarriers =
		CollectBarriers<FRigidBodyBarrier, UAGX_RigidBodyComponent>(GetOwner());

	// Initialize SignalHandler in Barrier module.
	SignalHandler.Init(
		*PLXFile, *SimulationBarrier, *PLXModelRegistryBarrier, RigidBodyBarriers,
		ConstraintBarriers);
}

void UPLX_SignalHandlerComponent::CopyFrom(
	const TArray<FPLX_Input>& InInputs, TArray<FPLX_Output> InOutputs, FAGX_ImportContext* Context)
{
	for (const auto& Input : InInputs)
	{
		Inputs.Add(Input.Name, Input);
		if (!Input.Alias.IsNone())
			InputAliases.Add(Input.Alias, Input.Name);
	}

	for (const auto& Output : InOutputs)
	{
		Outputs.Add(Output.Name, Output);
		if (!Output.Alias.IsNone())
			OutputAliases.Add(Output.Alias, Output.Name);
	}

	if (Context != nullptr)
	{
		AGX_CHECK(Context->SignalHandler == nullptr);
		Context->SignalHandler = this;
	}
}
