// Copyright 2024, Algoryx Simulation AB.

#include "OpenPLX/PLX_SignalHandlerComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
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
	TArray<FConstraintBarrier*> CollectConstraintBarriers(AActor* Owner)
	{
		if (Owner == nullptr)
			return TArray<FConstraintBarrier*>();

		TArray<UAGX_ConstraintComponent*> ConstraintsInThisActor =
			FAGX_ObjectUtilities::Filter<UAGX_ConstraintComponent>(Owner->GetComponents());
		TArray<FConstraintBarrier*> ConstraintBarriers;
		for (UAGX_ConstraintComponent* Constraint : ConstraintsInThisActor)
		{
			if (auto CBarrier = Constraint->GetOrCreateNative())
			{
				if (CBarrier->HasNative())
					ConstraintBarriers.Add(CBarrier);
			}
		}

		return ConstraintBarriers;
	}

	TOptional<FString> GetPLXFilePath(AActor* Owner)
	{
		if (Owner == nullptr)
			return {};

		auto ModelSource = Owner->GetComponentByClass<UAGX_ModelSourceComponent>();
		if (ModelSource == nullptr)
			return {};

		// Todo: this must point to the corresponding PLX file in the project files, copied from
		// import, not the source file itself. This is to ensure portability of standalone-projects.
		return ModelSource->FilePath;
	}
}

bool UPLX_SignalHandlerComponent::FindInput(EPLX_InputType Type, FString Name, FPLX_Input& OutInput)
{
	for (auto Elem : Inputs)
	{
		if (Elem.Value.Type == Type && Elem.Value.Name.Contains(Name))
		{
			OutInput = Elem.Value;
			return true;
		}
	}

	return false;
}

bool UPLX_SignalHandlerComponent::FindOutput(
	EPLX_OutputType Type, FString Name, FPLX_Output& OutOutput)
{
	for (auto Elem : Outputs)
	{
		if (Elem.Value.Type == Type && Elem.Value.Name.Contains(Name))
		{
			OutOutput = Elem.Value;
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

bool UPLX_SignalHandlerComponent::ReceiveReal(const FPLX_Output& Output, double& OutValue)
{
	if (!SignalHandler.IsInitialized())
		return false;

	return SignalHandler.Receive(Output, OutValue);
}

bool UPLX_SignalHandlerComponent::SendVector(const FPLX_Input& Input, FVector Value)
{
	if (!SignalHandler.IsInitialized())
		return false;

	return SignalHandler.Send(Input, Value);
}

bool UPLX_SignalHandlerComponent::ReceiveVector(const FPLX_Output& Output, FVector& OutValue)
{
	if (!SignalHandler.IsInitialized())
		return false;

	return SignalHandler.Receive(Output, OutValue);
}

bool UPLX_SignalHandlerComponent::SendInteger(const FPLX_Input& Input, int64 Value)
{
	if (!SignalHandler.IsInitialized())
		return false;

	return SignalHandler.Send(Input, Value);
}

bool UPLX_SignalHandlerComponent::ReceiveInteger(const FPLX_Output& Output, int64& OutValue)
{
	if (!SignalHandler.IsInitialized())
		return false;

	return SignalHandler.Receive(Output, OutValue);
}

bool UPLX_SignalHandlerComponent::SendBoolean(const FPLX_Input& Input, bool Value)
{
	if (!SignalHandler.IsInitialized())
		return false;

	return SignalHandler.Send(Input, Value);
}

bool UPLX_SignalHandlerComponent::ReceiveBoolean(const FPLX_Output& Output, bool& OutValue)
{
	if (!SignalHandler.IsInitialized())
		return false;

	return SignalHandler.Receive(Output, OutValue);
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
		// todo: log warning.
		return;
	}

	auto PLXModelRegistry = UPLX_ModelRegistry::GetFrom(GetWorld());
	auto PLXModelRegistryBarrier =
		PLXModelRegistry != nullptr ? PLXModelRegistry->GetNative() : nullptr;
	if (PLXModelRegistryBarrier == nullptr)
	{
		// Todo: log warning.
		return;
	}

	// Collect all Constraints in the same AActor as us.
	TArray<FConstraintBarrier*> ConstraintBarriers = CollectConstraintBarriers(GetOwner());

	// Initialize SignalHandler in Barrier module.
	SignalHandler.Init(*PLXFile, *SimulationBarrier, *PLXModelRegistryBarrier, ConstraintBarriers);
}

void UPLX_SignalHandlerComponent::CopyFrom(
	const TArray<FPLX_Input>& InInputs, TArray<FPLX_Output> InOutputs, FAGX_ImportContext* Context)
{
	for (const auto& Input : InInputs)
		Inputs.Add(Input.Name, Input);

	for (const auto& Output : InOutputs)
		Outputs.Add(Output.Name, Output);

	if (Context != nullptr)
	{
		AGX_CHECK(Context->SignalHandler == nullptr);
		Context->SignalHandler = this;
	}
}
