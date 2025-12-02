// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Contacts/ContactPointBarrier.h"
#include "Contacts/AGX_ContactState.h"
#include "Materials/ContactMaterialBarrier.h"
#include "RigidBodyBarrier.h"
#include "Shapes/ShapeBarrier.h"

struct FShapeContactEntity;

/**
 * A Barrier wrapping a ShapeContact, called a GeometryContact in AGX Dynamics.
 */
class AGXUNREALBARRIER_API FShapeContactBarrier
{
public:
	FShapeContactBarrier();
	FShapeContactBarrier(const FShapeContactBarrier& InBarrier);
	FShapeContactBarrier(std::unique_ptr<FShapeContactEntity> InNativeEntity);
	FShapeContactBarrier(FShapeContactBarrier&& InOther);
	~FShapeContactBarrier();

	FShapeContactBarrier& operator=(const FShapeContactBarrier& InOther);

	bool IsEnabled() const;

	EAGX_ContactState GetContactState();

	FRigidBodyBarrier GetBody1() const;
	FRigidBodyBarrier GetBody2() const;

	FShapeBarrier GetShape1() const;
	FShapeBarrier GetShape2() const;

	bool Contains(const FRigidBodyBarrier& Body) const;
	bool Contains(const FShapeBarrier& Shape) const;

	int32 IndexOf(const FRigidBodyBarrier& Body) const;
	int32 IndexOf(const FShapeBarrier& Shape) const;

	FVector CalculateRelativeVelocity(int32 PointIndex) const;

	FContactMaterialBarrier GetContactMaterial() const;

	int32 GetNumContactPoints() const;
	FContactPointBarrier GetContactPoint(int32 Index) const;

	TArray<FContactPointBarrier> GetContactPoints() const;

	bool HasNative() const;
	FShapeContactEntity* GetNative();
	const FShapeContactEntity* GetNative() const;

private:
	std::unique_ptr<FShapeContactEntity> NativeEntity;
};
