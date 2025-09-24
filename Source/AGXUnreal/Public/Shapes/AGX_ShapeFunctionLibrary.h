// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "RigidBodyBarrier.h"
#include "Shapes/ShapeBarrier.h"
#include "Utilities/AGX_BarrierUtilities.h"

// Unreal Engine includes.
#include "Kismet/BlueprintFunctionLibrary.h"

#include "AGX_ShapeFunctionLibrary.generated.h"



UCLASS()
class AGXUNREAL_API UAGX_ShapeFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintPure, Category = "AGX Shape Barrier")
	static bool HasNative(const FShapeBarrier& Shape)
	{
		return Shape.HasNative();
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Shape Barrier")
	static void SetIsSensor(FShapeBarrier& Shape, bool bIsSensor, bool bGenerateContactData)
	{
		// Cannot use the macro since this Set function has more than one parameter.
		if (!Shape.HasNative())
		{
			UE_LOG(LogAGX, Error, TEXT("Accessed (None) trying to set IsSensor."));
			return;
		}
		Shape.SetIsSensor(bIsSensor, bGenerateContactData);
	}

	UFUNCTION(BlueprintPure, Category = "AGX Shape Barrier")
	static bool GetIsSensor(const FShapeBarrier& Shape)
	{
		AGX_BARRIER_BP_GET_PROPERTY_BOOL(Shape, IsSensor, false);
	}

	UFUNCTION(BlueprintPure, Category = "AGX Shape Barrier")
	static bool GetIsSensorGeneratingContactData(const FShapeBarrier& Shape)
	{
		AGX_BARRIER_BP_GET_PROPERTY_BOOL(Shape, IsSensorGeneratingContactData, false);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Shape Barrier")
	static void SetSurfaceVelocity(FShapeBarrier& Shape, const FVector& SurfaceVelocity)
	{
		AGX_BARRIER_BP_SET_PROPERTY(Shape, SurfaceVelocity);
	}

	UFUNCTION(BlueprintPure, Category = "AGX Shape Barrier")
	static FVector GetSurfaceVelocity(const FShapeBarrier& Shape)
	{
		AGX_BARRIER_BP_GET_PROPERTY(Shape, SurfaceVelocity, FVector::ZeroVector);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Shape Barrier")
	static void SetLocalPosition(FShapeBarrier& Shape, const FVector& LocalPosition)
	{
		AGX_BARRIER_BP_SET_PROPERTY(Shape, LocalPosition);
	}

	UFUNCTION(BlueprintPure, Category = "AGX Shape Barrier")
	static FVector GetLocalPosition(const FShapeBarrier& Shape)
	{
		AGX_BARRIER_BP_GET_PROPERTY(Shape, LocalPosition, FVector::ZeroVector);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Shape Barrier")
	static void SetLocalRotation(FShapeBarrier& Shape, const FQuat& LocalRotation)
	{
		AGX_BARRIER_BP_SET_PROPERTY(Shape, LocalRotation);
	}

	UFUNCTION(BlueprintPure, Category = "AGX Shape Barrier")
	static FQuat GetLocalRotation(const FShapeBarrier& Shape)
	{
		AGX_BARRIER_BP_GET_PROPERTY(Shape, LocalRotation, FQuat::Identity);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Shape Barrier")
	static void SetWorldPosition(FShapeBarrier& Shape, const FVector& WorldPosition)
	{
		AGX_BARRIER_BP_SET_PROPERTY(Shape, WorldPosition);
	}

	UFUNCTION(BlueprintPure, Category = "AGX Shape Barrier")
	static FVector GetWorldPosition(const FShapeBarrier& Shape)
	{
		AGX_BARRIER_BP_GET_PROPERTY(Shape, WorldPosition, FVector::ZeroVector);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Shape Barrier")
	static void SetWorldRotation(FShapeBarrier& Shape, const FQuat& WorldRotation)
	{
		AGX_BARRIER_BP_SET_PROPERTY(Shape, WorldRotation);
	}

	UFUNCTION(BlueprintPure, Category = "AGX Shape Barrier")
	static FQuat GetWorldRotation(const FShapeBarrier& Shape)
	{
		AGX_BARRIER_BP_GET_PROPERTY(Shape, WorldRotation, FQuat::Identity);
	}

	UFUNCTION(BlueprintPure, Category = "AGX Shape Barrier")
	static FTransform GetGeometryToShapeTransform(const FShapeBarrier& Shape)
	{
		AGX_BARRIER_BP_GET_PROPERTY(Shape, GeometryToShapeTransform, FTransform());
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Shape Barrier")
	static void SetName(FShapeBarrier& Shape, const FString& Name)
	{
		AGX_BARRIER_BP_SET_PROPERTY(Shape, Name);
	}

	UFUNCTION(BlueprintPure, Category = "AGX Shape Barrier")
	static FString GetName(const FShapeBarrier& Shape)
	{
		AGX_BARRIER_BP_GET_PROPERTY(Shape, Name, FString("(None)"););
	}

// TODO Enable these once FShapeMaterialBarrier has been exposed to Blueprint script.
#if 0
	UFUNCTION(BlueprintCallable, Category = "AGX Shape Barrier")
	static void SetMaterial(FShapeBarrier& Shape, const FShapeMaterialBarrier& Material)
	{
		AGX_BARRIER_BP_SET_PROPERTY(Shape, Material);
	}

	UFUNCTION(BlueprintPure, Category = "AGX Shape Barrier")
	static FShapeMaterialBarrier GetMaterial(const FShapeBarrier& Shape)
	{
		AGX_BARRIER_BP_GET_PROPERTY(Shape, Material, FShapeMaterialBarrier());
	}
#endif

	UFUNCTION(BlueprintCallable, Category = "AGX Shape Barrier")
	static void SetEnableCollisions(FShapeBarrier& Shape, bool bEnableCollisions)
	{
		AGX_BARRIER_BP_SET_PROPERTY_BOOL(Shape, EnableCollisions);
	}

	UFUNCTION(BlueprintPure, Category = "AGX Shape Barrier")
	static bool GetEnableCollisions(const FShapeBarrier& Shape)
	{
		AGX_BARRIER_BP_GET_PROPERTY_BOOL(Shape, EnableCollisions, false);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Shape Barrier")
	static void SetEnabled(FShapeBarrier& Shape, bool bEnabled)
	{
		AGX_BARRIER_BP_SET_PROPERTY_BOOL(Shape, Enabled);
	}

	UFUNCTION(BlueprintPure, Category = "AGX Shape Barrier")
	static bool GetEnabled(const FShapeBarrier& Shape)
	{
		AGX_BARRIER_BP_GET_PROPERTY_BOOL(Shape, Enabled, false);
	}

	UFUNCTION(BlueprintPure, Category = "AGX Shape Barrier")
	static FRigidBodyBarrier GetRigidBody(const FShapeBarrier& Shape)
	{
		AGX_BARRIER_BP_GET_PROPERTY(Shape, RigidBody, FRigidBodyBarrier());
	}
};
