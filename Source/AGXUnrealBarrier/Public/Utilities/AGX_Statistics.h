// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_Statistics.generated.h"

/**
 * A collection of statistics reported by AGX Dynamics. Includes both timing data and counts. These
 * are read by UAGX_Simulation and forwarded to Unreal's 'stat' system using the counters defined
 * in AGX_Stats.h.
 */
USTRUCT(BlueprintType)
struct AGXUNREALBARRIER_API FAGX_Statistics
{
	GENERATED_BODY()

	/* Timings reported by Simulation. */

	/// Time taken from early in agxSDK::Simulation::stepForward until, but not including, Update
	/// Render Manager.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Statistics|Simulation")
	float Sim_StepForwardTime = -1.0f;

	/* Begin timings that are part of StepForwardTime. */

	/// Time reported by Simulation for running pre-collide callbacks.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Statistics|Simulation")
	float Sim_PreCollideEventTime = -1.0f;

	/// Time taken for the update task in Space, reported by Simulation
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Statistics|Simulation")
	float Sim_CollisionDetectionTime = -1.0f;

	/// Time reported by Simulation for running contact separation event listeners.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Statistics|Simulation")
	float Sim_TriggeringContactSeparationEventsTime = -1.0f;

	/// Time reported by Simulation for running contact event listeners.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Statistics|Simulation")
	float Sim_TriggeringContactEventsTime = -1.0f;

	/// Time reported by Simulation for performing contact reduction.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Statistics|Simulation")
	float Sim_ContactReductionTime = -1.0f;

	/// Time reported by Simulation for removing disabled contact points and disabled Shape
	/// Contacts.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Statistics|Simulation")
	float Sim_CullingContactsTime = -1.0f;

	/// Time taken to recreate Shape Contacts after user callbacks may have removed some of them.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Statistics|Simulation")
	float Sim_CommittingRemovedContactsTime = -1.0f;

	/// Time reported by Simulation for running pre-step callbacks. Pre-step is after collision
	/// detection but before solve, i.e. "the step". This includes both internal and user callbacks.
	/// Examples of internal callbacks are wire, linked structure, terrain, merge-split, generic
	/// constraints, and removed contacts.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Statistics|Simulation")
	float Sim_PreStepEventTime = -1.0f;

	/// Time taken for the update task in Dynamics System, reported by Simulation.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Statistics|Simulation")
	float Sim_DynamicsSystemTime = -1.0f;

	/// Time reported by Simulation for running post-step callbacks. This includes both internal and
	/// user callbacks. Examples of internal callbacks are post-solve for generic constraints,
	/// constraint force data, terrain post-update, and waiting for asynchronous sensor
	/// environments, wires, linked structures, and merge-split.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Statistics|Simulation")
	float Sim_PostStepTime = -1.0f;

	/* End timings that are part of StepForwardTime. */

	/// Time reported by Simulation for running last callbacks. This includes both internal and
	/// external callbacks. Examples of internal callbacks are terrain, linked structure, and
	/// stability report.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Statistics|Simulation")
	float Sim_LastStepTime = -1.0f;

	/// Time taken to update internal render data. Not part of StepForwardTime.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Statistics|Simulation")
	float Sim_UpdateRenderManagerTime = -1.0f;

	// Time between the end of one call to agxSDK::Simulation::stepForward and the beginning of the
	// next. If the Step Mode is set to Drop Immediately then this is the time spent in Unreal code.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Statistics|Simulation")
	float Sim_InterStepTime = -1.0f;

	/* Data reported by Simulation. */

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Statistics|Simulation")
	int32 Sim_NumParticles = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Statistics|Simulation")
	int32 Sim_NumContactsRemoved = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Statistics|Simulation")
	int32 Sim_NumWarmStartedParticleParticleContacts = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Statistics|Simulation")
	int32 Sim_NumWarmStartedParticleShapeContacts = -1;

	/* Timings reported by Space. */

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Statistics|Space")
	float Space_UpdateTime = -1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Statistics|Space")
	float Space_BroadPhaseTime = -1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Statistics|Space")
	float Space_NarrowPhaseTime = -1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Statistics|Space")
	float Space_SyncBoundsTime = -1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Statistics|Space")
	float Space_SyncTransformsTime = -1.0f;

	/* Data reported by Space. It is assumed that there is one Shape per Geometry. */

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Statistics|Space")
	int32 Space_NumShapes = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Statistics|Space")
	int32 Space_NumShapeShapeContactPoints = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Statistics|Space")
	int32 Space_NumShapeShapeContacts = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Statistics|Space")
	int32 Space_NumParticleParticleContacts = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Statistics|Space")
	int32 Space_NumShapeParticleContacts = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Statistics|Space")
	int32 Space_NumNarrowPhaseTests = -1;

	/* Timings reported by Dynamics System. */

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Statistics|Dynamics System")
	float DynSys_UpdateTime = -1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Statistics|Dynamics System")
	float DynSys_SolverTime = -1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Statistics|Dynamics System")
	float DynSys_SabreFactoringTime = -1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Statistics|Dynamics System")
	float DynSys_SabreSolveTime = -1.0f;


	/* Data reported by Dynamics System. */

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Statistics|Dynamics System")
	int32 DynSys_NumEnabledRigidBodies = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Statistics|Dynamics System")
	int32 DynSys_NumBinaryConstraints = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Statistics|Dynamics System")
	int32 DynSys_NumMultiBodyConstraints = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Statistics|Dynamics System")
	int32 DynSys_NumContactConstraints = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Statistics|Dynamics System")
	int32 DynSys_NumSolveIslands = -1;
};
