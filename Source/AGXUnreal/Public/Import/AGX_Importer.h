// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Import/AGX_ImportEnums.h"
#include "Import/AGX_ImportContext.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "AGX_Importer.generated.h"

struct FRigidBodyBarrier;
struct FShapeBarrier;
class FShovelBarrier;
class UAGX_ModelSourceComponent;
class UAGX_RigidBodyComponent;

struct FAGX_ImportSettings;
struct FObserverFrameBarrier;
struct FSensorBarrier;
struct FSimulationObjectCollection;

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

/**
 * AGX_Importer with complete runtime support.
 */
UCLASS()
class AGXUNREAL_API UAGX_Importer : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Import an .agx archive, OpenPLX or Urdf model to an Actor that can either be instantiated
	 * immediately in a world, or used to create a Blueprint from it.
	 * The Importer's Outer must be set to a World if doing runtime imports, otherwise it can be set
	 * to TransientPackage.
	 */
	FAGX_ImportResult Import(const FAGX_ImportSettings& Settings);
	const FAGX_ImportContext& GetContext() const;

private:
	EAGX_ImportResult AddComponents(
		const FAGX_ImportSettings& Settings, const FSimulationObjectCollection& SimObjects,
		AActor& OutActor);

	EAGX_ImportResult AddModelSourceComponent(AActor& OutActor);

	EAGX_ImportResult AddContactMaterialRegistrarComponent(
		const FSimulationObjectCollection& SimObjects, AActor& OutActor);

	EAGX_ImportResult AddCollisionGroupDisablerComponent(
		const FSimulationObjectCollection& SimObjects, AActor& OutActor);

	EAGX_ImportResult AddObserverFrame(
		const FObserverFrameBarrier& Frame, const FSimulationObjectCollection& SimObjects,
		AActor& OutActor);

	template <typename TComponent, typename TBarrier>
	EAGX_ImportResult AddComponent(
		const TBarrier& Barrier, USceneComponent& Parent, AActor& OutActor);

	template <typename TShapeComponent>
	EAGX_ImportResult AddShape(const FShapeBarrier& Shape, AActor& OutActor);

	EAGX_ImportResult AddTrimeshShape(const FShapeBarrier& Shape, AActor& OutActor);

	EAGX_ImportResult AddShovel(const FShovelBarrier& Shovel, AActor& OutActor);

	EAGX_ImportResult AddLidar(const FSensorBarrier& Sensor, AActor& OutActor);

	EAGX_ImportResult AddSignalHandlerComponent(
		const FSimulationObjectCollection& SimObjects, AActor& OutActor);

	void PostImport();

	UPROPERTY(Transient)
	TObjectPtr<AActor> ImportedActor;

	UPROPERTY(Transient)
	FAGX_ImportContext Context;
};
