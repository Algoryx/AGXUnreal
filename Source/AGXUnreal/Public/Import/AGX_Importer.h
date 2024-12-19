// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_AGXToUeContext.h"
#include "Import/AGX_ImportEnums.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

class FRigidBodyBarrier;
class UAGX_ModelSourceComponent;
class UAGX_RigidBodyComponent;

struct FAGX_ImporterSettings;
struct FSimulationObjectCollection;

struct AGXUNREAL_API FAGX_ImportResult
{
	explicit FAGX_ImportResult(EAGX_ImportResult InResult)
		: Result(InResult)
	{
	}

	FAGX_ImportResult(EAGX_ImportResult InResult, AActor* InActor, FAGX_AGXToUeContext* InContext)
		: Result(InResult)
		, Actor(InActor)
		, Context(InContext)
	{
	}

	EAGX_ImportResult Result {EAGX_ImportResult::Invalid};
	TObjectPtr<AActor> Actor;
	FAGX_AGXToUeContext* Context {nullptr};
};

#if 0
/** Mapped objects seen during import process. */
struct AGXUNREAL_API FAGX_ImporterObjectMaps
{
	TMap<FGuid, UAGX_RigidBodyComponent*> Bodies;
	TMap<FGuid, UAGX_MergeSplitThresholdsBase*> MSThresholds;
	UAGX_ModelSourceComponent* ModelSourceComponent {nullptr};
};
#endif

class AGXUNREAL_API FAGX_Importer
{
public:
	FAGX_Importer();
	FAGX_ImportResult Import(const FAGX_ImporterSettings& Settings);
	const FAGX_AGXToUeContext& GetContext() const;
	
private:
	EAGX_ImportResult AddComponents(
		const FSimulationObjectCollection& SimObjects, const FAGX_ImporterSettings& Settings,
		AActor& OutActor);

	EAGX_ImportResult AddModelSourceComponent(
		const FAGX_ImporterSettings& Settings, AActor& OutActor);
	EAGX_ImportResult AddRigidBody(const FRigidBodyBarrier& Barrier, AActor& OutActor);

	FAGX_AGXToUeContext Context;
};
