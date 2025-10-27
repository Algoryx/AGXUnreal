// Copyright 2025, Algoryx Simulation AB.

#include "OpenPLX/OpenPLX_SignalHandlerComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_ObserverFrameComponent.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_Simulation.h"
#include "Constraints/AGX_ConstraintComponent.h"
#include "Import/AGX_ImportContext.h"
#include "Import/AGX_ModelSourceComponent.h"
#include "OpenPLX/OpenPLX_ModelRegistry.h"
#include "OpenPLX/OpenPLX_SignalHandlerInstanceData.h"
#include "OpenPLX/OpenPLX_SignalHandlerNativeAddresses.h"
#include "OpenPLX/OpenPLXMappingBarriersCollection.h"
#include "Utilities/AGX_ObjectUtilities.h"
#include "Utilities/AGX_StringUtilities.h"
#include "Utilities/OpenPLX_Utilities.h"
#include "Utilities/OpenPLXUtilities.h"

// Unreal Engine includes.
#include "Misc/Paths.h"

UOpenPLX_SignalHandlerComponent::UOpenPLX_SignalHandlerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

namespace OpenPLX_SignalHandlerComponent_helpers
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

	/**
	 * Takes an absolute path to an OpenPLX-file which may have been created in a different project
	 * or computer and replaces everything before the OpenPLXModels/ part to ensure we are pointing
	 * to the corresponding file for this project. Works correctly in standalone apps as well.
	 */
	FString RebuildOpenPLXFilePath(const FString& AbsolutePath)
	{
		const FString Marker = TEXT("OpenPLXModels/");
		const int32 Index = AbsolutePath.Find(Marker, ESearchCase::IgnoreCase, ESearchDir::FromEnd);

		if (Index == INDEX_NONE)
			return AbsolutePath;

		// Slice out everything after "OpenPLXModels/"
		const int32 Start = Index + Marker.Len();
		const FString RelativeSubPath = AbsolutePath.Mid(Start);

		// Combine with GetModelsDirectory()
		const FString ModelsDir = FOpenPLXUtilities::GetModelsDirectory();
		return FPaths::Combine(ModelsDir, RelativeSubPath);
	}

	TOptional<FString> GetOpenPLXFilePath(AActor* Owner)
	{
		if (Owner == nullptr)
			return {};

		auto ModelSource = Owner->GetComponentByClass<UAGX_ModelSourceComponent>();
		if (ModelSource == nullptr)
			return {};

		return RebuildOpenPLXFilePath(ModelSource->FilePath);
	}

	void LogTypeMismatchWarning(
		const FString& FunctionName, const FString& InputOutputName, const FString& InputOrOutput)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("SignalHandlerComponent %s called with %s %s with unexpected %s type. Check the "
				 "type used."),
			*FunctionName, *InputOrOutput, *InputOutputName, *InputOrOutput);
	}
}

bool UOpenPLX_SignalHandlerComponent::GetInput(FName Name, FOpenPLX_Input& OutInput)
{
	FString TrimmedString = Name.ToString().TrimStartAndEnd();
	FName TrimmedName(*TrimmedString);
	if (const FName* FullName = InputAliases.Find(TrimmedName))
	{
		if (const FOpenPLX_Input* Input = Inputs.Find(*FullName))
		{
			OutInput = *Input;
			return true;
		}
	}

	if (const FOpenPLX_Input* Input = Inputs.Find(TrimmedName))
	{
		OutInput = *Input;
		return true;
	}

	return false;
}

bool UOpenPLX_SignalHandlerComponent::GetInputFromType(
	EOpenPLX_InputType Type, FName Name, FOpenPLX_Input& OutInput)
{
	FString TrimmedString = Name.ToString().TrimStartAndEnd();
	FName TrimmedName(*TrimmedString);
	if (const FName* FullName = InputAliases.Find(TrimmedName))
	{
		if (const FOpenPLX_Input* Input = Inputs.Find(*FullName))
		{
			if (Input->Type == Type)
			{
				OutInput = *Input;
				return true;
			}
		}
	}

	if (const FOpenPLX_Input* Input = Inputs.Find(TrimmedName))
	{
		if (Input->Type == Type)
		{
			OutInput = *Input;
			return true;
		}
	}

	return false;
}

bool UOpenPLX_SignalHandlerComponent::GetOutput(FName Name, FOpenPLX_Output& OutOutput)
{
	FString TrimmedString = Name.ToString().TrimStartAndEnd();
	FName TrimmedName(*TrimmedString);
	if (const FName* FullName = OutputAliases.Find(TrimmedName))
	{
		if (const FOpenPLX_Output* Output = Outputs.Find(*FullName))
		{
			OutOutput = *Output;
			return true;
		}
	}

	if (const FOpenPLX_Output* Output = Outputs.Find(TrimmedName))
	{
		OutOutput = *Output;
		return true;
	}

	return false;
}

bool UOpenPLX_SignalHandlerComponent::GetOutputFromType(
	EOpenPLX_OutputType Type, FName Name, FOpenPLX_Output& OutOutput)
{
	FString TrimmedString = Name.ToString().TrimStartAndEnd();
	FName TrimmedName(*TrimmedString);
	if (const FName* FullName = OutputAliases.Find(TrimmedName))
	{
		if (const FOpenPLX_Output* Output = Outputs.Find(*FullName))
		{
			if (Output->Type == Type)
			{
				OutOutput = *Output;
				return true;
			}
		}
	}

	if (const FOpenPLX_Output* Output = Outputs.Find(TrimmedName))
	{
		if (Output->Type == Type)
		{
			OutOutput = *Output;
			return true;
		}
	}

	return false;
}

bool UOpenPLX_SignalHandlerComponent::SendReal(const FOpenPLX_Input& Input, double Value)
{
	using namespace OpenPLX_SignalHandlerComponent_helpers;
	if (!SignalHandler.IsInitialized())
		return false;

	if (!FOpenPLX_Utilities::IsRealType(Input.Type))
	{
		LogTypeMismatchWarning("SendReal", Input.Name.ToString(), "Input");
		return false;
	}

	return SignalHandler.Send(Input, Value);
}

bool UOpenPLX_SignalHandlerComponent::SendRealByName(FName NameOrAlias, double Value)
{
	FOpenPLX_Input Input;
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

bool UOpenPLX_SignalHandlerComponent::ReceiveReal(const FOpenPLX_Output& Output, double& OutValue)
{
	using namespace OpenPLX_SignalHandlerComponent_helpers;
	if (!SignalHandler.IsInitialized())
		return false;

	if (!FOpenPLX_Utilities::IsRealType(Output.Type))
	{
		LogTypeMismatchWarning("ReceiveReal", Output.Name.ToString(), "Output");
		return false;
	}

	return SignalHandler.Receive(Output, OutValue);
}

bool UOpenPLX_SignalHandlerComponent::ReceiveRealByName(FName NameOrAlias, double& Value)
{
	FOpenPLX_Output Output;
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

bool UOpenPLX_SignalHandlerComponent::SendRangeReal(const FOpenPLX_Input& Input, FVector2D Value)
{
	using namespace OpenPLX_SignalHandlerComponent_helpers;
	if (!SignalHandler.IsInitialized())
		return false;

	if (!FOpenPLX_Utilities::IsRangeType(Input.Type))
	{
		LogTypeMismatchWarning("SendRangeReal", Input.Name.ToString(), "Input");
		return false;
	}

	return SignalHandler.Send(Input, Value);
}

bool UOpenPLX_SignalHandlerComponent::SendRangeRealByName(FName NameOrAlias, FVector2D Value)
{
	FOpenPLX_Input Input;
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

bool UOpenPLX_SignalHandlerComponent::ReceiveRangeReal(
	const FOpenPLX_Output& Output, FVector2D& OutValue)
{
	using namespace OpenPLX_SignalHandlerComponent_helpers;
	if (!SignalHandler.IsInitialized())
		return false;

	if (!FOpenPLX_Utilities::IsRangeType(Output.Type))
	{
		LogTypeMismatchWarning("ReceiveRangeReal", Output.Name.ToString(), "Output");
		return false;
	}

	return SignalHandler.Receive(Output, OutValue);
}

bool UOpenPLX_SignalHandlerComponent::ReceiveRangeRealByName(FName NameOrAlias, FVector2D& OutValue)
{
	FOpenPLX_Output Output;
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

bool UOpenPLX_SignalHandlerComponent::SendVector(const FOpenPLX_Input& Input, FVector Value)
{
	using namespace OpenPLX_SignalHandlerComponent_helpers;
	if (!SignalHandler.IsInitialized())
		return false;

	if (!FOpenPLX_Utilities::IsVectorType(Input.Type))
	{
		LogTypeMismatchWarning("SendVector", Input.Name.ToString(), "Input");
		return false;
	}

	return SignalHandler.Send(Input, Value);
}

bool UOpenPLX_SignalHandlerComponent::SendVectorByName(FName NameOrAlias, FVector Value)
{
	FOpenPLX_Input Input;
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

bool UOpenPLX_SignalHandlerComponent::ReceiveVector(
	const FOpenPLX_Output& Output, FVector& OutValue)
{
	using namespace OpenPLX_SignalHandlerComponent_helpers;
	if (!SignalHandler.IsInitialized())
		return false;

	if (!FOpenPLX_Utilities::IsVectorType(Output.Type))
	{
		LogTypeMismatchWarning("ReceiveVector", Output.Name.ToString(), "Output");
		return false;
	}

	return SignalHandler.Receive(Output, OutValue);
}

bool UOpenPLX_SignalHandlerComponent::ReceiveVectorByName(FName NameOrAlias, FVector& OutValue)
{
	FOpenPLX_Output Output;
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

bool UOpenPLX_SignalHandlerComponent::SendInteger(const FOpenPLX_Input& Input, int64 Value)
{
	using namespace OpenPLX_SignalHandlerComponent_helpers;
	if (!SignalHandler.IsInitialized())
		return false;

	if (!FOpenPLX_Utilities::IsIntegerType(Input.Type))
	{
		LogTypeMismatchWarning("SendInteger", Input.Name.ToString(), "Input");
		return false;
	}

	return SignalHandler.Send(Input, Value);
}

bool UOpenPLX_SignalHandlerComponent::SendIntegerByName(FName NameOrAlias, int64 Value)
{
	FOpenPLX_Input Input;
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

bool UOpenPLX_SignalHandlerComponent::ReceiveInteger(const FOpenPLX_Output& Output, int64& OutValue)
{
	using namespace OpenPLX_SignalHandlerComponent_helpers;
	if (!SignalHandler.IsInitialized())
		return false;

	if (!FOpenPLX_Utilities::IsIntegerType(Output.Type))
	{
		LogTypeMismatchWarning("ReceiveInteger", Output.Name.ToString(), "Output");
		return false;
	}

	return SignalHandler.Receive(Output, OutValue);
}

bool UOpenPLX_SignalHandlerComponent::ReceiveIntegerByName(FName NameOrAlias, int64& OutValue)
{
	FOpenPLX_Output Output;
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

bool UOpenPLX_SignalHandlerComponent::SendBoolean(const FOpenPLX_Input& Input, bool Value)
{
	using namespace OpenPLX_SignalHandlerComponent_helpers;
	if (!SignalHandler.IsInitialized())
		return false;

	if (!FOpenPLX_Utilities::IsBooleanType(Input.Type))
	{
		LogTypeMismatchWarning("SendBoolean", Input.Name.ToString(), "Input");
		return false;
	}

	return SignalHandler.Send(Input, Value);
}

bool UOpenPLX_SignalHandlerComponent::SendBooleanByName(FName NameOrAlias, bool Value)
{
	FOpenPLX_Input Input;
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

bool UOpenPLX_SignalHandlerComponent::ReceiveBoolean(const FOpenPLX_Output& Output, bool& OutValue)
{
	using namespace OpenPLX_SignalHandlerComponent_helpers;
	if (!SignalHandler.IsInitialized())
		return false;

	if (!FOpenPLX_Utilities::IsBooleanType(Output.Type))
	{
		LogTypeMismatchWarning("ReceiveBoolean", Output.Name.ToString(), "Output");
		return false;
	}

	return SignalHandler.Receive(Output, OutValue);
}

bool UOpenPLX_SignalHandlerComponent::ReceiveBooleanByName(FName NameOrAlias, bool& OutValue)
{
	FOpenPLX_Output Output;
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

void UOpenPLX_SignalHandlerComponent::BeginPlay()
{
	using namespace OpenPLX_SignalHandlerComponent_helpers;
	Super::BeginPlay();

	if (SignalHandler.IsInitialized() || GIsReconstructingBlueprintInstances)
		return;

	auto PLXFile = GetOpenPLXFilePath(GetOwner());
	if (!PLXFile.IsSet())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("UOpenPLX_SignalHandlerComponent '%s' in '%s' was unable to get OpenPLX file path "
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
			TEXT("OpenPLX Signal Hander Component in '%s' was unable to get the native AGX "
				 "Simulation. Signal handling may not work."),
			*GetLabelSafe(GetOwner()));
		return;
	}

	auto PLXModelRegistry = UOpenPLX_ModelRegistry::GetFrom(GetWorld());
	auto PLXModelRegistryBarrier =
		PLXModelRegistry != nullptr ? PLXModelRegistry->GetNative() : nullptr;
	if (PLXModelRegistryBarrier == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("OpenPLX Signal Hander Component in '%s' was unable to get the model registry "
				 "barrier object. Signal handling may not work."),
			*GetLabelSafe(GetOwner()));
		return;
	}

	// Collect all relevant AGX objects in the same AActor as us.
	FOpenPLXMappingBarriersCollection Barriers;
	Barriers.Constraints =
		CollectBarriers<FConstraintBarrier, UAGX_ConstraintComponent>(GetOwner());
	Barriers.Bodies =
		CollectBarriers<FRigidBodyBarrier, UAGX_RigidBodyComponent>(GetOwner());
	Barriers.ObserverFrames =
		CollectBarriers<FObserverFrameBarrier, UAGX_ObserverFrameComponent>(GetOwner());

	// Initialize SignalHandler in Barrier module.
	SignalHandler.Init(*PLXFile, *SimulationBarrier, *PLXModelRegistryBarrier, Barriers);
}

void UOpenPLX_SignalHandlerComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);
	if (!GIsReconstructingBlueprintInstances)
		SignalHandler.ReleaseNatives();
}

TStructOnScope<FActorComponentInstanceData>
UOpenPLX_SignalHandlerComponent::GetComponentInstanceData() const
{
	return MakeStructOnScope<FActorComponentInstanceData, FOpenPLX_SignalHandlerInstanceData>(
		*this);
}

void UOpenPLX_SignalHandlerComponent::CopyFrom(
	const TArray<FOpenPLX_Input>& InInputs, TArray<FOpenPLX_Output> InOutputs,
	FAGX_ImportContext* Context)
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

void UOpenPLX_SignalHandlerComponent::SetNativeAddresses(
	const FOpenPLX_SignalHandlerNativeAddresses& Addresses)
{
	SignalHandler.SetNativeAddresses(Addresses);
}

FOpenPLX_SignalHandlerNativeAddresses UOpenPLX_SignalHandlerComponent::GetNativeAddresses() const
{
	return SignalHandler.GetNativeAddresses();
}
