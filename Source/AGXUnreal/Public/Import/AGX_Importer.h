// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Import/AGX_ImportEnums.h"
#include "Import/AGX_ImportContext.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

class FRigidBodyBarrier;
class FShapeBarrier;
class FShovelBarrier;
class UAGX_ModelSourceComponent;
class UAGX_RigidBodyComponent;

struct FAGX_ImportSettings;
struct FSimulationObjectCollection;
struct FObserverFrameData;

struct AGXUNREAL_API FAGX_ImportResult
{
	explicit FAGX_ImportResult(EAGX_ImportResult InResult)
		: Result(InResult)
	{
	}

	FAGX_ImportResult(EAGX_ImportResult InResult, AActor* InActor, FAGX_ImportContext* InContext)
		: Result(InResult)
		, Actor(InActor)
		, Context(InContext)
	{
	}

	EAGX_ImportResult Result {EAGX_ImportResult::Invalid};
	TObjectPtr<AActor> Actor;
	FAGX_ImportContext* Context {nullptr};
};

class AGXUNREAL_API FAGX_Importer
{
public:
	FAGX_Importer();

	FAGX_ImportResult Import(const FAGX_ImportSettings& Settings);
	const FAGX_ImportContext& GetContext() const;

private:
	EAGX_ImportResult AddComponents(
		const FSimulationObjectCollection& SimObjects, AActor& OutActor);

	EAGX_ImportResult AddModelSourceComponent(AActor& OutActor);

	EAGX_ImportResult AddContactMaterialRegistrarComponent(
		const FSimulationObjectCollection& SimObjects, AActor& OutActor);

	EAGX_ImportResult AddCollisionGroupDisablerComponent(
		const FSimulationObjectCollection& SimObjects, AActor& OutActor);

	EAGX_ImportResult AddObserverFrame(
		const FObserverFrameData& Frame, const FSimulationObjectCollection& SimObjects, AActor& OutActor);

	template <typename TComponent, typename TBarrier>
	EAGX_ImportResult AddComponent(
		const TBarrier& Barrier, USceneComponent& Parent, AActor& OutActor);

	template <typename TShapeComponent>
	EAGX_ImportResult AddShape(const FShapeBarrier& Shape, AActor& OutActor);

	EAGX_ImportResult AddTrimeshShape(const FShapeBarrier& Shape, AActor& OutActor);

	EAGX_ImportResult AddShovel(const FShovelBarrier& Shovel, AActor& OutActor);

	FAGX_ImportContext Context;
};
