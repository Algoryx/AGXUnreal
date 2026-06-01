// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <memory>

struct FRigidBodyBarrier;
struct FShapeBarrier;
class FWireBarrier;
struct FWireControllerPtr;

class AGXUNREALBARRIER_API FWireControllerBarrier
{
public:
	FWireControllerBarrier();
	FWireControllerBarrier(const FWireControllerBarrier& Other);
	FWireControllerBarrier(FWireControllerBarrier&& Other);
	FWireControllerBarrier(std::unique_ptr<FWireControllerPtr> InNative);
	~FWireControllerBarrier();

	bool IsWireWireActive() const;
	bool SetCollisionsEnabled(FWireBarrier& Wire1, FWireBarrier& Wire2, bool bEnable);
	bool GetCollisionsEnabled(const FWireBarrier& Wire1, const FWireBarrier& Wire2) const;

	bool SetDynamicWireContactsEnabled(FShapeBarrier& Shape, bool bEnable);
	bool SetDynamicWireContactsEnabled(FRigidBodyBarrier& RigidBody, bool bEnable);
	void SetDynamicWireContactsGloballyEnabled(bool bEnable);

	bool GetDynamicWireContactsEnabled(const FShapeBarrier& Shape) const;
	bool GetDynamicWireContactsEnabled(const FRigidBodyBarrier& Shape) const;
	bool GetDynamicWireContactsGloballyEnabled() const;

	bool HasNative() const;
	void InitializeNative();

private:
	std::unique_ptr<FWireControllerPtr> Native;
};
