// Copyright 2024, Algoryx Simulation AB.

#include "AGX_ObserverFrameComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "Import/AGX_ImportContext.h"
#include "Import/SimulationObjectCollection.h"
#include "Utilities/AGX_ImportRuntimeUtilities.h"
#include "Utilities/AGX_ObjectUtilities.h"

void UAGX_ObserverFrameComponent::CopyFrom(
	const FObserverFrameData& Data, FAGX_ImportContext* Context)
{
	const FString Name = FAGX_ObjectUtilities::SanitizeAndMakeNameUnique(
		GetOwner(), Data.Name, UAGX_ObserverFrameComponent::StaticClass());
	Rename(*Name);

	SetRelativeTransform(Data.Transform);
	ImportGuid = Data.ObserverGuid;

	if (Context != nullptr && Context->ObserverFrames != nullptr)
	{
		AGX_CHECK(!Context->ObserverFrames->Contains(ImportGuid));
		Context->ObserverFrames->Add(ImportGuid, this);
	}
}
