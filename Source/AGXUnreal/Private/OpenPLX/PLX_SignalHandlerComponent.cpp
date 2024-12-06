// Copyright 2024, Algoryx Simulation AB.

#include "OpenPLX/PLX_SignalHandlerComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Simulation.h"
#include "Constraints/AGX_ConstraintComponent.h"
#include "Import/AGX_ModelSourceComponent.h"
#include "OpenPLX/PLX_ModelRegistry.h"
#include "Utilities/AGX_ObjectUtilities.h"
#include "Utilities/AGX_StringUtilities.h"

UPLX_SignalHandlerComponent::UPLX_SignalHandlerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UPLX_SignalHandlerComponent::Send(const FPLX_LinearVelocity1DInput& Input, double Value)
{
	if (!SignalHandler.IsInitialized())
		return false;

	return SignalHandler.Send(Input, Value);
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

bool UPLX_SignalHandlerComponent::Receive(const FPLX_AngleOutput& Output, double& OutValue)
{
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
