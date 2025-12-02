// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Contacts/AGX_ContactEnums.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Math/Vector.h"

// Standard library includes.
#include <memory>

struct FContactPointEntity;

class AGXUNREALBARRIER_API FContactPointBarrier
{
public:
	FContactPointBarrier();
	FContactPointBarrier(std::unique_ptr<FContactPointEntity> InNativeEntity);
	FContactPointBarrier(FContactPointBarrier&& InOther);
	FContactPointBarrier(const FContactPointBarrier& InOther);

	~FContactPointBarrier();

	FContactPointBarrier& operator=(const FContactPointBarrier& InOther);

	// Collision detection state getters.
	bool IsEnabled() const;
	FVector GetLocation() const;
	FVector GetNormal() const;
	FVector GetTangentU() const;
	FVector GetTangentV() const;
	double GetDepth() const;
	FVector GetVelocity() const;
	double GetArea() const;
	FVector GetWitnessPoint(int32 Index) const;

	// Collision detection state setters. May only be called before the solver.
	void SetEnabled(bool Enable);
	void SetLocation(const FVector& Location);
	void SetNormal(const FVector& Normal);
	void SetTangentU(const FVector& TangentU);
	void SetTangentV(const FVector& TangentV);
	void SetDepth(const double Depth);
	void SetVelocity(const FVector& Velocity);
	void SetArea(double Area) const;

	// Solver state getters. May only be called after the solver.
	FVector GetForce() const;
	double GetForceMagnitude() const;
	FVector GetNormalForce() const;
	double GetNormalForceMagnitude() const;
	FVector GetTangentialForce() const;
	double GetTangentialForceMagnitude() const;
	double GetTangentialForceUMagnitude() const;
	double GetTangentialForceVMagnitude() const;
	FVector GetLocalForce() const;
	double GetLocalForce(EAGX_ContactForceComponents Index);

	// Native management.
	bool HasNative() const;
	FContactPointEntity* GetNative();
	const FContactPointEntity* GetNative() const;

private:
	std::unique_ptr<FContactPointEntity> NativeEntity;
};
