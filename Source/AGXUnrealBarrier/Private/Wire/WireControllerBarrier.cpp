// Copyright 2025, Algoryx Simulation AB.

#include "Wire/WireControllerBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "BarrierOnly/AGXRefs.h"
#include "RigidBodyBarrier.h"
#include "Shapes/ShapeBarrier.h"
#include "BarrierOnly/Wire/WireControllerPtr.h"
#include "Wire/WireBarrier.h"
#include "BarrierOnly/Wire/WireRef.h"

FWireControllerBarrier::FWireControllerBarrier()
	: Native {new FWireControllerPtr}
{
}

FWireControllerBarrier::FWireControllerBarrier(const FWireControllerBarrier& Other)
	: Native(new FWireControllerPtr(Other.Native->Native))
{
}

FWireControllerBarrier::FWireControllerBarrier(FWireControllerBarrier&& Other)
	: Native(std::move(Other.Native))
{
	Other.Native.reset();
}

FWireControllerBarrier::FWireControllerBarrier(std::unique_ptr<FWireControllerPtr> InNative)
	: Native(std::move(InNative))
{
}

FWireControllerBarrier::~FWireControllerBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the
	// std::unique_ptr NativeRef's destructor must be able to see the definition,
	// not just the forward declaration, of FWireControllerPtr.
}

/*
 * Wire-Wire collision detection functions.
 */

bool FWireControllerBarrier::IsWireWireActive() const
{
	check(HasNative());
	return Native->Native->isWireWireActive();
}

bool FWireControllerBarrier::SetCollisionsEnabled(
	FWireBarrier& Wire1, FWireBarrier& Wire2, bool bEnable)
{
	check(HasNative());
	check(Wire1.HasNative());
	check(Wire2.HasNative());
	return Native->Native->setEnableCollisions(
		Wire1.GetNative()->Native, Wire2.GetNative()->Native, bEnable);
}

bool FWireControllerBarrier::GetCollisionsEnabled(
	const FWireBarrier& Wire1, const FWireBarrier& Wire2) const
{
	check(HasNative());
	return Native->Native->getEnableCollisions(
		Wire1.GetNative()->Native, Wire2.GetNative()->Native);
}

/*
 * Dynamic wire contacts functions.
 */

bool FWireControllerBarrier::SetDynamicWireContactsEnabled(FShapeBarrier& Shape, bool bEnable)
{
	check(HasNative());
	check(Shape.HasNativeGeometry());
	return Native->Native->setEnableDynamicWireContacts(Shape.GetNative()->NativeGeometry, bEnable);
}

bool FWireControllerBarrier::SetDynamicWireContactsEnabled(
	FRigidBodyBarrier& RigidBody, bool bEnable)
{
	check(HasNative());
	check(RigidBody.HasNative());
	return Native->Native->setEnableDynamicWireContacts(RigidBody.GetNative()->Native, bEnable);
}

void FWireControllerBarrier::SetDynamicWireContactsGloballyEnabled(bool bEnable)
{
	check(HasNative());
	Native->Native->setEnableDynamicWireContactsGlobally(bEnable);
}

bool FWireControllerBarrier::GetDynamicWireContactsEnabled(const FShapeBarrier& Shape) const
{
	check(HasNative());
	check(Shape.HasNativeGeometry());
	return Native->Native->getEnableDynamicWireContacts(Shape.GetNative()->NativeGeometry);
}

bool FWireControllerBarrier::GetDynamicWireContactsEnabled(const FRigidBodyBarrier& RigidBody) const
{
	check(HasNative());
	check(RigidBody.HasNative());
	return Native->Native->getEnableDynamicWireContacts(RigidBody.GetNative()->Native);
}

bool FWireControllerBarrier::GetDynamicWireContactsGloballyEnabled() const
{
	check(HasNative());
	return Native->Native->getEnableDynamicWireContactsGlobally();
}

bool FWireControllerBarrier::HasNative() const
{
	AGX_CHECK(Native != nullptr); // Must always have an FWireControllerPtr.
	return Native != nullptr && Native->Native != nullptr;
}

void FWireControllerBarrier::InitializeNative()
{
	Native->Native = agxWire::WireController::instance();
}
