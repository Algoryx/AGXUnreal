// Copyright 2026, Algoryx Simulation AB.

#pragma once


// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/Class.h"

#include "OpenPLXMappingBarriersCollection.generated.h"

class FConstraintBarrier;
struct FObserverFrameBarrier;
struct FRigidBodyBarrier;
struct FSteeringBarrier;


/**
 * Collection of all Barrier objects needed to do OpenPLX signal and object mappings.
 * These are inserted into an agxSDK::Assembly and given to mapper classes in the agxopenplx
 * namespace.
 */
USTRUCT()
struct AGXUNREALBARRIER_API FOpenPLXMappingBarriersCollection
{
	GENERATED_BODY()

	TArray<FRigidBodyBarrier*> Bodies;
	TArray<FConstraintBarrier*> Constraints;
	TArray<FObserverFrameBarrier*> ObserverFrames;
	TArray<FSteeringBarrier*> Steerings;
};
