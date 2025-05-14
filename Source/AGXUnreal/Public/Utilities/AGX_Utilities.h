// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "AGX_Utilities.generated.h"

class UAGX_RigidBodyComponent;
struct FAGX_ImportSettings;

/**
 * Helper type that calls the given callback when destructed. Useful when cleanup is needed
 * regardless of from where we return from a function or leave a scope.
 * @tparam FuncT Callback to call on destruction. Will be moved from.
 */
template <typename FuncT>
struct FAGX_Finalizer
{
	FAGX_Finalizer(FuncT InCallback)
		: Callback(std::move(InCallback))
	{
	}

	~FAGX_Finalizer()
	{
		Callback();
	}

	FuncT Callback;
};

UCLASS(ClassGroup = "AGX Utilities")
class AGXUNREAL_API UAGX_AGXUtilities : public UBlueprintFunctionLibrary
{
public:
	GENERATED_BODY()

	/**
	 * Add the Parent velocity/angular velocity to the Body's velocity as if rigidly attached.
	 * This function is only valid during Play and does nothing if not in Play.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Utilities")
	static void AddParentVelocity(UAGX_RigidBodyComponent* Parent, UAGX_RigidBodyComponent* Body);

	/**
	 * Add the Parent velocity/angular velocity to all Bodies velocities as if rigidly attached.
	 * If the Parent body is among the provided Bodies array, it's velocity will not be updated.
	 * This function is only valid during Play and does nothing if not in Play.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Utilities")
	static void AddParentVelocityMany(
		UAGX_RigidBodyComponent* Parent, const TArray<UAGX_RigidBodyComponent*>& Bodies);

	/**
	 * Calculates the center of mass of the given Rigid Bodies [cm].
	 * This function is only valid during Play and returns zero vector if not.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Utilities")
	static FVector CalculateCenterOfMass(const TArray<UAGX_RigidBodyComponent*>& Bodies);

	/**
	 * Import an .agx archive or Urdf file and get a AActor template back.
	 * This template AActor must be instantiated to be part of the world, see InstantiateActor.
	 * The FilePath of the ImportSettings struct may be absolute or relative to the current Projects
	 * root directory or stand-alone application root directory.
	 */
	UFUNCTION(
		BlueprintCallable, Category = "AGX Import", meta = (WorldContext = "WorldContextObject"))
	static AActor* Import(UObject* WorldContextObject, FAGX_ImportSettings Settings);

	/**
	 * Instantiate an imported AActor. The AActor transform is set to the given Transform.
	 */
	UFUNCTION(
		BlueprintCallable, Category = "AGX Import", meta = (WorldContext = "WorldContextObject"))
	static AActor* InstantiateActor(
		UObject* WorldContextObject, AActor* Template, const FTransform& Transform);
};
