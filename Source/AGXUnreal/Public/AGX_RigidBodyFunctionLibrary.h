// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AMOR/MergeSplitPropertiesBarrier.h"
#include "RigidBodyBarrier.h"
#include "Utilities/AGX_BarrierUtilities.h"
#include "AGX_LogCategory.h"

// Unreal Engine includes.
#include "Kismet/BlueprintFunctionLibrary.h"

#include "AGX_RigidBodyFunctionLibrary.generated.h"

UCLASS()
class AGXUNREAL_API URigidBodyBarrier_FL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintPure, Category="AGX Rigid Body Barrier")
	static bool HasNative(const FRigidBodyBarrier& RigidBody)
	{
		return RigidBody.HasNative();
	}

	UFUNCTION(BlueprintCallable, Category="AGX Rigid Body Barrier")
	static void SetEnabled(UPARAM(Ref) FRigidBodyBarrier& RigidBody, bool bEnabled)
	{
		AGX_BARRIER_BP_SET_PROPERTY_BOOL(RigidBody, Enabled);
	}

	UFUNCTION(BlueprintPure, Category="AGX Rigid Body Barrier")
	static bool GetEnabled(const FRigidBodyBarrier& RigidBody)
	{
		AGX_BARRIER_BP_GET_PROPERTY_BOOL(RigidBody, Enabled, false);
	}

	UFUNCTION(BlueprintCallable, Category="AGX Rigid Body Barrier")
	static void SetPosition(UPARAM(Ref) FRigidBodyBarrier& RigidBody, const FVector& Position)
	{
		AGX_BARRIER_BP_SET_PROPERTY(RigidBody, Position);
	}

	UFUNCTION(BlueprintPure, Category="AGX Rigid Body Barrier")
	static FVector GetPosition(const FRigidBodyBarrier& RigidBody)
	{
		AGX_BARRIER_BP_GET_PROPERTY(RigidBody, Position, FVector::ZeroVector);
	}

	UFUNCTION(BlueprintCallable, Category="AGX Rigid Body Barrier")
	static void SetRotation(UPARAM(Ref) FRigidBodyBarrier& RigidBody, const FQuat& Rotation)
	{
		AGX_BARRIER_BP_SET_PROPERTY(RigidBody, Rotation);
	}

	UFUNCTION(BlueprintPure, Category="AGX Rigid Body Barrier")
	static FQuat GetRotation(const FRigidBodyBarrier& RigidBody)
	{
		AGX_BARRIER_BP_GET_PROPERTY(RigidBody, Rotation, FQuat::Identity);
	}

	UFUNCTION(BlueprintCallable, Category="AGX Rigid Body Barrier")
	static void SetVelocity(UPARAM(Ref) FRigidBodyBarrier& RigidBody, const FVector& Velocity)
	{
		AGX_BARRIER_BP_SET_PROPERTY(RigidBody, Velocity);
	}

	UFUNCTION(BlueprintPure, Category="AGX Rigid Body Barrier")
	static FVector GetVelocity(const FRigidBodyBarrier& RigidBody)
	{
		AGX_BARRIER_BP_GET_PROPERTY(RigidBody, Velocity, FVector::ZeroVector);
	}

	UFUNCTION(BlueprintCallable, Category="AGX Rigid Body Barrier")
	static void SetAngularVelocity(UPARAM(Ref) FRigidBodyBarrier& RigidBody, const FVector& AngularVelocity)
	{
		AGX_BARRIER_BP_SET_PROPERTY(RigidBody, AngularVelocity);
	}

	UFUNCTION(BlueprintPure, Category="AGX Rigid Body Barrier")
	static FVector GetAngularVelocity(const FRigidBodyBarrier& RigidBody)
	{
		AGX_BARRIER_BP_GET_PROPERTY(RigidBody, AngularVelocity, FVector::ZeroVector);
	}

	UFUNCTION(BlueprintCallable, Category="AGX Rigid Body Barrier")
	static void SetLinearVelocityDamping(UPARAM(Ref) FRigidBodyBarrier& RigidBody, const FVector& LinearVelocityDamping)
	{
		AGX_BARRIER_BP_SET_PROPERTY(RigidBody, LinearVelocityDamping);
	}

	UFUNCTION(BlueprintPure, Category="AGX Rigid Body Barrier")
	static FVector GetLinearVelocityDamping(const FRigidBodyBarrier& RigidBody)
	{
		AGX_BARRIER_BP_GET_PROPERTY(RigidBody, LinearVelocityDamping, FVector::ZeroVector);
	}

	UFUNCTION(BlueprintCallable, Category="AGX Rigid Body Barrier")
	static void SetAngularVelocityDamping(UPARAM(Ref) FRigidBodyBarrier& RigidBody, const FVector& AngularVelocityDamping)
	{
		AGX_BARRIER_BP_SET_PROPERTY(RigidBody, AngularVelocityDamping);
	}

	UFUNCTION(BlueprintPure, Category="AGX Rigid Body Barrier")
	static FVector GetAngularVelocityDamping(const FRigidBodyBarrier& RigidBody)
	{
		AGX_BARRIER_BP_GET_PROPERTY(RigidBody, AngularVelocityDamping, FVector::ZeroVector);
	}

	UFUNCTION(BlueprintCallable, Category="AGX Rigid Body Barrier")
	static void SetCenterOfMassOffset(UPARAM(Ref) FRigidBodyBarrier& RigidBody, const FVector& CenterOfMassOffset)
	{
		AGX_BARRIER_BP_SET_PROPERTY(RigidBody, CenterOfMassOffset);
	}

	UFUNCTION(BlueprintPure, Category="AGX Rigid Body Barrier")
	static FVector GetCenterOfMassOffset(const FRigidBodyBarrier& RigidBody)
	{
		AGX_BARRIER_BP_GET_PROPERTY(RigidBody, CenterOfMassOffset, FVector::ZeroVector);
	}

	UFUNCTION(BlueprintCallable, Category="AGX Rigid Body Barrier")
	static void SetName(UPARAM(Ref) FRigidBodyBarrier& RigidBody, const FString& Name)
	{
		AGX_BARRIER_BP_SET_PROPERTY(RigidBody, Name);
	}

	UFUNCTION(BlueprintPure, Category="AGX Rigid Body Barrier")
	static FString GetName(const FRigidBodyBarrier& RigidBody)
	{
		AGX_BARRIER_BP_GET_PROPERTY(RigidBody, Name, TEXT(""));
	}

	UFUNCTION(BlueprintCallable, Category="AGX Rigid Body Barrier")
	static void SetMotionControl(UPARAM(Ref) FRigidBodyBarrier& RigidBody, EAGX_MotionControl MotionControl)
	{
		AGX_BARRIER_BP_SET_PROPERTY(RigidBody, MotionControl);
	}

	UFUNCTION(BlueprintPure, Category="AGX Rigid Body Barrier")
	static EAGX_MotionControl GetMotionControl(const FRigidBodyBarrier& RigidBody)
	{
		AGX_BARRIER_BP_GET_PROPERTY(RigidBody, MotionControl, EAGX_MotionControl::MC_DYNAMICS);
	}

	UFUNCTION(BlueprintPure, Category="AGX Rigid Body Barrier")
	static FMergeSplitPropertiesBarrier GetMergeSplitProperties(const FRigidBodyBarrier& RigidBody)
	{
		AGX_BARRIER_BP_GET_PROPERTY(RigidBody, MergeSplitProperties, FMergeSplitPropertiesBarrier());
	}
};
